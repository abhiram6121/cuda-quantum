/*******************************************************************************
 * Copyright (c) 2022 - 2023 NVIDIA Corporation & Affiliates.                  *
 * All rights reserved.                                                        *
 *                                                                             *
 * This source code and the accompanying materials are made available under    *
 * the terms of the Apache License 2.0 which accompanies this distribution.    *
 ******************************************************************************/

// clang-format off
// RUN: nvq++ --target quantinuum --emulate %s -o %basename_t.x && ./%basename_t.x | FileCheck %s
// XFAIL: *
// ^^^^^ Produces error: 'cc.loop' op not a simple counted loop
// clang-format on

#include <cudaq.h>
#include <iostream>

struct kernel {
  void operator()(const int n_iter) __qpu__ {
    cudaq::qubit q0;
    for (int i = 0; i < n_iter; i++) {
      h(q0);
      auto q0result = mz(q0);
      if (q0result)
        break; // loop until it lands heads
    }
  }
};

int main() {

  int nShots = 100;
  int nIter = 20;
  cudaq::set_random_seed(13);

  // Sample
  auto counts = cudaq::sample(/*shots=*/nShots, kernel{}, nIter);

  // Count the maximum number of iterations it took
  int nIterRan = nIter;
  for (int i = 0; i < nIter; i++) {
    char regName1[32];
    snprintf(regName1, sizeof(regName1), "q0result%%%02d", i);
    char regName2[32];
    snprintf(regName2, sizeof(regName2), "auto_register_%d", i);
    if (counts.size(regName1) == 0 && counts.size(regName2) == 0) {
      nIterRan = i;
      break;
    }
  }

  int ret = 0; // return status

  if (nIterRan < nIter) {
    std::cout << "SUCCESS: nIterRan (" << nIterRan << ") < nIter (" << nIter << ")\n";
    ret = 0;
  } else {
    std::cout << "FAILURE: nIterRan (" << nIterRan << ") >= nIter (" << nIter << ")\n";
    ret = 1;
  }

  return ret;
}

// CHECK: SUCCESS

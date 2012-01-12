/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SOLVER_H_
#define _SOLVER_H_

class mgcLinearSystemD
{
public:
  mgcLinearSystemD() {;}

  double** NewMatrix (int N);
  void DeleteMatrix (int N, double** A);
  double* NewVector (int N);

  int Solve (int N, double** A, double* b);
  // Input:
  //     A[N][N] coefficient matrix, entries are A[row][col]
  //     b[N] vector, entries are b[row]
  // Output:
  //     return value is TRUE if successful, FALSE if pivoting failed
  //     A[N][N] is inverse matrix
  //     b[N] is solution x to Ax = b
};

#endif

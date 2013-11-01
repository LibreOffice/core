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

#include "ResIDGenerator.hxx"

/**
   * Destructor
   *
   * @param
   * @return
   */
ResIDGenerator::~ResIDGenerator()
{
}

/**
   * SubList stores those IDs that were ever generated and deleted, the method
   * return the ID from subList first if subList is not empty,else return ++max.
   * Add the obsolete IDs by calling SetSub method
   *
   * @param
   * @return  new resource ID.
   */
long ResIDGenerator::GenerateNewResID()
{
    if (max == LONG_MAX)
    {
        if (!subList.empty())
        {
            long nRes = *(subList.begin());
            subList.pop_front();
            return nRes;
        }
    }
    return -(++max);
}

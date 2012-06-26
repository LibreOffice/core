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


#ifndef INCLUDED_APRENV_HXX
#define INCLUDED_APRENV_HXX

#include <apr_pools.h>

namespace apr_environment
{

// singleton class providing environment for APR libraries
class AprEnv
{
    public:
        ~AprEnv();

        static AprEnv* getAprEnv();

        apr_pool_t* getAprPool();

    private:
        apr_pool_t* mpAprPool;

        AprEnv();

};

} // namespace apr_environment

#endif // INCLUDED_APRENV_HXX

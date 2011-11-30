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



#include <386/builtin.h>

#include "system.h"

#include <osl/interlck.h>
#include <osl/diagnose.h>

/*****************************************************************************/
/* osl_incrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_incrementInterlockedCount(oslInterlockedCount* pCount)
{
    return __atomic_increment_s32( pCount);
}

/*****************************************************************************/
/* osl_decrementInterlockedCount */
/*****************************************************************************/
oslInterlockedCount SAL_CALL osl_decrementInterlockedCount(oslInterlockedCount* pCount)
{
    return __atomic_decrement_s32( pCount);
}



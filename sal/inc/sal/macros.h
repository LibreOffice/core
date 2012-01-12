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



#ifndef _SAL_MACROS_H_
#define _SAL_MACROS_H_

#ifndef SAL_MAX
#   define SAL_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef SAL_MIN
#   define SAL_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef SAL_FIELDOFFSET
#   define SAL_FIELDOFFSET(type, field) ((sal_Int32)(&((type *)16)->field) - 16)
#endif

#ifndef SAL_BOUND
#   define SAL_BOUND(x,l,h)        ((x) <= (l) ? (l) : ((x) >= (h) ? (h) : (x)))
#endif

#ifndef SAL_SWAP
#   define SAL_SWAP(a,b)           ((a) ^= (b) ^= (a) ^= (b))
#endif


#endif

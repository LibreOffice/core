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


#ifndef CHART2_NOWARNINGTHISINCTOR_HXX
#define CHART2_NOWARNINGTHISINCTOR_HXX

/** Include this file, if you have to use "this" in the base initializer list of
    a constructor.

    Include it only, if the usage of this is unavoidable, like in the
    initialization of controls in a dialog.

    Do not include this header in other header files, because this might result
    in unintended suppression of the warning via indirect inclusion.
 */

#ifdef _MSC_VER
// warning C4355: 'this' : used in base member initializer list
#  pragma warning (disable : 4355)
#endif

// CHART2_NOWARNINGTHISINCTOR_HXX
#endif

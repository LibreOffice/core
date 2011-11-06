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


#ifndef CHART2_TRUEGUARD_HXX
#define CHART2_TRUEGUARD_HXX

#include "charttoolsdllapi.hxx"

namespace chart
{
/** This guard sets the given boolean reference to true in the constructor and to false in the destructor
 */
class OOO_DLLPUBLIC_CHARTTOOLS TrueGuard
{
public:
    explicit TrueGuard( bool& rbTrueDuringGuardedTime );
    virtual ~TrueGuard();

private:
    bool& m_rbTrueDuringGuardedTime;
};

}
// CHART2_TRUEGUARD_HXX
#endif

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



#ifndef SC_NAVICFG_HXX
#define SC_NAVICFG_HXX

#include <tools/solar.h>


//==================================================================
// CfgItem fuer Navigator-Zustand
//==================================================================

class ScNavipiCfg
{
private:
    sal_uInt16  nListMode;
    sal_uInt16  nDragMode;
    sal_uInt16  nRootType;

public:
            ScNavipiCfg();

    void    SetListMode(sal_uInt16 nNew);
    sal_uInt16  GetListMode() const         { return nListMode; }
    void    SetDragMode(sal_uInt16 nNew);
    sal_uInt16  GetDragMode() const         { return nDragMode; }
    void    SetRootType(sal_uInt16 nNew);
    sal_uInt16  GetRootType() const         { return nRootType; }
};


#endif


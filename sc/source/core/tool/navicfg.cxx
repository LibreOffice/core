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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

#include "navicfg.hxx"

//------------------------------------------------------------------

//! #define CFGPATH_NAVIPI          "Office.Calc/Navigator"

//------------------------------------------------------------------

ScNavipiCfg::ScNavipiCfg() :
//! ConfigItem( OUString::createFromAscii( CFGPATH_NAVIPI ) ),
    nListMode(0),
    nDragMode(0),
    nRootType(0)
{
}

//------------------------------------------------------------------------

void ScNavipiCfg::SetListMode(sal_uInt16 nNew)
{
    if ( nListMode != nNew )
    {
        nListMode = nNew;
//!     SetModified();
    }
}

void ScNavipiCfg::SetDragMode(sal_uInt16 nNew)
{
    if ( nDragMode != nNew )
    {
        nDragMode = nNew;
//!     SetModified();
    }
}

void ScNavipiCfg::SetRootType(sal_uInt16 nNew)
{
    if ( nRootType != nNew )
    {
        nRootType = nNew;
//!     SetModified();
    }
}



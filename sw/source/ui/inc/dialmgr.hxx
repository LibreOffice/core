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


#ifndef _SW_DIALMGR_HXX
#define _SW_DIALMGR_HXX

// include ---------------------------------------------------------------

#include <tools/resid.hxx>

// forward ---------------------------------------------------------------

class ResMgr;
struct SwDialogsResMgr
{
    static ResMgr*      GetResMgr();
};

#define SWDIALOG_MGR()  SwDialogsResMgr::GetResMgr()
#define SW_RES(i)       ResId(i,SWDIALOG_MGR())
#define SW_RESSTR(i)    UniString(ResId(i,SWDIALOG_MGR()))
#define SW_RESSSTR(i)   String(ResId(i,SWDIALOG_MGR()))

#endif


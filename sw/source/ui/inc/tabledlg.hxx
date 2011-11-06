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


#ifndef _TABLEDLG_HXX
#define _TABLEDLG_HXX

#include <sfx2/tabdlg.hxx>


#include "swtypes.hxx"
#include "swtablerep.hxx"

class SwWrtShell;
class SwTabCols;
struct TColumn;

/*-------------------------------------------------------
 Tabellendialog
--------------------------------------------------------- */
class SwTableTabDlg : public SfxTabDialog
{
    SwWrtShell* pShell;
    sal_uInt16      nHtmlMode;

    virtual void        PageCreated(sal_uInt16 nId, SfxTabPage& rPage);
public:
     SwTableTabDlg( Window* pParent, SfxItemPool& Pool,
                    const SfxItemSet* pItemSet, SwWrtShell* pSh );
};


#endif

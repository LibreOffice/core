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



#ifndef _FRMDLG_HXX
#define _FRMDLG_HXX

#include "globals.hrc"
#include <sfx2/tabdlg.hxx>
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung:    Rahmendialog
 --------------------------------------------------------------------*/

class SwFrmDlg : public SfxTabDialog
{
    sal_Bool                m_bFormat;
    sal_Bool                m_bNew;
    sal_Bool                m_bHTMLMode;
    bool                m_bEnableVertPos;
    const SfxItemSet&   m_rSet;
    sal_uInt16              m_nDlgType;
    SwWrtShell*         m_pWrtShell;


    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:
    SwFrmDlg(   SfxViewFrame *pFrame, Window *pParent,
                const SfxItemSet& rCoreSet,
                sal_Bool            bNewFrm  = sal_True,
                sal_uInt16          nResType = DLG_FRM_STD,
                sal_Bool            bFmt     = sal_False,
                sal_uInt16          nDefPage = 0,
                const String*   pFmtStr  = 0);

    ~SwFrmDlg();

    inline SwWrtShell*  GetWrtShell()   { return m_pWrtShell; }
};


#endif // _FRMDLG_HXX

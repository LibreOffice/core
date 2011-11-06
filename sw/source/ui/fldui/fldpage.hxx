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


#ifndef _SWFLDPAGE_HXX
#define _SWFLDPAGE_HXX

#include <sfx2/tabdlg.hxx>
#include <fldmgr.hxx>

class ListBox;

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

const int coLBCount = 3;

class SwFldPage : public SfxTabPage
{
    String              m_aLstStrArr[ coLBCount ];
    SwFldMgr            m_aMgr;
    SwField             *m_pCurFld;
    SwWrtShell*         m_pWrtShell;
    sal_uInt16              m_nPageId;
    sal_uInt16              m_nTypeSel;
    sal_uInt16              m_nSelectionSel;
    sal_Bool                m_bFldEdit;
    sal_Bool                m_bInsert;
    sal_Bool                m_bFldDlgHtmlMode;
    sal_Bool                m_bRefresh;
    sal_Bool                m_bFirstHTMLInit;

protected:

    sal_uInt16              GetTypeSel() const          { return m_nTypeSel;}
    void                SetTypeSel(sal_uInt16 nSet)     { m_nTypeSel = nSet;}
    sal_uInt16              GetSelectionSel() const     { return m_nSelectionSel;}
    void                SetSelectionSel(sal_uInt16 nSet){ m_nSelectionSel = nSet;}
    sal_Bool                IsFldDlgHtmlMode() const    { return m_bFldDlgHtmlMode;}
    sal_Bool                IsRefresh() const           { return m_bRefresh;}
    SwField*            GetCurField()               { return m_pCurFld;}
    SwWrtShell*         GetWrtShell() { return m_pWrtShell;}

    DECL_LINK( InsertHdl, Button *pBtn = 0 );
    DECL_LINK( NumFormatHdl, ListBox *pBtn = 0 );

    void                Init();
    void                SavePos( const ListBox* pLst1,
                                 const ListBox* pLst2 = 0,
                                 const ListBox* pLst3 = 0);
    void                RestorePos( ListBox* pLst1, ListBox* pLst2 = 0,
                                    ListBox* pLst3 = 0 );
    void                EnableInsert(sal_Bool bEnable = sal_True);
    inline sal_Bool         IsFldEdit() const   { return m_bFldEdit; }

    // Feld einfuegen
    sal_Bool                InsertFld(  sal_uInt16 nTypeId,
                                    sal_uInt16 nSubType,
                                    const String& rPar1,
                                    const String& rPar2,
                                    sal_uLong nFormatId,
                                    sal_Unicode cDelim = ' ',
                                    sal_Bool bIsAutomaticLanguage = sal_True);

    using SfxTabPage::ActivatePage;

public:
                        SwFldPage(  Window *pParent,
                                    const ResId &rId,
                                    const SfxItemSet &rAttrSet );

                        virtual ~SwFldPage();

    virtual void        ActivatePage();

    inline SwFldMgr&    GetFldMgr()         { return m_aMgr; }
    void                SetWrtShell( SwWrtShell* m_pWrtShell );
    void                EditNewField( sal_Bool bOnlyActivate = sal_False );
    virtual sal_uInt16      GetGroup() = 0;
};


#endif


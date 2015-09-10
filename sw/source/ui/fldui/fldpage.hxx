/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDPAGE_HXX

#include <sfx2/tabdlg.hxx>
#include <fldmgr.hxx>

#define FIELD_COLUMN_WIDTH 76

class ListBox;
class SvTreeListBox;

const int coLBCount = 3;

class SwFieldPage : public SfxTabPage
{
    OUString            m_aLstStrArr[ coLBCount ];
    SwFieldMgr            m_aMgr;
    SwField             *m_pCurField;
    SwWrtShell*         m_pWrtShell;
    sal_Int32           m_nTypeSel;
    sal_Int32           m_nSelectionSel;
    bool                m_bFieldEdit;
    bool                m_bInsert;
    bool                m_bFieldDlgHtmlMode;
    bool                m_bRefresh;
    bool                m_bFirstHTMLInit;

protected:

    sal_Int32           GetTypeSel() const          { return m_nTypeSel;}
    void                SetTypeSel(sal_Int32  nSet)     { m_nTypeSel = nSet;}
    sal_Int32           GetSelectionSel() const     { return m_nSelectionSel;}
    void                SetSelectionSel(sal_Int32  nSet){ m_nSelectionSel = nSet;}
    bool                IsFieldDlgHtmlMode() const    { return m_bFieldDlgHtmlMode;}
    bool                IsRefresh() const           { return m_bRefresh;}
    SwField*            GetCurField()               { return m_pCurField;}
    SwWrtShell*         GetWrtShell() { return m_pWrtShell;}

    DECL_LINK( InsertHdl, void * = 0);
    DECL_LINK_TYPED( TreeListBoxInsertHdl, SvTreeListBox*, bool);
    DECL_LINK(NumFormatHdl, void *);

    void                Init();
    void                SavePos( const ListBox* pLst1,
                                 const ListBox* pLst2 = 0,
                                 const ListBox* pLst3 = 0);
    void                RestorePos( ListBox* pLst1, ListBox* pLst2 = 0,
                                    ListBox* pLst3 = 0 );
    void                EnableInsert(bool bEnable = true);
    inline bool         IsFieldEdit() const   { return m_bFieldEdit; }

    // insert field
    bool                InsertField(  sal_uInt16 nTypeId,
                                    sal_uInt16 nSubType,
                                    const OUString& rPar1,
                                    const OUString& rPar2,
                                    sal_uLong nFormatId,
                                    sal_Unicode cDelim = ' ',
                                    bool bIsAutomaticLanguage = true);

    using SfxTabPage::ActivatePage;

public:
    SwFieldPage(vcl::Window *pParent, const OString& rID,
        const OUString& rUIXMLDescription, const SfxItemSet &rAttrSet);

    virtual ~SwFieldPage();

    virtual void        ActivatePage() SAL_OVERRIDE;

    inline SwFieldMgr&    GetFieldMgr()         { return m_aMgr; }
    void                SetWrtShell( SwWrtShell* m_pWrtShell );
    void                EditNewField( bool bOnlyActivate = false );
    virtual sal_uInt16      GetGroup() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#define FIELD_COLUMN_WIDTH 19

const int coLBCount = 3;

class SwFieldPage : public SfxTabPage
{
    OUString            m_aLstStrArr[ coLBCount ];
    SwFieldMgr          m_aMgr;
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

    DECL_LINK( TreeViewInsertHdl, weld::TreeView&, bool );
    DECL_LINK( NumFormatHdl, weld::TreeView&, bool );
    void                InsertHdl(weld::Widget*);

    void                Init();
    void                SavePos( const weld::TreeView& rLst1);
    void                RestorePos( weld::TreeView& rLst1 );
    bool                IsCurrentPage() const;
    void                EnableInsert(bool bEnable, bool bIsCurrentPage);
    bool                IsFieldEdit() const   { return m_bFieldEdit; }

    // insert field
    void                InsertField(SwFieldTypesEnum nTypeId,
                                    sal_uInt16 nSubType,
                                    const OUString& rPar1,
                                    const OUString& rPar2,
                                    sal_uInt32 nFormatId,
                                    sal_Unicode cDelim = ' ',
                                    bool bIsAutomaticLanguage = true);

public:
    SwFieldPage(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription,
        const OUString& rID, const SfxItemSet *pAttrSet);

    virtual ~SwFieldPage() override;

    virtual void        Activate() override;

    SwFieldMgr&    GetFieldMgr()         { return m_aMgr; }
    void                SetWrtShell( SwWrtShell* m_pWrtShell );
    void                EditNewField( bool bOnlyActivate = false );
    virtual sal_uInt16      GetGroup() = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SWFLDPAGE_HXX
#define _SWFLDPAGE_HXX

#include <sfx2/tabdlg.hxx>
#include <fldmgr.hxx>

class ListBox;

const int coLBCount = 3;

class SwFldPage : public SfxTabPage
{
    String              m_aLstStrArr[ coLBCount ];
    SwFldMgr            m_aMgr;
    SwField             *m_pCurFld;
    SwWrtShell*         m_pWrtShell;
    USHORT              m_nPageId;
    USHORT              m_nTypeSel;
    USHORT              m_nSelectionSel;
    BOOL                m_bFldEdit;
    BOOL                m_bInsert;
    BOOL                m_bFldDlgHtmlMode;
    BOOL                m_bRefresh;
    BOOL                m_bFirstHTMLInit;

protected:

    USHORT              GetTypeSel() const          { return m_nTypeSel;}
    void                SetTypeSel(USHORT nSet)     { m_nTypeSel = nSet;}
    USHORT              GetSelectionSel() const     { return m_nSelectionSel;}
    void                SetSelectionSel(USHORT nSet){ m_nSelectionSel = nSet;}
    BOOL                IsFldDlgHtmlMode() const    { return m_bFldDlgHtmlMode;}
    BOOL                IsRefresh() const           { return m_bRefresh;}
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
    void                EnableInsert(BOOL bEnable = TRUE);
    inline BOOL         IsFldEdit() const   { return m_bFldEdit; }

    // insert field
    BOOL                InsertFld(  USHORT nTypeId,
                                    USHORT nSubType,
                                    const String& rPar1,
                                    const String& rPar2,
                                    ULONG nFormatId,
                                    sal_Unicode cDelim = ' ',
                                    BOOL bIsAutomaticLanguage = TRUE);

    using SfxTabPage::ActivatePage;

public:
                        SwFldPage(  Window *pParent,
                                    const ResId &rId,
                                    const SfxItemSet &rAttrSet );

                        virtual ~SwFldPage();

    virtual void        ActivatePage();

    inline SwFldMgr&    GetFldMgr()         { return m_aMgr; }
    void                SetWrtShell( SwWrtShell* m_pWrtShell );
    void                EditNewField( BOOL bOnlyActivate = FALSE );
    virtual USHORT      GetGroup() = 0;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

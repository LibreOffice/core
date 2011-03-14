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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

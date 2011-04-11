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
#ifndef _SWFLDREF_HXX
#define _SWFLDREF_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
// --> OD 2007-11-15 #i83479#
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentListItems.hxx>
#include <FldRefTreeListBox.hxx>
class SwTxtNode;
// <--

class SwFldRefPage : public SwFldPage
{
    FixedText       aTypeFT;
    ListBox         aTypeLB;
    FixedText       aSelectionFT;
    ListBox         aSelectionLB;
    // --> OD 2007-11-21 #i83479#
    SwFldRefTreeListBox aSelectionToolTipLB;
    // <--
    FixedText       aFormatFT;
    ListBox         aFormatLB;
    FixedText       aNameFT;
    Edit            aNameED;
    FixedText       aValueFT;
    Edit            aValueED;
    const String    sBookmarkTxt;
    const String    sFootnoteTxt;
    const String    sEndnoteTxt;
    // --> OD 2007-11-09 #i83479#
    const String    sHeadingTxt;
    const String    sNumItemTxt;

    IDocumentOutlineNodes::tSortedOutlineNodeList maOutlineNodes;
    IDocumentListItems::tSortedNodeNumList maNumItems;

    // selected text node in the listbox for headings and numbered items
    // in order to restore selection after update of selection listbox
    const SwTxtNode* mpSavedSelectedTxtNode;
    // fallback, if previously selected text node doesn't exist anymore
    sal_uInt16 mnSavedSelectedPos;
    // <--

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );
    DECL_LINK( ModifyHdl, Edit *pEd = 0 );

    void                UpdateSubType();
    sal_uInt16              FillFormatLB(sal_uInt16 nTypeId);

    // --> OD 2007-12-05 #i83479#
    void SaveSelectedTxtNode();
    const SwTxtNode* GetSavedSelectedTxtNode() const;
    sal_uInt16 GetSavedSelectedPos() const;
    // <--

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldRefPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldRefPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

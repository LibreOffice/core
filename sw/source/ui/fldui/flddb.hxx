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
#ifndef _SWFLDDB_HXX
#define _SWFLDDB_HXX

#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>

#include "condedit.hxx"
#include "dbtree.hxx"
#include "numfmtlb.hxx"

#include "fldpage.hxx"

class SwFldDBPage : public SwFldPage
{
    FixedText           aTypeFT;
    ListBox             aTypeLB;
    FixedText           aSelectionFT;
    SwDBTreeList        aDatabaseTLB;

    FixedText           aAddDBFT;
    PushButton          aAddDBPB;

    FixedText           aConditionFT;
    ConditionEdit       aConditionED;
    FixedText           aValueFT;
    Edit                aValueED;
    RadioButton         aDBFormatRB;
    RadioButton         aNewFormatRB;
    NumFormatListBox    aNumFormatLB;
    ListBox             aFormatLB;
    FixedLine           aFormatFL;
    FixedLine           aFormatVertFL;

    String              sOldDBName;
    String              sOldTableName;
    String              sOldColumnName;
    sal_uLong               nOldFormat;
    sal_uInt16              nOldSubType;
    Link                aOldNumSelectHdl;

    DECL_LINK( TypeHdl, ListBox* );
    DECL_LINK( NumSelectHdl, NumFormatListBox* pLB = 0);
    DECL_LINK( TreeSelectHdl, SvTreeListBox* pBox );
    DECL_LINK( ModifyHdl, Edit *pED = 0 );
    DECL_LINK( AddDBHdl, PushButton* );

    void                CheckInsert();

    using SwFldPage::SetWrtShell;

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldDBPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldDBPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
    void                ActivateMailMergeAddress();

    void                SetWrtShell(SwWrtShell& rSh);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

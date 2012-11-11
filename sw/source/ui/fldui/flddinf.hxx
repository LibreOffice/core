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
#ifndef _SWFLDDINF_HXX
#define _SWFLDDINF_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <svtools/treelistbox.hxx>

#include "numfmtlb.hxx"
#include "fldpage.hxx"

namespace com{namespace sun{ namespace star{ namespace beans{
    class XPropertySet;
}}}}

class SwFldDokInfPage : public SwFldPage
{
    FixedText           aTypeFT;
    SvTreeListBox       aTypeTLB;
    FixedText           aSelectionFT;
    ListBox             aSelectionLB;
    FixedText           aFormatFT;
    NumFormatListBox    aFormatLB;
    CheckBox            aFixedCB;

    SvTreeListEntry*        pSelEntry;
    com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > xCustomPropertySet;

    String              aInfoStr;

    sal_uInt16              nOldSel;
    sal_uLong               nOldFormat;
    ::rtl::OUString     m_sOldCustomFieldName;

    DECL_LINK(TypeHdl, void * = 0);
    DECL_LINK(SubTypeHdl, void * = 0);

    sal_uInt16              FillSelectionLB(sal_uInt16 nSubTypeId);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldDokInfPage(Window* pWindow, const SfxItemSet& rSet);

                        ~SwFldDokInfPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef _SWFLDDINF_HXX
#define _SWFLDDINF_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
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
    SvTreeListBox*      m_pTypeTLB;
    VclContainer*       m_pSelection;
    ListBox*            m_pSelectionLB;
    VclContainer*       m_pFormat;
    NumFormatListBox*   m_pFormatLB;
    CheckBox*           m_pFixedCB;

    SvTreeListEntry*        pSelEntry;
    com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > xCustomPropertySet;

    sal_uInt16              nOldSel;
    sal_uLong               nOldFormat;
    OUString     m_sOldCustomFieldName;

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

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

#if !TEST_LAYOUT
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#endif /*! TEST_LAYOUT */

#undef SC_DLLIMPLEMENTATION



#include "tpsort.hxx"
#include "sortdlg.hxx"
#include "scresid.hxx"
#include "sortdlg.hrc"

#if !LAYOUT_SFX_TABDIALOG_BROKEN
#include <layout/layout-pre.hxx>
#endif

ScSortDlg::ScSortDlg( Window*           pParent,
                      const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SORT ),
                      pArgSet ),
        bIsHeaders  ( FALSE ),
        bIsByRows   ( FALSE )

{
#if LAYOUT_SFX_TABDIALOG_BROKEN
    AddTabPage( TP_FIELDS,  ScTabPageSortFields::Create,  0 );
    AddTabPage( TP_OPTIONS, ScTabPageSortOptions::Create, 0 );
#else /* !LAYOUT_SFX_TABDIALOG_BROKEN */
    String fields(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("fields")));
    AddTabPage( TP_FIELDS, fields, ScTabPageSortFields::Create, 0, FALSE, TAB_APPEND);
    String options(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("options")));
    AddTabPage( TP_OPTIONS, options, ScTabPageSortOptions::Create, 0, FALSE, TAB_APPEND);
#endif /* !LAYOUT_SFX_TABDIALOG_BROKEN */
    FreeResource();
}

__EXPORT ScSortDlg::~ScSortDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

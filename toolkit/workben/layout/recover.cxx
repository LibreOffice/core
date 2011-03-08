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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#if TEST_LAYOUT
#include <cstdio>
#endif /* TEST_LAYOUT */
#include <com/sun/star/awt/XDialog2.hpp>
#include <tools/shl.hxx>
#include <svtools/itemset.hxx>
#include <svtools/itempool.hxx>
#include <sfx2/objsh.hxx>
#include <vcl/msgbox.hxx>
#include <toolkit/awt/vclxwindow.hxx>


#include <com/sun/star/awt/PosSize.hpp> //redrawAlready

using namespace com::sun::star;

#define _SVX_RECOVER_CXX

#include "recover.hxx"

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT
#undef SVX_RES
#define SVX_RES(x) #x
#undef SfxModalDialog
#define SfxModalDialog( parent, id ) Dialog( parent, "recover.xml", id )
#endif /* ENABLE_LAYOUT */

#if TEST_LAYOUT
SvxRecoverDialog::SvxRecoverDialog( Window* pParent )
#else /* !TEST_LAYOUT */
SvxRecoverDialog::SvxRecoverDialog( Window* pParent, const SfxItemSet& rCoreSet )
#endif /* !TEST_LAYOUT */
: SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_RECOVER ) )

    , aHeaderImage( this, SVX_RES( FI_HEADER ) )
    , aHeaderText( this, SVX_RES( FT_HEADER ) )
    , aHeaderLine( this, SVX_RES( FL_HEADER ) )
    , aRecoverText( this, SVX_RES( FT_RECOVER ) )
    , aTextAdvanced( this, SVX_RES( FT_ADVANCED ) )

    , aCheckBoxDoc( this, SVX_RES( CB_DOC ) )
    , aImageDoc( this, SVX_RES( FI_DOC ) )
    , aTextDoc( this, SVX_RES( FT_DOC ) )

    , aCheckBoxSheet( this, SVX_RES( CB_SHEET ) )
    , aImageSheet( this, SVX_RES( FI_SHEET ) )
    , aTextSheet( this, SVX_RES( FT_SHEET ) )

    , aCheckBoxDraw( this, SVX_RES( CB_DRAW ) )
    , aImageDraw( this, SVX_RES( FI_DRAW ) )
    , aTextDraw( this, SVX_RES( FT_DRAW ) )

    , aCheckBoxPresent( this, SVX_RES( CB_PRESENT ) )
    , aImagePresent( this, SVX_RES( FI_PRESENT ) )
    , aTextPresent( this, SVX_RES( FT_PRESENT ) )

    , aButtonAdvanced( this, SVX_RES( PB_ADVANCED ) )

    , aProgressText( this, SVX_RES( FT_PROGRESS ) )
    , aProgressBar( this, SVX_RES( PB_RECOVER ) )
    , aCheckBoxLogFile( this, SVX_RES( CH_LOGFILE ) )
    , aOKBtn( this, SVX_RES( BTN_OK ) )
    , aCancelBtn( this, SVX_RES( BTN_CANCEL ) )
    , aHelpBtn( this, SVX_RES( BTN_HELP ) )
{
    aButtonAdvanced.AddAdvanced( &aTextAdvanced );
    aButtonAdvanced.AddAdvanced( &aCheckBoxDoc );
    aButtonAdvanced.AddAdvanced( &aCheckBoxSheet );
    aButtonAdvanced.AddAdvanced( &aCheckBoxDraw );
    aButtonAdvanced.AddAdvanced( &aCheckBoxPresent );
    aButtonAdvanced.AddAdvanced( &aCheckBoxLogFile );
}


SvxRecoverDialog::~SvxRecoverDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

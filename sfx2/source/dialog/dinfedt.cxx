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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif

#include "sfx2/dinfedt.hxx"
#include "sfx2/sfxresid.hxx"

#include <sfx2/sfx.hrc>
#include "dinfedt.hrc"

// class InfoEdit_Impl ---------------------------------------------------

void InfoEdit_Impl::KeyInput( const KeyEvent& rKEvent )
{
    if ( rKEvent.GetCharCode() != '~' )
        Edit::KeyInput( rKEvent );
}

// class SfxDocInfoEditDlg -----------------------------------------------

SfxDocInfoEditDlg::SfxDocInfoEditDlg( Window* pParent ) :

    ModalDialog( pParent, SfxResId( DLG_DOCINFO_EDT ) ),

    aInfoFL     ( this, SfxResId( FL_INFO ) ),
    aInfo1ED    ( this, SfxResId( ED_INFO1 ) ),
    aInfo2ED    ( this, SfxResId( ED_INFO2 ) ),
    aInfo3ED    ( this, SfxResId( ED_INFO3 ) ),
    aInfo4ED    ( this, SfxResId( ED_INFO4 ) ),
    aOkBT       ( this, SfxResId( BT_OK ) ),
    aCancelBT   ( this, SfxResId( BT_CANCEL ) ),
    aHelpBtn    ( this, SfxResId( BTN_HELP ) )

{
    FreeResource();
}


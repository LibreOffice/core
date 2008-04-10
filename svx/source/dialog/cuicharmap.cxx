/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cuicharmap.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#include <stdio.h>

#define _CUI_CHARMAP_CXX_
#include <tools/shl.hxx>
#include <tools/debug.hxx>
#include <vcl/sound.hxx>
#include <vcl/svapp.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <svtools/colorcfg.hxx>

#include <rtl/textenc.h>
#include <svx/ucsubset.hxx>

#include <svx/dialogs.hrc>
#include "charmap.hrc"
#include <svx/charmap.hxx> //add CHINA001
#include <svx/dialmgr.hxx>
#include "cuicharmap.hxx"
//CHINA001 #include "charmapacc.hxx"
//CHINA001 #ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTOBJECT_HPP_
//CHINA001 #include <com/sun/star/accessibility/AccessibleEventObject.hpp>
//CHINA001 #endif
//CHINA001 #ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
//CHINA001 #include <com/sun/star/accessibility/AccessibleEventId.hpp>
//CHINA001 #endif
//CHINA001 #ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
//CHINA001 #include <com/sun/star/accessibility/AccessibleStateType.hpp>
//CHINA001 #endif
//CHINA001 #ifndef _COMPHELPER_TYPES_HXX_
//CHINA001 #include <comphelper/types.hxx>
//CHINA001 #endif
//CHINA001
//CHINA001 using namespace ::com::sun::star::accessibility;
//CHINA001 using namespace ::com::sun::star::uno;

// class SvxCharacterMap =================================================

SvxCharacterMap::SvxCharacterMap( Window* pParent, BOOL bOne ) :
    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_CHARMAP ) ),
    mpCharMapData( new SvxCharMapData( this, bOne, &DIALOG_MGR() ) )
{
    FreeResource();
}

// -----------------------------------------------------------------------

SvxCharacterMap::~SvxCharacterMap()
{
    delete mpCharMapData;
}

// -----------------------------------------------------------------------

const Font& SvxCharacterMap::GetCharFont() const
{
    return mpCharMapData->aFont;
}

// -----------------------------------------------------------------------

void SvxCharacterMap::SetChar( sal_UCS4 c )
{
    mpCharMapData->aShowSet.SelectCharacter( c );
}

// -----------------------------------------------------------------------

sal_UCS4 SvxCharacterMap::GetChar() const
{
    return mpCharMapData->aShowSet.GetSelectCharacter();
}

// -----------------------------------------------------------------------

String SvxCharacterMap::GetCharacters() const
{
    return mpCharMapData->aShowText.GetText();
}


// -----------------------------------------------------------------------

void SvxCharacterMap::DisableFontSelection()
{
    mpCharMapData->aFontText.Disable();
    mpCharMapData->aFontLB.Disable();
}
void SvxCharacterMap::SetCharFont( const Font& rFont )
{
    mpCharMapData->SetCharFont( rFont );
}


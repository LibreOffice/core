/*************************************************************************
 *
 *  $RCSfile: cuicharmap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:30:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#include <stdio.h>

#define _CUI_CHARMAP_CXX_

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <vcl/sound.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#pragma hdrstop

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif

#include <rtl/textenc.h>
#include <ucsubset.hxx>

#include "dialogs.hrc"
#include "charmap.hrc"
#include "charmap.hxx" //add CHINA001
#include "dialmgr.hxx"
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
    mpCharMapData( new SvxCharMapData( this, bOne ) )
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

void SvxCharacterMap::SetChar( sal_Unicode c )
{
    mpCharMapData->aShowSet.SelectCharacter( c );
}

// -----------------------------------------------------------------------

sal_Unicode SvxCharacterMap::GetChar() const
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

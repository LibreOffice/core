/*************************************************************************
 *
 *  $RCSfile: langbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pb $ $Date: 2000-11-24 10:22:55 $
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

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#pragma hdrstop

#include "langbox.hxx"
#include "langtab.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "unolingu.hxx"

using namespace ::com::sun::star::util;

//========================================================================
//  class SvxLanguageBox
//========================================================================

USHORT TypeToPos_Impl( LanguageType eType, const ListBox& rLb )
{
    USHORT  nPos   = LISTBOX_ENTRY_NOTFOUND;
    USHORT  nCount = rLb.GetEntryCount();

    for ( USHORT i=0; nPos == LISTBOX_ENTRY_NOTFOUND && i<nCount; i++ )
        if ( eType == LanguageType((ULONG)rLb.GetEntryData(i)) )
            nPos = i;

    return nPos;
}

//-----------------------------------------------------------------------
/*!!! (pb) obsolete
SvxLanguageBox::SvxLanguageBox( Window* pParent, WinBits nWinStyle ) :

    ListBox( pParent, nWinStyle )

{
    m_pLangTable = new SvxLanguageTable;
    aNotCheckedImage = Image( SVX_RES( RID_SVXIMG_NOTCHECKED ) );
    aCheckedImage = Image( SVX_RES( RID_SVXIMG_CHECKED ) );
}
*/
//------------------------------------------------------------------------

SvxLanguageBox::SvxLanguageBox( Window* pParent, const ResId& rResId, BOOL bCheck ) :

    ListBox( pParent, rResId ),

    m_bWithCheckmark( bCheck )

{
    m_pLangTable = new SvxLanguageTable;
    m_aNotCheckedImage = Image( SVX_RES( RID_SVXIMG_NOTCHECKED ) );
    m_aCheckedImage = Image( SVX_RES( RID_SVXIMG_CHECKED ) );

    if ( m_bWithCheckmark )
    {
        const USHORT nCount = International::GetAvailableFormatCount();
        for ( USHORT i = 0; i < nCount; i++ )
        {
            LanguageType eLngType = International::GetAvailableFormat( i );
            if ( eLngType != LANGUAGE_SYSTEM )
                InsertLanguage( eLngType );
        }
    }
}

//------------------------------------------------------------------------

SvxLanguageBox::~SvxLanguageBox()
{
    delete m_pLangTable;
}

//------------------------------------------------------------------------

USHORT SvxLanguageBox::InsertLanguage( const LanguageType eLangType, USHORT nPos )
{
    String aStrEntry = m_pLangTable->GetString( eLangType );
    USHORT nAt = 0;
    if ( m_bWithCheckmark )
    {
        const USHORT nLanguageCount = SvxGetSelectableLanguages().getLength();
        const Language* pLangList = SvxGetSelectableLanguages().getConstArray();
        sal_Bool bFound = sal_False;
        for ( USHORT i = 0; i < nLanguageCount; ++i )
        {
            if ( eLangType == pLangList[i] )
            {
                bFound = sal_True;
                break;
            }
        }

        if ( !bFound )
            nAt = InsertEntry( aStrEntry, m_aNotCheckedImage, nPos );
        else
            nAt = InsertEntry( aStrEntry, m_aCheckedImage, nPos );
    }
    else
        nAt = InsertEntry( aStrEntry, nPos );

    SetEntryData( nAt, (void*)(ULONG)eLangType );
    return nPos;
}

//------------------------------------------------------------------------

void SvxLanguageBox::RemoveLanguage( const LanguageType eLangType )
{
    USHORT nAt = TypeToPos_Impl( eLangType, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        RemoveEntry( nAt );
}

//------------------------------------------------------------------------

LanguageType SvxLanguageBox::GetSelectLanguage() const
{
    LanguageType eType  = LanguageType(LANGUAGE_DONTKNOW);
    USHORT       nPos   = GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return LanguageType( (ULONG)GetEntryData(nPos) );
    else
        return LanguageType( LANGUAGE_DONTKNOW );
}

//------------------------------------------------------------------------

void SvxLanguageBox::SelectLanguage( const LanguageType eLangType, BOOL bSelect )
{
    USHORT nAt = TypeToPos_Impl( eLangType, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        SelectEntryPos( nAt, bSelect );
}

//------------------------------------------------------------------------

BOOL SvxLanguageBox::IsLanguageSelected( const LanguageType eLangType ) const
{
    USHORT nAt = TypeToPos_Impl( eLangType, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nAt );
    else
        return FALSE;
}


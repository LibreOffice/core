/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newdatatype.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 12:00:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HXX
#include "newdatatype.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_NEWDATATYPE_HRC
#include "newdatatype.hrc"
#endif

#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= NewDataTypeDialog
    //====================================================================
    //--------------------------------------------------------------------
    NewDataTypeDialog::NewDataTypeDialog( Window* _pParent, const ::rtl::OUString& _rNameBase, const ::std::vector< ::rtl::OUString >& _rProhibitedNames )
        :ModalDialog( _pParent, PcrRes( RID_DLG_NEW_DATA_TYPE ) )
        ,m_aLabel   ( this, ResId( FT_LABEL  ) )
        ,m_aName    ( this, ResId( ED_NAME   ) )
        ,m_aOK      ( this, ResId( PB_OK     ) )
        ,m_aCancel  ( this, ResId( PB_CANCEL ) )
        ,m_aProhibitedNames( _rProhibitedNames.begin(), _rProhibitedNames.end() )
    {
        FreeResource();

        m_aName.SetModifyHdl( LINK( this, NewDataTypeDialog, OnNameModified ) );

        // find an initial name
        // for this, first remove trailing digits
        sal_Int32 nStripUntil = _rNameBase.getLength();
        while ( nStripUntil > 0 )
        {
            sal_Unicode nChar = _rNameBase[ --nStripUntil ];
            if ( ( nChar < '0' ) || ( nChar > '9' ) )
            {
                if ( nChar == ' ' )
                    --nStripUntil;  // strip the space, too
                break;
            }
        }

        String sNameBase( _rNameBase.copy( 0, nStripUntil ? nStripUntil + 1 : 0 ) );
        sNameBase.Append( ' ' );
        String sInitialName;
        sal_Int32 nPostfixNumber = 1;
        do
        {
            ( sInitialName = sNameBase ) += String::CreateFromInt32( nPostfixNumber++ );
        }
        while ( m_aProhibitedNames.find( sInitialName ) != m_aProhibitedNames.end() );

        m_aName.SetText( sInitialName );
        OnNameModified( NULL );
    }

    //--------------------------------------------------------------------
    IMPL_LINK( NewDataTypeDialog, OnNameModified, void*, /*_pNotInterestedIn*/ )
    {
        String sCurrentName = GetName();
        bool bNameIsOK = ( sCurrentName.Len() > 0 )
                      && ( m_aProhibitedNames.find( sCurrentName ) == m_aProhibitedNames.end() );

        m_aOK.Enable( bNameIsOK );

        return 0L;
    }

//........................................................................
} // namespace pcr
//........................................................................


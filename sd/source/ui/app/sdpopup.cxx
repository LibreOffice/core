/*************************************************************************
 *
 *  $RCSfile: sdpopup.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:31 $
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

#pragma hdrstop

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include <svx/editeng.hxx>
#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>

#include "strings.hrc"
#include "sdpopup.hxx"
#include "sdresid.hxx"

/*************************************************************************
|*
|* PopupMenu zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

SdFieldPopup::SdFieldPopup( const SvxFieldData* pInField ) :
        PopupMenu   (),
        pField      ( pInField )
{
    Fill();
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SdFieldPopup::~SdFieldPopup()
{
}

/*************************************************************************
|*
|* Fill
|*
\************************************************************************/

void SdFieldPopup::Fill()
{
    USHORT nID = 1;
    USHORT nStyle = MIB_RADIOCHECK | MIB_AUTOCHECK;
    InsertItem( nID++, String( SdResId( STR_FIX ) ), nStyle );
    InsertItem( nID++, String( SdResId( STR_VAR ) ), nStyle );
    InsertSeparator();

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SVXDATETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        //SVXDATEFORMAT_APPDEFAULT,     // Wird nicht benutzt
        //SVXDATEFORMAT_SYSTEM,         // Wird nicht benutzt
        InsertItem( nID++, String( SdResId( STR_STANDARD_SMALL ) ), nStyle );
        InsertItem( nID++, String( SdResId( STR_STANDARD_BIG ) ), nStyle );

        aDateField.SetFormat( SVXDATEFORMAT_A );    // 13.02.96
        InsertItem( nID++, aDateField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_B );    // 13.02.1996
        InsertItem( nID++, aDateField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_C );    // 13.Feb 1996
        InsertItem( nID++, aDateField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );

        aDateField.SetFormat( SVXDATEFORMAT_D );    // 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_E );    // Die, 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_F );    // Dienstag, 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );

        CheckItem( (USHORT) ( pDateField->GetFormat() ) + 1 ); // - 2 + 3 !
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SVXTIMETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        //SVXTIMEFORMAT_APPDEFAULT,     // Wird nicht benutzt
        //SVXTIMEFORMAT_SYSTEM,         // Wird nicht benutzt
        InsertItem( nID++, String( SdResId( STR_STANDARD_NORMAL ) ), nStyle );

        aTimeField.SetFormat( SVXTIMEFORMAT_24_HM );    // 13:49
        InsertItem( nID++, aTimeField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMS );   // 13:49:38
        InsertItem( nID++, aTimeField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMSH );  // 13:49:38.78
        InsertItem( nID++, aTimeField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );

        aTimeField.SetFormat( SVXTIMEFORMAT_12_HM );    // 01:49
        InsertItem( nID++, aTimeField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMS );   // 01:49:38
        InsertItem( nID++, aTimeField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMSH );  // 01:49:38.78
        InsertItem( nID++, aTimeField.GetFormatted( LANGUAGE_SYSTEM, LANGUAGE_SYSTEM ), nStyle );
        //SVXTIMEFORMAT_AM_HM,  // 01:49 PM
        //SVXTIMEFORMAT_AM_HMS, // 01:49:38 PM
        //SVXTIMEFORMAT_AM_HMSH // 01:49:38.78 PM

        CheckItem( (USHORT) ( pTimeField->GetFormat() ) + 1 ); // - 2 + 3 !
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        //SvxExtFileField aFileField( *pFileField );

        if( pFileField->GetType() == SVXFILETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        InsertItem( nID++, String( SdResId( STR_FILEFORMAT_NAME_EXT ) ), nStyle );
        InsertItem( nID++, String( SdResId( STR_FILEFORMAT_FULLPATH ) ), nStyle );
        InsertItem( nID++, String( SdResId( STR_FILEFORMAT_PATH ) ), nStyle );
        InsertItem( nID++, String( SdResId( STR_FILEFORMAT_NAME ) ), nStyle );

        CheckItem( (USHORT) ( pFileField->GetFormat() ) + 3 );
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorField aAuthorField( *pAuthorField );

        if( pAuthorField->GetType() == SVXAUTHORTYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        for( USHORT i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( (SvxAuthorFormat) i );
            InsertItem( nID++, aAuthorField.GetFormatted(), nStyle );
        }
        CheckItem( (USHORT) ( pAuthorField->GetFormat() ) + 3 );
    }
}

/*************************************************************************
|*
|* Gibt das neue Feld zurueck, gehoert dem Caller.
|* Liefert NULL, wenn sich nichts geaendert hat.
|*
\************************************************************************/

SvxFieldData* SdFieldPopup::GetField()
{
    SvxFieldData* pNewField = NULL;
    USHORT nCount = GetItemCount();

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
        SvxDateType   eType;
        SvxDateFormat eFormat;

        if( IsItemChecked( 1 ) )
            eType = SVXDATETYPE_FIX;
        else
            eType = SVXDATETYPE_VAR;

        for( USHORT i = 3; i <= nCount; i++ )
        {
            if( IsItemChecked( i ) )
                break;
        }
        eFormat = (SvxDateFormat) ( i - 1 );

        if( pDateField->GetFormat() != eFormat ||
            pDateField->GetType() != eType )
        {
            pNewField = new SvxDateField( *pDateField );
            ( (SvxDateField*) pNewField )->SetType( eType );
            ( (SvxDateField*) pNewField )->SetFormat( eFormat );
        }
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxTimeType   eType;
        SvxTimeFormat eFormat;

        if( IsItemChecked( 1 ) )
            eType = SVXTIMETYPE_FIX;
        else
            eType = SVXTIMETYPE_VAR;

        for( USHORT i = 3; i <= nCount; i++ )
        {
            if( IsItemChecked( i ) )
                break;
        }
        eFormat = (SvxTimeFormat) ( i - 1 );

        if( pTimeField->GetFormat() != eFormat ||
            pTimeField->GetType() != eType )
        {
            pNewField = new SvxExtTimeField( *pTimeField );
            ( (SvxExtTimeField*) pNewField )->SetType( eType );
            ( (SvxExtTimeField*) pNewField )->SetFormat( eFormat );
        }
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        SvxFileType   eType;
        SvxFileFormat eFormat;

        if( IsItemChecked( 1 ) )
            eType = SVXFILETYPE_FIX;
        else
            eType = SVXFILETYPE_VAR;

        for( USHORT i = 3; i <= nCount; i++ )
        {
            if( IsItemChecked( i ) )
                break;
        }
        eFormat = (SvxFileFormat) ( i - 3 );

        if( pFileField->GetFormat() != eFormat ||
            pFileField->GetType() != eType )
        {
            pNewField = new SvxExtFileField( *pFileField );
            ( (SvxExtFileField*) pNewField )->SetType( eType );
            ( (SvxExtFileField*) pNewField )->SetFormat( eFormat );
        }
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorType   eType;
        SvxAuthorFormat eFormat;

        if( IsItemChecked( 1 ) )
            eType = SVXAUTHORTYPE_FIX;
        else
            eType = SVXAUTHORTYPE_VAR;

        for( USHORT i = 3; i <= nCount; i++ )
        {
            if( IsItemChecked( i ) )
                break;
        }
        eFormat = (SvxAuthorFormat) ( i - 3 );

        if( pAuthorField->GetFormat() != eFormat ||
            pAuthorField->GetType() != eType )
        {
            pNewField = new SvxAuthorField( *pAuthorField );
            ( (SvxAuthorField*) pNewField )->SetType( eType );
            ( (SvxAuthorField*) pNewField )->SetFormat( eFormat );
        }
    }
    return( pNewField );
}



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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/flditem.hxx>
#include <svl/zforlist.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/useroptions.hxx>

#include "strings.hrc"
#include "sdpopup.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"

/*************************************************************************
|*
|* PopupMenu zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

SdFieldPopup::SdFieldPopup( const SvxFieldData* pInField, LanguageType eLanguage ) :
        PopupMenu   (),
        pField      ( pInField )
{
    Fill( eLanguage );
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

void SdFieldPopup::Fill( LanguageType eLanguage )
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

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SVXDATEFORMAT_A );    // 13.02.96
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_B );    // 13.02.1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_C );    // 13.Feb 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

        aDateField.SetFormat( SVXDATEFORMAT_D );    // 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_E );    // Die, 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_F );    // Dienstag, 13.Februar 1996
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

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

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HM );    // 13:49
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMS );   // 13:49:38
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMSH );  // 13:49:38.78
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

        aTimeField.SetFormat( SVXTIMEFORMAT_12_HM );    // 01:49
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMS );   // 01:49:38
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMSH );  // 01:49:38.78
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
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
        USHORT i;

        if( IsItemChecked( 1 ) )
            eType = SVXDATETYPE_FIX;
        else
            eType = SVXDATETYPE_VAR;

        for( i = 3; i <= nCount; i++ )
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

            if( (pDateField->GetType() == SVXDATETYPE_VAR) && (eType == SVXDATETYPE_FIX) )
            {
                Date aDate;
                ( (SvxDateField*) pNewField )->SetFixDate( aDate );
            }
        }
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxTimeType   eType;
        SvxTimeFormat eFormat;
        USHORT i;

        if( IsItemChecked( 1 ) )
            eType = SVXTIMETYPE_FIX;
        else
            eType = SVXTIMETYPE_VAR;

        for( i = 3; i <= nCount; i++ )
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

            if( (pTimeField->GetType() == SVXTIMETYPE_VAR) && (eType == SVXTIMETYPE_FIX) )
            {
                Time aTime;
                ( (SvxExtTimeField*) pNewField )->SetFixTime( aTime );
            }

        }
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        SvxFileType   eType;
        SvxFileFormat eFormat;
        USHORT i;

        if( IsItemChecked( 1 ) )
            eType = SVXFILETYPE_FIX;
        else
            eType = SVXFILETYPE_VAR;

        for( i = 3; i <= nCount; i++ )
        {
            if( IsItemChecked( i ) )
                break;
        }
        eFormat = (SvxFileFormat) ( i - 3 );

        if( pFileField->GetFormat() != eFormat ||
            pFileField->GetType() != eType )
        {
            ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell,
                SfxObjectShell::Current() );

            if( pDocSh )
            {
                SvxExtFileField aFileField( *pFileField );

                String aName;
                if( pDocSh->HasName() )
                    aName = pDocSh->GetMedium()->GetName();

                // Get current filename, not the one stored in the old field
                pNewField = new SvxExtFileField( aName );
                ( (SvxExtFileField*) pNewField )->SetType( eType );
                ( (SvxExtFileField*) pNewField )->SetFormat( eFormat );
            }
        }
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorType   eType;
        SvxAuthorFormat eFormat;
        USHORT i;

        if( IsItemChecked( 1 ) )
            eType = SVXAUTHORTYPE_FIX;
        else
            eType = SVXAUTHORTYPE_VAR;

        for( i = 3; i <= nCount; i++ )
        {
            if( IsItemChecked( i ) )
                break;
        }
        eFormat = (SvxAuthorFormat) ( i - 3 );

        if( pAuthorField->GetFormat() != eFormat ||
            pAuthorField->GetType() != eType )
        {
            // Get current state of address, not the old one
            SvtUserOptions aUserOptions;
            pNewField = new SvxAuthorField( aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() );
            ( (SvxAuthorField*) pNewField )->SetType( eType );
            ( (SvxAuthorField*) pNewField )->SetFormat( eFormat );
        }
    }
    return( pNewField );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

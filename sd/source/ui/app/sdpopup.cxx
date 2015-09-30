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

/*
 * Popup menu for editing of field command
 */
SdFieldPopup::SdFieldPopup( const SvxFieldData* pInField, LanguageType eLanguage ) :
        PopupMenu   (),
        pField      ( pInField )
{
    Fill( eLanguage );
}

SdFieldPopup::~SdFieldPopup()
{
}

void SdFieldPopup::Fill( LanguageType eLanguage )
{
    sal_uInt16 nID = 1;
    MenuItemBits nStyle = MenuItemBits::RADIOCHECK | MenuItemBits::AUTOCHECK;
    InsertItem( nID++, SD_RESSTR( STR_FIX ), nStyle );
    InsertItem( nID++, SD_RESSTR( STR_VAR ), nStyle );
    InsertSeparator();

    if( dynamic_cast< const SvxDateField *>( pField ) !=  nullptr )
    {
        const SvxDateField* pDateField = static_cast<const SvxDateField*>( pField );
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SVXDATETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        //SVXDATEFORMAT_APPDEFAULT,     // is not used
        //SVXDATEFORMAT_SYSTEM,         // is not used
        InsertItem( nID++, SD_RESSTR( STR_STANDARD_SMALL ), nStyle );
        InsertItem( nID++, SD_RESSTR( STR_STANDARD_BIG ), nStyle );

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

        CheckItem( (sal_uInt16) ( pDateField->GetFormat() ) + 1 ); // - 2 + 3 !
    }
    else if( dynamic_cast< const SvxExtTimeField *>( pField ) !=  nullptr )
    {
        const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>( pField );
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SVXTIMETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        //SVXTIMEFORMAT_APPDEFAULT,     // is not used
        //SVXTIMEFORMAT_SYSTEM,         // is not used
        InsertItem( nID++, SD_RESSTR( STR_STANDARD_NORMAL ), nStyle );

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

        CheckItem( (sal_uInt16) ( pTimeField->GetFormat() ) + 1 ); // - 2 + 3 !
    }
    else if( dynamic_cast< const SvxExtFileField *>( pField ) !=  nullptr )
    {
        const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>(pField);
        //SvxExtFileField aFileField( *pFileField );

        if( pFileField->GetType() == SVXFILETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        InsertItem( nID++, SD_RESSTR( STR_FILEFORMAT_NAME_EXT ), nStyle );
        InsertItem( nID++, SD_RESSTR( STR_FILEFORMAT_FULLPATH ), nStyle );
        InsertItem( nID++, SD_RESSTR( STR_FILEFORMAT_PATH ), nStyle );
        InsertItem( nID++, SD_RESSTR( STR_FILEFORMAT_NAME ), nStyle );

        CheckItem( (sal_uInt16) ( pFileField->GetFormat() ) + 3 );
    }
    else if( dynamic_cast< const SvxAuthorField *>( pField ) !=  nullptr )
    {
        const SvxAuthorField* pAuthorField = static_cast<const SvxAuthorField*>(pField);
        SvxAuthorField aAuthorField( *pAuthorField );

        if( pAuthorField->GetType() == SVXAUTHORTYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        for( sal_uInt16 i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( (SvxAuthorFormat) i );
            InsertItem( nID++, aAuthorField.GetFormatted(), nStyle );
        }
        CheckItem( (sal_uInt16) ( pAuthorField->GetFormat() ) + 3 );
    }
}

/**
 * Returns a new field, owned by caller.
 * Returns NULL if nothing changed.
 */
SvxFieldData* SdFieldPopup::GetField()
{
    SvxFieldData* pNewField = NULL;
    sal_uInt16 nCount = GetItemCount();

    if( dynamic_cast< const SvxDateField *>( pField ) !=  nullptr )
    {
        const SvxDateField* pDateField = static_cast<const SvxDateField*>(pField);
        SvxDateType   eType;
        SvxDateFormat eFormat;
        sal_uInt16 i;

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
            static_cast<SvxDateField*>( pNewField )->SetType( eType );
            static_cast<SvxDateField*>( pNewField )->SetFormat( eFormat );

            if( (pDateField->GetType() == SVXDATETYPE_VAR) && (eType == SVXDATETYPE_FIX) )
            {
                Date aDate( Date::SYSTEM );
                static_cast<SvxDateField*>( pNewField )->SetFixDate( aDate );
            }
        }
    }
    else if( dynamic_cast< const SvxExtTimeField *>( pField ) !=  nullptr )
    {
        const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>(pField);
        SvxTimeType   eType;
        SvxTimeFormat eFormat;
        sal_uInt16 i;

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
            static_cast<SvxExtTimeField*>( pNewField )->SetType( eType );
            static_cast<SvxExtTimeField*>( pNewField )->SetFormat( eFormat );

            if( (pTimeField->GetType() == SVXTIMETYPE_VAR) && (eType == SVXTIMETYPE_FIX) )
            {
                tools::Time aTime( tools::Time::SYSTEM );
                static_cast<SvxExtTimeField*>( pNewField )->SetFixTime( aTime );
            }

        }
    }
    else if( dynamic_cast< const SvxExtFileField *>( pField ) !=  nullptr )
    {
        const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>(pField);
        SvxFileType   eType;
        SvxFileFormat eFormat;
        sal_uInt16 i;

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
            ::sd::DrawDocShell* pDocSh = dynamic_cast<::sd::DrawDocShell* >( SfxObjectShell::Current() );

            if( pDocSh )
            {
                SvxExtFileField aFileField( *pFileField );

                OUString aName;
                if( pDocSh->HasName() )
                    aName = pDocSh->GetMedium()->GetName();

                // Get current filename, not the one stored in the old field
                pNewField = new SvxExtFileField( aName );
                static_cast<SvxExtFileField*>( pNewField )->SetType( eType );
                static_cast<SvxExtFileField*>( pNewField )->SetFormat( eFormat );
            }
        }
    }
    else if( dynamic_cast< const SvxAuthorField *>( pField ) !=  nullptr )
    {
        const SvxAuthorField* pAuthorField = static_cast<const SvxAuthorField*>(pField);
        SvxAuthorType   eType;
        SvxAuthorFormat eFormat;
        sal_uInt16 i;

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
            static_cast<SvxAuthorField*>( pNewField )->SetType( eType );
            static_cast<SvxAuthorField*>( pNewField )->SetFormat( eFormat );
        }
    }
    return pNewField;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    sal_uInt16 nStyle = MIB_RADIOCHECK | MIB_AUTOCHECK;
    InsertItem( nID++, SD_RESSTR( STR_FIX ), nStyle );
    InsertItem( nID++, SD_RESSTR( STR_VAR ), nStyle );
    InsertSeparator();

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SVXDATETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        
        
        InsertItem( nID++, SD_RESSTR( STR_STANDARD_SMALL ), nStyle );
        InsertItem( nID++, SD_RESSTR( STR_STANDARD_BIG ), nStyle );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SVXDATEFORMAT_A );    
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_B );    
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_C );    
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

        aDateField.SetFormat( SVXDATEFORMAT_D );    
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_E );    
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aDateField.SetFormat( SVXDATEFORMAT_F );    
        InsertItem( nID++, aDateField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

        CheckItem( (sal_uInt16) ( pDateField->GetFormat() ) + 1 ); 
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SVXTIMETYPE_FIX )
            CheckItem( 1 );
        else
            CheckItem( 2 );

        
        
        InsertItem( nID++, SD_RESSTR( STR_STANDARD_NORMAL ), nStyle );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HM );    
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMS );   
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMSH );  
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );

        aTimeField.SetFormat( SVXTIMEFORMAT_12_HM );    
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMS );   
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMSH );  
        InsertItem( nID++, aTimeField.GetFormatted( *pNumberFormatter, eLanguage ), nStyle );
        
        
        

        CheckItem( (sal_uInt16) ( pTimeField->GetFormat() ) + 1 ); 
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        

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
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
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

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
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
            ( (SvxDateField*) pNewField )->SetType( eType );
            ( (SvxDateField*) pNewField )->SetFormat( eFormat );

            if( (pDateField->GetType() == SVXDATETYPE_VAR) && (eType == SVXDATETYPE_FIX) )
            {
                Date aDate( Date::SYSTEM );
                ( (SvxDateField*) pNewField )->SetFixDate( aDate );
            }
        }
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
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
            ( (SvxExtTimeField*) pNewField )->SetType( eType );
            ( (SvxExtTimeField*) pNewField )->SetFormat( eFormat );

            if( (pTimeField->GetType() == SVXTIMETYPE_VAR) && (eType == SVXTIMETYPE_FIX) )
            {
                Time aTime( Time::SYSTEM );
                ( (SvxExtTimeField*) pNewField )->SetFixTime( aTime );
            }

        }
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
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
            ::sd::DrawDocShell* pDocSh = PTR_CAST(::sd::DrawDocShell,
                SfxObjectShell::Current() );

            if( pDocSh )
            {
                SvxExtFileField aFileField( *pFileField );

                OUString aName;
                if( pDocSh->HasName() )
                    aName = pDocSh->GetMedium()->GetName();

                
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
            
            SvtUserOptions aUserOptions;
            pNewField = new SvxAuthorField( aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() );
            ( (SvxAuthorField*) pNewField )->SetType( eType );
            ( (SvxAuthorField*) pNewField )->SetFormat( eFormat );
        }
    }
    return( pNewField );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

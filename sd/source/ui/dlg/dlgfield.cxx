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

#include <editeng/editeng.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <svl/zforlist.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <svl/itemset.hxx>
#include <editeng/langitem.hxx>
#include <unotools/useroptions.hxx>

#include "strings.hrc"
#include "dlgfield.hrc"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "dlgfield.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"

/**
 * dialog to edit field commands
 */
SdModifyFieldDlg::SdModifyFieldDlg( Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) :
    ModalDialog ( pWindow, SdResId( DLG_FIELD_MODIFY ) ),
    aGrpType    ( this, SdResId( GRP_TYPE ) ),
    aRbtFix     ( this, SdResId( RBT_FIX ) ),
    aRbtVar     ( this, SdResId( RBT_VAR ) ),
    maFtLanguage( this, SdResId( FT_LANGUAGE ) ),
    maLbLanguage( this, SdResId( LB_LANGUAGE ) ),
    aFtFormat   ( this, SdResId( FT_FORMAT ) ),
    aLbFormat   ( this, SdResId( LB_FORMAT ) ),
    aBtnOK      ( this, SdResId( BTN_OK ) ),
    aBtnCancel  ( this, SdResId( BTN_CANCEL ) ),
    aBtnHelp    ( this, SdResId( BTN_HELP ) ),
    maInputSet  ( rSet ),
    pField      ( pInField )
{
    FreeResource();

    maLbLanguage.SetLanguageList( LANG_LIST_ALL|LANG_LIST_ONLY_KNOWN, false );
    maLbLanguage.SetSelectHdl( LINK( this, SdModifyFieldDlg, LanguageChangeHdl ) );
    FillControls();
}

/**
 * Returns the new field, owned by caller.
 * Returns NULL if nothing has changed.
 */
SvxFieldData* SdModifyFieldDlg::GetField()
{
    SvxFieldData* pNewField = NULL;

    if( aRbtFix.IsChecked() != aRbtFix.GetSavedValue() ||
        aRbtVar.IsChecked() != aRbtVar.GetSavedValue() ||
        aLbFormat.GetSelectEntryPos() != aLbFormat.GetSavedValue() )
    {
        if( pField->ISA( SvxDateField ) )
        {
            const SvxDateField* pDateField = (const SvxDateField*) pField;
            SvxDateType   eType;
            SvxDateFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXDATETYPE_FIX;
            else
                eType = SVXDATETYPE_VAR;

            eFormat = (SvxDateFormat) ( aLbFormat.GetSelectEntryPos() + 2 );

            pNewField = new SvxDateField( *pDateField );
            ( (SvxDateField*) pNewField )->SetType( eType );
            ( (SvxDateField*) pNewField )->SetFormat( eFormat );
        }
        else if( pField->ISA( SvxExtTimeField ) )
        {
            const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
            SvxTimeType   eType;
            SvxTimeFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXTIMETYPE_FIX;
            else
                eType = SVXTIMETYPE_VAR;

            eFormat = (SvxTimeFormat) ( aLbFormat.GetSelectEntryPos() + 2 );

            pNewField = new SvxExtTimeField( *pTimeField );
            ( (SvxExtTimeField*) pNewField )->SetType( eType );
            ( (SvxExtTimeField*) pNewField )->SetFormat( eFormat );
        }
        else if( pField->ISA( SvxExtFileField ) )
        {
            const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
            SvxFileType   eType;
            SvxFileFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXFILETYPE_FIX;
            else
                eType = SVXFILETYPE_VAR;

            eFormat = (SvxFileFormat) ( aLbFormat.GetSelectEntryPos() );

            ::sd::DrawDocShell* pDocSh = PTR_CAST( ::sd::DrawDocShell,
                                               SfxObjectShell::Current() );

            if( pDocSh )
            {
                SvxExtFileField aFileField( *pFileField );

                OUString aName;
                if( pDocSh->HasName() )
                    aName = pDocSh->GetMedium()->GetName();

                // Get current filename, not the one stored in the old field
                pNewField = new SvxExtFileField( aName );
                ( (SvxExtFileField*) pNewField )->SetType( eType );
                ( (SvxExtFileField*) pNewField )->SetFormat( eFormat );
            }
        }
        else if( pField->ISA( SvxAuthorField ) )
        {
            SvxAuthorType   eType;
            SvxAuthorFormat eFormat;

            if( aRbtFix.IsChecked() )
                eType = SVXAUTHORTYPE_FIX;
            else
                eType = SVXAUTHORTYPE_VAR;

            eFormat = (SvxAuthorFormat) ( aLbFormat.GetSelectEntryPos() );

            // Get current state of address, not the old one
            SvtUserOptions aUserOptions;
            pNewField = new SvxAuthorField( aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() );
            ( (SvxAuthorField*) pNewField )->SetType( eType );
            ( (SvxAuthorField*) pNewField )->SetFormat( eFormat );
        }
    }

    return( pNewField );
}

void SdModifyFieldDlg::FillFormatList()
{
    LanguageType eLangType = maLbLanguage.GetSelectLanguage();

    aLbFormat.Clear();

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
        SvxDateField aDateField( *pDateField );

        //SVXDATEFORMAT_APPDEFAULT,     // not used
        //SVXDATEFORMAT_SYSTEM,         // not used
        aLbFormat.InsertEntry( SD_RESSTR( STR_STANDARD_SMALL ) );
        aLbFormat.InsertEntry( SD_RESSTR( STR_STANDARD_BIG ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SVXDATEFORMAT_A );    // 13.02.96
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_B );    // 13.02.1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_C );    // 13.Feb 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_D );    // 13.Februar 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_E );    // Die, 13.Februar 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_F );    // Dienstag, 13.Februar 1996
        aLbFormat.InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );

        aLbFormat.SelectEntryPos( (sal_uInt16) ( pDateField->GetFormat() - 2 ) );
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxExtTimeField aTimeField( *pTimeField );

        //SVXTIMEFORMAT_APPDEFAULT,     // not used
        //SVXTIMEFORMAT_SYSTEM,         // not used
        aLbFormat.InsertEntry( SD_RESSTR( STR_STANDARD_NORMAL ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HM );    // 13:49
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMS );   // 13:49:38
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMSH );  // 13:49:38.78
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HM );    // 01:49
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMS );   // 01:49:38
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMSH );  // 01:49:38.78
        aLbFormat.InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        //SVXTIMEFORMAT_AM_HM,  // 01:49 PM
        //SVXTIMEFORMAT_AM_HMS, // 01:49:38 PM
        //SVXTIMEFORMAT_AM_HMSH // 01:49:38.78 PM

        aLbFormat.SelectEntryPos( (sal_uInt16) ( pTimeField->GetFormat() - 2 ) );
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        SvxExtFileField aFileField( *pFileField );

        aLbFormat.InsertEntry( SD_RESSTR( STR_FILEFORMAT_NAME_EXT ) );
        aLbFormat.InsertEntry( SD_RESSTR( STR_FILEFORMAT_FULLPATH ) );
        aLbFormat.InsertEntry( SD_RESSTR( STR_FILEFORMAT_PATH ) );
        aLbFormat.InsertEntry( SD_RESSTR( STR_FILEFORMAT_NAME ) );

        aLbFormat.SelectEntryPos( (sal_uInt16) ( pFileField->GetFormat() ) );
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorField aAuthorField( *pAuthorField );

        for( sal_uInt16 i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( (SvxAuthorFormat) i );
            aLbFormat.InsertEntry( aAuthorField.GetFormatted() );
        }

        aLbFormat.SelectEntryPos( (sal_uInt16) ( pAuthorField->GetFormat() ) );

    }


}

void SdModifyFieldDlg::FillControls()
{
    aLbFormat.Clear();

    if( pField->ISA( SvxDateField ) )
    {
        const SvxDateField* pDateField = (const SvxDateField*) pField;
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SVXDATETYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SVXTIMETYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        SvxExtFileField aFileField( *pFileField );

        if( pFileField->GetType() == SVXFILETYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorField aAuthorField( *pAuthorField );

        if( pAuthorField->GetType() == SVXAUTHORTYPE_FIX )
            aRbtFix.Check();
        else
            aRbtVar.Check();
    }
    aRbtFix.SaveValue();
    aRbtVar.SaveValue();

    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == maInputSet.GetItemState(EE_CHAR_LANGUAGE, sal_True, &pItem ) )
        maLbLanguage.SelectLanguage( static_cast<const SvxLanguageItem*>(pItem)->GetLanguage() );

    maLbLanguage.SaveValue();

    FillFormatList();
    aLbFormat.SaveValue();
}


IMPL_LINK_NOARG(SdModifyFieldDlg, LanguageChangeHdl)
{
    FillFormatList();

    return 0L;
}

SfxItemSet SdModifyFieldDlg::GetItemSet()
{
    SfxItemSet aOutput( *maInputSet.GetPool(), EE_CHAR_LANGUAGE, EE_CHAR_LANGUAGE_CTL );

    if( maLbLanguage.GetSelectEntryPos() != maLbLanguage.GetSavedValue() )
    {
        LanguageType eLangType = maLbLanguage.GetSelectLanguage();
        SvxLanguageItem aItem( eLangType, EE_CHAR_LANGUAGE );
        aOutput.Put( aItem );

        SvxLanguageItem aItemCJK( eLangType, EE_CHAR_LANGUAGE_CJK );
        aOutput.Put( aItemCJK );

        SvxLanguageItem aItemCTL( eLangType, EE_CHAR_LANGUAGE_CTL );
        aOutput.Put( aItemCTL );
    }

    return aOutput;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

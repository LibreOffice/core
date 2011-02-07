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

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif



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

/*************************************************************************
|*
|* Dialog zum Bearbeiten von Feldbefehlen
|*
\************************************************************************/

SdModifyFieldDlg::SdModifyFieldDlg( Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) :
    ModalDialog ( pWindow, SdResId( DLG_FIELD_MODIFY ) ),
    aRbtFix     ( this, SdResId( RBT_FIX ) ),
    aRbtVar     ( this, SdResId( RBT_VAR ) ),
    aGrpType    ( this, SdResId( GRP_TYPE ) ),
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

/*************************************************************************
|*
|* Gibt das neue Feld zurueck, gehoert dem Caller.
|* Liefert NULL, wenn sich nichts geaendert hat.
|*
\************************************************************************/

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

                String aName;
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

        //SVXDATEFORMAT_APPDEFAULT,     // Wird nicht benutzt
        //SVXDATEFORMAT_SYSTEM,         // Wird nicht benutzt
        aLbFormat.InsertEntry( String( SdResId( STR_STANDARD_SMALL ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_STANDARD_BIG ) ) );

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

        aLbFormat.SelectEntryPos( (USHORT) ( pDateField->GetFormat() - 2 ) );
    }
    else if( pField->ISA( SvxExtTimeField ) )
    {
        const SvxExtTimeField* pTimeField = (const SvxExtTimeField*) pField;
        SvxExtTimeField aTimeField( *pTimeField );

        //SVXTIMEFORMAT_APPDEFAULT,     // Wird nicht benutzt
        //SVXTIMEFORMAT_SYSTEM,         // Wird nicht benutzt
        aLbFormat.InsertEntry( String( SdResId( STR_STANDARD_NORMAL ) ) );

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

        aLbFormat.SelectEntryPos( (USHORT) ( pTimeField->GetFormat() - 2 ) );
    }
    else if( pField->ISA( SvxExtFileField ) )
    {
        const SvxExtFileField* pFileField = (const SvxExtFileField*) pField;
        SvxExtFileField aFileField( *pFileField );

        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_NAME_EXT ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_FULLPATH ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_PATH ) ) );
        aLbFormat.InsertEntry( String( SdResId( STR_FILEFORMAT_NAME ) ) );

        aLbFormat.SelectEntryPos( (USHORT) ( pFileField->GetFormat() ) );
    }
    else if( pField->ISA( SvxAuthorField ) )
    {
        const SvxAuthorField* pAuthorField = (const SvxAuthorField*) pField;
        SvxAuthorField aAuthorField( *pAuthorField );

        for( USHORT i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( (SvxAuthorFormat) i );
            aLbFormat.InsertEntry( aAuthorField.GetFormatted() );
        }

        aLbFormat.SelectEntryPos( (USHORT) ( pAuthorField->GetFormat() ) );

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
    if( SFX_ITEM_SET == maInputSet.GetItemState(EE_CHAR_LANGUAGE, TRUE, &pItem ) )
        maLbLanguage.SelectLanguage( static_cast<const SvxLanguageItem*>(pItem)->GetLanguage() );

    maLbLanguage.SaveValue();

    FillFormatList();
    aLbFormat.SaveValue();
}


IMPL_LINK( SdModifyFieldDlg, LanguageChangeHdl, void *, EMPTYARG )
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

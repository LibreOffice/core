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
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "sdmod.hxx"
#include "dlgfield.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"

/**
 * dialog to edit field commands
 */
SdModifyFieldDlg::SdModifyFieldDlg( vcl::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) :
    ModalDialog ( pWindow, "EditFieldsDialog", "modules/simpress/ui/dlgfield.ui" ),
    maInputSet  ( rSet ),
    pField      ( pInField )
{
    get(m_pRbtFix, "fixedRB");
    get(m_pRbtVar, "varRB");
    get(m_pLbLanguage, "languageLB");
    get(m_pLbFormat, "formatLB");

    m_pLbLanguage->SetLanguageList( SvxLanguageListFlags::ALL|SvxLanguageListFlags::ONLY_KNOWN, false );
    m_pLbLanguage->SetSelectHdl( LINK( this, SdModifyFieldDlg, LanguageChangeHdl ) );
    FillControls();
}

SdModifyFieldDlg::~SdModifyFieldDlg()
{
    disposeOnce();
}

void SdModifyFieldDlg::dispose()
{
    m_pRbtFix.clear();
    m_pRbtVar.clear();
    m_pLbLanguage.clear();
    m_pLbFormat.clear();
    ModalDialog::dispose();
}


/**
 * Returns the new field, owned by caller.
 * Returns NULL if nothing has changed.
 */
SvxFieldData* SdModifyFieldDlg::GetField()
{
    SvxFieldData* pNewField = NULL;

    if( m_pRbtFix->IsValueChangedFromSaved() ||
        m_pRbtVar->IsValueChangedFromSaved() ||
        m_pLbFormat->IsValueChangedFromSaved() )
    {
        if( dynamic_cast< const SvxDateField *>( pField ) !=  nullptr )
        {
            const SvxDateField* pDateField = static_cast<const SvxDateField*>(pField);
            SvxDateType   eType;
            SvxDateFormat eFormat;

            if( m_pRbtFix->IsChecked() )
                eType = SVXDATETYPE_FIX;
            else
                eType = SVXDATETYPE_VAR;

            eFormat = (SvxDateFormat) ( m_pLbFormat->GetSelectEntryPos() + 2 );

            pNewField = new SvxDateField( *pDateField );
            static_cast<SvxDateField*>( pNewField )->SetType( eType );
            static_cast<SvxDateField*>( pNewField )->SetFormat( eFormat );
        }
        else if( dynamic_cast< const SvxExtTimeField *>( pField ) !=  nullptr )
        {
            const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>( pField );
            SvxTimeType   eType;
            SvxTimeFormat eFormat;

            if( m_pRbtFix->IsChecked() )
                eType = SVXTIMETYPE_FIX;
            else
                eType = SVXTIMETYPE_VAR;

            eFormat = (SvxTimeFormat) ( m_pLbFormat->GetSelectEntryPos() + 2 );

            pNewField = new SvxExtTimeField( *pTimeField );
            static_cast<SvxExtTimeField*>( pNewField )->SetType( eType );
            static_cast<SvxExtTimeField*>( pNewField )->SetFormat( eFormat );
        }
        else if( dynamic_cast< const SvxExtFileField *>( pField ) !=  nullptr )
        {
            const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>( pField );
            SvxFileType   eType;
            SvxFileFormat eFormat;

            if( m_pRbtFix->IsChecked() )
                eType = SVXFILETYPE_FIX;
            else
                eType = SVXFILETYPE_VAR;

            eFormat = (SvxFileFormat) ( m_pLbFormat->GetSelectEntryPos() );

            ::sd::DrawDocShell* pDocSh = dynamic_cast< ::sd::DrawDocShell* >(SfxObjectShell::Current() );

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
        else if( dynamic_cast< const SvxAuthorField *>( pField ) !=  nullptr )
        {
            SvxAuthorType   eType;
            SvxAuthorFormat eFormat;

            if( m_pRbtFix->IsChecked() )
                eType = SVXAUTHORTYPE_FIX;
            else
                eType = SVXAUTHORTYPE_VAR;

            eFormat = (SvxAuthorFormat) ( m_pLbFormat->GetSelectEntryPos() );

            // Get current state of address, not the old one
            SvtUserOptions aUserOptions;
            pNewField = new SvxAuthorField( aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() );
            static_cast<SvxAuthorField*>( pNewField )->SetType( eType );
            static_cast<SvxAuthorField*>( pNewField )->SetFormat( eFormat );
        }
    }

    return pNewField;
}

void SdModifyFieldDlg::FillFormatList()
{
    LanguageType eLangType = m_pLbLanguage->GetSelectLanguage();

    m_pLbFormat->Clear();

    if( dynamic_cast< const SvxDateField *>( pField ) !=  nullptr )
    {
        const SvxDateField* pDateField = static_cast<const SvxDateField*>( pField );
        SvxDateField aDateField( *pDateField );

        //SVXDATEFORMAT_APPDEFAULT,     // not used
        //SVXDATEFORMAT_SYSTEM,         // not used
        m_pLbFormat->InsertEntry( SD_RESSTR( STR_STANDARD_SMALL ) );
        m_pLbFormat->InsertEntry( SD_RESSTR( STR_STANDARD_BIG ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SVXDATEFORMAT_A );    // 13.02.96
        m_pLbFormat->InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_B );    // 13.02.1996
        m_pLbFormat->InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_C );    // 13.Feb 1996
        m_pLbFormat->InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_D );    // 13.Februar 1996
        m_pLbFormat->InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_E );    // Die, 13.Februar 1996
        m_pLbFormat->InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SVXDATEFORMAT_F );    // Dienstag, 13.Februar 1996
        m_pLbFormat->InsertEntry( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );

        m_pLbFormat->SelectEntryPos( (sal_uInt16) ( pDateField->GetFormat() - 2 ) );
    }
    else if( dynamic_cast< const SvxExtTimeField *>( pField ) !=  nullptr )
    {
        const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>( pField );
        SvxExtTimeField aTimeField( *pTimeField );

        //SVXTIMEFORMAT_APPDEFAULT,     // not used
        //SVXTIMEFORMAT_SYSTEM,         // not used
        m_pLbFormat->InsertEntry( SD_RESSTR( STR_STANDARD_NORMAL ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HM );    // 13:49
        m_pLbFormat->InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMS );   // 13:49:38
        m_pLbFormat->InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_24_HMSH );  // 13:49:38.78
        m_pLbFormat->InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HM );    // 01:49
        m_pLbFormat->InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMS );   // 01:49:38
        m_pLbFormat->InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SVXTIMEFORMAT_12_HMSH );  // 01:49:38.78
        m_pLbFormat->InsertEntry( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        //SVXTIMEFORMAT_AM_HM,  // 01:49 PM
        //SVXTIMEFORMAT_AM_HMS, // 01:49:38 PM
        //SVXTIMEFORMAT_AM_HMSH // 01:49:38.78 PM

        m_pLbFormat->SelectEntryPos( (sal_uInt16) ( pTimeField->GetFormat() - 2 ) );
    }
    else if( dynamic_cast< const SvxExtFileField *>( pField ) !=  nullptr )
    {
        const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>( pField );
        SvxExtFileField aFileField( *pFileField );

        m_pLbFormat->InsertEntry( SD_RESSTR( STR_FILEFORMAT_NAME_EXT ) );
        m_pLbFormat->InsertEntry( SD_RESSTR( STR_FILEFORMAT_FULLPATH ) );
        m_pLbFormat->InsertEntry( SD_RESSTR( STR_FILEFORMAT_PATH ) );
        m_pLbFormat->InsertEntry( SD_RESSTR( STR_FILEFORMAT_NAME ) );

        m_pLbFormat->SelectEntryPos( (sal_uInt16) ( pFileField->GetFormat() ) );
    }
    else if( dynamic_cast< const SvxAuthorField *>( pField ) !=  nullptr )
    {
        const SvxAuthorField* pAuthorField = static_cast<const SvxAuthorField*>( pField );
        SvxAuthorField aAuthorField( *pAuthorField );

        for( sal_uInt16 i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( (SvxAuthorFormat) i );
            m_pLbFormat->InsertEntry( aAuthorField.GetFormatted() );
        }

        m_pLbFormat->SelectEntryPos( (sal_uInt16) ( pAuthorField->GetFormat() ) );

    }

}

void SdModifyFieldDlg::FillControls()
{
    m_pLbFormat->Clear();

    if( dynamic_cast< const SvxDateField *>( pField ) !=  nullptr )
    {
        const SvxDateField* pDateField = static_cast<const SvxDateField*>(pField);
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SVXDATETYPE_FIX )
            m_pRbtFix->Check();
        else
            m_pRbtVar->Check();
    }
    else if( dynamic_cast< const SvxExtTimeField *>( pField ) !=  nullptr )
    {
        const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>(pField);
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SVXTIMETYPE_FIX )
            m_pRbtFix->Check();
        else
            m_pRbtVar->Check();
    }
    else if( dynamic_cast< const SvxExtFileField *>( pField ) !=  nullptr )
    {
        const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>(pField);
        SvxExtFileField aFileField( *pFileField );

        if( pFileField->GetType() == SVXFILETYPE_FIX )
            m_pRbtFix->Check();
        else
            m_pRbtVar->Check();
    }
    else if( dynamic_cast< const SvxAuthorField *>( pField ) !=  nullptr )
    {
        const SvxAuthorField* pAuthorField = static_cast<const SvxAuthorField*>(pField);
        SvxAuthorField aAuthorField( *pAuthorField );

        if( pAuthorField->GetType() == SVXAUTHORTYPE_FIX )
            m_pRbtFix->Check();
        else
            m_pRbtVar->Check();
    }
    m_pRbtFix->SaveValue();
    m_pRbtVar->SaveValue();

    const SfxPoolItem* pItem;
    if( SfxItemState::SET == maInputSet.GetItemState(EE_CHAR_LANGUAGE, true, &pItem ) )
        m_pLbLanguage->SelectLanguage( static_cast<const SvxLanguageItem*>(pItem)->GetLanguage() );

    m_pLbLanguage->SaveValue();

    FillFormatList();
    m_pLbFormat->SaveValue();
}

IMPL_LINK_NOARG_TYPED(SdModifyFieldDlg, LanguageChangeHdl, ListBox&, void)
{
    FillFormatList();
}

SfxItemSet SdModifyFieldDlg::GetItemSet()
{
    SfxItemSet aOutput( *maInputSet.GetPool(), EE_CHAR_LANGUAGE, EE_CHAR_LANGUAGE_CTL );

    if( m_pLbLanguage->IsValueChangedFromSaved() )
    {
        LanguageType eLangType = m_pLbLanguage->GetSelectLanguage();
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

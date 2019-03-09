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
#include <editeng/flditem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <svl/itemset.hxx>
#include <svx/langbox.hxx>
#include <editeng/langitem.hxx>
#include <unotools/useroptions.hxx>

#include <strings.hrc>
#include <sdresid.hxx>
#include <sdmod.hxx>
#include <dlgfield.hxx>
#include <DrawDocShell.hxx>

/**
 * dialog to edit field commands
 */
SdModifyFieldDlg::SdModifyFieldDlg(weld::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet)
    : GenericDialogController(pWindow, "modules/simpress/ui/dlgfield.ui", "EditFieldsDialog")
    , m_aInputSet(rSet)
    , m_pField(pInField)
    , m_xRbtFix(m_xBuilder->weld_radio_button("fixedRB"))
    , m_xRbtVar(m_xBuilder->weld_radio_button("varRB"))
    , m_xLbLanguage(new LanguageBox(m_xBuilder->weld_combo_box("languageLB")))
    , m_xLbFormat(m_xBuilder->weld_combo_box("formatLB"))
{
    m_xLbLanguage->SetLanguageList( SvxLanguageListFlags::ALL|SvxLanguageListFlags::ONLY_KNOWN, false );
    m_xLbLanguage->connect_changed(LINK(this, SdModifyFieldDlg, LanguageChangeHdl));
    FillControls();
}

SdModifyFieldDlg::~SdModifyFieldDlg()
{
}

/**
 * Returns the new field, owned by caller.
 * Returns NULL if nothing has changed.
 */
SvxFieldData* SdModifyFieldDlg::GetField()
{
    SvxFieldData* pNewField = nullptr;

    if( m_xRbtFix->get_state_changed_from_saved() ||
        m_xRbtVar->get_state_changed_from_saved() ||
        m_xLbFormat->get_value_changed_from_saved() )
    {
        if( dynamic_cast< const SvxDateField *>( m_pField ) !=  nullptr )
        {
            const SvxDateField* pDateField = static_cast<const SvxDateField*>(m_pField);
            SvxDateType   eType;
            SvxDateFormat eFormat;

            if( m_xRbtFix->get_active() )
                eType = SvxDateType::Fix;
            else
                eType = SvxDateType::Var;

            eFormat = static_cast<SvxDateFormat>( m_xLbFormat->get_active() + 2 );

            pNewField = new SvxDateField( *pDateField );
            static_cast<SvxDateField*>( pNewField )->SetType( eType );
            static_cast<SvxDateField*>( pNewField )->SetFormat( eFormat );
        }
        else if( dynamic_cast< const SvxExtTimeField *>( m_pField ) !=  nullptr )
        {
            const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>( m_pField );
            SvxTimeType   eType;
            SvxTimeFormat eFormat;

            if( m_xRbtFix->get_active() )
                eType = SvxTimeType::Fix;
            else
                eType = SvxTimeType::Var;

            eFormat = static_cast<SvxTimeFormat>( m_xLbFormat->get_active() + 2 );

            pNewField = new SvxExtTimeField( *pTimeField );
            static_cast<SvxExtTimeField*>( pNewField )->SetType( eType );
            static_cast<SvxExtTimeField*>( pNewField )->SetFormat( eFormat );
        }
        else if( dynamic_cast< const SvxExtFileField *>( m_pField ) !=  nullptr )
        {
            const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>( m_pField );
            SvxFileType   eType;
            SvxFileFormat eFormat;

            if( m_xRbtFix->get_active() )
                eType = SvxFileType::Fix;
            else
                eType = SvxFileType::Var;

            eFormat = static_cast<SvxFileFormat>( m_xLbFormat->get_active() );

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
        else if( dynamic_cast< const SvxAuthorField *>( m_pField ) !=  nullptr )
        {
            SvxAuthorType   eType;
            SvxAuthorFormat eFormat;

            if( m_xRbtFix->get_active() )
                eType = SvxAuthorType::Fix;
            else
                eType = SvxAuthorType::Var;

            eFormat = static_cast<SvxAuthorFormat>( m_xLbFormat->get_active() );

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
    LanguageType eLangType = m_xLbLanguage->get_active_id();

    m_xLbFormat->clear();

    if( dynamic_cast< const SvxDateField *>( m_pField ) !=  nullptr )
    {
        const SvxDateField* pDateField = static_cast<const SvxDateField*>( m_pField );
        SvxDateField aDateField( *pDateField );

        //SvxDateFormat::AppDefault,     // not used
        //SvxDateFormat::System,         // not used
        m_xLbFormat->append_text( SdResId( STR_STANDARD_SMALL ) );
        m_xLbFormat->append_text( SdResId( STR_STANDARD_BIG ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SvxDateFormat::A );    // 13.02.96
        m_xLbFormat->append_text( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SvxDateFormat::B );    // 13.02.1996
        m_xLbFormat->append_text( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SvxDateFormat::C );    // 13.Feb 1996
        m_xLbFormat->append_text( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SvxDateFormat::D );    // 13.Februar 1996
        m_xLbFormat->append_text( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SvxDateFormat::E );    // Die, 13.Februar 1996
        m_xLbFormat->append_text( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );
        aDateField.SetFormat( SvxDateFormat::F );    // Dienstag, 13.Februar 1996
        m_xLbFormat->append_text( aDateField.GetFormatted( *pNumberFormatter, eLangType ) );

        m_xLbFormat->set_active( static_cast<sal_uInt16>(pDateField->GetFormat()) - 2 );
    }
    else if( dynamic_cast< const SvxExtTimeField *>( m_pField ) !=  nullptr )
    {
        const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>( m_pField );
        SvxExtTimeField aTimeField( *pTimeField );

        //SvxTimeFormat::AppDefault,     // not used
        //SvxTimeFormat::System,         // not used
        m_xLbFormat->append_text( SdResId( STR_STANDARD_NORMAL ) );

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aTimeField.SetFormat( SvxTimeFormat::HH24_MM );    // 13:49
        m_xLbFormat->append_text( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SvxTimeFormat::HH24_MM_SS );   // 13:49:38
        m_xLbFormat->append_text( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SvxTimeFormat::HH24_MM_SS_00 );  // 13:49:38.78
        m_xLbFormat->append_text( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SvxTimeFormat::HH12_MM );    // 01:49
        m_xLbFormat->append_text( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SvxTimeFormat::HH12_MM_SS );   // 01:49:38
        m_xLbFormat->append_text( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        aTimeField.SetFormat( SvxTimeFormat::HH12_MM_SS_00 );  // 01:49:38.78
        m_xLbFormat->append_text( aTimeField.GetFormatted( *pNumberFormatter, eLangType ) );
        //SvxTimeFormat::HH12_MM_AMPM,  // 01:49 PM
        //SvxTimeFormat::HH12_MM_SS_AMPM, // 01:49:38 PM
        //SvxTimeFormat::HH12_MM_SS_00_AMPM // 01:49:38.78 PM

        m_xLbFormat->set_active( static_cast<sal_uInt16>(pTimeField->GetFormat()) - 2 );
    }
    else if( dynamic_cast< const SvxExtFileField *>( m_pField ) !=  nullptr )
    {
        const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>( m_pField );
        SvxExtFileField aFileField( *pFileField );

        m_xLbFormat->append_text( SdResId( STR_FILEFORMAT_NAME_EXT ) );
        m_xLbFormat->append_text( SdResId( STR_FILEFORMAT_FULLPATH ) );
        m_xLbFormat->append_text( SdResId( STR_FILEFORMAT_PATH ) );
        m_xLbFormat->append_text( SdResId( STR_FILEFORMAT_NAME ) );

        m_xLbFormat->set_active( static_cast<sal_uInt16>( pFileField->GetFormat() ) );
    }
    else if( dynamic_cast< const SvxAuthorField *>( m_pField ) !=  nullptr )
    {
        const SvxAuthorField* pAuthorField = static_cast<const SvxAuthorField*>( m_pField );
        SvxAuthorField aAuthorField( *pAuthorField );

        for( sal_uInt16 i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( static_cast<SvxAuthorFormat>(i) );
            m_xLbFormat->append_text( aAuthorField.GetFormatted() );
        }

        m_xLbFormat->set_active( static_cast<sal_uInt16>( pAuthorField->GetFormat() ) );

    }

}

void SdModifyFieldDlg::FillControls()
{
    m_xLbFormat->clear();

    if( dynamic_cast< const SvxDateField *>( m_pField ) !=  nullptr )
    {
        const SvxDateField* pDateField = static_cast<const SvxDateField*>(m_pField);
        SvxDateField aDateField( *pDateField );

        if( pDateField->GetType() == SvxDateType::Fix )
            m_xRbtFix->set_active(true);
        else
            m_xRbtVar->set_active(true);
    }
    else if( dynamic_cast< const SvxExtTimeField *>( m_pField ) !=  nullptr )
    {
        const SvxExtTimeField* pTimeField = static_cast<const SvxExtTimeField*>(m_pField);
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SvxTimeType::Fix )
            m_xRbtFix->set_active(true);
        else
            m_xRbtVar->set_active(true);
    }
    else if( dynamic_cast< const SvxExtFileField *>( m_pField ) !=  nullptr )
    {
        const SvxExtFileField* pFileField = static_cast<const SvxExtFileField*>(m_pField);
        SvxExtFileField aFileField( *pFileField );

        if( pFileField->GetType() == SvxFileType::Fix )
            m_xRbtFix->set_active(true);
        else
            m_xRbtVar->set_active(true);
    }
    else if( dynamic_cast< const SvxAuthorField *>( m_pField ) !=  nullptr )
    {
        const SvxAuthorField* pAuthorField = static_cast<const SvxAuthorField*>(m_pField);
        SvxAuthorField aAuthorField( *pAuthorField );

        if( pAuthorField->GetType() == SvxAuthorType::Fix )
            m_xRbtFix->set_active(true);
        else
            m_xRbtVar->set_active(true);
    }
    m_xRbtFix->save_state();
    m_xRbtVar->save_state();

    const SfxPoolItem* pItem;
    if( SfxItemState::SET == m_aInputSet.GetItemState(EE_CHAR_LANGUAGE, true, &pItem ) )
        m_xLbLanguage->set_active_id(static_cast<const SvxLanguageItem*>(pItem)->GetLanguage());

    m_xLbLanguage->save_active_id();

    FillFormatList();
    m_xLbFormat->save_value();
}

IMPL_LINK_NOARG(SdModifyFieldDlg, LanguageChangeHdl, weld::ComboBox&, void)
{
    FillFormatList();
}

SfxItemSet SdModifyFieldDlg::GetItemSet()
{
    SfxItemSet aOutput( *m_aInputSet.GetPool(), svl::Items<EE_CHAR_LANGUAGE, EE_CHAR_LANGUAGE_CTL>{} );

    if (m_xLbLanguage->get_active_id_changed_from_saved())
    {
        LanguageType eLangType = m_xLbLanguage->get_active_id();
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

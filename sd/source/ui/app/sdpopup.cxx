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

#include <editeng/flditem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/svapp.hxx>

#include <strings.hrc>
#include <sdpopup.hxx>
#include <sdresid.hxx>
#include <sdmod.hxx>
#include <DrawDocShell.hxx>

/*
 * Popup menu for editing of field command
 */
SdFieldPopup::SdFieldPopup(const SvxFieldData* pInField, LanguageType eLanguage)
    : m_xBuilder(Application::CreateBuilder(nullptr, "modules/simpress/ui/fieldmenu.ui"))
    , m_xPopup(m_xBuilder->weld_menu("menu"))
    , m_pField(pInField)
{
    Fill(eLanguage);
}

SdFieldPopup::~SdFieldPopup()
{
}

void SdFieldPopup::Fill( LanguageType eLanguage )
{
    sal_uInt16 nID = 1;
    m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_FIX));
    m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_VAR));
    m_xPopup->append_separator("separator1");

    if( auto pDateField = dynamic_cast< const SvxDateField *>( m_pField ) )
    {
        SvxDateField aDateField( *pDateField );

        if (pDateField->GetType() == SvxDateType::Fix)
            m_xPopup->set_active("1", true);
        else
            m_xPopup->set_active("2", true);

        //SvxDateFormat::AppDefault,     // is not used
        //SvxDateFormat::System,         // is not used
        m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_STANDARD_SMALL));
        m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_STANDARD_BIG));

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aDateField.SetFormat( SvxDateFormat::A );    // 13.02.96
        m_xPopup->append_radio(OUString::number(nID++), aDateField.GetFormatted(*pNumberFormatter, eLanguage));
        aDateField.SetFormat( SvxDateFormat::B );    // 13.02.1996
        m_xPopup->append_radio(OUString::number(nID++), aDateField.GetFormatted(*pNumberFormatter, eLanguage));
        aDateField.SetFormat( SvxDateFormat::C );    // 13.Feb 1996
        m_xPopup->append_radio(OUString::number(nID++), aDateField.GetFormatted(*pNumberFormatter, eLanguage));

        aDateField.SetFormat( SvxDateFormat::D );    // 13.Februar 1996
        m_xPopup->append_radio(OUString::number(nID++), aDateField.GetFormatted(*pNumberFormatter, eLanguage));
        aDateField.SetFormat( SvxDateFormat::E );    // Die, 13.Februar 1996
        m_xPopup->append_radio(OUString::number(nID++), aDateField.GetFormatted(*pNumberFormatter, eLanguage));
        aDateField.SetFormat( SvxDateFormat::F );    // Dienstag, 13.Februar 1996
        m_xPopup->append_radio(OUString::number(nID++), aDateField.GetFormatted(*pNumberFormatter, eLanguage));

        m_xPopup->set_active(OString::number(static_cast<sal_uInt16>( pDateField->GetFormat() ) + 1), true); // - 2 + 3 !
    }
    else if( auto pTimeField = dynamic_cast< const SvxExtTimeField *>( m_pField ) )
    {
        SvxExtTimeField aTimeField( *pTimeField );

        if( pTimeField->GetType() == SvxTimeType::Fix )
            m_xPopup->set_active("1", true);
        else
            m_xPopup->set_active("2", true);

        //SvxTimeFormat::AppDefault,     // is not used
        //SvxTimeFormat::System,         // is not used
        m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_STANDARD_NORMAL));

        SvNumberFormatter* pNumberFormatter = SD_MOD()->GetNumberFormatter();
        aTimeField.SetFormat( SvxTimeFormat::HH24_MM );    // 13:49
        m_xPopup->append_radio(OUString::number(nID++), aTimeField.GetFormatted(*pNumberFormatter, eLanguage));
        aTimeField.SetFormat( SvxTimeFormat::HH24_MM_SS );   // 13:49:38
        m_xPopup->append_radio(OUString::number(nID++), aTimeField.GetFormatted(*pNumberFormatter, eLanguage));
        aTimeField.SetFormat( SvxTimeFormat::HH24_MM_SS_00 );  // 13:49:38.78
        m_xPopup->append_radio(OUString::number(nID++), aTimeField.GetFormatted(*pNumberFormatter, eLanguage));

        aTimeField.SetFormat( SvxTimeFormat::HH12_MM );    // 01:49
        m_xPopup->append_radio(OUString::number(nID++), aTimeField.GetFormatted(*pNumberFormatter, eLanguage));
        aTimeField.SetFormat( SvxTimeFormat::HH12_MM_SS );   // 01:49:38
        m_xPopup->append_radio(OUString::number(nID++), aTimeField.GetFormatted(*pNumberFormatter, eLanguage));
        aTimeField.SetFormat( SvxTimeFormat::HH12_MM_SS_00 );  // 01:49:38.78
        m_xPopup->append_radio(OUString::number(nID++), aTimeField.GetFormatted(*pNumberFormatter, eLanguage));
        //SvxTimeFormat::HH12_MM_AMPM,  // 01:49 PM
        //SvxTimeFormat::HH12_MM_SS_AMPM, // 01:49:38 PM
        //SvxTimeFormat::HH12_MM_SS_00_AMPM // 01:49:38.78 PM

        m_xPopup->set_active(OString::number(static_cast<sal_uInt16>( pTimeField->GetFormat() ) + 1), true); // - 2 + 3 !
    }
    else if( auto pFileField = dynamic_cast< const SvxExtFileField *>( m_pField ) )
    {
        //SvxExtFileField aFileField( *pFileField );

        if( pFileField->GetType() == SvxFileType::Fix )
            m_xPopup->set_active("1", true);
        else
            m_xPopup->set_active("2", true);

        m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_FILEFORMAT_NAME_EXT));
        m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_FILEFORMAT_FULLPATH));
        m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_FILEFORMAT_PATH));
        m_xPopup->append_radio(OUString::number(nID++), SdResId(STR_FILEFORMAT_NAME));

        m_xPopup->set_active(OString::number(static_cast<sal_uInt16>( pFileField->GetFormat() ) + 3), true);
    }
    else if( auto pAuthorField = dynamic_cast< const SvxAuthorField *>( m_pField ) )
    {
        SvxAuthorField aAuthorField( *pAuthorField );

        if( pAuthorField->GetType() == SvxAuthorType::Fix )
            m_xPopup->set_active("1", true);
        else
            m_xPopup->set_active("2", true);

        for( sal_uInt16 i = 0; i < 4; i++ )
        {
            aAuthorField.SetFormat( static_cast<SvxAuthorFormat>(i) );
            m_xPopup->append_radio(OUString::number(nID++), aAuthorField.GetFormatted());
        }
        m_xPopup->set_active(OString::number(static_cast<sal_uInt16>( pAuthorField->GetFormat() ) + 3), true);
    }
}

void SdFieldPopup::Execute(weld::Window* pParent, const tools::Rectangle& rRect)
{
    OString sIdent = m_xPopup->popup_at_rect(pParent, rRect);
    if (sIdent.isEmpty())
        return;

    if (sIdent == "1" || sIdent == "2")
    {
        m_xPopup->set_active("1", sIdent == "1");
        m_xPopup->set_active("2", sIdent == "2");
    }
    else
    {
        int nCount = m_xPopup->n_children();
        for (int i = 3; i < nCount; i++)
            m_xPopup->set_active(OString::number(i), sIdent == OString::number(i));
    }
}

/**
 * Returns a new field, owned by caller.
 * Returns NULL if nothing changed.
 */
SvxFieldData* SdFieldPopup::GetField()
{
    SvxFieldData* pNewField = nullptr;

    sal_uInt16 nCount = m_xPopup->n_children();

    if( auto pDateField = dynamic_cast< const SvxDateField *>( m_pField ) )
    {
        SvxDateType   eType;
        SvxDateFormat eFormat;
        sal_uInt16 i;

        if (m_xPopup->get_active("1"))
            eType = SvxDateType::Fix;
        else
            eType = SvxDateType::Var;

        for( i = 3; i < nCount; i++ )
        {
            if (m_xPopup->get_active(OString::number(i)))
                break;
        }
        eFormat = static_cast<SvxDateFormat>( i - 1 );

        if( pDateField->GetFormat() != eFormat ||
            pDateField->GetType() != eType )
        {
            pNewField = new SvxDateField( *pDateField );
            static_cast<SvxDateField*>( pNewField )->SetType( eType );
            static_cast<SvxDateField*>( pNewField )->SetFormat( eFormat );

            if( (pDateField->GetType() == SvxDateType::Var) && (eType == SvxDateType::Fix) )
            {
                Date aDate( Date::SYSTEM );
                static_cast<SvxDateField*>( pNewField )->SetFixDate( aDate );
            }
        }
    }
    else if( auto pTimeField = dynamic_cast< const SvxExtTimeField *>( m_pField ) )
    {
        SvxTimeType   eType;
        SvxTimeFormat eFormat;
        sal_uInt16 i;

        if (m_xPopup->get_active("1"))
            eType = SvxTimeType::Fix;
        else
            eType = SvxTimeType::Var;

        for( i = 3; i < nCount; i++ )
        {
            if (m_xPopup->get_active(OString::number(i)))
                break;
        }
        eFormat = static_cast<SvxTimeFormat>( i - 1 );

        if( pTimeField->GetFormat() != eFormat ||
            pTimeField->GetType() != eType )
        {
            pNewField = new SvxExtTimeField( *pTimeField );
            static_cast<SvxExtTimeField*>( pNewField )->SetType( eType );
            static_cast<SvxExtTimeField*>( pNewField )->SetFormat( eFormat );

            if( (pTimeField->GetType() == SvxTimeType::Var) && (eType == SvxTimeType::Fix) )
            {
                tools::Time aTime( tools::Time::SYSTEM );
                static_cast<SvxExtTimeField*>( pNewField )->SetFixTime( aTime );
            }

        }
    }
    else if( auto pFileField = dynamic_cast< const SvxExtFileField *>( m_pField ) )
    {
        SvxFileType   eType;
        SvxFileFormat eFormat;
        sal_uInt16 i;

        if (m_xPopup->get_active("1"))
            eType = SvxFileType::Fix;
        else
            eType = SvxFileType::Var;

        for( i = 3; i < nCount; i++ )
        {
            if (m_xPopup->get_active(OString::number(i)))
                break;
        }
        eFormat = static_cast<SvxFileFormat>( i - 3 );

        if( pFileField->GetFormat() != eFormat ||
            pFileField->GetType() != eType )
        {
            ::sd::DrawDocShell* pDocSh = dynamic_cast<::sd::DrawDocShell* >( SfxObjectShell::Current() );

            if( pDocSh )
            {
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
    else if( auto pAuthorField = dynamic_cast< const SvxAuthorField *>( m_pField ) )
    {
        SvxAuthorType   eType;
        SvxAuthorFormat eFormat;
        sal_uInt16 i;

        if (m_xPopup->get_active("1"))
            eType = SvxAuthorType::Fix;
        else
            eType = SvxAuthorType::Var;

        for( i = 3; i < nCount; i++ )
        {
            if (m_xPopup->get_active(OString::number(i)))
                break;
        }
        eFormat = static_cast<SvxAuthorFormat>( i - 3 );

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

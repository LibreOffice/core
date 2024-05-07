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

#include <core_resource.hxx>
#include "TextConnectionHelper.hxx"
#include <strings.hrc>
#include <strings.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <dsitems.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/mnemonic.hxx>
#include <o3tl/string_view.hxx>

namespace
{

OUString lcl_getListEntry(std::u16string_view rStr, sal_Int32& rIdx)
{
    const OUString sTkn {o3tl::getToken(rStr, 0, '\t', rIdx )};
    if (rIdx>=0)
    {
        size_t nFnd = rStr.find('\t', rIdx);
        if (nFnd == std::u16string_view::npos)
            rIdx = -1;
        else
        {
            rIdx = nFnd + 1;
            if (rIdx >= static_cast<sal_Int32>(rStr.size()))
                rIdx = -1;
        }
    }
    return sTkn;
}

}

namespace dbaui
{

    OTextConnectionHelper::OTextConnectionHelper(weld::Widget* pParent, const short _nAvailableSections)
        : m_aFieldSeparatorList      (DBA_RES(STR_AUTOFIELDSEPARATORLIST))
        , m_aTextSeparatorList       (STR_AUTOTEXTSEPARATORLIST)
        , m_aTextNone                (DBA_RES(STR_AUTOTEXT_FIELD_SEP_NONE))
        , m_nAvailableSections( _nAvailableSections )
        , m_xBuilder(Application::CreateBuilder(pParent, u"dbaccess/ui/textpage.ui"_ustr))
        , m_xContainer(m_xBuilder->weld_widget(u"TextPage"_ustr))
        , m_xExtensionHeader(m_xBuilder->weld_widget(u"extensionframe"_ustr))
        , m_xAccessTextFiles(m_xBuilder->weld_radio_button(u"textfile"_ustr))
        , m_xAccessCSVFiles(m_xBuilder->weld_radio_button(u"csvfile"_ustr))
        , m_xAccessOtherFiles(m_xBuilder->weld_radio_button(u"custom"_ustr))
        , m_xOwnExtension(m_xBuilder->weld_entry(u"extension"_ustr))
        , m_xExtensionExample(m_xBuilder->weld_label(u"example"_ustr))
        , m_xFormatHeader(m_xBuilder->weld_widget(u"formatframe"_ustr))
        , m_xFieldSeparatorLabel(m_xBuilder->weld_label(u"fieldlabel"_ustr))
        , m_xFieldSeparator(m_xBuilder->weld_combo_box(u"fieldseparator"_ustr))
        , m_xTextSeparatorLabel(m_xBuilder->weld_label(u"textlabel"_ustr))
        , m_xTextSeparator(m_xBuilder->weld_combo_box(u"textseparator"_ustr))
        , m_xDecimalSeparatorLabel(m_xBuilder->weld_label(u"decimallabel"_ustr))
        , m_xDecimalSeparator(m_xBuilder->weld_combo_box(u"decimalseparator"_ustr))
        , m_xThousandsSeparatorLabel(m_xBuilder->weld_label(u"thousandslabel"_ustr))
        , m_xThousandsSeparator(m_xBuilder->weld_combo_box(u"thousandsseparator"_ustr))
        , m_xRowHeader(m_xBuilder->weld_check_button(u"containsheaders"_ustr))
        , m_xCharSetHeader(m_xBuilder->weld_widget(u"charsetframe"_ustr))
        , m_xCharSetLabel(m_xBuilder->weld_label(u"charsetlabel"_ustr))
        , m_xCharSet(new CharSetListBox(m_xBuilder->weld_combo_box(u"charset"_ustr)))
    {
        for(sal_Int32 nIdx {0}; nIdx>=0;)
            m_xFieldSeparator->append_text( lcl_getListEntry(m_aFieldSeparatorList, nIdx) );

        for(sal_Int32 nIdx {0}; nIdx>=0;)
            m_xTextSeparator->append_text( lcl_getListEntry(m_aTextSeparatorList, nIdx) );
        m_xTextSeparator->append_text(m_aTextNone);

        m_xOwnExtension->connect_changed(LINK(this, OTextConnectionHelper, OnEditModified));
        m_xAccessTextFiles->connect_toggled(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_xAccessCSVFiles->connect_toggled(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_xAccessOtherFiles->connect_toggled(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_xAccessCSVFiles->set_active(true);

        struct SectionDescriptor
        {
            short   nFlag;
            weld::Widget* pFrame;
        } const aSections[] = {
            { TC_EXTENSION,     m_xExtensionHeader.get() },
            { TC_SEPARATORS,    m_xFormatHeader.get() },
            { TC_HEADER,        m_xRowHeader.get() },
            { TC_CHARSET,       m_xCharSetHeader.get() }
        };

        for (auto const & section: aSections)
        {
            if ( ( m_nAvailableSections & section.nFlag ) != 0 )
            {
                // the section is visible, no need to do anything here
                continue;
            }

            // hide all elements from this section
            section.pFrame->hide();
        }

        m_xContainer->show();
    }

    IMPL_LINK_NOARG(OTextConnectionHelper, OnEditModified, weld::Entry&, void)
    {
        m_aGetExtensionHandler.Call(this);
    }

    IMPL_LINK_NOARG(OTextConnectionHelper, OnSetExtensionHdl, weld::Toggleable&, void)
    {
        bool bDoEnable = m_xAccessOtherFiles->get_active();
        m_xOwnExtension->set_sensitive(bDoEnable);
        m_xExtensionExample->set_sensitive(bDoEnable);
        m_aGetExtensionHandler.Call(this);
    }

    void OTextConnectionHelper::fillControls(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xFieldSeparator.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xTextSeparator.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xDecimalSeparator.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xThousandsSeparator.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::Toggleable>(m_xRowHeader.get()));
        _rControlList.emplace_back(new OSaveValueWidgetWrapper<weld::ComboBox>(m_xCharSet->get_widget()));
    }

    void OTextConnectionHelper::fillWindows(std::vector< std::unique_ptr<ISaveValueWrapper> >& _rControlList)
    {
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xFieldSeparatorLabel.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xTextSeparatorLabel.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xDecimalSeparatorLabel.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xThousandsSeparatorLabel.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Widget>(m_xCharSetHeader.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::Label>(m_xCharSetLabel.get()));
        _rControlList.emplace_back(new ODisableWidgetWrapper<weld::ComboBox>(m_xCharSet->get_widget()));
    }

    void OTextConnectionHelper::implInitControls(const SfxItemSet& _rSet, bool _bValid)
    {
        if ( !_bValid )
            return;

        const SfxStringItem* pDelItem = _rSet.GetItem<SfxStringItem>(DSID_FIELDDELIMITER);
        const SfxStringItem* pStrItem = _rSet.GetItem<SfxStringItem>(DSID_TEXTDELIMITER);
        const SfxStringItem* pDecdelItem = _rSet.GetItem<SfxStringItem>(DSID_DECIMALDELIMITER);
        const SfxStringItem* pThodelItem = _rSet.GetItem<SfxStringItem>(DSID_THOUSANDSDELIMITER);
        const SfxStringItem* pExtensionItem = _rSet.GetItem<SfxStringItem>(DSID_TEXTFILEEXTENSION);
        const SfxStringItem* pCharsetItem = _rSet.GetItem<SfxStringItem>(DSID_CHARSET);

        if ( ( m_nAvailableSections & TC_EXTENSION ) != 0 )
        {
            m_aOldExtension = pExtensionItem->GetValue();
            SetExtension( m_aOldExtension );
        }

        if ( ( m_nAvailableSections & TC_HEADER ) != 0 )
        {
            const SfxBoolItem* pHdrItem = _rSet.GetItem<SfxBoolItem>(DSID_TEXTFILEHEADER);
            m_xRowHeader->set_active(pHdrItem->GetValue());
        }

        if ( ( m_nAvailableSections & TC_SEPARATORS ) != 0 )
        {
            SetSeparator(*m_xFieldSeparator, m_aFieldSeparatorList, pDelItem->GetValue());
            SetSeparator(*m_xTextSeparator, m_aTextSeparatorList, pStrItem->GetValue());
            m_xDecimalSeparator->set_entry_text( pDecdelItem->GetValue() );
            m_xThousandsSeparator->set_entry_text( pThodelItem->GetValue() );
        }

        if ( ( m_nAvailableSections & TC_CHARSET ) != 0 )
        {
            m_xCharSet->SelectEntryByIanaName( pCharsetItem->GetValue() );
        }
    }

    bool OTextConnectionHelper::prepareLeave()
    {
        OUString sExtension = GetExtension();
        OUString aErrorText;
        weld::Widget* pErrorWin = nullptr;
        OUString aDelText(m_xFieldSeparator->get_active_text());
        if(aDelText.isEmpty())
        {   // No FieldSeparator
            aErrorText = DBA_RES(STR_AUTODELIMITER_MISSING);
            aErrorText = aErrorText.replaceFirst("#1",m_xFieldSeparatorLabel->get_label());
            pErrorWin = m_xFieldSeparator.get();
        }
        else if (m_xDecimalSeparator->get_active_text().isEmpty())
        {   // No DecimalSeparator
            aErrorText = DBA_RES(STR_AUTODELIMITER_MISSING);
            aErrorText = aErrorText.replaceFirst("#1",m_xDecimalSeparatorLabel->get_label());
            pErrorWin = m_xDecimalSeparator.get();
        }
        else if (m_xTextSeparator->get_active_text() == m_xFieldSeparator->get_active_text())
        {   // Field and TextSeparator must not be the same
            aErrorText = DBA_RES(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_xTextSeparatorLabel->get_label());
            aErrorText = aErrorText.replaceFirst("#2",m_xFieldSeparatorLabel->get_label());
            pErrorWin = m_xTextSeparator.get();
        }
        else if (m_xDecimalSeparator->get_active_text() == m_xThousandsSeparator->get_active_text())
        {   // Thousands and DecimalSeparator must not be the same
            aErrorText = DBA_RES(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_xDecimalSeparatorLabel->get_label());
            aErrorText = aErrorText.replaceFirst("#2",m_xThousandsSeparatorLabel->get_label());
            pErrorWin = m_xDecimalSeparator.get();
        }
        else if (m_xFieldSeparator->get_active_text() == m_xThousandsSeparator->get_active_text())
        {   // Thousands and FieldSeparator must not be the same
            aErrorText = DBA_RES(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_xFieldSeparatorLabel->get_label());
            aErrorText = aErrorText.replaceFirst("#2",m_xThousandsSeparatorLabel->get_label());
            pErrorWin = m_xFieldSeparator.get();
        }
        else if (m_xFieldSeparator->get_active_text() == m_xDecimalSeparator->get_active_text())
        {   // Tenner and FieldSeparator must not be the same
            aErrorText = DBA_RES(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_xFieldSeparatorLabel->get_label());
            aErrorText = aErrorText.replaceFirst("#2",m_xDecimalSeparatorLabel->get_label());
            pErrorWin = m_xFieldSeparator.get();
        }
        else if (m_xTextSeparator->get_active_text() == m_xThousandsSeparator->get_active_text())
        {   // Thousands and TextSeparator must not be the same
            aErrorText = DBA_RES(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_xTextSeparatorLabel->get_label());
            aErrorText = aErrorText.replaceFirst("#2",m_xThousandsSeparatorLabel->get_label());
            pErrorWin = m_xTextSeparator.get();
        }
        else if (m_xTextSeparator->get_active_text() == m_xDecimalSeparator->get_active_text())
        {   // Tenner and TextSeparator must not be the same
            aErrorText = DBA_RES(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_xTextSeparatorLabel->get_label());
            aErrorText = aErrorText.replaceFirst("#2",m_xDecimalSeparatorLabel->get_label());
            pErrorWin = m_xTextSeparator.get();
        }
        else if ((sExtension.indexOf('*') != -1) || (sExtension.indexOf('?') != -1))
        {
            aErrorText = DBA_RES(STR_AUTONO_WILDCARDS);
            aErrorText = aErrorText.replaceFirst("#1",sExtension);
            pErrorWin = m_xOwnExtension.get();
        }
        else
            return true;
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xContainer.get(),
                                                  VclMessageType::Warning, VclButtonsType::Ok,
                                                  MnemonicGenerator::EraseAllMnemonicChars(aErrorText)));
        xBox->run();
        pErrorWin->grab_focus();
        return false;
    }

    bool OTextConnectionHelper::FillItemSet( SfxItemSet& rSet, const bool _bChangedSomething )
    {
        bool bChangedSomething = _bChangedSomething;

        if ( ( m_nAvailableSections & TC_EXTENSION ) != 0 )
        {
            OUString sExtension = GetExtension();
            if( m_aOldExtension != sExtension )
            {
                rSet.Put( SfxStringItem( DSID_TEXTFILEEXTENSION, sExtension ) );
                bChangedSomething = true;
            }
        }

        if ( ( m_nAvailableSections & TC_HEADER ) != 0 )
        {
            if (m_xRowHeader->get_state_changed_from_saved())
            {
                rSet.Put(SfxBoolItem(DSID_TEXTFILEHEADER, m_xRowHeader->get_active()));
                bChangedSomething = true;
            }
        }

        if ( ( m_nAvailableSections & TC_SEPARATORS ) != 0 )
        {
            if (m_xFieldSeparator->get_value_changed_from_saved())
            {
                rSet.Put( SfxStringItem(DSID_FIELDDELIMITER, GetSeparator( *m_xFieldSeparator, m_aFieldSeparatorList) ) );
                bChangedSomething = true;
            }
            if (m_xTextSeparator->get_value_changed_from_saved())
            {
                rSet.Put( SfxStringItem(DSID_TEXTDELIMITER, GetSeparator( *m_xTextSeparator, m_aTextSeparatorList) ) );
                bChangedSomething = true;
            }

            if (m_xDecimalSeparator->get_value_changed_from_saved())
            {
                rSet.Put( SfxStringItem(DSID_DECIMALDELIMITER, m_xDecimalSeparator->get_active_text().copy(0, 1) ) );
                bChangedSomething = true;
            }
            if (m_xThousandsSeparator->get_value_changed_from_saved())
            {
                rSet.Put( SfxStringItem(DSID_THOUSANDSDELIMITER, m_xThousandsSeparator->get_active_text().copy(0,1) ) );
                bChangedSomething = true;
            }
        }

        if ( ( m_nAvailableSections & TC_CHARSET ) != 0 )
        {
            if ( m_xCharSet->StoreSelectedCharSet( rSet, DSID_CHARSET ) )
                bChangedSomething = true;
        }

        return bChangedSomething;
    }

    void OTextConnectionHelper::SetExtension(const OUString& _rVal)
    {
        if (_rVal == "txt")
            m_xAccessTextFiles->set_active(true);
        else if (_rVal == "csv")
            m_xAccessCSVFiles->set_active(true);
        else
        {
            m_xAccessOtherFiles->set_active(true);
            m_xExtensionExample->set_label(_rVal);
        }
    }

    OUString OTextConnectionHelper::GetExtension() const
    {
        OUString sExtension;
        if (m_xAccessTextFiles->get_active())
            sExtension = "txt";
        else if (m_xAccessCSVFiles->get_active())
            sExtension = "csv";
        else
        {
            sExtension = m_xOwnExtension->get_text();
            if ( sExtension.startsWith("*.") )
                sExtension = sExtension.copy(2);
        }
        return sExtension;
    }

    OUString OTextConnectionHelper::GetSeparator(const weld::ComboBox& rBox, std::u16string_view rList)
    {
        sal_Unicode const nTok = '\t';
        int nPos(rBox.find_text(rBox.get_active_text()));

        if (nPos == -1)
            return rBox.get_active_text();

        if ( m_xTextSeparator.get() != &rBox || nPos != (rBox.get_count()-1) )
            return OUString(
                static_cast< sal_Unicode >( o3tl::toInt32(o3tl::getToken(rList, (nPos*2)+1, nTok )) ));
        // somewhat strange ... translates for instance an "32" into " "
        return OUString();
    }

    void OTextConnectionHelper::SetSeparator( weld::ComboBox& rBox, std::u16string_view rList, const OUString& rVal )
    {
        if (rVal.getLength()==1)
        {
            const sal_Unicode nVal {rVal[0]};
            for(sal_Int32 nIdx {0}; nIdx>=0;)
            {
                sal_Int32 nPrevIdx {nIdx};
                if (static_cast<sal_Unicode>(o3tl::toInt32(o3tl::getToken(rList, 1, '\t', nIdx))) == nVal)
                {
                    rBox.set_entry_text(OUString(o3tl::getToken(rList,0, '\t', nPrevIdx)));
                    return;
                }
            }
            rBox.set_entry_text( rVal );
        }
        else if ( m_xTextSeparator.get() == &rBox && rVal.isEmpty() )
            rBox.set_entry_text(m_aTextNone);
        else
            rBox.set_entry_text(rVal.copy(0, 1));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

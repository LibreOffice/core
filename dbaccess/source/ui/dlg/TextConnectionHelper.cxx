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

#include <config_features.h>

#include "moduledbu.hxx"
#include "TextConnectionHelper.hxx"
#include "sqlmessage.hxx"
#include "dbu_dlg.hrc"
#include "dbu_resource.hrc"
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include "dsitems.hxx"
#include "dbfindex.hxx"
#include "dbaccess_helpid.hrc"
#include "localresaccess.hxx"
#include <vcl/layout.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/cjkoptions.hxx>
#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#endif
#include <connectivity/CommonTools.hxx>
#include "DriverSettings.hxx"
#include "dbadmin.hxx"
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <svl/filenotation.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include "finteraction.hxx"
#include "DBSetupConnectionPages.hxx"
#include <unotools/pathoptions.hxx>
#include <svtools/roadmapwizard.hxx>

namespace dbaui
{

    OTextConnectionHelper::OTextConnectionHelper( vcl::Window* pParent, const short _nAvailableSections )
        :TabPage(pParent, "TextPage", "dbaccess/ui/textpage.ui")
        ,m_aFieldSeparatorList      (ModuleRes(STR_AUTOFIELDSEPARATORLIST))
        ,m_aTextSeparatorList       (ModuleRes(STR_AUTOTEXTSEPARATORLIST))
        ,m_aTextNone                (ModuleRes(STR_AUTOTEXT_FIELD_SEP_NONE))
        ,m_nAvailableSections( _nAvailableSections )
    {
        get(m_pExtensionHeader, "extensionheader");
        get(m_pAccessTextFiles, "textfile");
        get(m_pAccessCSVFiles, "csvfile");
        get(m_pAccessOtherFiles, "custom");
        get(m_pOwnExtension, "extension");
        get(m_pExtensionExample, "example");
        get(m_pFormatHeader, "formatlabel");
        get(m_pFieldSeparatorLabel, "fieldlabel");
        get(m_pFieldSeparator, "fieldseparator");
        get(m_pTextSeparatorLabel, "textlabel");
        get(m_pTextSeparator, "textseparator");
        get(m_pDecimalSeparatorLabel, "decimallabel");
        get(m_pDecimalSeparator, "decimalseparator");
        get(m_pThousandsSeparatorLabel, "thousandslabel");
        get(m_pThousandsSeparator, "thousandsseparator");
        get(m_pRowHeader, "containsheaders");
        get(m_pCharSetHeader, "charsetheader");
        get(m_pCharSetLabel, "charsetlabel");
        get(m_pCharSet, "charset");

        sal_Int32 nCnt = comphelper::string::getTokenCount(m_aFieldSeparatorList, '\t');
        sal_Int32 i;

        for( i = 0 ; i < nCnt ; i += 2 )
            m_pFieldSeparator->InsertEntry( m_aFieldSeparatorList.getToken( i, '\t' ) );

        nCnt = comphelper::string::getTokenCount(m_aTextSeparatorList, '\t');
        for( i=0 ; i<nCnt ; i+=2 )
            m_pTextSeparator->InsertEntry( m_aTextSeparatorList.getToken( i, '\t' ) );
        m_pTextSeparator->InsertEntry(m_aTextNone);

        // set the modify handlers
        m_pFieldSeparator->SetUpdateDataHdl(getControlModifiedLink());
        m_pFieldSeparator->SetSelectHdl(getControlModifiedLink());
        m_pTextSeparator->SetUpdateDataHdl(getControlModifiedLink());
        m_pTextSeparator->SetSelectHdl(getControlModifiedLink());
        m_pCharSet->SetSelectHdl(getControlModifiedLink());

        m_pFieldSeparator->SetModifyHdl(getControlModifiedLink());
        m_pTextSeparator->SetModifyHdl(getControlModifiedLink());
        m_pDecimalSeparator->SetModifyHdl(getControlModifiedLink());
        m_pThousandsSeparator->SetModifyHdl(getControlModifiedLink());
        m_pOwnExtension->SetModifyHdl(LINK(this, OTextConnectionHelper, OnEditModified));
        m_pAccessTextFiles->SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_pAccessCSVFiles->SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_pAccessOtherFiles->SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_pAccessCSVFiles->Check(true);

        struct SectionDescriptor
        {
            short   nFlag;
            vcl::Window* pFirstControl;
        } aSections[] = {
            { TC_EXTENSION,     m_pExtensionHeader },
            { TC_SEPARATORS,    m_pFormatHeader },
            { TC_HEADER,        m_pRowHeader },
            { TC_CHARSET,       m_pCharSetHeader },
            { 0, NULL }
        };

        for ( size_t section=0; section < sizeof( aSections ) / sizeof( aSections[0] ) - 1; ++section )
        {
            if ( ( m_nAvailableSections & aSections[section].nFlag ) != 0 )
            {
                // the section is visible, no need to do anything here
                continue;
            }

            vcl::Window* pThisSection = aSections[section].pFirstControl;
            vcl::Window* pNextSection = aSections[section+1].pFirstControl;

            // hide all elements from this section
            vcl::Window* pControl = pThisSection;
            while ( ( pControl != pNextSection ) && pControl )
            {
                vcl::Window* pRealWindow = pControl->GetWindow( WINDOW_CLIENT );
            #if OSL_DEBUG_LEVEL > 0
                OUString sWindowText( pRealWindow->GetText() );
                (void)sWindowText;
            #endif
                pRealWindow->Hide();
                pControl = pControl->GetWindow( WINDOW_NEXT );
            }
        }

        Show();
    }

    OTextConnectionHelper::~OTextConnectionHelper()
    {

    }

    IMPL_LINK(OTextConnectionHelper, OnControlModified, Control*, /*EMPTYARG*/)
    {
        callModifiedHdl();
        return 0L;
    }

    IMPL_LINK(OTextConnectionHelper, OnEditModified, Edit*, /*_pEdit*/)
    {
        m_aGetExtensionHandler.Call(this);
        return 0L;
    }

    IMPL_LINK(OTextConnectionHelper, OnSetExtensionHdl, RadioButton*, /*_pRadioButton*/)
    {
        bool bDoEnable = m_pAccessOtherFiles->IsChecked();
        m_pOwnExtension->Enable(bDoEnable);
        m_pExtensionExample->Enable(bDoEnable);
        m_aGetExtensionHandler.Call(this);
        return 0L;
    }

    void OTextConnectionHelper::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(m_pFieldSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(m_pTextSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(m_pDecimalSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(m_pThousandsSeparator));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(m_pRowHeader));
        _rControlList.push_back(new OSaveValueWrapper<ListBox>(m_pCharSet));
    }

    void OTextConnectionHelper::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pFieldSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pTextSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pDecimalSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pThousandsSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pCharSetHeader));
        _rControlList.push_back(new ODisableWrapper<FixedText>(m_pCharSetLabel));
        _rControlList.push_back(new ODisableWrapper<ListBox>(m_pCharSet));
    }

    void OTextConnectionHelper::implInitControls(const SfxItemSet& _rSet, bool _bValid)
    {
        if ( !_bValid )
            return;

        SFX_ITEMSET_GET( _rSet, pDelItem, SfxStringItem, DSID_FIELDDELIMITER, true );
        SFX_ITEMSET_GET( _rSet, pStrItem, SfxStringItem, DSID_TEXTDELIMITER, true );
        SFX_ITEMSET_GET( _rSet, pDecdelItem, SfxStringItem, DSID_DECIMALDELIMITER, true );
        SFX_ITEMSET_GET( _rSet, pThodelItem, SfxStringItem, DSID_THOUSANDSDELIMITER, true );
        SFX_ITEMSET_GET( _rSet, pExtensionItem, SfxStringItem, DSID_TEXTFILEEXTENSION, true );
        SFX_ITEMSET_GET( _rSet, pCharsetItem, SfxStringItem, DSID_CHARSET, true );

        if ( ( m_nAvailableSections & TC_EXTENSION ) != 0 )
        {
            m_aOldExtension = pExtensionItem->GetValue();
            SetExtension( m_aOldExtension );
        }

        if ( ( m_nAvailableSections & TC_HEADER ) != 0 )
        {
            SFX_ITEMSET_GET( _rSet, pHdrItem, SfxBoolItem, DSID_TEXTFILEHEADER, true );
            m_pRowHeader->Check( pHdrItem->GetValue() );
        }

        if ( ( m_nAvailableSections & TC_SEPARATORS ) != 0 )
        {
            SetSeparator( *m_pFieldSeparator, m_aFieldSeparatorList, pDelItem->GetValue() );
            SetSeparator( *m_pTextSeparator, m_aTextSeparatorList, pStrItem->GetValue() );
            m_pDecimalSeparator->SetText( pDecdelItem->GetValue() );
            m_pThousandsSeparator->SetText( pThodelItem->GetValue() );
        }

        if ( ( m_nAvailableSections & TC_CHARSET ) != 0 )
        {
            m_pCharSet->SelectEntryByIanaName( pCharsetItem->GetValue() );
        }
    }

    bool OTextConnectionHelper::prepareLeave()
    {
        OUString sExtension = GetExtension();
        OUString aErrorText;
        Control* pErrorWin = NULL;
        OUString aDelText(m_pFieldSeparator->GetText());
        if(aDelText.isEmpty())
        {   // No FieldSeparator
            aErrorText = ModuleRes(STR_AUTODELIMITER_MISSING);
            aErrorText = aErrorText.replaceFirst("#1",m_pFieldSeparatorLabel->GetText());
            pErrorWin = m_pFieldSeparator;
        }
        else if (m_pDecimalSeparator->GetText().isEmpty())
        {   // No DecimalSeparator
            aErrorText = ModuleRes(STR_AUTODELIMITER_MISSING);
            aErrorText = aErrorText.replaceFirst("#1",m_pDecimalSeparatorLabel->GetText());
            pErrorWin = m_pDecimalSeparator;
        }
        else if (m_pTextSeparator->GetText() == m_pFieldSeparator->GetText())
        {   // Field and TextSeparator must not be the same
            aErrorText = ModuleRes(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_pTextSeparatorLabel->GetText());
            aErrorText = aErrorText.replaceFirst("#2",m_pFieldSeparatorLabel->GetText());
            pErrorWin = m_pTextSeparator;
        }
        else if (m_pDecimalSeparator->GetText() == m_pThousandsSeparator->GetText())
        {   // Thousands and DecimalSeparator must not be the same
            aErrorText = ModuleRes(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_pDecimalSeparatorLabel->GetText());
            aErrorText = aErrorText.replaceFirst("#2",m_pThousandsSeparatorLabel->GetText());
            pErrorWin = m_pDecimalSeparator;
        }
        else if (m_pFieldSeparator->GetText() == m_pThousandsSeparator->GetText())
        {   // Thousands and FieldSeparator must not be the same
            aErrorText = ModuleRes(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_pFieldSeparatorLabel->GetText());
            aErrorText = aErrorText.replaceFirst("#2",m_pThousandsSeparatorLabel->GetText());
            pErrorWin = m_pFieldSeparator;
        }
        else if (m_pFieldSeparator->GetText() == m_pDecimalSeparator->GetText())
        {   // Tenner and FieldSeparator must not be the same
            aErrorText = ModuleRes(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_pFieldSeparatorLabel->GetText());
            aErrorText = aErrorText.replaceFirst("#2",m_pDecimalSeparatorLabel->GetText());
            pErrorWin = m_pFieldSeparator;
        }
        else if (m_pTextSeparator->GetText() == m_pThousandsSeparator->GetText())
        {   // Thousands and TextSeparator must not be the same
            aErrorText = ModuleRes(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_pTextSeparatorLabel->GetText());
            aErrorText = aErrorText.replaceFirst("#2",m_pThousandsSeparatorLabel->GetText());
            pErrorWin = m_pTextSeparator;
        }
        else if (m_pTextSeparator->GetText() == m_pDecimalSeparator->GetText())
        {   // Tenner and TextSeparator must not be the same
            aErrorText = ModuleRes(STR_AUTODELIMITER_MUST_DIFFER);
            aErrorText = aErrorText.replaceFirst("#1",m_pTextSeparatorLabel->GetText());
            aErrorText = aErrorText.replaceFirst("#2",m_pDecimalSeparatorLabel->GetText());
            pErrorWin = m_pTextSeparator;
        }
        else if ((sExtension.indexOf('*') != -1) || (sExtension.indexOf('?') != -1))
        {
            aErrorText = ModuleRes(STR_AUTONO_WILDCARDS);
            aErrorText = aErrorText.replaceFirst("#1",sExtension);
            pErrorWin = m_pOwnExtension;
        }
        else
            return true;
        MessageDialog(NULL, MnemonicGenerator::EraseAllMnemonicChars(aErrorText)).Execute();
        pErrorWin->GrabFocus();
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
            if( m_pRowHeader->IsValueChangedFromSaved() )
            {
                rSet.Put(SfxBoolItem(DSID_TEXTFILEHEADER, m_pRowHeader->IsChecked()));
                bChangedSomething = true;
            }
        }

        if ( ( m_nAvailableSections & TC_SEPARATORS ) != 0 )
        {
            if( m_pFieldSeparator->IsValueChangedFromSaved() )
            {
                rSet.Put( SfxStringItem(DSID_FIELDDELIMITER, GetSeparator( *m_pFieldSeparator, m_aFieldSeparatorList) ) );
                bChangedSomething = true;
            }
            if( m_pTextSeparator->IsValueChangedFromSaved() )
            {
                rSet.Put( SfxStringItem(DSID_TEXTDELIMITER, GetSeparator( *m_pTextSeparator, m_aTextSeparatorList) ) );
                bChangedSomething = true;
            }

            if( m_pDecimalSeparator->IsValueChangedFromSaved() )
            {
                rSet.Put( SfxStringItem(DSID_DECIMALDELIMITER, m_pDecimalSeparator->GetText().copy(0, 1) ) );
                bChangedSomething = true;
            }
            if( m_pThousandsSeparator->IsValueChangedFromSaved() )
            {
                rSet.Put( SfxStringItem(DSID_THOUSANDSDELIMITER, m_pThousandsSeparator->GetText().copy(0,1) ) );
                bChangedSomething = true;
            }
        }

        if ( ( m_nAvailableSections & TC_CHARSET ) != 0 )
        {
            if ( m_pCharSet->StoreSelectedCharSet( rSet, DSID_CHARSET ) )
                bChangedSomething = true;
        }

        return bChangedSomething;
    }

    void OTextConnectionHelper::SetExtension(const OUString& _rVal)
    {
        if (_rVal == "txt")
            m_pAccessTextFiles->Check(true);
        else if (_rVal == "csv")
            m_pAccessCSVFiles->Check(true);
        else
        {
            m_pAccessOtherFiles->Check(true);
            m_pExtensionExample->SetText(_rVal);
        }
    }

    OUString OTextConnectionHelper::GetExtension()
    {
        OUString sExtension;
        if (m_pAccessTextFiles->IsChecked())
            sExtension = "txt";
        else if (m_pAccessCSVFiles->IsChecked())
            sExtension = "csv";
        else
        {
            sExtension = m_pOwnExtension->GetText();
            if ( comphelper::string::equals(sExtension.getToken(0,'.'), '*') )
                sExtension = sExtension.copy(2);
        }
        return sExtension;
    }

    OUString OTextConnectionHelper::GetSeparator( const ComboBox& rBox, const OUString& rList )
    {
        sal_Unicode nTok = '\t';
        sal_Int32 nPos(rBox.GetEntryPos( rBox.GetText() ));

        if( nPos == COMBOBOX_ENTRY_NOTFOUND )
            return rBox.GetText().copy(0);

        if ( !( m_pTextSeparator == &rBox && nPos == (rBox.GetEntryCount()-1) ) )
            return OUString(
                static_cast< sal_Unicode >( rList.getToken((nPos*2)+1, nTok ).toInt32() ));
        // somewhat strange ... translates for instance an "32" into " "
        return OUString();
    }

    void OTextConnectionHelper::SetSeparator( ComboBox& rBox, const OUString& rList, const OUString& rVal )
    {
        char    nTok = '\t';
        sal_Int32   nCnt = comphelper::string::getTokenCount(rList, nTok);
        sal_Int32 i;

        for( i=0 ; i<nCnt ; i+=2 )
        {
            OUString  sTVal(
                static_cast< sal_Unicode >( rList.getToken( (i+1), nTok ).toInt32() ));

            if( sTVal.equals(rVal) )
            {
                rBox.SetText( rList.getToken( i, nTok ) );
                break;
            }
        }

        if ( i >= nCnt )
        {
            if ( m_pTextSeparator == &rBox && rVal.isEmpty() )
                rBox.SetText(m_aTextNone);
            else
                rBox.SetText( rVal.copy(0, 1) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

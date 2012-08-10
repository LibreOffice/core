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

#include "moduledbu.hxx"
#include "TextConnectionHelper.hxx"
#include "sqlmessage.hxx"
#include "dbu_dlg.hrc"
#include "dbu_resource.hrc"
#include "AutoControls.hrc"
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include "dsitems.hxx"
#include "dbfindex.hxx"
#include "dbaccess_helpid.hrc"
#include "localresaccess.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/mnemonic.hxx>
#include <svl/cjkoptions.hxx>
#include <jvmaccess/virtualmachine.hxx>
#include <connectivity/CommonTools.hxx>
#include "DriverSettings.hxx"
#include "dbadmin.hxx"
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
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
DBG_NAME(OTextConnectionHelper)

    OTextConnectionHelper::OTextConnectionHelper( Window* pParent, const short _nAvailableSections )
        :Control( pParent, WB_DIALOGCONTROL )
        ,m_aFTExtensionHeader       (this, ModuleRes(FT_AUTOEXTENSIONHEADER))
        ,m_aRBAccessTextFiles       (this, ModuleRes(RB_AUTOACCESSCTEXTFILES))
        ,m_aRBAccessCSVFiles        (this, ModuleRes(RB_AUTOACCESSCCSVFILES))
        ,m_aRBAccessOtherFiles      (this, ModuleRes(RB_AUTOACCESSOTHERS))
        ,m_aETOwnExtension          (this, ModuleRes(ET_AUTOOWNEXTENSION))
        ,m_aFTExtensionExample      (this, ModuleRes(FT_AUTOOWNEXTENSIONAPPENDIX))
        ,m_aLineFormat              (this, ModuleRes(FL_AUTOSEPARATOR2))
        ,m_aFieldSeparatorLabel     (this, ModuleRes(FT_AUTOFIELDSEPARATOR))
        ,m_aFieldSeparator          (this, ModuleRes(CM_AUTOFIELDSEPARATOR))
        ,m_aTextSeparatorLabel      (this, ModuleRes(FT_AUTOTEXTSEPARATOR))
        ,m_aTextSeparator           (this, ModuleRes(CM_AUTOTEXTSEPARATOR))
        ,m_aDecimalSeparatorLabel   (this, ModuleRes(FT_AUTODECIMALSEPARATOR))
        ,m_aDecimalSeparator        (this, ModuleRes(CM_AUTODECIMALSEPARATOR))
        ,m_aThousandsSeparatorLabel (this, ModuleRes(FT_AUTOTHOUSANDSSEPARATOR))
        ,m_aThousandsSeparator      (this, ModuleRes(CM_AUTOTHOUSANDSSEPARATOR))
        ,m_aRowHeader               (this, ModuleRes(CB_AUTOHEADER))
        ,m_aCharSetHeader           (this, ModuleRes(FL_DATACONVERT))
        ,m_aCharSetLabel            (this, ModuleRes(FT_CHARSET))
        ,m_aCharSet                 (this, ModuleRes(LB_CHARSET))
        ,m_aFieldSeparatorList      (ModuleRes(STR_AUTOFIELDSEPARATORLIST))
        ,m_aTextSeparatorList       (ModuleRes(STR_AUTOTEXTSEPARATORLIST))
        ,m_aTextNone                (ModuleRes(STR_AUTOTEXT_FIELD_SEP_NONE))
        ,m_nAvailableSections( _nAvailableSections )
    {
        DBG_CTOR(OTextConnectionHelper,NULL);

        xub_StrLen nCnt = comphelper::string::getTokenCount(m_aFieldSeparatorList, '\t');
        xub_StrLen i;

        for( i = 0 ; i < nCnt ; i += 2 )
            m_aFieldSeparator.InsertEntry( m_aFieldSeparatorList.GetToken( i, '\t' ) );

        nCnt = comphelper::string::getTokenCount(m_aTextSeparatorList, '\t');
        for( i=0 ; i<nCnt ; i+=2 )
            m_aTextSeparator.InsertEntry( m_aTextSeparatorList.GetToken( i, '\t' ) );
        m_aTextSeparator.InsertEntry(m_aTextNone);

        // set the modify handlers
        m_aFieldSeparator.SetUpdateDataHdl(getControlModifiedLink());
        m_aFieldSeparator.SetSelectHdl(getControlModifiedLink());
        m_aTextSeparator.SetUpdateDataHdl(getControlModifiedLink());
        m_aTextSeparator.SetSelectHdl(getControlModifiedLink());
        m_aCharSet.SetSelectHdl(getControlModifiedLink());

        m_aFieldSeparator.SetModifyHdl(getControlModifiedLink());
        m_aTextSeparator.SetModifyHdl(getControlModifiedLink());
        m_aDecimalSeparator.SetModifyHdl(getControlModifiedLink());
        m_aThousandsSeparator.SetModifyHdl(getControlModifiedLink());
        m_aETOwnExtension.SetModifyHdl(LINK(this, OTextConnectionHelper, OnEditModified));
           m_aRBAccessTextFiles.SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
           m_aRBAccessCSVFiles.SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_aRBAccessOtherFiles.SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_aRBAccessCSVFiles.Check(sal_True);

        struct SectionDescriptor
        {
            short   nFlag;
            Window* pFirstControl;
        } aSections[] = {
            { TC_EXTENSION,     &m_aFTExtensionHeader },
            { TC_SEPARATORS,    &m_aLineFormat },
            { TC_HEADER,        &m_aRowHeader },
            { TC_CHARSET,       &m_aCharSetHeader },
            { 0, NULL }
        };

        for ( size_t section=0; section < sizeof( aSections ) / sizeof( aSections[0] ) - 1; ++section )
        {
            if ( ( m_nAvailableSections & aSections[section].nFlag ) != 0 )
            {
                // the section is visible, no need to do anything here
                continue;
            }

            Window* pThisSection = aSections[section].pFirstControl;
            Window* pNextSection = aSections[section+1].pFirstControl;

            // hide all elements from this section
            Window* pControl = pThisSection;
            while ( ( pControl != pNextSection ) && pControl )
            {
                Window* pRealWindow = pControl->GetWindow( WINDOW_CLIENT );
            #if OSL_DEBUG_LEVEL > 0
                String sWindowText( pRealWindow->GetText() );
                (void)sWindowText;
            #endif
                pRealWindow->Hide();
                pControl = pControl->GetWindow( WINDOW_NEXT );
            }

            // move all controls in following sections up
            if ( !pNextSection )
                continue;
            const long nThisSectionStart = pThisSection->GetPosPixel().Y();
            const long nNextSectionStart = pNextSection->GetPosPixel().Y();
            const long nMoveOffset( nThisSectionStart - nNextSectionStart );
            while ( pControl )
            {
                Point aPos = pControl->GetPosPixel();
                aPos.Move( 0, nMoveOffset );
                pControl->SetPosPixel( aPos );
                pControl = pControl->GetWindow( WINDOW_NEXT );
            }
        }

        Rectangle aControlRectUnion;
        for (   Window* pControl = aSections[0].pFirstControl;
                pControl != NULL;
                pControl = pControl->GetWindow( WINDOW_NEXT )
            )
        {
            aControlRectUnion = aControlRectUnion.Union( Rectangle( pControl->GetPosPixel(), pControl->GetSizePixel() ) );
        }

        // need some adjustments to the positions, since the resource-specified
        // positions for the control were relative to *our* parent, while by now
        // the controls have |this| as parent.

        // first, move ourself to the upper left of the area occupied by all controls
        SetPosPixel( aControlRectUnion.TopLeft() );

        // then, compensate in the control positions, by moving them the up/left
        for (   Window* pControl = aSections[0].pFirstControl;
                pControl != NULL;
                pControl = pControl->GetWindow( WINDOW_NEXT )
            )
        {
            Point aPos( pControl->GetPosPixel() );
            aPos.Move( -aControlRectUnion.Left(), -aControlRectUnion.Top() );
            pControl->SetPosPixel( aPos );

            // while we are here ... the controls should not have an own background
            // (this would not be needed when our outer dialog were also the parent
            // of the controls)
            pControl->SetBackground();
        }

        // now, change our own size so all controls fit
        SetSizePixel( aControlRectUnion.GetSize() );

        SetBackground();
        Show();
    }

    OTextConnectionHelper::~OTextConnectionHelper()
    {

        DBG_DTOR(OTextConnectionHelper,NULL);
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
        sal_Bool bDoEnable = m_aRBAccessOtherFiles.IsChecked();
        m_aETOwnExtension.Enable(bDoEnable);
        m_aFTExtensionExample.Enable(bDoEnable);
        m_aGetExtensionHandler.Call(this);
        return 0L;
    }

    void OTextConnectionHelper::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aFieldSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aTextSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aDecimalSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aThousandsSeparator));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aRowHeader));
        _rControlList.push_back(new OSaveValueWrapper<ListBox>(&m_aCharSet));
    }

    void OTextConnectionHelper::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFieldSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aTextSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aDecimalSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aThousandsSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedLine>(&m_aCharSetHeader));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aCharSetLabel));
        _rControlList.push_back(new ODisableWrapper<ListBox>(&m_aCharSet));
    }

    void OTextConnectionHelper::implInitControls(const SfxItemSet& _rSet, sal_Bool _bValid)
    {
        if ( !_bValid )
            return;

        SFX_ITEMSET_GET( _rSet, pDelItem, SfxStringItem, DSID_FIELDDELIMITER, sal_True );
        SFX_ITEMSET_GET( _rSet, pStrItem, SfxStringItem, DSID_TEXTDELIMITER, sal_True );
        SFX_ITEMSET_GET( _rSet, pDecdelItem, SfxStringItem, DSID_DECIMALDELIMITER, sal_True );
        SFX_ITEMSET_GET( _rSet, pThodelItem, SfxStringItem, DSID_THOUSANDSDELIMITER, sal_True );
        SFX_ITEMSET_GET( _rSet, pExtensionItem, SfxStringItem, DSID_TEXTFILEEXTENSION, sal_True );
        SFX_ITEMSET_GET( _rSet, pCharsetItem, SfxStringItem, DSID_CHARSET, sal_True );

        if ( ( m_nAvailableSections & TC_EXTENSION ) != 0 )
        {
            m_aOldExtension = pExtensionItem->GetValue();
            SetExtension( m_aOldExtension );
        }

        if ( ( m_nAvailableSections & TC_HEADER ) != 0 )
        {
            SFX_ITEMSET_GET( _rSet, pHdrItem, SfxBoolItem, DSID_TEXTFILEHEADER, sal_True );
            m_aRowHeader.Check( pHdrItem->GetValue() );
        }

        if ( ( m_nAvailableSections & TC_SEPARATORS ) != 0 )
        {
            SetSeparator( m_aFieldSeparator, m_aFieldSeparatorList, pDelItem->GetValue() );
            SetSeparator( m_aTextSeparator, m_aTextSeparatorList, pStrItem->GetValue() );
            m_aDecimalSeparator.SetText( pDecdelItem->GetValue() );
            m_aThousandsSeparator.SetText( pThodelItem->GetValue() );
        }

        if ( ( m_nAvailableSections & TC_CHARSET ) != 0 )
        {
            m_aCharSet.SelectEntryByIanaName( pCharsetItem->GetValue() );
        }
    }

    sal_Bool OTextConnectionHelper::prepareLeave()
    {
        LocalResourceAccess aStringResAccess(PAGE_TEXT, RSC_TABPAGE);
            // for accessing the strings which are local to our own resource block
        String sExtension = GetExtension();
        String aErrorText;
        Control* pErrorWin = NULL;
        String aDelText(m_aFieldSeparator.GetText());
        if(!aDelText.Len())
        {   // No FieldSeparator
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MISSING));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (!m_aDecimalSeparator.GetText().Len())
        {   // No DecimalSeparator
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MISSING));
            aErrorText.SearchAndReplaceAscii("#1",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aDecimalSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aFieldSeparator.GetText())
        {   // Field and TextSeparator must not be the same
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aTextSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aFieldSeparatorLabel.GetText());
            pErrorWin = &m_aTextSeparator;
        }
        else if (m_aDecimalSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Thousands and DecimalSeparator must not be the same
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aDecimalSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aDecimalSeparator;
        }
        else if (m_aFieldSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Thousands and FieldSeparator must not be the same
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (m_aFieldSeparator.GetText() == m_aDecimalSeparator.GetText())
        {   // Tenner and FieldSeparator must not be the same
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Thousands and TextSeparator must not be the same
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aTextSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aTextSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aDecimalSeparator.GetText())
        {   // Zehner und TextTrenner duerfen nicht gleich sein
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aTextSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aTextSeparator;
        }
        else if ((sExtension.Search('*') != STRING_NOTFOUND) || (sExtension.Search('?') != STRING_NOTFOUND))
        {
            aErrorText = String(ModuleRes(STR_AUTONO_WILDCARDS));
            aErrorText.SearchAndReplaceAscii("#1",sExtension);
            pErrorWin = &m_aETOwnExtension;
        }
        else
            return sal_True;
        ErrorBox(NULL, WB_OK, MnemonicGenerator::EraseAllMnemonicChars( aErrorText)).Execute();
        pErrorWin->GrabFocus();
        return 0;
    }

    sal_Bool OTextConnectionHelper::FillItemSet( SfxItemSet& rSet, const sal_Bool _bChangedSomething )
    {
        sal_Bool bChangedSomething = _bChangedSomething;

        if ( ( m_nAvailableSections & TC_EXTENSION ) != 0 )
        {
            String sExtension = GetExtension();
            if( !m_aOldExtension.Equals( sExtension ) )
            {
                rSet.Put( SfxStringItem( DSID_TEXTFILEEXTENSION, sExtension ) );
                bChangedSomething = sal_True;
            }
        }

        if ( ( m_nAvailableSections & TC_HEADER ) != 0 )
        {
            if( (m_aRowHeader.GetState() != m_aRowHeader.GetSavedValue()) )
            {
                rSet.Put(SfxBoolItem(DSID_TEXTFILEHEADER, m_aRowHeader.IsChecked()));
                bChangedSomething = sal_True;
            }
        }

        if ( ( m_nAvailableSections & TC_SEPARATORS ) != 0 )
        {
            if( m_aFieldSeparator.GetText() != m_aFieldSeparator.GetSavedValue() )
            {
                rSet.Put( SfxStringItem(DSID_FIELDDELIMITER, GetSeparator( m_aFieldSeparator, m_aFieldSeparatorList) ) );
                bChangedSomething = sal_True;
            }
            if( m_aTextSeparator.GetText() != m_aTextSeparator.GetSavedValue() )
            {
                rSet.Put( SfxStringItem(DSID_TEXTDELIMITER, GetSeparator( m_aTextSeparator, m_aTextSeparatorList) ) );
                bChangedSomething = sal_True;
            }

            if( m_aDecimalSeparator.GetText() != m_aDecimalSeparator.GetSavedValue() )
            {
                rSet.Put( SfxStringItem(DSID_DECIMALDELIMITER, m_aDecimalSeparator.GetText().Copy(0, 1) ) );
                bChangedSomething = sal_True;
            }
            if( m_aThousandsSeparator.GetText() != m_aThousandsSeparator.GetSavedValue() )
            {
                rSet.Put( SfxStringItem(DSID_THOUSANDSDELIMITER, m_aThousandsSeparator.GetText().Copy(0,1) ) );
                bChangedSomething = sal_True;
            }
        }

        if ( ( m_nAvailableSections & TC_CHARSET ) != 0 )
        {
            if ( m_aCharSet.StoreSelectedCharSet( rSet, DSID_CHARSET ) )
                bChangedSomething = sal_True;
        }

        return bChangedSomething;
    }

    void OTextConnectionHelper::SetExtension(const String& _rVal)
    {
        if (_rVal.EqualsAscii("txt"))
            m_aRBAccessTextFiles.Check(sal_True);
        else if (_rVal.EqualsAscii( "csv" ))
            m_aRBAccessCSVFiles.Check(sal_True);
        else
        {
            m_aRBAccessOtherFiles.Check(sal_True);
            m_aFTExtensionExample.SetText(_rVal);
        }
    }

    String OTextConnectionHelper::GetExtension()
    {
        String sExtension;
        if (m_aRBAccessTextFiles.IsChecked())
            sExtension = rtl::OUString("txt");
        else if (m_aRBAccessCSVFiles.IsChecked())
            sExtension = rtl::OUString("csv");
        else
        {
            sExtension = m_aETOwnExtension.GetText();
            if ( comphelper::string::equals(sExtension.GetToken(0,'.'), '*') )
                sExtension.Erase(0,2);
        }
        return sExtension;
    }

    String OTextConnectionHelper::GetSeparator( const ComboBox& rBox, const String& rList )
    {
        sal_Unicode nTok = '\t';
        xub_StrLen  nPos(rBox.GetEntryPos( rBox.GetText() ));

        if( nPos == COMBOBOX_ENTRY_NOTFOUND )
            return rBox.GetText().Copy(0);

        if ( !( &m_aTextSeparator == &rBox && nPos == (rBox.GetEntryCount()-1) ) )
            return rtl::OUString(
                static_cast< sal_Unicode >(
                    rList.GetToken(((nPos*2)+1), nTok ).ToInt32()));
        // somewhat strange ... translates for instance an "32" into " "
        return rtl::OUString();
    }

    void OTextConnectionHelper::SetSeparator( ComboBox& rBox, const String& rList, const String& rVal )
    {
        char    nTok = '\t';
        xub_StrLen  nCnt(comphelper::string::getTokenCount(rList, nTok));
        xub_StrLen  i;

        for( i=0 ; i<nCnt ; i+=2 )
        {
            rtl::OUString  sTVal(
                static_cast< sal_Unicode >(
                    rList.GetToken( (i+1), nTok ).ToInt32()));

            if( sTVal.equals(rVal) )
            {
                rBox.SetText( rList.GetToken( i, nTok ) );
                break;
            }
        }

        if ( i >= nCnt )
        {
            if ( &m_aTextSeparator == &rBox && !rVal.Len() )
                rBox.SetText(m_aTextNone);
            else
                rBox.SetText( rVal.Copy(0, 1) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

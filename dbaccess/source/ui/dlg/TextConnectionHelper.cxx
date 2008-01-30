/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextConnectionHelper.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:44:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

#ifndef DBAUI_TEXTCONNECTIONHELPER_HXX
#include "TextConnectionHelper.hxx"
#endif

#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_AUTOCONTROLS_HRC_
#include "AutoControls.hrc"
#endif

//#ifndef _DBAUI_DBADMIN_HRC_
//#include "dbadmin.hrc"
//#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif
#ifndef _DBAUI_DBFINDEX_HXX_
#include "dbfindex.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#include <jvmaccess/virtualmachine.hxx>
#ifndef _DBAUI_ADASTAT_HXX_
#include "AdabasStat.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef DBAUI_DRIVERSETTINGS_HXX
#include "DriverSettings.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
// #106016# ------------------------------------
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif

#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#include <ucbhelper/commandenvironment.hxx>
#endif
#ifndef DBAUI_FILEPICKER_INTERACTION_HXX
#include "finteraction.hxx"
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif

#ifndef DBAUI_DBSETUPCONNECTIONPAGES_HXX
#include "DBSetupConnectionPages.hxx"
#endif

#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
#endif

#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif

#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#include <svtools/roadmapwizard.hxx>
#endif


namespace dbaui
{

    //========================================================================
    //= OTextConnectionPageSetup
    //========================================================================
DBG_NAME(OTextConnectionHelper)
//------------------------------------------------------------------------
    OTextConnectionHelper::OTextConnectionHelper( Window* pParent, sal_Bool _bWizardMode )
        :Control(pParent)
           ,m_aHeader                   (pParent, ModuleRes(CB_AUTOHEADER))
        ,m_aLineFormat              (pParent, ModuleRes(FL_AUTOSEPARATOR2))
        ,m_aFTExtensionHeader       (pParent, ModuleRes(FT_AUTOEXTENSIONHEADER))
        ,m_aRBAccessTextFiles       (pParent, ModuleRes(RB_AUTOACCESSCTEXTFILES))
        ,m_aRBAccessCSVFiles        (pParent, ModuleRes(RB_AUTOACCESSCCSVFILES))
        ,m_aRBAccessOtherFiles      (pParent, ModuleRes(RB_AUTOACCESSOTHERS))
        ,m_aETOwnExtension          (pParent, ModuleRes(ET_AUTOOWNEXTENSION))
        ,m_aFTExtensionExample      (pParent, ModuleRes(FT_AUTOOWNEXTENSIONAPPENDIX))
        ,m_aFieldSeparatorLabel     (pParent, ModuleRes(FT_AUTOFIELDSEPARATOR))
        ,m_aFieldSeparator          (pParent, ModuleRes(CM_AUTOFIELDSEPARATOR))
        ,m_aTextSeparatorLabel      (pParent, ModuleRes(FT_AUTOTEXTSEPARATOR))
        ,m_aTextSeparator           (pParent, ModuleRes(CM_AUTOTEXTSEPARATOR))
        ,m_aDecimalSeparatorLabel   (pParent, ModuleRes(FT_AUTODECIMALSEPARATOR))
        ,m_aDecimalSeparator        (pParent, ModuleRes(CM_AUTODECIMALSEPARATOR))
        ,m_aThousandsSeparatorLabel (pParent, ModuleRes(FT_AUTOTHOUSANDSSEPARATOR))
        ,m_aThousandsSeparator      (pParent, ModuleRes(CM_AUTOTHOUSANDSSEPARATOR))
        ,m_aFieldSeparatorList      (ModuleRes(STR_AUTOFIELDSEPARATORLIST))
        ,m_aTextSeparatorList       (ModuleRes(STR_AUTOTEXTSEPARATORLIST))
        ,m_aTextNone                (ModuleRes(STR_AUTOTEXT_FIELD_SEP_NONE))
    {
        DBG_CTOR(OTextConnectionHelper,NULL);

        m_bWizardMode = _bWizardMode;
        xub_StrLen nCnt = m_aFieldSeparatorList.GetTokenCount( '\t' );
        xub_StrLen i;

        for( i = 0 ; i < nCnt ; i += 2 )
            m_aFieldSeparator.InsertEntry( m_aFieldSeparatorList.GetToken( i, '\t' ) );

        nCnt = m_aTextSeparatorList.GetTokenCount( '\t' );
        for( i=0 ; i<nCnt ; i+=2 )
            m_aTextSeparator.InsertEntry( m_aTextSeparatorList.GetToken( i, '\t' ) );
        m_aTextSeparator.InsertEntry(m_aTextNone);

        // set the modify handlers
        m_aFieldSeparator.SetUpdateDataHdl(getControlModifiedLink());
        m_aFieldSeparator.SetSelectHdl(getControlModifiedLink());
        m_aTextSeparator.SetUpdateDataHdl(getControlModifiedLink());
        m_aTextSeparator.SetSelectHdl(getControlModifiedLink());

        m_aFieldSeparator.SetModifyHdl(getControlModifiedLink());
        m_aTextSeparator.SetModifyHdl(getControlModifiedLink());
        m_aDecimalSeparator.SetModifyHdl(getControlModifiedLink());
        m_aThousandsSeparator.SetModifyHdl(getControlModifiedLink());
        m_aETOwnExtension.SetModifyHdl(LINK(this, OTextConnectionHelper, OnEditModified));
           m_aRBAccessTextFiles.SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
           m_aRBAccessCSVFiles.SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_aRBAccessOtherFiles.SetToggleHdl(LINK(this, OTextConnectionHelper, OnSetExtensionHdl));
        m_aRBAccessCSVFiles.Check(sal_True);
        Show();
    }

    // -----------------------------------------------------------------------
    OTextConnectionHelper::~OTextConnectionHelper()
    {

        DBG_DTOR(OTextConnectionHelper,NULL);
    }


    // -----------------------------------------------------------------------
    IMPL_LINK(OTextConnectionHelper, OnControlModified, Control*, /*EMPTYARG*/)
    {
        callModifiedHdl();
        return 0L;
    }

    // -----------------------------------------------------------------------
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


    // -----------------------------------------------------------------------
    void OTextConnectionHelper::fillControls(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aFieldSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aTextSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aDecimalSeparator));
        _rControlList.push_back(new OSaveValueWrapper<ComboBox>(&m_aThousandsSeparator));
        _rControlList.push_back(new OSaveValueWrapper<CheckBox>(&m_aHeader));
    }
    // -----------------------------------------------------------------------
    void OTextConnectionHelper::fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList)
    {
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aFieldSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aTextSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aDecimalSeparatorLabel));
        _rControlList.push_back(new ODisableWrapper<FixedText>(&m_aThousandsSeparatorLabel));
    }

    // -----------------------------------------------------------------------
    void OTextConnectionHelper::implInitControls(const SfxItemSet& _rSet, sal_Bool /*_bSaveValue*/, sal_Bool _bValid)
    {
        m_aHeader.Show(!m_bWizardMode);

        // first check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        SFX_ITEMSET_GET(_rSet, pDelItem, SfxStringItem, DSID_FIELDDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pStrItem, SfxStringItem, DSID_TEXTDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pDecdelItem, SfxStringItem, DSID_DECIMALDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pThodelItem, SfxStringItem, DSID_THOUSANDSDELIMITER, sal_True);
        SFX_ITEMSET_GET(_rSet, pExtensionItem, SfxStringItem, DSID_TEXTFILEEXTENSION, sal_True);
        if (_bValid)
        {
            if (!m_bWizardMode)
            {
                SFX_ITEMSET_GET(_rSet, pHdrItem, SfxBoolItem, DSID_TEXTFILEHEADER, sal_True);
                m_aHeader.Check( pHdrItem->GetValue() );
            }
            m_aOldExtension = pExtensionItem->GetValue();
            SetExtension(m_aOldExtension);
            SetSeparator(m_aFieldSeparator, m_aFieldSeparatorList, pDelItem->GetValue());
            SetSeparator(m_aTextSeparator, m_aTextSeparatorList, pStrItem->GetValue());

            m_aDecimalSeparator.SetText(pDecdelItem->GetValue());
            m_aThousandsSeparator.SetText(pThodelItem->GetValue());
            m_aETOwnExtension.Show();
        }
    }



    // -----------------------------------------------------------------------
    sal_Bool OTextConnectionHelper::prepareLeave()
    {
        LocalResourceAccess aStringResAccess(PAGE_TEXT, RSC_TABPAGE);
            // for accessing the strings which are local to our own resource block
        String sExtension = GetExtension();
        String aErrorText;
        Control* pErrorWin = NULL;
        //  if (!m_aFieldSeparator.GetText().Len())
            // bug (#42168) if this line is compiled under OS2 (in a product environent)
            // -> use a temporary variable
        String aDelText(m_aFieldSeparator.GetText());
        if(!aDelText.Len())
        {   // Kein FeldTrenner
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MISSING));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (!m_aDecimalSeparator.GetText().Len())
        {   // kein Decimaltrenner
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MISSING));
            aErrorText.SearchAndReplaceAscii("#1",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aDecimalSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aFieldSeparator.GetText())
        {   // Feld und TextTrenner duerfen nicht gleich sein
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aTextSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aFieldSeparatorLabel.GetText());
            pErrorWin = &m_aTextSeparator;
        }
        else if (m_aDecimalSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Tausender und DecimalTrenner duerfen nicht gleich sein
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aDecimalSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aDecimalSeparator;
        }
        else if (m_aFieldSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Tausender und FeldTrenner duerfen nicht gleich sein
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aThousandsSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (m_aFieldSeparator.GetText() == m_aDecimalSeparator.GetText())
        {   // Zehner und FeldTrenner duerfen nicht gleich sein
            aErrorText = String(ModuleRes(STR_AUTODELIMITER_MUST_DIFFER));
            aErrorText.SearchAndReplaceAscii("#1",m_aFieldSeparatorLabel.GetText());
            aErrorText.SearchAndReplaceAscii("#2",m_aDecimalSeparatorLabel.GetText());
            pErrorWin = &m_aFieldSeparator;
        }
        else if (m_aTextSeparator.GetText() == m_aThousandsSeparator.GetText())
        {   // Tausender und TextTrenner duerfen nicht gleich sein
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


    // -----------------------------------------------------------------------
    sal_Bool OTextConnectionHelper::FillItemSet( SfxItemSet& rSet, const sal_Bool _bChangedSomething )
    {
        sal_Bool bChangedSomething = _bChangedSomething;
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
        String sExtension = GetExtension();
        if(!m_aOldExtension.Equals(sExtension))
        {
            rSet.Put(SfxStringItem(DSID_TEXTFILEEXTENSION, sExtension));
            bChangedSomething = sal_True;
        }

        if (!m_bWizardMode)
        {
            if( (m_aHeader.GetState() != m_aHeader.GetSavedValue()) )
            {
                rSet.Put(SfxBoolItem(DSID_TEXTFILEHEADER, m_aHeader.IsChecked()));
                bChangedSomething = sal_True;
            }
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
            sExtension = String::CreateFromAscii("txt");
        else if (m_aRBAccessCSVFiles.IsChecked())
            sExtension = String::CreateFromAscii("csv");
        else
        {
            sExtension = m_aETOwnExtension.GetText();
            if ( sExtension.GetToken(0,'.').Equals('*') )
                sExtension.Erase(0,2);
        }
        return sExtension;
    }


    //------------------------------------------------------------------------
    String OTextConnectionHelper::GetSeparator( const ComboBox& rBox, const String& rList )
    {
        sal_Unicode nTok = '\t';
        xub_StrLen  nPos(rBox.GetEntryPos( rBox.GetText() ));

        if( nPos == COMBOBOX_ENTRY_NOTFOUND )
            return rBox.GetText().Copy(0);

        if ( !( &m_aTextSeparator == &rBox && nPos == (rBox.GetEntryCount()-1) ) )
            return String(
                static_cast< sal_Unicode >(
                    rList.GetToken(((nPos*2)+1), nTok ).ToInt32()));
        // somewhat strange ... translates for instance an "32" into " "
        return String();
    }

    //------------------------------------------------------------------------
    void OTextConnectionHelper::SetSeparator( ComboBox& rBox, const String& rList, const String& rVal )
    {
        char    nTok = '\t';
        xub_StrLen  nCnt(rList.GetTokenCount( nTok ));
        xub_StrLen  i;

        for( i=0 ; i<nCnt ; i+=2 )
        {
            String  sTVal(
                static_cast< sal_Unicode >(
                    rList.GetToken( (i+1), nTok ).ToInt32()));

            if( sTVal == rVal )
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

//.........................................................................
}   // namespace dbaui
//.........................................................................

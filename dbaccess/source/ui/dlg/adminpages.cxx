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

#include "adminpages.hxx"
#include "dbadmin.hrc"
#include "dbadmin.hxx"
#include "dbu_dlg.hrc"
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include "dbustrings.hrc"
#include "dsitems.hxx"
#include "dsselect.hxx"
#include "localresaccess.hxx"
#include "odbcconfig.hxx"
#include "optionalboolitem.hxx"
#include "sqlmessage.hxx"

#include <osl/file.hxx>
#include <vcl/accel.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>

#include <algorithm>
#include <stdlib.h>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::dbtools;
    using namespace ::svt;

    ISaveValueWrapper::~ISaveValueWrapper()
    {
    }

    // OGenericAdministrationPage
    DBG_NAME(OGenericAdministrationPage)
    OGenericAdministrationPage::OGenericAdministrationPage(Window* _pParent, const ResId& _rId, const SfxItemSet& _rAttrSet)
        :SfxTabPage(_pParent, _rId, _rAttrSet)
        ,m_abEnableRoadmap(sal_False)
        ,m_pAdminDialog(NULL)
        ,m_pItemSetHelper(NULL)
        ,m_pFT_HeaderText(NULL)
    {
        DBG_CTOR(OGenericAdministrationPage,NULL);

        SetExchangeSupport(sal_True);
    }

    OGenericAdministrationPage::OGenericAdministrationPage(Window* _pParent, const OString& _rId, const OUString& _rUIXMLDescription, const SfxItemSet& _rAttrSet)
        :SfxTabPage(_pParent, _rId, _rUIXMLDescription, _rAttrSet)
        ,m_abEnableRoadmap(sal_False)
        ,m_pAdminDialog(NULL)
        ,m_pItemSetHelper(NULL)
        ,m_pFT_HeaderText(NULL)
    {
        DBG_CTOR(OGenericAdministrationPage,NULL);

        SetExchangeSupport(sal_True);
    }

    OGenericAdministrationPage::~OGenericAdministrationPage()
    {
        DELETEZ(m_pFT_HeaderText);

        DBG_DTOR(OGenericAdministrationPage,NULL);
    }

    int OGenericAdministrationPage::DeactivatePage(SfxItemSet* _pSet)
    {
        if (_pSet)
        {
            if (!prepareLeave())
                return KEEP_PAGE;
            FillItemSet(*_pSet);
        }

        return LEAVE_PAGE;
    }

    void OGenericAdministrationPage::Reset(const SfxItemSet& _rCoreAttrs)
    {
        implInitControls(_rCoreAttrs, sal_False);
    }
    void OGenericAdministrationPage::ActivatePage()
    {
        TabPage::ActivatePage();
        OSL_ENSURE(m_pItemSetHelper,"NO ItemSetHelper set!");
        if ( m_pItemSetHelper )
            ActivatePage(*m_pItemSetHelper->getOutputSet());
    }
    void OGenericAdministrationPage::ActivatePage(const SfxItemSet& _rSet)
    {
        implInitControls(_rSet, sal_True);
    }

    void OGenericAdministrationPage::getFlags(const SfxItemSet& _rSet, sal_Bool& _rValid, sal_Bool& _rReadonly)
    {
        SFX_ITEMSET_GET(_rSet, pInvalid, SfxBoolItem, DSID_INVALID_SELECTION, sal_True);
        _rValid = !pInvalid || !pInvalid->GetValue();
        SFX_ITEMSET_GET(_rSet, pReadonly, SfxBoolItem, DSID_READONLY, sal_True);
        _rReadonly = !_rValid || (pReadonly && pReadonly->GetValue());
    }

    IMPL_LINK_NOARG(OGenericAdministrationPage, OnControlModified)
    {
        callModifiedHdl();
        return 0L;
    }
    sal_Bool OGenericAdministrationPage::getSelectedDataSource(OUString& _sReturn,OUString& _sCurr)
    {
        // collect all ODBC data source names
        StringBag aOdbcDatasources;
        OOdbcEnumeration aEnumeration;
        if (!aEnumeration.isLoaded())
        {
            // show an error message
            String sError( ModuleRes( STR_COULD_NOT_LOAD_ODBC_LIB ) );
            sError.SearchAndReplaceAscii("#lib#", aEnumeration.getLibraryName());
            ErrorBox aDialog(this, WB_OK, sError);
            aDialog.Execute();
            return sal_False;
        }
        else
        {
            aEnumeration.getDatasourceNames(aOdbcDatasources);
            // execute the select dialog
            ODatasourceSelectDialog aSelector(GetParent(), aOdbcDatasources);
            if (!_sCurr.isEmpty())
                aSelector.Select(_sCurr);
            if ( RET_OK == aSelector.Execute() )
                _sReturn = aSelector.GetSelected();
        }
        return sal_True;
    }

    void OGenericAdministrationPage::implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
    {
        // check whether or not the selection is invalid or readonly (invalid implies readonly, but not vice versa)
        sal_Bool bValid, bReadonly;
        getFlags(_rSet, bValid, bReadonly);

        ::std::vector< ISaveValueWrapper* > aControlList;
        if ( _bSaveValue )
        {
            fillControls(aControlList);
            ::std::for_each(aControlList.begin(),aControlList.end(),TSaveValueWrapperFunctor());
        }

        if ( bReadonly )
        {
            fillWindows(aControlList);
            ::std::for_each(aControlList.begin(),aControlList.end(),TDisableWrapperFunctor());
        }

        ::std::for_each(aControlList.begin(),aControlList.end(),TDeleteWrapperFunctor());
        aControlList.clear();
    }

    void OGenericAdministrationPage::initializePage()
    {
        OSL_ENSURE(m_pItemSetHelper,"NO ItemSetHelper set!");
        if ( m_pItemSetHelper )
            Reset(*m_pItemSetHelper->getOutputSet());
    }
    sal_Bool OGenericAdministrationPage::commitPage( ::svt::WizardTypes::CommitPageReason )
    {
        return sal_True;
    }
    bool OGenericAdministrationPage::canAdvance() const
    {
        return true;
    }
    void OGenericAdministrationPage::fillBool( SfxItemSet& _rSet, CheckBox* _pCheckBox, sal_uInt16 _nID, sal_Bool& _bChangedSomething, bool _bRevertValue )
    {
        if ( (_pCheckBox != NULL ) && ( _pCheckBox->GetState() != _pCheckBox->GetSavedValue() ) )
        {
            sal_Bool bValue = _pCheckBox->IsChecked();
            if ( _bRevertValue )
                bValue = !bValue;

            if ( _pCheckBox->IsTriStateEnabled() )
            {
                OptionalBoolItem aValue( _nID );
                if ( _pCheckBox->GetState() != STATE_DONTKNOW )
                    aValue.SetValue( bValue );
                _rSet.Put( aValue );
            }
            else
                _rSet.Put( SfxBoolItem( _nID, bValue ) );

            _bChangedSomething = sal_True;
        }
    }
    void OGenericAdministrationPage::fillInt32(SfxItemSet& _rSet,NumericField* _pEdit,sal_uInt16 _nID,sal_Bool& _bChangedSomething)
    {
        if( (_pEdit != NULL) && (_pEdit->GetValue() != _pEdit->GetSavedValue().toInt32()) )
        {
            _rSet.Put(SfxInt32Item(_nID, static_cast<sal_Int32>(_pEdit->GetValue())));
            _bChangedSomething = sal_True;
        }
    }
    void OGenericAdministrationPage::fillString(SfxItemSet& _rSet,Edit* _pEdit,sal_uInt16 _nID,sal_Bool& _bChangedSomething)
    {
        if( (_pEdit != NULL) && (_pEdit->GetText() != _pEdit->GetSavedValue()) )
        {
            _rSet.Put(SfxStringItem(_nID, _pEdit->GetText()));
            _bChangedSomething = sal_True;
        }
    }

    void OGenericAdministrationPage::SetControlFontWeight(Window* _pWindow, FontWeight _eWeight)
    {
        Font aFont = _pWindow->GetControlFont();
        aFont.SetWeight( _eWeight );
        _pWindow->SetControlFont( aFont );
    }

    IMPL_LINK(OGenericAdministrationPage, OnTestConnectionClickHdl, PushButton*, /*_pButton*/)
    {
        OSL_ENSURE(m_pAdminDialog,"No Admin dialog set! ->GPF");
        sal_Bool bSuccess = sal_False;
        if ( m_pAdminDialog )
        {
            m_pAdminDialog->saveDatasource();
            OGenericAdministrationPage::implInitControls(*m_pItemSetHelper->getOutputSet(), sal_True);
            sal_Bool bShowMessage = sal_True;
            try
            {
                ::std::pair< Reference<XConnection>,sal_Bool> xConnection = m_pAdminDialog->createConnection();
                bShowMessage = xConnection.second;
                bSuccess = xConnection.first.is();
                ::comphelper::disposeComponent(xConnection.first);
            }
            catch(Exception&)
            {
            }
            if ( bShowMessage )
            {
                OSQLMessageBox::MessageType eImage = OSQLMessageBox::Info;
                String aMessage,sTitle;
                sTitle = String (ModuleRes(STR_CONNECTION_TEST));
                if ( bSuccess )
                {
                    aMessage = String(ModuleRes(STR_CONNECTION_SUCCESS));
                }
                else
                {
                    eImage = OSQLMessageBox::Error;
                    aMessage = String(ModuleRes(STR_CONNECTION_NO_SUCCESS));
                }
                OSQLMessageBox aMsg( this, sTitle, aMessage, WB_OK, eImage );
                aMsg.Execute();
            }
            if ( !bSuccess )
                m_pAdminDialog->clearPassword();
        }
        return 0L;
    }

    void OGenericAdministrationPage::SetHeaderText( sal_uInt16 _nFTResId, sal_uInt16 _StringResId)
    {
        delete(m_pFT_HeaderText);
        m_pFT_HeaderText = new FixedText(this, ModuleRes(_nFTResId));
        String sHeaderText = String(ModuleRes(_StringResId));
        m_pFT_HeaderText->SetText(sHeaderText);
        SetControlFontWeight(m_pFT_HeaderText);
    }

    // LayoutHelper
    void LayoutHelper::positionBelow( const Control& _rReference, Control& _rControl, const ControlRelation _eRelation,
        const long _nIndentAppFont )
    {
        Point aReference = _rReference.GetPosPixel();
        aReference.Y() += _rReference.GetSizePixel().Height();

        const Window* pConverter = _rControl.GetParent();
        Size aOffset = pConverter->LogicToPixel( Size( _nIndentAppFont, ( _eRelation == RelatedControls ? 3 : 6 ) ), MAP_APPFONT );

        Point aControlPos( aReference.X() + aOffset.Width(), aReference.Y() + aOffset.Height() );
        _rControl.SetPosPixel( aControlPos );
    }

    void LayoutHelper::fitSizeRightAligned( PushButton& io_button )
    {
        const Point aOldPos = io_button.GetPosPixel();
        const Size aOldSize = io_button.GetSizePixel();
        const Size aMinSize( io_button.CalcMinimumSize() );
        if ( aMinSize.Width() > aOldSize.Width() )
        {
            io_button.setPosSizePixel(
                aOldPos.X() + aOldSize.Width() - aMinSize.Width(),
                0,
                aMinSize.Width(),
                0,
                WINDOW_POSSIZE_X | WINDOW_POSSIZE_WIDTH
            );
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

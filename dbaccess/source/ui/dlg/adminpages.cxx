/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adminpages.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:16:10 $
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

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _DBU_DLG_HRC_
#include "dbu_dlg.hrc"
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
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _SV_ACCEL_HXX
#include <vcl/accel.hxx>
#endif
#include <algorithm>
#include <stdlib.h>
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _DBAUI_DSSELECT_HXX_
#include "dsselect.hxx"
#endif
#ifndef _DBAUI_ODBC_CONFIG_HXX_
#include "odbcconfig.hxx"
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif



//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::dbtools;
    using namespace ::svt;


    //=========================================================================
    //= OGenericAdministrationPage
    //=========================================================================
    DBG_NAME(OGenericAdministrationPage)
    //-------------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
    OGenericAdministrationPage::~OGenericAdministrationPage()
    {
        DELETEZ(m_pFT_HeaderText);

        DBG_DTOR(OGenericAdministrationPage,NULL);
    }

    //-------------------------------------------------------------------------
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

    //-------------------------------------------------------------------------
    void OGenericAdministrationPage::Reset(const SfxItemSet& _rCoreAttrs)
    {
        implInitControls(_rCoreAttrs, sal_False);
    }
    //-------------------------------------------------------------------------
    void OGenericAdministrationPage::ActivatePage()
    {
        TabPage::ActivatePage();
        OSL_ENSURE(m_pItemSetHelper,"NO ItemSetHelper set!");
        if ( m_pItemSetHelper )
            ActivatePage(*m_pItemSetHelper->getOutputSet());
    }
    //-------------------------------------------------------------------------
    void OGenericAdministrationPage::ActivatePage(const SfxItemSet& _rSet)
    {
        implInitControls(_rSet, sal_True);
    }

    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::getFlags(const SfxItemSet& _rSet, sal_Bool& _rValid, sal_Bool& _rReadonly)
    {
        SFX_ITEMSET_GET(_rSet, pInvalid, SfxBoolItem, DSID_INVALID_SELECTION, sal_True);
        _rValid = !pInvalid || !pInvalid->GetValue();
        SFX_ITEMSET_GET(_rSet, pReadonly, SfxBoolItem, DSID_READONLY, sal_True);
        _rReadonly = !_rValid || (pReadonly && pReadonly->GetValue());
    }


    // -----------------------------------------------------------------------
    IMPL_LINK(OGenericAdministrationPage, OnControlModified, Control*, EMPTYARG)
    {
        callModifiedHdl();
        return 0L;
    }
    // -----------------------------------------------------------------------
    sal_Bool OGenericAdministrationPage::getSelectedDataSource(DATASOURCE_TYPE _eType,::rtl::OUString& _sReturn,::rtl::OUString& _sCurr)
    {
        // collect all ODBC data source names
        StringBag aOdbcDatasources;
        OOdbcEnumeration aEnumeration;
        if (!aEnumeration.isLoaded())
        {
            // show an error message
            LocalResourceAccess aLocRes( PAGE_GENERAL, RSC_TABPAGE );
            String sError(ModuleRes(STR_COULDNOTLOAD_ODBCLIB));
            sError.SearchAndReplaceAscii("#lib#", aEnumeration.getLibraryName());
            ErrorBox aDialog(this, WB_OK, sError);
            aDialog.Execute();
            return sal_False;
        }
        else
        {
            aEnumeration.getDatasourceNames(aOdbcDatasources);
            // excute the select dialog
            ODatasourceSelectDialog aSelector(GetParent(), aOdbcDatasources, _eType);
            if (_sCurr.getLength())
                aSelector.Select(_sCurr);
            if ( RET_OK == aSelector.Execute() )
                _sReturn = aSelector.GetSelected();
        }
        return sal_True;
    }

    // -----------------------------------------------------------------------
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

    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::initializePage()
    {
        OSL_ENSURE(m_pItemSetHelper,"NO ItemSetHelper set!");
        if ( m_pItemSetHelper )
            Reset(*m_pItemSetHelper->getOutputSet());
    }
    // -----------------------------------------------------------------------
    sal_Bool OGenericAdministrationPage::commitPage( CommitPageReason )
    {
        return sal_True;
    }
    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::fillBool( SfxItemSet& _rSet, CheckBox* _pCheckBox, USHORT _nID, sal_Bool& _bChangedSomething, bool _bRevertValue )
    {
        if ( (_pCheckBox != NULL ) && ( _pCheckBox->GetState() != _pCheckBox->GetSavedValue() ) )
        {
            sal_Bool bValue = _pCheckBox->IsChecked();
            if ( _bRevertValue )
                bValue = !bValue;

            _rSet.Put( SfxBoolItem( _nID, bValue ) );
            _bChangedSomething = sal_True;
        }
    }
    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::fillInt32(SfxItemSet& _rSet,NumericField* _pEdit,USHORT _nID,sal_Bool& _bChangedSomething)
    {
        if( (_pEdit != NULL) && (_pEdit->GetValue() != _pEdit->GetSavedValue().ToInt32()) )
        {
            _rSet.Put(SfxInt32Item(_nID, static_cast<INT32>(_pEdit->GetValue())));
            _bChangedSomething = sal_True;
        }
    }
    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::fillString(SfxItemSet& _rSet,Edit* _pEdit,USHORT _nID,sal_Bool& _bChangedSomething)
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

    // -----------------------------------------------------------------------
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
                OSQLMessageBox aMsg(this,sTitle,aMessage);
                aMsg.Execute();
            }
            if ( !bSuccess )
                m_pAdminDialog->clearPassword();
        }
        return 0L;
    }

    void OGenericAdministrationPage::SetHeaderText( USHORT _nFTResId, USHORT _StringResId)
    {
        delete(m_pFT_HeaderText);
        m_pFT_HeaderText = new FixedText(this, ModuleRes(_nFTResId));
        String sHeaderText = String(ModuleRes(_StringResId));
        m_pFT_HeaderText->SetText(sHeaderText);
        SetControlFontWeight(m_pFT_HeaderText);
    }


    Point OGenericAdministrationPage::MovePoint(Point _aPixelBasePoint, sal_Int32 _XShift, sal_Int32 _YShift)
    {
        Point rLogicPoint = PixelToLogic( _aPixelBasePoint, MAP_APPFONT );
        sal_uInt32 XPos = rLogicPoint.X() + _XShift;
        sal_uInt32 YPos = rLogicPoint.Y() + _YShift;
        Point aNewPixelPoint = LogicToPixel(Point(XPos, YPos), MAP_APPFONT);
        return aNewPixelPoint;
    }



//.........................................................................
}   // namespace dbaui
//.........................................................................


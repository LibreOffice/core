/*************************************************************************
 *
 *  $RCSfile: adminpages.cxx,v $
 *
 *  $Revision: 1.35 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:42:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HRC_
#include "dbadmin.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
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
    //= OPageSettings
    //=========================================================================
    //-------------------------------------------------------------------------
    OPageSettings::~OPageSettings()
    {
    }

    //=========================================================================
    //= OGenericAdministrationPage
    //=========================================================================
    //-------------------------------------------------------------------------
    OGenericAdministrationPage::OGenericAdministrationPage(Window* _pParent, const ResId& _rId, const SfxItemSet& _rAttrSet)
        :SfxTabPage(_pParent, _rId, _rAttrSet)
        ,m_pAdminDialog(NULL)
        ,m_pItemSetHelper(NULL)
    {
        SetExchangeSupport(sal_True);
    }

    //-------------------------------------------------------------------------
    OGenericAdministrationPage::~OGenericAdministrationPage()
    {
    }

    //-------------------------------------------------------------------------
    int OGenericAdministrationPage::DeactivatePage(SfxItemSet* _pSet)
    {
        if (_pSet)
        {
            if (!checkItems())
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
    OPageSettings* OGenericAdministrationPage::createViewSettings()
    {
        return NULL;
    }

    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::fillViewSettings(OPageSettings* _rSettings)
    {
        // nothing to do
    }

    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::restoreViewSettings(const OPageSettings* _pPageState)
    {
        // nothing to do
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
    sal_Bool OGenericAdministrationPage::getSelectedDataSource(DATASOURCE_TYPE _eType,::rtl::OUString& _sReturn)
    {
        // collect all ODBC data source names
        StringBag aOdbcDatasources;
        OOdbcEnumeration aEnumeration;
        if (!aEnumeration.isLoaded())
        {
            // show an error message
            OLocalResourceAccess aLocRes( PAGE_GENERAL, RSC_TABPAGE );
            String sError(ResId(STR_COULDNOTLOAD_ODBCLIB));
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
            if ( RET_OK == aSelector.Execute() )
                _sReturn = aSelector.GetSelected();
        }
        return sal_True;
    }
    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::postInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue)
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
    void OGenericAdministrationPage::enableHeader( const Bitmap& _rBitmap, sal_Int32 _nPixelHeight, GrantAccess )
    {
    }
    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::initializePage()
    {
        OSL_ENSURE(m_pItemSetHelper,"NO ItemSetHelper set!");
        if ( m_pItemSetHelper )
            Reset(*m_pItemSetHelper->getOutputSet());
    }
    // -----------------------------------------------------------------------
    sal_Bool OGenericAdministrationPage::commitPage(COMMIT_REASON _eReason)
    {
        return sal_True;
    }
    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::fillBool(SfxItemSet& _rSet,CheckBox* _pCheckBox,USHORT _nID,sal_Bool& _bChangedSomething)
    {
        if( (_pCheckBox != NULL) && (_pCheckBox->GetState() != _pCheckBox->GetSavedValue()) )
        {
            _rSet.Put(SfxBoolItem(_nID, _pCheckBox->IsChecked()));
            _bChangedSomething = sal_True;
        }
    }
    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::fillInt32(SfxItemSet& _rSet,NumericField* _pEdit,USHORT _nID,sal_Bool& _bChangedSomething)
    {
        if( (_pEdit != NULL) && (_pEdit->GetValue() != _pEdit->GetSavedValue().ToInt32()) )
        {
            _rSet.Put(SfxInt32Item(_nID, _pEdit->GetValue()));
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

//.........................................................................
}   // namespace dbaui
//.........................................................................


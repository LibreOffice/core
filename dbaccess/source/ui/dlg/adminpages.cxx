/*************************************************************************
 *
 *  $RCSfile: adminpages.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:19 $
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
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif

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


//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
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
        ,m_pToolBox( NULL )
        ,m_pKeyAccel( NULL )
    {
        SetExchangeSupport(sal_True);
    }

    //-------------------------------------------------------------------------
    OGenericAdministrationPage::~OGenericAdministrationPage()
    {
        if ( m_pKeyAccel )
            delete m_pKeyAccel;
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
    sal_Bool OGenericAdministrationPage::prepareConnectionAction( ODbAdminDialog* _pDialog, const String& _rActionDescription, OPageSettings** _pViewSettings )
    {
        sal_Bool bDeleteSettings = sal_True;
        sal_Bool bContinueAction = sal_True;
        if (_pDialog->isCurrentModified())
        {
            // the current data source is modified, so we need to save it in case we're
            // about to do something which requires a connection
            if (!_pDialog->isApplyable())
            {
                ErrorBox aError(this, ModuleRes(ERR_CANTDOTABLEACTION));
                aError.Execute();
            }
            else
            {
                QueryBox aAskForSave(this, ModuleRes(QUERY_NEED_TO_SAVE_FILTER));
                aAskForSave.SetText(_rActionDescription);
                if (RET_YES == aAskForSave.Execute())
                {
                    _pDialog->applyChangesAsync(_pViewSettings ? *_pViewSettings : NULL);
                    bDeleteSettings = sal_False;
                }
            }
            bContinueAction = sal_False;
        }

        if (bDeleteSettings && _pViewSettings)
        {
            delete *_pViewSettings;
            *_pViewSettings = NULL;
        }
        return bContinueAction;
    }

    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::enableToolBoxAcceleration( ToolBox* _pDerivedClassToolBox )
    {
        DBG_ASSERT( !m_pKeyAccel && !m_pToolBox, "OGenericAdministrationPage::enableToolBoxAcceleration: already enabled!" );
        if ( !m_pKeyAccel )
        {
            m_pKeyAccel = new Accelerator;
            m_pKeyAccel->SetSelectHdl( LINK( this, OGenericAdministrationPage, OnAccelSelected ) );
        }

        m_pToolBox = _pDerivedClassToolBox;
    }

    // -----------------------------------------------------------------------
    void OGenericAdministrationPage::addToolboxAccelerator( sal_uInt16 _nToolboxItemId, const KeyCode& _rKey )
    {
        DBG_ASSERT( m_pKeyAccel && m_pToolBox, "OGenericAdministrationPage::addToolboxAccelerator: toolbox acceleration not enabled!" );
        if ( !(m_pKeyAccel && m_pToolBox) )
            return;

        // assert that the toolbox knows this item
        DBG_ASSERT( TOOLBOX_ITEM_NOTFOUND != m_pToolBox->GetItemPos( _nToolboxItemId ),
            "OGenericAdministrationPage::addToolboxAccelerator: invalid id!" );
        // assert that the accelerator does not know this item, yet
        OSL_ENSURE( !m_pKeyAccel->IsIdValid( _nToolboxItemId ), "OGenericAdministrationPage::addToolboxAccelerator: already have this id!" );

        // add the item to the accelerator
        m_pKeyAccel->InsertItem( _nToolboxItemId, _rKey );
    }

    //------------------------------------------------------------------------
    long OGenericAdministrationPage::PreNotify( NotifyEvent& _rNEvt )
    {
        if ( m_pKeyAccel )
        {
            if ( EVENT_KEYINPUT == _rNEvt.GetType() )
            {
                const KeyEvent* pEvent = _rNEvt.GetKeyEvent();
                if ( m_pKeyAccel->Call( pEvent->GetKeyCode() ) )
                    return 1;
            }
        }
        return SfxTabPage::PreNotify( _rNEvt );
    }

    //------------------------------------------------------------------------
    void OGenericAdministrationPage::onToolBoxAction( sal_uInt16 _nClickedItemId )
    {
    }

    //------------------------------------------------------------------------
    IMPL_LINK( OGenericAdministrationPage, OnAccelSelected, void*, NOTINTERESTEDIN )
    {
        sal_uInt16 nId = m_pKeyAccel->GetCurItemId();
        if ( m_pToolBox && m_pToolBox->IsItemEnabled( nId ) )
            onToolBoxAction( nId );

        return 0L;
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


//.........................................................................
}   // namespace dbaui
//.........................................................................


/*************************************************************************
 *
 *  $RCSfile: adminpages.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: fs $ $Date: 2001-05-23 13:47:00 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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

#include <stdlib.h>
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
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
    {
        SetExchangeSupport(sal_True);
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
    IMPL_LINK(OGenericAdministrationPage, OnControlModified, Control*, EMPTYARG)
    {
        callModifiedHdl();
        return 0L;
    }

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.29  2001/05/10 13:34:28  fs
 *  #86223# +OPageSettings/createViewSettings/filleViewSettings/restoreViewSettings
 *
 *  Revision 1.28  2001/01/26 16:12:12  fs
 *  split up the file
 *
 *  Revision 1.27  2001/01/26 06:59:12  fs
 *  some basics for the query administration page - not enabled yet
 *
 *  Revision 1.26  2001/01/25 12:14:03  fs
 *  #83192# initialization of m_sDsn corrected
 *
 *  Revision 1.25  2001/01/04 11:21:45  fs
 *  #81485# +OAdoDetailsPage
 *
 *  Revision 1.24  2001/01/04 09:43:26  fs
 *  #81615# auto completion for the extension checkbox is case sensitive
 *
 *  Revision 1.23  2000/12/11 16:33:15  fs
 *  reversed the semantics of the SuppressVersionColumns checkbox
 *
 *  Revision 1.22  2000/12/07 15:04:40  fs
 *  #81490# reset the password when changing the user
 *
 *  Revision 1.21  2000/12/07 14:27:53  fs
 *  #80939# clear the tables list when cancelling the password dialog
 *
 *  Revision 1.20  2000/12/07 14:15:42  oj
 *  #81131# check installed adabas dbs
 *
 *  Revision 1.0 26.09.00 11:47:18  fs
 ************************************************************************/


/*************************************************************************
 *
 *  $RCSfile: dsselect.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2000-10-26 13:11:36 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_ODBC_CONFIG_HXX_
#include "odbcconfig.hxx"
#endif

#ifndef _DBAUI_DSSELECT_HXX_
#include "dsselect.hxx"
#endif
#ifndef _DBAUI_DSSELECT_HRC_
#include "dsselect.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBAUI_LOCALRESACCESS_HXX_
#include "localresaccess.hxx"
#endif
#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif

#if defined( WIN ) || defined( WNT )
#define HWND    void*
    // was unable to include windows.h, that's why this direct define
#endif
#ifndef _SV_SYSDATA_HXX
#include <vcl/sysdata.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//==================================================================
ODatasourceSelectDialog::ODatasourceSelectDialog(Window* _pParent, const StringBag& _rDatasources, DATASOURCE_TYPE _eType)
     :ModalDialog(_pParent, ModuleRes(DLG_DATASOURCE_SELECTION))
     ,m_aDescription        (this, ResId(FT_DESCRIPTION))
     ,m_aDatasource         (this, ResId(LB_DATASOURCE))
     ,m_aOk                 (this, ResId(PB_OK))
     ,m_aCancel             (this, ResId(PB_CANCEL))
     ,m_aHelp               (this, ResId(PB_HELP))
     ,m_aManageDatasources  (this, ResId(PB_MANAGE))
{
    if (DST_ADABAS == _eType)
    {   // set a new title (indicating that we're browsing local data sources only)
        SetText(ResId(STR_LOCAL_DATASOURCES));

        // resize the dialog a little bit, 'cause Adabas data source names are usually somewhat shorter
        // than ODBC ones are

        // shrink the listbox
        Size aOldSize = m_aDatasource.GetSizePixel();
        Size aNewSize(3 * aOldSize.Width() / 4, aOldSize.Height());
        m_aDatasource.SetSizePixel(aNewSize);

        sal_Int32 nLostPixels = aOldSize.Width() - aNewSize.Width();

        // shrink the fixed text
        aOldSize = m_aDescription.GetSizePixel();
        m_aDescription.SetSizePixel(Size(aOldSize.Width() - nLostPixels, aOldSize.Height()));

        // move the buttons
        PushButton* pButtons[] = { &m_aOk, &m_aCancel, &m_aHelp };
        for (sal_Int32 i=0; i<sizeof(pButtons)/sizeof(pButtons[0]); ++i)
        {
            Point aOldPos = pButtons[i]->GetPosPixel();
            pButtons[i]->SetPosPixel(Point(aOldPos.X() - nLostPixels, aOldPos.Y()));
        }

        // resize the dialog itself
        aOldSize = GetSizePixel();
        SetSizePixel(Size(aOldSize.Width() - nLostPixels, aOldSize.Height()));
    }

    // fill the list
    for (   ConstStringBagIterator aDS = _rDatasources.begin();
            aDS != _rDatasources.end();
            ++aDS
        )
    {
        m_aDatasource.InsertEntry( aDS->getStr() );
    }

    // select the first entry
    if (m_aDatasource.GetEntryCount())
        m_aDatasource.SelectEntryPos(0);

    // allow ODBC datasource managenment
    if (DST_ODBC == _eType)
    {
        m_aManageDatasources.Show();
        m_aManageDatasources.Enable();
        m_aManageDatasources.SetClickHdl(LINK(this,ODatasourceSelectDialog,ManageClickHdl));
    }

    m_aDatasource.SetDoubleClickHdl(LINK(this,ODatasourceSelectDialog,ListDblClickHdl));
    FreeResource();
}

// -----------------------------------------------------------------------
IMPL_LINK( ODatasourceSelectDialog, ListDblClickHdl, ListBox *, pListBox )
{
    if (pListBox->GetSelectEntryCount())
        EndDialog(RET_OK);
    return 0;
}

// -----------------------------------------------------------------------
IMPL_LINK( ODatasourceSelectDialog, ManageClickHdl, PushButton*, pButton )
{
    OOdbcManagement aOdbcConfig;
#ifdef HAVE_ODBC_ADMINISTRATION
    if (!aOdbcConfig.isLoaded())
    {
#endif
        // show an error message
        OLocalResourceAccess aLocRes(DLG_DATASOURCE_SELECTION, RSC_MODALDIALOG);
        String sError(ModuleRes(STR_COULDNOTLOAD_CONFIGLIB));
        sError.SearchAndReplaceAscii("#lib#", aOdbcConfig.getLibraryName());
        ErrorBox aDialog(this, WB_OK, sError);
        aDialog.Execute();
        m_aDatasource.GrabFocus();
        m_aManageDatasources.Disable();
        return 1L;
#ifdef HAVE_ODBC_ADMINISTRATION
    }

    aOdbcConfig.manageDataSources(GetSystemData()->hWnd);
    return 0L;
#endif
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/10/24 12:13:30  fs
 *  initial checkin - dialog for selecting system datasources
 *
 *
 *  Revision 1.0 24.10.00 09:25:01  fs
 ************************************************************************/


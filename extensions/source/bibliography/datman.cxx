/*************************************************************************
 *
 *  $RCSfile: datman.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:38:07 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif

#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDATABASEENVIRONMENT_HPP_
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XDATAINPUTSTREAM_HPP_
#include <com/sun/star/io/XDataInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTIDENTIFIER_HXX
#include <ucbhelper/contentidentifier.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen wg. form::component::ListBox
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen wg. Edit
#include <vcl/edit.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef _BIB_DATMAN_HXX
#include "datman.hxx"
#endif
#ifndef ADRRESID_HXX
#include "bibresid.hxx"
#endif
#ifndef BIBMOD_HXX
#include "bibmod.hxx"
#endif
#ifndef _BIB_VIEW_HXX
#include "bibview.hxx"
#endif
// #100312# ---------
#ifndef ADRBEAM_HXX
#include "bibbeam.hxx"
#endif
#ifndef _BIB_FMPROP_HRC
#include "bibprop.hrc"
#endif
#ifndef _BIB_TOOLBAR_HXX
#include "toolbar.hxx"
#endif
#include "toolbar.hrc"
#ifndef _BIBCONFIG_HXX
#include "bibconfig.hxx"
#endif
#ifndef ADRBEAM_HXX
#include "bibbeam.hxx"
#endif
#ifndef BIB_HRC
#include "bib.hrc"
#endif
#include "datman.hrc"
#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include "extensio.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::lang;
using namespace ::rtl;
using namespace ::ucb;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)
#define MAP_TOKEN ';'
#define PAIR_TOKEN ':'


/* -----------------17.01.00 14:38-------------------

 --------------------------------------------------*/
Reference< XConnection >    getConnection(const rtl::OUString& _rURL)
{
    // first get the sdb::DataSource corresponding to the url
    Reference< XDataSource >    xDataSource;
    // is it a favorite title ?
    Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
    Reference<XInterface> xNamingContextIfc = xMgr->createInstance(C2U("com.sun.star.sdb.DatabaseContext"));
    Reference< XNameAccess >  xNamingContext(xNamingContextIfc, UNO_QUERY);
    if (xNamingContext.is() && xNamingContext->hasByName(_rURL))
    {
        DBG_ASSERT(Reference< XNamingService > (xNamingContext, UNO_QUERY).is(), "::getDataSource : no NamingService interface on the sdb::DatabaseAccessContext !");
        xDataSource = Reference< XDataSource > (Reference< XNamingService > (xNamingContext, UNO_QUERY)->getRegisteredObject(_rURL), UNO_QUERY);
    }
    // build the connection from the data source
    Reference< XConnection >    xConn;
    if (xDataSource.is())
    {
        // need user/pwd for this
        rtl::OUString sUser, sPwd;
        Reference< XPropertySet >  xDataSourceProps(xDataSource, UNO_QUERY);
        Reference< XCompletedConnection > xComplConn(xDataSource, UNO_QUERY);
/*      Reference< XPropertySetInfo >  xInfo = xDataSourceProps.is() ? xDataSourceProps->getPropertySetInfo() : Reference< XPropertySetInfo > ();
        if (xInfo.is() && xInfo->hasPropertyByName(C2U("User")))
            xDataSourceProps->getPropertyValue(C2U("User")) >>= sUser;
        if (xInfo.is() && xInfo->hasPropertyByName(C2U("Password")))
            xDataSourceProps->getPropertyValue(C2U("Password"))>>= sPwd;
*/
        try
        {

            Reference<XInterface> xHdl = xMgr->createInstance(C2U("com.sun.star.sdb.InteractionHandler"));
            Reference<task::XInteractionHandler> xIHdl(xHdl, UNO_QUERY);
            xConn = xComplConn->connectWithCompletion(xIHdl);
//          xConn = xDataSource->getConnection(sUser, sPwd);
        }
        catch(SQLException&)
        {
            // TODO : a real error handling
        }
        catch(Exception& e )
        {
            e;  // make compiler happy
        }

    }
    return xConn;
}

/* -----------------17.01.00 14:46-------------------

 --------------------------------------------------*/
Reference< XConnection >    getConnection(const Reference< XInterface > & xRowSet)
{
    Reference< XConnection >    xConn;
    try
    {
        Reference< XPropertySet >  xFormProps(xRowSet, UNO_QUERY);
        if (!xFormProps.is())
            return xConn;

        xConn = Reference< XConnection > (*(Reference< XInterface > *)xFormProps->getPropertyValue(C2U("ActiveConnection")).getValue(), UNO_QUERY);
        if (!xConn.is())
        {
            DBG_WARNING("no active connection")
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("exception in getConnection");
    }

    return xConn;
}
/* -----------------17.01.00 16:07-------------------

 --------------------------------------------------*/
Reference< XNameAccess >  getColumns(const Reference< XForm > & _rxForm)
{
    Reference< XNameAccess >  xReturn;
    // check if the form is alive
    Reference< XColumnsSupplier >  xSupplyCols( _rxForm, UNO_QUERY );
    if (xSupplyCols.is())
        xReturn = xSupplyCols->getColumns();

    if (!xReturn.is() || (xReturn->getElementNames().getLength() == 0))
    {   // no ....
        xReturn = NULL;
        // -> get the table the form is bound to and ask it for their columns
        Reference< XTablesSupplier >  xSupplyTables( getConnection( _rxForm ), UNO_QUERY );
        Reference< XPropertySet >  xFormProps( _rxForm, UNO_QUERY );
        if (xFormProps.is() && xSupplyTables.is())
        {
            try
            {
                DBG_ASSERT((*(sal_Int32*)xFormProps->getPropertyValue(C2U("CommandType")).getValue()) == CommandType::TABLE,
                    "::getColumns : invalid form (has no table as data source) !");
                rtl::OUString sTable;
                xFormProps->getPropertyValue(C2U("Command")) >>= sTable;
                Reference< XNameAccess >  xTables = xSupplyTables->getTables();
                if (xTables.is() && xTables->hasByName(sTable))
                    xSupplyCols = Reference< XColumnsSupplier > (
                        *(Reference< XInterface > *)xTables->getByName(sTable).getValue(), UNO_QUERY);
                if (xSupplyCols.is())
                    xReturn = xSupplyCols->getColumns();
            }
#ifdef DBG_UTIL
            catch(Exception& e )
#else
            catch(Exception&)
#endif
            {
#ifdef DBG_UTIL
                String sMsg(String::CreateFromAscii("::getColumns : catched an exception ("));
                sMsg += String(e.Message);
                sMsg.AppendAscii(") ...");
                DBG_ERROR( ByteString(sMsg, RTL_TEXTENCODING_ASCII_US ).GetBuffer());
#endif
            }

        }
    }
    return xReturn;
}

/* -----------------11.11.99 15:54-------------------

 --------------------------------------------------*/
class MappingDialog_Impl : public ModalDialog
{
    FixedLine       aMapGB;
    FixedText       aIdentifierFT;
    ListBox         aIdentifierLB;
    FixedText       aAuthorityTypeFT;
    ListBox         aAuthorityTypeLB;
    FixedText       aYearFT;
    ListBox         aYearLB;
    FixedText       aAuthorFT;
    ListBox         aAuthorLB;
    FixedText       aTitleFT;
    ListBox         aTitleLB;
    FixedText       aPublisherFT;
    ListBox         aPublisherLB;
    FixedText       aAddressFT;
    ListBox         aAddressLB;
    FixedText       aISBNFT;
    ListBox         aISBNLB;
    FixedText       aChapterFT;
    ListBox         aChapterLB;
    FixedText       aPagesFT;
    ListBox         aPagesLB;
    FixedText       aEditorFT;
    ListBox         aEditorLB;
    FixedText       aEditionFT;
    ListBox         aEditionLB;
    FixedText       aBooktitleFT;
    ListBox         aBooktitleLB;
    FixedText       aVolumeFT;
    ListBox         aVolumeLB;
    FixedText       aHowpublishedFT;
    ListBox         aHowpublishedLB;
    FixedText       aOrganizationsFT;
    ListBox         aOrganizationsLB;
    FixedText       aInstitutionFT;
    ListBox         aInstitutionLB;
    FixedText       aSchoolFT;
    ListBox         aSchoolLB;
    FixedText       aReportTypeFT;
    ListBox         aReportTypeLB;
    FixedText       aMonthFT;
    ListBox         aMonthLB;
    FixedText       aJournalFT;
    ListBox         aJournalLB;
    FixedText       aNumberFT;
    ListBox         aNumberLB;
    FixedText       aSeriesFT;
    ListBox         aSeriesLB;
    FixedText       aAnnoteFT;
    ListBox         aAnnoteLB;
    FixedText       aNoteFT;
    ListBox         aNoteLB;
    FixedText       aURLFT;
    ListBox         aURLLB;
    FixedText       aCustom1FT;
    ListBox         aCustom1LB;
    FixedText       aCustom2FT;
    ListBox         aCustom2LB;
    FixedText       aCustom3FT;
    ListBox         aCustom3LB;
    FixedText       aCustom4FT;
    ListBox         aCustom4LB;
    FixedText       aCustom5FT;
    ListBox         aCustom5LB;

    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;

    ListBox*        aListBoxes[COLUMN_COUNT];
    String          sNone;

    sal_Bool        bModified;

    BibDataManager* pDatMan;

    DECL_LINK(OkHdl, OKButton*);
    DECL_LINK(ListBoxSelectHdl, ListBox*);

public:
    MappingDialog_Impl(Window* pParent, BibDataManager* pDatMan);
    ~MappingDialog_Impl();

    void    SetModified() {bModified = TRUE;}

};
/* -----------------11.11.99 16:42-------------------

 --------------------------------------------------*/
sal_uInt16 lcl_FindLogicalName(BibConfig* pConfig ,
                                    const OUString& rLogicalColumnName)
{
    for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
    {
        if(rLogicalColumnName == pConfig->GetDefColumnName(i))
            return i;
    }
    return USHRT_MAX;
}
//-----------------------------------------------------------------------------
MappingDialog_Impl::MappingDialog_Impl(Window* pParent, BibDataManager* pMan) :
    ModalDialog(pParent, BibResId(RID_DLG_MAPPING) ),
    pDatMan(pMan),
    aOKBT(this,                 ResId( BT_OK            )),
    aCancelBT(this,             ResId( BT_CANCEL        )),
    aHelpBT(this,               ResId( BT_HELP          )),

    aMapGB(this,                ResId( GB_MAPPING       )),

    aIdentifierFT(this,         ResId( FT_IDENTIFIER    )),
    aIdentifierLB(this,         ResId( LB_IDENTIFIER    )),
    aAuthorityTypeFT(this,      ResId( FT_AUTHORITYTYPE )),
    aAuthorityTypeLB(this,      ResId( LB_AUTHORITYTYPE )),
    aAuthorFT(this,             ResId( FT_AUTHOR        )),
    aAuthorLB(this,             ResId( LB_AUTHOR        )),
    aTitleFT(this,              ResId( FT_TITLE         )),
    aTitleLB(this,              ResId( LB_TITLE         )),
    aMonthFT(this,              ResId( FT_MONTH         )),
    aMonthLB(this,              ResId( LB_MONTH         )),
    aYearFT(this,               ResId( FT_YEAR          )),
    aYearLB(this,               ResId( LB_YEAR          )),
    aISBNFT(this,               ResId( FT_ISBN          )),
    aISBNLB(this,               ResId( LB_ISBN          )),
    aBooktitleFT(this,          ResId( FT_BOOKTITLE     )),
    aBooktitleLB(this,          ResId( LB_BOOKTITLE     )),
    aChapterFT(this,            ResId( FT_CHAPTER       )),
    aChapterLB(this,            ResId( LB_CHAPTER       )),
    aEditionFT(this,            ResId( FT_EDITION       )),
    aEditionLB(this,            ResId( LB_EDITION       )),
    aEditorFT(this,             ResId( FT_EDITOR        )),
    aEditorLB(this,             ResId( LB_EDITOR        )),
    aHowpublishedFT(this,       ResId( FT_HOWPUBLISHED  )),
    aHowpublishedLB(this,       ResId( LB_HOWPUBLISHED  )),
    aInstitutionFT(this,        ResId( FT_INSTITUTION   )),
    aInstitutionLB(this,        ResId( LB_INSTITUTION   )),
    aJournalFT(this,            ResId( FT_JOURNAL       )),
    aJournalLB(this,            ResId( LB_JOURNAL       )),
    aNoteFT(this,               ResId( FT_NOTE          )),
    aNoteLB(this,               ResId( LB_NOTE          )),
    aAnnoteFT(this,             ResId( FT_ANNOTE        )),
    aAnnoteLB(this,             ResId( LB_ANNOTE        )),
    aNumberFT(this,             ResId( FT_NUMBER        )),
    aNumberLB(this,             ResId( LB_NUMBER        )),
    aOrganizationsFT(this,      ResId( FT_ORGANIZATIONS )),
    aOrganizationsLB(this,      ResId( LB_ORGANIZATIONS )),
    aPagesFT(this,              ResId( FT_PAGES         )),
    aPagesLB(this,              ResId( LB_PAGES         )),
    aPublisherFT(this,          ResId( FT_PUBLISHER     )),
    aPublisherLB(this,          ResId( LB_PUBLISHER     )),
    aAddressFT(this,            ResId( FT_ADDRESS       )),
    aAddressLB(this,            ResId( LB_ADDRESS       )),
    aSchoolFT(this,             ResId( FT_SCHOOL        )),
    aSchoolLB(this,             ResId( LB_SCHOOL        )),
    aSeriesFT(this,             ResId( FT_SERIES        )),
    aSeriesLB(this,             ResId( LB_SERIES        )),
    aReportTypeFT(this,         ResId( FT_REPORTTYPE    )),
    aReportTypeLB(this,         ResId( LB_REPORTTYPE    )),
    aVolumeFT(this,             ResId( FT_VOLUME        )),
    aVolumeLB(this,             ResId( LB_VOLUME        )),
    aURLFT(this,                ResId( FT_URL           )),
    aURLLB(this,                ResId( LB_URL           )),
    aCustom1FT(this,            ResId( FT_CUSTOM1       )),
    aCustom1LB(this,            ResId( LB_CUSTOM1       )),
    aCustom2FT(this,            ResId( FT_CUSTOM2       )),
    aCustom2LB(this,            ResId( LB_CUSTOM2       )),
    aCustom3FT(this,            ResId( FT_CUSTOM3       )),
    aCustom3LB(this,            ResId( LB_CUSTOM3       )),
    aCustom4FT(this,            ResId( FT_CUSTOM4       )),
    aCustom4LB(this,            ResId( LB_CUSTOM4       )),
    aCustom5FT(this,            ResId( FT_CUSTOM5       )),
    aCustom5LB(this,            ResId( LB_CUSTOM5       )),
    sNone(ResId(ST_NONE)),
    bModified(sal_False)
{
    FreeResource();

    aIdentifierFT.SetText(String(         BibResId( ST_IDENTIFIER    )));
    aAuthorityTypeFT.SetText(String(      BibResId( ST_AUTHTYPE )));
    aAuthorFT.SetText(String(             BibResId( ST_AUTHOR        )));
    aTitleFT.SetText(String(              BibResId( ST_TITLE         )));
    aMonthFT.SetText(String(              BibResId( ST_MONTH         )));
    aYearFT.SetText(String(               BibResId( ST_YEAR          )));
    aISBNFT.SetText(String(               BibResId( ST_ISBN          )));
    aBooktitleFT.SetText(String(          BibResId( ST_BOOKTITLE     )));
    aChapterFT.SetText(String(            BibResId( ST_CHAPTER       )));
    aEditionFT.SetText(String(            BibResId( ST_EDITION       )));
    aEditorFT.SetText(String(             BibResId( ST_EDITOR        )));
    aHowpublishedFT.SetText(String(       BibResId( ST_HOWPUBLISHED  )));
    aInstitutionFT.SetText(String(        BibResId( ST_INSTITUTION   )));
    aJournalFT.SetText(String(            BibResId( ST_JOURNAL       )));
    aNoteFT.SetText(String(               BibResId( ST_NOTE          )));
    aAnnoteFT.SetText(String(             BibResId( ST_ANNOTE        )));
    aNumberFT.SetText(String(             BibResId( ST_NUMBER        )));
    aOrganizationsFT.SetText(String(      BibResId( ST_ORGANIZATION )));
    aPagesFT.SetText(String(              BibResId( ST_PAGE         )));
    aPublisherFT.SetText(String(          BibResId( ST_PUBLISHER     )));
    aAddressFT.SetText(String(            BibResId( ST_ADDRESS       )));
    aSchoolFT.SetText(String(             BibResId( ST_SCHOOL        )));
    aSeriesFT.SetText(String(             BibResId( ST_SERIES        )));
    aReportTypeFT.SetText(String(         BibResId( ST_REPORT    )));
    aVolumeFT.SetText(String(             BibResId( ST_VOLUME        )));
    aURLFT.SetText(String(                BibResId( ST_URL           )));
    aCustom1FT.SetText(String(            BibResId( ST_CUSTOM1       )));
    aCustom2FT.SetText(String(            BibResId( ST_CUSTOM2       )));
    aCustom3FT.SetText(String(            BibResId( ST_CUSTOM3       )));
    aCustom4FT.SetText(String(            BibResId( ST_CUSTOM4       )));
    aCustom5FT.SetText(String(            BibResId( ST_CUSTOM5       )));

    aOKBT.SetClickHdl(LINK(this, MappingDialog_Impl, OkHdl));
    String sTitle = GetText();
    sTitle.SearchAndReplace(C2S("%1"), pDatMan->getActiveDataTable(), 0);
    SetText(sTitle);

    aListBoxes[0] = &aIdentifierLB;
    aListBoxes[1] = &aAuthorityTypeLB;
    aListBoxes[2] = &aAuthorLB;
    aListBoxes[3] = &aTitleLB;
    aListBoxes[4] = &aYearLB;
    aListBoxes[5] = &aISBNLB;
    aListBoxes[6] = &aBooktitleLB;
    aListBoxes[7] = &aChapterLB;
    aListBoxes[8] = &aEditionLB;
    aListBoxes[9] = &aEditorLB;
    aListBoxes[10] = &aHowpublishedLB;
    aListBoxes[11] = &aInstitutionLB;
    aListBoxes[12] = &aJournalLB;
    aListBoxes[13] = &aMonthLB;
    aListBoxes[14] = &aNoteLB;
    aListBoxes[15] = &aAnnoteLB;
    aListBoxes[16] = &aNumberLB;
    aListBoxes[17] = &aOrganizationsLB;
    aListBoxes[18] = &aPagesLB;
    aListBoxes[19] = &aPublisherLB;
    aListBoxes[20] = &aAddressLB;
    aListBoxes[21] = &aSchoolLB;
    aListBoxes[22] = &aSeriesLB;
    aListBoxes[23] = &aReportTypeLB;
    aListBoxes[24] = &aVolumeLB;
    aListBoxes[25] = &aURLLB;
    aListBoxes[26] = &aCustom1LB;
    aListBoxes[27] = &aCustom2LB;
    aListBoxes[28] = &aCustom3LB;
    aListBoxes[29] = &aCustom4LB;
    aListBoxes[30] = &aCustom5LB;

    aListBoxes[0]->InsertEntry(sNone);
    Reference< XNameAccess >  xFields = getColumns( pDatMan->getForm() );
    DBG_ASSERT(xFields.is(), "MappingDialog_Impl::MappingDialog_Impl : gave me an invalid form !");
    Sequence<rtl::OUString> aNames = xFields->getElementNames();
    sal_Int32 nFieldsCount = aNames.getLength();
    const rtl::OUString* pNames = aNames.getConstArray();

    for(sal_Int32 nField = 0; nField < nFieldsCount; nField++)
        aListBoxes[0]->InsertEntry(pNames[nField]);

    Link aLnk = LINK(this, MappingDialog_Impl, ListBoxSelectHdl);

    aListBoxes[0]->SelectEntryPos(0);
    aListBoxes[0]->SetSelectHdl(aLnk);
    for(sal_uInt16 i = 1; i < COLUMN_COUNT; i++)
    {
        for(sal_uInt16 j = 0; j < aListBoxes[0]->GetEntryCount();j++)
            aListBoxes[i]->InsertEntry(aListBoxes[0]->GetEntry(j));
        aListBoxes[i]->SelectEntryPos(0);
        aListBoxes[i]->SetSelectHdl(aLnk);
    }
    BibConfig* pConfig = BibModul::GetConfig();
    BibDBDescriptor aDesc;
    aDesc.sDataSource = pDatMan->getActiveDataSource();
    aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
    aDesc.nCommandType = CommandType::TABLE;
    const Mapping* pMapping = pConfig->GetMapping(aDesc);
    if(pMapping)
    {
        for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
        {
            sal_uInt16 nListBoxIndex = lcl_FindLogicalName( pConfig, pMapping->aColumnPairs[nEntry].sLogicalColumnName);
            if(nListBoxIndex < COLUMN_COUNT)
            {
                aListBoxes[nListBoxIndex]->SelectEntry(pMapping->aColumnPairs[nEntry].sRealColumnName);
            }
        }
    }
}
/* -----------------11.11.99 16:44-------------------

 --------------------------------------------------*/
MappingDialog_Impl::~MappingDialog_Impl()
{}
/* -----------------15.11.99 10:38-------------------

 --------------------------------------------------*/
IMPL_LINK(MappingDialog_Impl, ListBoxSelectHdl, ListBox*, pListBox)
{
    sal_uInt16 nEntryPos = pListBox->GetSelectEntryPos();
    if(0 < nEntryPos)
    {
        for(sal_uInt16 i = 0; i < COLUMN_COUNT; i++)
        {
            if(pListBox != aListBoxes[i] && aListBoxes[i]->GetSelectEntryPos() == nEntryPos)
                aListBoxes[i]->SelectEntryPos(0);
        }
    }
    SetModified();
    return 0;
}
/* -----------------12.11.99 14:50-------------------

 --------------------------------------------------*/
IMPL_LINK(MappingDialog_Impl, OkHdl, OKButton*, EMPTYARG)
{
    if(bModified)
    {
        Mapping aNew;
        aNew.sTableName = String(pDatMan->getActiveDataTable());
        aNew.sURL = String(pDatMan->getActiveDataSource());

        sal_uInt16 nWriteIndex = 0;
        BibConfig* pConfig = BibModul::GetConfig();
        for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
        {
            String sSel = aListBoxes[nEntry]->GetSelectEntry();
            if(sSel != sNone)
            {
                aNew.aColumnPairs[nWriteIndex].sRealColumnName = sSel;
                aNew.aColumnPairs[nWriteIndex].sLogicalColumnName = pConfig->GetDefColumnName(nEntry);
                nWriteIndex++;
            }
        }
        BibDBDescriptor aDesc;
        aDesc.sDataSource = pDatMan->getActiveDataSource();
        aDesc.sTableOrQuery = pDatMan->getActiveDataTable();
        aDesc.nCommandType = CommandType::TABLE;
        pDatMan->ResetIdentifierMapping();
        pConfig->SetMapping(aDesc, &aNew);
    }
    EndDialog(bModified ? RET_OK : RET_CANCEL);
    return 0;
}
/* -----------------18.11.99 10:23-------------------

 --------------------------------------------------*/
class DBChangeDialog_Impl : public ModalDialog
{
    FixedLine       aSelectionGB;
    SvTabListBox    aSelectionLB;
    HeaderBar       aSelectionHB;

    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;

    DBChangeDialogConfig_Impl   aConfig;
    String          aEntryST;
    String          aURLST;

    BibDataManager* pDatMan;

//  DECL_LINK(EndDragHdl, HeaderBar*);
    DECL_LINK(DoubleClickHdl, SvTabListBox*);
public:
    DBChangeDialog_Impl(Window* pParent, BibDataManager* pMan );
    ~DBChangeDialog_Impl();

    String      GetCurrentURL()const;
};

/*-- 18.11.99 10:35:20---------------------------------------------------

  -----------------------------------------------------------------------*/
DBChangeDialog_Impl::DBChangeDialog_Impl(Window* pParent, BibDataManager* pMan ) :
    ModalDialog(pParent, BibResId(RID_DLG_DBCHANGE) ),
    aOKBT(this,         ResId( BT_OK        )),
    aCancelBT(this,     ResId( BT_CANCEL    )),
    aHelpBT(this,       ResId( BT_HELP      )),
    aSelectionGB(this,  ResId( GB_SELECTION )),
    aSelectionLB(this,  ResId( LB_SELECTION )),
    aSelectionHB(this,  ResId( HB_SELECTION )),
    aEntryST(ResId(ST_ENTRY)),
    aURLST( ResId(ST_URL)),
    pDatMan(pMan)
{
    FreeResource();
    aSelectionLB.SetDoubleClickHdl( LINK(this, DBChangeDialog_Impl, DoubleClickHdl));
    try
    {
        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();

        ::Size aSize = aSelectionHB.GetSizePixel();
        long nTabs[2];
        nTabs[0] = 1;// Number of Tabs
        nTabs[1] = aSize.Width() / 4;

        aSelectionHB.SetStyle(aSelectionHB.GetStyle()|WB_STDHEADERBAR);
        aSelectionHB.InsertItem( 1, aEntryST, aSize.Width());
        aSelectionHB.SetSizePixel(aSelectionHB.CalcWindowSizePixel());
        aSelectionHB.Show();

        aSelectionLB.SetTabs( &nTabs[0], MAP_PIXEL );
        aSelectionLB.SetWindowBits(WB_CLIPCHILDREN|WB_SORT);
        aSelectionLB.GetModel()->SetSortMode(SortAscending);

        OUString sActiveSource = pDatMan->getActiveDataSource();
        const Sequence<OUString>& rSources = aConfig.GetDataSourceNames();
        const OUString* pSourceNames = rSources.getConstArray();
        for(int i = 0; i < rSources.getLength(); i++)
        {
            SvLBoxEntry* pEntry = aSelectionLB.InsertEntry(pSourceNames[i]);
            if(pSourceNames[i] == sActiveSource)
            {
                aSelectionLB.Select(pEntry);
            }
        }
        aSelectionLB.GetModel()->Resort();
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("Exception in BibDataManager::DBChangeDialog_Impl::DBChangeDialog_Impl")
    }


}
/* -----------------06.12.99 12:09-------------------

 --------------------------------------------------*/
IMPL_LINK(DBChangeDialog_Impl, DoubleClickHdl, SvTabListBox*, pLB)
{
    EndDialog(RET_OK);
    return 0;
}
/* -----------------18.11.99 11:17-------------------

 --------------------------------------------------*/
/*IMPL_LINK(DBChangeDialog_Impl, EndDragHdl, HeaderBar*, pHB)
{
    long nTabs[3];
    nTabs[0] = 2;// Number of Tabs
    nTabs[1] = 0;
    nTabs[2] = pHB->GetItemSize( 1 );
    aSelectionLB.SetTabs( &nTabs[0], MAP_PIXEL );
    return 0;
};*/

/*-- 18.11.99 10:35:20---------------------------------------------------

  -----------------------------------------------------------------------*/
DBChangeDialog_Impl::~DBChangeDialog_Impl()
{
}
/* -----------------18.11.99 12:36-------------------

 --------------------------------------------------*/
String  DBChangeDialog_Impl::GetCurrentURL()const
{
    String sRet;
    SvLBoxEntry* pEntry = aSelectionLB.FirstSelected();
    if(pEntry)
    {
        sRet = aSelectionLB.GetEntryText(pEntry, 0);
    }
    return sRet;
}

// #100312# --------------------------------------------------------------------
// XDispatchProvider
BibInterceptorHelper::BibInterceptorHelper( ::bib::BibBeamer* pBibBeamer, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatch)
{
    if( pBibBeamer )
    {
        xInterception = pBibBeamer->getDispatchProviderInterception();
        if( xInterception.is() )
            xInterception->registerDispatchProviderInterceptor( this );
    }
    if( xDispatch.is() )
        xFormDispatch = xDispatch;
}

BibInterceptorHelper::~BibInterceptorHelper( )
{
}

void BibInterceptorHelper::ReleaseInterceptor()
{
    if ( xInterception.is() )
        xInterception->releaseDispatchProviderInterceptor( this );
    xInterception.clear();
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL
    BibInterceptorHelper::queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags ) throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XDispatch > xReturn;

    String aCommand( aURL.Path );
    if ( aCommand.EqualsAscii("FormSlots/ConfirmDeletion") )
        xReturn = xFormDispatch;
    else
        if ( xSlaveDispatchProvider.is() )
            xReturn = xSlaveDispatchProvider->queryDispatch( aURL, aTargetFrameName, nSearchFlags);

    return xReturn;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL
    BibInterceptorHelper::queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts ) throw (::com::sun::star::uno::RuntimeException)
{
    Sequence< Reference< XDispatch> > aReturn( aDescripts.getLength() );
    Reference< XDispatch >* pReturn = aReturn.getArray();
    const DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for ( sal_Int16 i=0; i<aDescripts.getLength(); ++i, ++pReturn, ++pDescripts )
    {
        *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
    }
    return aReturn;
}

// XDispatchProviderInterceptor
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getSlaveDispatchProvider(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return xSlaveDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setSlaveDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewSlaveDispatchProvider ) throw (::com::sun::star::uno::RuntimeException)
{
    xSlaveDispatchProvider = xNewSlaveDispatchProvider;
}

::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > SAL_CALL
    BibInterceptorHelper::getMasterDispatchProvider(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return xMasterDispatchProvider;
}

void SAL_CALL BibInterceptorHelper::setMasterDispatchProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& xNewMasterDispatchProvider ) throw (::com::sun::star::uno::RuntimeException)
{
    xMasterDispatchProvider = xNewMasterDispatchProvider;
}

//-----------------------------------------------------------------------------
#define STR_UID "uid"
rtl::OUString gGridName(C2U("theGrid"));
rtl::OUString gViewName(C2U("theView"));
rtl::OUString gGlobalName(C2U("theGlobals"));
rtl::OUString gBeamerSize(C2U("theBeamerSize"));
rtl::OUString gViewSize(C2U("theViewSize"));

BibDataManager::BibDataManager()
    :BibDataManager_Base( GetMutex() )
    ,pToolbar(0)
    ,pBibView( NULL )
    ,m_aLoadListeners(m_aMutex)
    // #100312# --------------
    ,m_pInterceptorHelper( NULL )
{
}
/* --------------------------------------------------

 --------------------------------------------------*/
BibDataManager::~BibDataManager()
{
    Reference< XLoadable >      xLoad( m_xForm, UNO_QUERY );
    Reference< XPropertySet >   xPrSet( m_xForm, UNO_QUERY );
    Reference< XComponent >     xComp( m_xForm, UNO_QUERY );
    if ( m_xForm.is() )
    {
        Reference< XComponent >  xConnection;
        xPrSet->getPropertyValue(C2U("ActiveConnection")) >>= xConnection;
        RemoveMeAsUidListener();
        if (xLoad.is())
            xLoad->unload();
        if (xComp.is())
            xComp->dispose();
        if(xConnection.is())
            xConnection->dispose();
        m_xForm = NULL;
    }
    // #100312# ----------------
    if( m_pInterceptorHelper )
    {
        m_pInterceptorHelper->ReleaseInterceptor();
        m_pInterceptorHelper->release();
        m_pInterceptorHelper = NULL;
    }
}
//------------------------------------------------------------------------
void BibDataManager::InsertFields(const Reference< XFormComponent > & _rxGrid)
{
    if ( !_rxGrid.is() )
        return;

    try
    {
        Reference< XNameContainer > xColContainer( _rxGrid, UNO_QUERY );
        // remove the old fields
        if ( xColContainer->hasElements() )
        {
            Sequence< ::rtl::OUString > aNames = xColContainer->getElementNames();
            const ::rtl::OUString* pNames = aNames.getConstArray();
            const ::rtl::OUString* pNamesEnd = pNames + aNames.getLength();
            for ( ; pNames != pNamesEnd; ++pNames )
                xColContainer->removeByName( *pNames );
        }

        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return;

        Reference< XGridColumnFactory > xColFactory( _rxGrid, UNO_QUERY );

        Reference< XPropertySet >  xField;

        Sequence< rtl::OUString > aFields( xFields->getElementNames() );
        const rtl::OUString* pFields = aFields.getConstArray();
        const rtl::OUString* pFieldsEnd = pFields + aFields.getLength();

        for ( ; pFields != pFieldsEnd; ++pFields )
        {
            xFields->getByName( *pFields ) >>= xField;

            OUString sCurrentModelType;
            const OUString sType(C2U("Type"));
            sal_Int32 nType = 0;
            sal_Bool bIsFormatted           = sal_False;
            sal_Bool bFormattedIsNumeric    = sal_True;
            xField->getPropertyValue(sType) >>= nType;
            switch(nType)
            {
                case DataType::BIT:
                    sCurrentModelType = C2U("CheckBox");
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                    sCurrentModelType = C2U("TextField");
                    break;

                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                case DataType::CHAR:
                    bFormattedIsNumeric = sal_False;
                    // _NO_ break !
                default:
                    sCurrentModelType = C2U("FormattedField");
                    bIsFormatted = sal_True;
                    break;
            }

            Reference< XPropertySet >  xCurrentCol = xColFactory->createColumn(sCurrentModelType);
            if (bIsFormatted)
            {
                OUString sFormatKey(C2U("FormatKey"));
                xCurrentCol->setPropertyValue(sFormatKey, xField->getPropertyValue(sFormatKey));
                Any aFormatted(&bFormattedIsNumeric, ::getBooleanCppuType());
                xCurrentCol->setPropertyValue(C2U("TreatAsNumber"), aFormatted);
            }
            Any aColName = makeAny( *pFields );
            xCurrentCol->setPropertyValue(FM_PROP_CONTROLSOURCE,    aColName);
            xCurrentCol->setPropertyValue(FM_PROP_LABEL, aColName);

            xColContainer->insertByName( *pFields, makeAny( xCurrentCol ) );
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("Exception in BibDataManager::InsertFields")
    }
}
/* --------------------------------------------------

 --------------------------------------------------*/
Reference< awt::XControlModel > BibDataManager::updateGridModel()
{
    return updateGridModel( m_xForm );
}
/* --------------------------------------------------

 --------------------------------------------------*/
Reference< awt::XControlModel > BibDataManager::updateGridModel(const Reference< XForm > & xDbForm)
{
    try
    {
        Reference< XPropertySet >  aFormPropSet( xDbForm, UNO_QUERY );
        rtl::OUString sName;
        aFormPropSet->getPropertyValue(C2U("Command")) >>= sName;

        if ( !m_xGridModel.is() )
        {
            m_xGridModel = createGridModel( gGridName );

            Reference< XNameContainer >  xNameCont(xDbForm, UNO_QUERY);
//          if (xNameCont->hasByName(sName))
//              xNameCont->removeByName(sName);
//
            xNameCont->insertByName( sName, makeAny( m_xGridModel ) );
        }

        // insert the fields
        Reference< XFormComponent > xFormComp( m_xGridModel, UNO_QUERY );
        InsertFields( xFormComp );
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::updateGridModel: something went wrong !");
    }


    return m_xGridModel;
}
/* --------------------------------------------------

 --------------------------------------------------*/
Reference< XForm >  BibDataManager::createDatabaseForm(BibDBDescriptor& rDesc)
{
    Reference< XForm >  xResult;
    try
    {
        Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        m_xForm = Reference< XForm > ( xMgr->createInstance( C2U("com.sun.star.form.component.Form") ), UNO_QUERY );

        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );

        aDataSourceURL = rDesc.sDataSource;
        if(aPropertySet.is())
        {
            Any aVal;
            aVal <<= (sal_Int32)ResultSetType::SCROLL_INSENSITIVE;
            aPropertySet->setPropertyValue(C2U("ResultSetType"),aVal );
            aVal <<= (sal_Int32)ResultSetConcurrency::READ_ONLY;
            aPropertySet->setPropertyValue(C2U("ResultSetConcurrency"), aVal);

            //Caching for Performance
            aVal <<= (sal_Int32)50;
            aPropertySet->setPropertyValue(C2U("FetchSize"), aVal);

            Reference< XConnection >    xConnection = getConnection(rDesc.sDataSource);
            aVal <<= xConnection;
            aPropertySet->setPropertyValue(C2U("ActiveConnection"), aVal);

            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
            Reference< XNameAccess >  xTables = xSupplyTables.is() ?
                                xSupplyTables->getTables() : Reference< XNameAccess > ();

            Sequence<rtl::OUString> aTableNameSeq;
            if (xTables.is())
                aTableNameSeq = xTables->getElementNames();

            if(aTableNameSeq.getLength() > 0)
            {
                const rtl::OUString* pTableNames = aTableNameSeq.getConstArray();
                if(rDesc.sTableOrQuery.getLength())
                    aActiveDataTable = rDesc.sTableOrQuery;
                else
                {
                    rDesc.sTableOrQuery = aActiveDataTable = pTableNames[0];
                    rDesc.nCommandType = CommandType::TABLE;
                }

                aVal <<= aActiveDataTable;
                aPropertySet->setPropertyValue(C2U("Command"), aVal);
                aVal <<= rDesc.nCommandType;
                aPropertySet->setPropertyValue(C2U("CommandType"), aVal);


                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                aQuoteChar = xMetaData->getIdentifierQuoteString();

                Reference< sdb::XSQLQueryComposerFactory >  xFactory(xConnection, UNO_QUERY);
                m_xParser = xFactory->createQueryComposer();

                rtl::OUString aString(C2U("SELECT * FROM "));
                sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
                sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);
                aString += ::dbtools::quoteTableName(xMetaData,aActiveDataTable,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
                m_xParser->setQuery(aString);
                BibConfig* pConfig = BibModul::GetConfig();
                pConfig->setQueryField(getQueryField());
                startQueryWith(pConfig->getQueryText());

                xResult = m_xForm;
            }
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::createDatabaseForm: something went wrong !");
    }


    return xResult;
}
//------------------------------------------------------------------------
Sequence<rtl::OUString> BibDataManager::getDataSources()
{
    Sequence<rtl::OUString> aTableNameSeq;

    try
    {
        Reference< XTablesSupplier >  xSupplyTables( getConnection( m_xForm ), UNO_QUERY );
        Reference< XNameAccess >  xTables;
        if (xSupplyTables.is())
            xTables = xSupplyTables->getTables();
        if (xTables.is())
            aTableNameSeq = xTables->getElementNames();
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::getDataSources: something went wrong !");
    }


    return aTableNameSeq;
}
//------------------------------------------------------------------------
rtl::OUString BibDataManager::getActiveDataTable()
{
    return aActiveDataTable;
}
//------------------------------------------------------------------------
void BibDataManager::setFilter(const rtl::OUString& rQuery)
{
    if(!m_xParser.is())
        return;
    try
    {
        m_xParser->setFilter(rQuery);
        rtl::OUString aQuery=m_xParser->getFilter();
        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );
        Any aVal; aVal <<= aQuery;
        aPropertySet->setPropertyValue(C2U("Filter"), aVal);
        BOOL bVal = sal_True;
        aVal.setValue(&bVal, ::getBooleanCppuType());
        aPropertySet->setPropertyValue(C2U("ApplyFilter"), aVal);
        reload();
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::setFilterOnActiveDataSource: something went wrong !");
    }


}
//------------------------------------------------------------------------
rtl::OUString BibDataManager::getFilter()
{

    rtl::OUString aQueryString;
    try
    {
        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );
        Any aQuery=aPropertySet->getPropertyValue(C2U("Filter"));

        if(aQuery.getValueType() == ::getCppuType((OUString*)0))
        {
            aQueryString=*(OUString*)aQuery.getValue();
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::getFilterOnActiveDataSource: something went wrong !");
    }


    return aQueryString;

}
//------------------------------------------------------------------------
Sequence<rtl::OUString> BibDataManager::getQueryFields()
{
    Sequence<rtl::OUString> aFieldSeq;
    Reference< XNameAccess >  xFields = getColumns( m_xForm );
    if (xFields.is())
        aFieldSeq = xFields->getElementNames();
    return aFieldSeq;
}
//------------------------------------------------------------------------
rtl::OUString BibDataManager::getQueryField()
{
    BibConfig* pConfig = BibModul::GetConfig();
    OUString aFieldString = pConfig->getQueryField();
    if(!aFieldString.getLength())
    {
        Sequence<rtl::OUString> aSeq = getQueryFields();
        const rtl::OUString* pFields = aSeq.getConstArray();
        if(aSeq.getLength()>0)
        {
            aFieldString=pFields[0];
        }
    }
    return aFieldString;
}
//------------------------------------------------------------------------
void BibDataManager::startQueryWith(const OUString& rQuery)
{
    BibConfig* pConfig = BibModul::GetConfig();
    pConfig->setQueryText( rQuery );

    rtl::OUString aQueryString;
    if(rQuery.getLength()>0)
    {
        aQueryString=aQuoteChar;
        aQueryString+=getQueryField();
        aQueryString+=aQuoteChar;
        aQueryString+=C2U(" like '");
        String sQuery(rQuery);
        sQuery.SearchAndReplaceAll('?','_');
        sQuery.SearchAndReplaceAll('*','%');
        aQueryString += sQuery;
        aQueryString+=C2U("%'");
    }
    setFilter(aQueryString);
}
/* -----------------03.12.99 15:05-------------------

 --------------------------------------------------*/
void BibDataManager::setActiveDataSource(const rtl::OUString& rURL)
{
    rtl::OUString uTable;
    rtl::OUString sTmp(aDataSourceURL);
    aDataSourceURL = rURL;

    Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );
    if(aPropertySet.is())
    {
        unload();

        Reference< XComponent >  xOldConnection;
        aPropertySet->getPropertyValue(C2U("ActiveConnection")) >>= xOldConnection;

        Reference< XConnection >    xConnection = getConnection(rURL);
        if(!xConnection.is())
        {
            aDataSourceURL = sTmp;
            return;
        }
        Any aVal; aVal <<= xConnection;
        aPropertySet->setPropertyValue(C2U("ActiveConnection"), aVal);
        Reference< sdb::XSQLQueryComposerFactory >  xFactory(xConnection, UNO_QUERY);
        m_xParser = xFactory->createQueryComposer();

        if(xOldConnection.is())
            xOldConnection->dispose();

        Sequence<rtl::OUString> aTableNameSeq;
        Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
        if(xSupplyTables.is())
        {
            Reference< XNameAccess >  xAccess = xSupplyTables->getTables();
            aTableNameSeq = xAccess->getElementNames();
        }
        if(aTableNameSeq.getLength() > 0)
        {
            const rtl::OUString* pTableNames = aTableNameSeq.getConstArray();
            aActiveDataTable = pTableNames[0];
            aVal <<= aActiveDataTable;
            aPropertySet->setPropertyValue(C2U("Command"), aVal);
            aPropertySet->setPropertyValue(C2U("CommandType"), makeAny(CommandType::TABLE));
            //Caching for Performance
            aVal <<= (sal_Int32)50;
            aPropertySet->setPropertyValue(C2U("FetchSize"), aVal);
            rtl::OUString aString(C2U("SELECT * FROM "));
            // quote the table name which may contain catalog.schema.table
            Reference<XDatabaseMetaData> xMetaData(xConnection->getMetaData(),UNO_QUERY);
            aQuoteChar = xMetaData->getIdentifierQuoteString();
            sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
            sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);
            aString += ::dbtools::quoteTableName(xMetaData,aActiveDataTable,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
            m_xParser->setQuery(aString);
            BibConfig* pConfig = BibModul::GetConfig();
            pConfig->setQueryField(getQueryField());
            startQueryWith(pConfig->getQueryText());
            setActiveDataTable(aActiveDataTable);
        }
        FeatureStateEvent aEvent;
        util::URL aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        aEvent.FeatureDescriptor = getActiveDataTable();

        aEvent.State = makeAny( getDataSources() );

        if(pToolbar)
        {
            aURL.Complete =C2U(".uno:Bib/source");
            aEvent.FeatureURL = aURL;
            pToolbar->statusChanged( aEvent );
        }

        updateGridModel();
        load();
    }
}

/* --------------------------------------------------

 --------------------------------------------------*/
void BibDataManager::setActiveDataTable(const rtl::OUString& rTable)
{
    ResetIdentifierMapping();
    try
    {
        Reference< XPropertySet >  aPropertySet( m_xForm, UNO_QUERY );

        if(aPropertySet.is())
        {
            Reference< XConnection >    xConnection = getConnection( m_xForm );
            Reference< XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
            Reference< XNameAccess > xAccess = xSupplyTables->getTables();
            Sequence<rtl::OUString> aTableNameSeq = xAccess->getElementNames();
            sal_uInt32 nCount = aTableNameSeq.getLength();

            const rtl::OUString* pTableNames = aTableNameSeq.getConstArray();
            const rtl::OUString* pTableNamesEnd = pTableNames + nCount;

            for ( ; pTableNames != pTableNamesEnd; ++pTableNames )
            {
                if ( rTable == *pTableNames )
                {
                    aActiveDataTable = rTable;
                    Any aVal; aVal <<= rTable;
                    aPropertySet->setPropertyValue( C2U("Command"), aVal );
                    break;
                }
            }
            if (pTableNames != pTableNamesEnd)
            {
                Reference< XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                aQuoteChar = xMetaData->getIdentifierQuoteString();

                Reference< sdb::XSQLQueryComposerFactory >  xFactory(xConnection, UNO_QUERY);
                m_xParser = xFactory->createQueryComposer();

                rtl::OUString aString(C2U("SELECT * FROM "));
                sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
                sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);
                aString += ::dbtools::quoteTableName(xMetaData,aActiveDataTable,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);
                m_xParser->setQuery(aString);

                BibConfig* pConfig = BibModul::GetConfig();
                pConfig->setQueryField(getQueryField());
                startQueryWith(pConfig->getQueryText());

                BibDBDescriptor aDesc;
                aDesc.sDataSource = aDataSourceURL;
                aDesc.sTableOrQuery = aActiveDataTable;
                aDesc.nCommandType = CommandType::TABLE;
                BibModul::GetConfig()->SetBibliographyURL(aDesc);
            }
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::setActiveDataTable: something went wrong !");
    }

}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::load(  ) throw (RuntimeException)
{
    if ( isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable > xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::load: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        xFormAsLoadable->load();
        SetMeAsUidListener();

        EventObject aEvt( static_cast< XWeak* >( this ) );
        NOTIFY_LISTENERS( m_aLoadListeners, XLoadListener, loaded, aEvt );
    }
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::unload(  ) throw (RuntimeException)
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        EventObject aEvt( static_cast< XWeak* >( this ) );

        {
            NOTIFY_LISTENERS( m_aLoadListeners, XLoadListener, unloading, aEvt );
        }

        RemoveMeAsUidListener();
        xFormAsLoadable->unload();

        {
            NOTIFY_LISTENERS( m_aLoadListeners, XLoadListener, unloaded, aEvt );
        }
    }
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::reload(  ) throw (RuntimeException)
{
    if ( !isLoaded() )
        // nothing to do
        return;

    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::unload: invalid form!");
    if ( xFormAsLoadable.is() )
    {
        EventObject aEvt( static_cast< XWeak* >( this ) );

        {
            NOTIFY_LISTENERS( m_aLoadListeners, XLoadListener, reloading, aEvt );
        }

        xFormAsLoadable->reload();

        {
            NOTIFY_LISTENERS( m_aLoadListeners, XLoadListener, reloaded, aEvt );
        }
    }
}

//------------------------------------------------------------------------
sal_Bool SAL_CALL BibDataManager::isLoaded(  ) throw (RuntimeException)
{
    Reference< XLoadable >xFormAsLoadable( m_xForm, UNO_QUERY );
    DBG_ASSERT( xFormAsLoadable.is() || !m_xForm.is(), "BibDataManager::isLoaded: invalid form!");

    sal_Bool bLoaded = sal_False;
    if ( xFormAsLoadable.is() )
        bLoaded = xFormAsLoadable->isLoaded();
    return bLoaded;
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::addLoadListener( const Reference< XLoadListener >& aListener ) throw (RuntimeException)
{
    m_aLoadListeners.addInterface( aListener );
}

//------------------------------------------------------------------------
void SAL_CALL BibDataManager::removeLoadListener( const Reference< XLoadListener >& aListener ) throw (RuntimeException)
{
    m_aLoadListeners.removeInterface( aListener );
}

//------------------------------------------------------------------------
Reference< awt::XControlModel > BibDataManager::createGridModel(const rtl::OUString& rName)
{
    Reference< awt::XControlModel > xModel;

    try
    {
        // create the control model
        Reference< XMultiServiceFactory >  xMgr = ::comphelper::getProcessServiceFactory();
        Reference< XInterface >  xObject = xMgr->createInstance(C2U("com.sun.star.form.component.GridControl"));
        xModel=Reference< awt::XControlModel > ( xObject, UNO_QUERY );

        // set the
        Reference< XPropertySet > xPropSet( xModel, UNO_QUERY );
        xPropSet->setPropertyValue( C2U("Name"), makeAny( rName ) );

        // set the name of the to-be-created control
        rtl::OUString aControlName(C2U("com.sun.star.form.control.InteractionGridControl"));
        Any aAny; aAny <<= aControlName;
        xPropSet->setPropertyValue( C2U("DefaultControl"),aAny );

        // the the helpURL
        rtl::OUString uProp(C2U("HelpURL"));
        Reference< XPropertySetInfo > xPropInfo = xPropSet->getPropertySetInfo();
        if (xPropInfo->hasPropertyByName(uProp))
        {
            ::rtl::OUString sId = ::rtl::OUString::createFromAscii( "HID:" );
            sId += ::rtl::OUString::valueOf( (sal_Int32) HID_BIB_DB_GRIDCTRL );
            xPropSet->setPropertyValue( uProp, makeAny( ::rtl::OUString( sId ) ) );
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::createGridModel: something went wrong !");
    }


    return xModel;
}
//------------------------------------------------------------------------
rtl::OUString BibDataManager::getControlName(sal_Int32 nFormatKey )
{
    rtl::OUString aResStr;
    switch (nFormatKey)
    {
        case DataType::BIT:
            aResStr=C2U("CheckBox");
            break;
        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
            aResStr=C2U("NumericField");   ;
            break;
        case DataType::REAL:
        case DataType::DOUBLE:
        case DataType::NUMERIC:
        case DataType::DECIMAL:
            aResStr=C2U("FormattedField");
            break;
        case DataType::TIMESTAMP:
            aResStr=C2U("FormattedField");
            break;
        case DataType::DATE:
            aResStr=C2U("DateField");
            break;
        case DataType::TIME:
            aResStr=C2U("TimeField");
            break;
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
        default:
            aResStr=C2U("TextField");
            break;
    }
    return aResStr;
}
//------------------------------------------------------------------------
Reference< awt::XControlModel > BibDataManager::loadControlModel(
                    const rtl::OUString& rName, sal_Bool bForceListBox)
{
    Reference< awt::XControlModel > xModel;
    rtl::OUString aName(C2U("View_"));
    aName += rName;

    try
    {
        Reference< XNameAccess >  xFields = getColumns( m_xForm );
        if (!xFields.is())
            return xModel;
        Reference< XPropertySet >  xField;

        Any aElement;

        if(xFields->hasByName(rName))
        {
            aElement = xFields->getByName(rName);
            aElement >>= xField;
            Reference< XPropertySetInfo >  xInfo = xField.is() ? xField->getPropertySetInfo() : Reference< XPropertySetInfo > ();

            OUString sCurrentModelType;
            const OUString sType(C2U("Type"));
            sal_Int32 nFormatKey = 0;
            sal_Bool bIsFormatted           = sal_False;
            sal_Bool bFormattedIsNumeric    = sal_True;
            xField->getPropertyValue(sType) >>= nFormatKey;

            rtl::OUString aInstanceName(C2U("com.sun.star.form.component."));

            if (bForceListBox)
                aInstanceName += C2U("ListBox");
            else
                aInstanceName += getControlName(nFormatKey);

            Reference< XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
            Reference< XInterface >  xObject = xMgr->createInstance(aInstanceName);
            xModel=Reference< awt::XControlModel > ( xObject, UNO_QUERY );
            Reference< XPropertySet >  xPropSet( xModel, UNO_QUERY );
            Any aFieldName; aFieldName <<= aName;
            xPropSet->setPropertyValue( FM_PROP_NAME,aFieldName);

            xPropSet->setPropertyValue(FM_PROP_CONTROLSOURCE, makeAny( rName ) );

            Reference< XFormComponent >  aFormComp(xModel,UNO_QUERY );

            Reference< XNameContainer >  xNameCont( m_xForm, UNO_QUERY );
            xNameCont->insertByName(aName, makeAny( aFormComp ) );

            // now if the form where we inserted the new model is already loaded, notify the model of this
            // Note that this implementation below is a HACK as it relies on the fact that the model adds itself
            // as load listener to it's parent, which is an implementation detail of the model.
            //
            // the better solution would be the following:
            // in the current scenario, we insert a control model into a form. This results in the control model
            // adding itself as load listener to the form. Now, the form should realize that it's already loaded
            // and notify the model (which it knows as XLoadListener only) immediately. This seems to make sense.
            // (as an anologon to the XStatusListener semantics).
            //
            // But this would be way too risky for this last-day fix here.
            // 97140 - 30.01.2002 - fs@openoffice.org
            Reference< XLoadable > xLoad( m_xForm, UNO_QUERY );
            if ( xLoad.is() && xLoad->isLoaded() )
            {
                Reference< XLoadListener > xListener( aFormComp, UNO_QUERY );
                if ( xListener.is() )
                {
                    EventObject aLoadSource;
                    aLoadSource.Source = xLoad;
                    xListener->loaded( aLoadSource );
                }
            }
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::loadControlModel: something went wrong !");
    }
    return xModel;
}
//------------------------------------------------------------------------
void BibDataManager::saveCtrModel(const rtl::OUString& rName,const Reference< awt::XControlModel > & rCtrModel)
{
    if(m_xSourceProps.is())
    {
        try
        {
            rtl::OUString aName(C2U("View_"));
            aName+=rName;

            Reference< io::XPersistObject >  aPersistObject(rCtrModel, UNO_QUERY );

            Any aModel(&aPersistObject, ::getCppuType((Reference<io::XPersistObject>*)0));

            m_xSourceProps->setPropertyValue(aName,aModel);
        }
        catch(Exception& e )
        {
            e;  // make compiler happy
            DBG_ERROR("::saveCtrModel: something went wrong !");
        }
    }

}
//------------------------------------------------------------------------
void BibDataManager::disposing( const EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException )
{
    // not interested in
}

//------------------------------------------------------------------------
void BibDataManager::propertyChange(const beans::PropertyChangeEvent& evt) throw( RuntimeException )
{
    try
    {
        sal_Bool bFlag=sal_False;
        if(evt.PropertyName == FM_PROP_VALUE)
        {
            if( evt.NewValue.getValueType() == ::getCppuType((Reference<io::XInputStream>*)0) )
            {
                Reference< io::XDataInputStream >  xStream(
                    *(const Reference< io::XInputStream > *)evt.NewValue.getValue(), UNO_QUERY );
                aUID <<= xStream->readUTF();
            }
            else
                aUID = evt.NewValue;

            Reference< XRowLocate > xLocate(xBibCursor, UNO_QUERY);
            DBG_ASSERT(xLocate.is(), "BibDataManager::propertyChange : invalid cursor !");
            bFlag = xLocate->moveToBookmark(aUID);
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("::propertyChange: something went wrong !");
    }


}
//------------------------------------------------------------------------
void BibDataManager::SetMeAsUidListener()
{
try
{
    Reference< XNameAccess >  xFields = getColumns( m_xForm );
    if (!xFields.is())
        return;

    Sequence<rtl::OUString> aFields(xFields->getElementNames());
    const rtl::OUString* pFields = aFields.getConstArray();
    sal_Int32 nCount=aFields.getLength();
    String StrUID(C2S(STR_UID));
    rtl::OUString theFieldName;
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        String aName= pFields[i];

        if(aName.EqualsIgnoreCaseAscii(StrUID))
        {
            theFieldName=pFields[i];
            break;
        }
    }

    if(theFieldName.getLength()>0)
    {
        Reference< XPropertySet >  xPropSet;
        Any aElement;

        aElement = xFields->getByName(theFieldName);
        xPropSet = *(Reference< XPropertySet > *)aElement.getValue();

        xPropSet->addPropertyChangeListener(FM_PROP_VALUE, this);
    }

}
catch(Exception& e )
{
    e;  // make compiler happy
    DBG_ERROR("Exception in BibDataManager::SetMeAsUidListener")
}


}
//------------------------------------------------------------------------
void BibDataManager::RemoveMeAsUidListener()
{
try
{
    Reference< XNameAccess >  xFields = getColumns( m_xForm );
    if (!xFields.is())
        return;


    Sequence<rtl::OUString> aFields(xFields->getElementNames());
    const rtl::OUString* pFields = aFields.getConstArray();
    sal_Int32 nCount=aFields.getLength();
    String StrUID(C2S(STR_UID));
    rtl::OUString theFieldName;
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        String aName= pFields[i];

        if(aName.EqualsIgnoreCaseAscii(StrUID))
        {
            theFieldName=pFields[i];
            break;
        }
    }

    if(theFieldName.getLength()>0)
    {
        Reference< XPropertySet >  xPropSet;
        Any aElement;

        aElement = xFields->getByName(theFieldName);
        xPropSet = *(Reference< XPropertySet > *)aElement.getValue();

        xPropSet->removePropertyChangeListener(FM_PROP_VALUE, this);
    }

}
catch(Exception& e )
{
    e;  // make compiler happy
    DBG_ERROR("Exception in BibDataManager::RemoveMeAsUidListener")
}


}
/* -----------------02.11.99 16:11-------------------

 --------------------------------------------------*/
sal_Bool BibDataManager::moveRelative(sal_Int32 nCount)
{
    sal_Bool bRet = sal_False;
    try
    {
        Reference< XResultSet >  xCursor( m_xForm, UNO_QUERY );
        Reference< XResultSetUpdate >  xCursorUpdate( m_xForm, UNO_QUERY );
        if (xCursor.is() && xCursorUpdate.is())
        {
            bRet = xCursor->relative(nCount);
            if (xCursor->isAfterLast())
                xCursorUpdate->moveToInsertRow();
        }
    }
    catch(Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR("Exception in BibDataManager::moveRelative")
    }

    return bRet;
}
/* -----------------11.11.99 15:51-------------------

 --------------------------------------------------*/
void BibDataManager::CreateMappingDialog(Window* pParent)
{
    MappingDialog_Impl* pDlg = new MappingDialog_Impl(pParent, this);
    if(RET_OK == pDlg->Execute() && pBibView)
    {
        reload();
//      unload();
//      pBibView->UpdatePages();
//      load();
    }
    delete pDlg;
}
/* --------------------------------------------------

 --------------------------------------------------*/
rtl::OUString BibDataManager::CreateDBChangeDialog(Window* pParent)
{
    rtl::OUString uRet;
    DBChangeDialog_Impl * pDlg = new DBChangeDialog_Impl(pParent, this );
    if(RET_OK == pDlg->Execute())
    {
        String sNewURL = pDlg->GetCurrentURL();
        if(sNewURL != String(getActiveDataSource()))
        {
            uRet = sNewURL;
        }
    }
    delete pDlg;
    return uRet;
}
/*-- 18.05.2004 15:20:15---------------------------------------------------

  -----------------------------------------------------------------------*/
void BibDataManager::DispatchDBChangeDialog()
{
    if(pToolbar)
        pToolbar->SendDispatch(TBC_BT_CHANGESOURCE, Sequence< PropertyValue >());
}
/* -----------------06.12.99 15:11-------------------

 --------------------------------------------------*/
const OUString& BibDataManager::GetIdentifierMapping()
{
    if(!sIdentifierMapping.getLength())
    {
        BibConfig* pConfig = BibModul::GetConfig();
        BibDBDescriptor aDesc;
        aDesc.sDataSource = getActiveDataSource();
        aDesc.sTableOrQuery = getActiveDataTable();
        aDesc.nCommandType = CommandType::TABLE;
        const Mapping* pMapping = pConfig->GetMapping(aDesc);
        sIdentifierMapping = pConfig->GetDefColumnName(IDENTIFIER_POS);
        if(pMapping)
        {
            for(sal_uInt16 nEntry = 0; nEntry < COLUMN_COUNT; nEntry++)
            {
                if(pMapping->aColumnPairs[nEntry].sLogicalColumnName == sIdentifierMapping)
                {
                    sIdentifierMapping = pMapping->aColumnPairs[nEntry].sRealColumnName;
                    break;
                }
            }
        }
    }
    return sIdentifierMapping;
}
/* -----------------------------20.11.00 10:31--------------------------------

 ---------------------------------------------------------------------------*/
void BibDataManager::SetToolbar(BibToolBar* pSet)
{
    pToolbar = pSet;
    if(pToolbar)
        pToolbar->SetDatMan(*this);
}
/* -----------------------------08.05.2002 09:26------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< form::XFormController > BibDataManager::GetFormController()
{
    if(!m_xFormCtrl.is())
    {
        Reference< lang::XMultiServiceFactory > xMgr = comphelper::getProcessServiceFactory();
        m_xFormCtrl = uno::Reference< form::XFormController > (
            xMgr->createInstance(C2U("com.sun.star.form.FormController")), UNO_QUERY);
        m_xFormCtrl->setModel(uno::Reference< awt::XTabControllerModel > (getForm(), UNO_QUERY));
        // #100312# -------------
        m_xFormDispatch = uno::Reference< frame::XDispatch > ( m_xFormCtrl, UNO_QUERY);
    }
    return m_xFormCtrl;
}

// #100312# ----------
void BibDataManager::RegisterInterceptor( ::bib::BibBeamer* pBibBeamer)
{
    DBG_ASSERT( !m_pInterceptorHelper, "BibDataManager::RegisterInterceptor: called twice!" );

    if( pBibBeamer )
        m_pInterceptorHelper = new BibInterceptorHelper( pBibBeamer, m_xFormDispatch);
    if( m_pInterceptorHelper )
        m_pInterceptorHelper->acquire();
}

/*-- 18.05.2004 17:04:20---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool BibDataManager::HasActiveConnection()const
{
    sal_Bool bRet = sal_False;
    Reference< XPropertySet >   xPrSet( m_xForm, UNO_QUERY );
    if( xPrSet.is() )
    {
        Reference< XComponent >  xConnection;
        xPrSet->getPropertyValue(C2U("ActiveConnection")) >>= xConnection;
        bRet = xConnection.is();
    }
    return bRet;
}
/*-- 04.06.2004 14:37:29---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool BibDataManager::HasActiveConnection()
{
    return getConnection( m_xForm ).is();
}

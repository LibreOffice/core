/*************************************************************************
 *
 *  $RCSfile: datman.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-11-14 08:43:41 $
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
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
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
#ifndef _COM_SUN_STAR_DATA_XDATABASEFAVORITES_HPP_
#include <com/sun/star/data/XDatabaseFavorites.hpp>     // TODO : this is obsolete ....
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

#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#include <svtools/iniprop.hxx>
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
#ifndef _BIB_REGISTRY_HXX
#include "registry.hxx"
#endif
#ifndef _BIB_FMPROP_HRC
#include "bibprop.hrc"
#endif
#ifndef _BIB_TOOLBAR_HXX
#include "toolbar.hxx"
#endif
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ucb;
using namespace ::rtl;
using namespace ::ucb;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)
#define MAP_TOKEN ';'
#define PAIR_TOKEN ':'


/* -----------------------------15.06.00 16:12--------------------------------

 ---------------------------------------------------------------------------*/
BOOL lcl_IsCaseSensitive(const String& rPathURL)
{
    BOOL bCaseSensitive = FALSE;
#ifdef DBG_UTIL
    static BOOL bFirstCall = TRUE;
    if(bFirstCall)
    {
        DBG_ERROR("No case checking due to #76253#")
        bFirstCall = FALSE;
    }
#endif
/*  Reference< lang::XMultiServiceFactory > xMSF = utl::getProcessServiceFactory();
    INetURLObject aTempObj(rPathURL);
    try
    {
        aTempObj.SetBase(aTempObj.GetBase().ToLowerAscii());
        Reference<XContentIdentifier> xRef1 = new ::ucb::ContentIdentifier( xMSF, aTempObj.GetMainURL());
        aTempObj.SetBase(aTempObj.GetBase().ToUpperAscii());
        Reference<XContentIdentifier> xRef2 = new ::ucb::ContentIdentifier( xMSF, aTempObj.GetMainURL());

        ContentBroker& rBroker = *ContentBroker::get();
        Reference<XContentProvider > xProv = rBroker.getContentProviderInterface();
        sal_Int32 nCompare = xProv->compareContentIds( xRef1, xRef2 );
        bCaseSensitive = nCompare != 0;
    }
    catch(Exception& rEx){}
*/  return bCaseSensitive;
}
/* -----------------17.01.00 14:38-------------------

 --------------------------------------------------*/
Reference< sdbc::XConnection >  getConnection(const rtl::OUString& _rURL)
{
    // first get the sdb::DataSource corresponding to the url
    Reference< sdbc::XDataSource >  xDataSource;
    // is it a favorite title ?
    Reference< lang::XMultiServiceFactory >  xMgr = utl::getProcessServiceFactory();
    Reference<XInterface> xNamingContextIfc = xMgr->createInstance(C2U("com.sun.star.sdb.DatabaseContext"));
    Reference< XNameAccess >  xNamingContext(xNamingContextIfc, UNO_QUERY);
    if (xNamingContext.is() && xNamingContext->hasByName(_rURL))
    {
        DBG_ASSERT(Reference< XNamingService > (xNamingContext, UNO_QUERY).is(), "::getDataSource : no NamingService interface on the sdb::DatabaseAccessContext !");
        xDataSource = Reference< sdbc::XDataSource > (Reference< XNamingService > (xNamingContext, UNO_QUERY)->getRegisteredObject(_rURL), UNO_QUERY);
    }
    // build the connection from the data source
    Reference< sdbc::XConnection >  xConn;
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
        catch(sdbc::SQLException&)
        {
            // TODO : a real error handling
        }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
        {
        }

    }
    return xConn;
}

/* -----------------17.01.00 14:46-------------------

 --------------------------------------------------*/
Reference< sdbc::XConnection >  getConnection(const Reference< XInterface > & xRowSet)
{
    Reference< sdbc::XConnection >  xConn;
    try
    {
        Reference< XPropertySet >  xFormProps(xRowSet, UNO_QUERY);
        if (!xFormProps.is())
            return xConn;

        xConn = Reference< sdbc::XConnection > (*(Reference< XInterface > *)xFormProps->getPropertyValue(C2U("ActiveConnection")).getValue(), UNO_QUERY);
        if (!xConn.is())
        {
            OUString uTmp;
            xFormProps->getPropertyValue(C2U("DataSourceName")) >>= uTmp;
            xConn = getConnection(uTmp);
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("exception in getConnection");
    }

    return xConn;
}
/* -----------------17.01.00 16:07-------------------

 --------------------------------------------------*/
Reference< XNameAccess >  getColumns(const Reference< XForm > & xForm)
{
    Reference< XNameAccess >  xReturn;
    // check if the form is alive
    Reference< sdbcx::XColumnsSupplier >  xSupplyCols(xForm, UNO_QUERY);
    if (xSupplyCols.is())
        xReturn = xSupplyCols->getColumns();

    if (!xReturn.is() || (xReturn->getElementNames().getLength() == 0))
    {   // no ....
        xReturn = NULL;
        // -> get the table the form is bound to and ask it for their columns
        Reference< sdbcx::XTablesSupplier >  xSupplyTables(getConnection(xForm), UNO_QUERY);
        Reference< XPropertySet >  xFormProps(xForm, UNO_QUERY);
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
                    xSupplyCols = Reference< sdbcx::XColumnsSupplier > (
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
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;

    GroupBox        aMapGB;
    FixedText       aIdentifierFT;
    ListBox         aIdentifierLB;
    FixedText       aAuthorityTypeFT;
    ListBox         aAuthorityTypeLB;
    FixedText       aAuthorFT;
    ListBox         aAuthorLB;
    FixedText       aTitleFT;
    ListBox         aTitleLB;
    FixedText       aMonthFT;
    ListBox         aMonthLB;
    FixedText       aYearFT;
    ListBox         aYearLB;
    FixedText       aISBNFT;
    ListBox         aISBNLB;
    FixedText       aBooktitleFT;
    ListBox         aBooktitleLB;
    FixedText       aChapterFT;
    ListBox         aChapterLB;
    FixedText       aEditionFT;
    ListBox         aEditionLB;
    FixedText       aEditorFT;
    ListBox         aEditorLB;
    FixedText       aHowpublishedFT;
    ListBox         aHowpublishedLB;
    FixedText       aInstitutionFT;
    ListBox         aInstitutionLB;
    FixedText       aJournalFT;
    ListBox         aJournalLB;
    FixedText       aNoteFT;
    ListBox         aNoteLB;
    FixedText       aAnnoteFT;
    ListBox         aAnnoteLB;
    FixedText       aNumberFT;
    ListBox         aNumberLB;
    FixedText       aOrganizationsFT;
    ListBox         aOrganizationsLB;
    FixedText       aPagesFT;
    ListBox         aPagesLB;
    FixedText       aPublisherFT;
    ListBox         aPublisherLB;
    FixedText       aAddressFT;
    ListBox         aAddressLB;
    FixedText       aSchoolFT;
    ListBox         aSchoolLB;
    FixedText       aSeriesFT;
    ListBox         aSeriesLB;
    FixedText       aReportTypeFT;
    ListBox         aReportTypeLB;
    FixedText       aVolumeFT;
    ListBox         aVolumeLB;
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

    ListBox*        aListBoxes[COLUMN_COUNT];
    String          sNone;

    BibDataManager* pDatMan;

    DECL_LINK(OkHdl, OKButton*);
    DECL_LINK(ListBoxSelectHdl, ListBox*);

public:
    MappingDialog_Impl(Window* pParent, BibDataManager* pDatMan);
    ~MappingDialog_Impl();

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
    sNone(ResId(ST_NONE))
{
    FreeResource();

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
    Reference< XNameAccess >  xFields = getColumns(pDatMan->getDatabaseForm());
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
    return 0;
}
/* -----------------12.11.99 14:50-------------------

 --------------------------------------------------*/
IMPL_LINK(MappingDialog_Impl, OkHdl, OKButton*, EMPTYARG)
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
    EndDialog(RET_OK);
    return 0;
}
/* -----------------18.11.99 10:23-------------------

 --------------------------------------------------*/
class DBChangeDialog_Impl : public ModalDialog
{
    OKButton        aOKBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBT;

    GroupBox        aSelectionGB;
    SvTabListBox    aSelectionLB;
    HeaderBar       aSelectionHB;

    String          aEntryST;
    String          aURLST;

    BibDataManager* pDatMan;

    DECL_LINK(EndDragHdl, HeaderBar*);
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
    aSelectionHB.SetEndDragHdl( LINK(this, DBChangeDialog_Impl, EndDragHdl));
    aSelectionLB.SetDoubleClickHdl( LINK(this, DBChangeDialog_Impl, DoubleClickHdl));
    try
    {
        Reference< lang::XMultiServiceFactory >  xMgr = utl::getProcessServiceFactory();
        Reference< data::XDatabaseFavorites >  xFav(xMgr->createInstance( C2U("com.sun.star.data.DatabaseEngine") ), UNO_QUERY );
        // TODO : XDatabaseFavorites is an obsolete interface, the whole dialog has to be based on
        // the sdb::DatabaseAccessContext service

        Sequence< beans::PropertyValue > aFavs = xFav->getFavorites();
        const beans::PropertyValue* pValues = aFavs.getConstArray();

        Size aSize = aSelectionHB.GetSizePixel();
        long nTabs[3];
        nTabs[0] = 2;// Number of Tabs
        nTabs[1] = 0;
        nTabs[2] = aSize.Width() / 4;

        aSelectionHB.SetStyle(aSelectionHB.GetStyle()|WB_STDHEADERBAR);
        aSelectionHB.InsertItem( 1, aEntryST, nTabs[2]);
        aSelectionHB.InsertItem( 2, aURLST,  aSize.Width() - nTabs[2]);
        aSelectionHB.SetSizePixel(aSelectionHB.CalcWindowSizePixel());
        aSelectionHB.Show();

        aSelectionLB.SetHelpId(HID_SELECTION_TLB);
        aSelectionLB.SetTabs( &nTabs[0], MAP_PIXEL );
        aSelectionLB.SetWindowBits(WB_CLIPCHILDREN|WB_SORT);
        //aSelectionLB.SetSelectHdl(LINK(this, SwGlossaryGroupDlg, SelectHdl));
        aSelectionLB.GetModel()->SetSortMode(SortAscending);

        String sActiveURL = pDatMan->getActiveDataSource();
        sActiveURL = URIHelper::SmartRelToAbs(sActiveURL);
        for(int i = 0; i < aFavs.getLength(); i++)
        {
            String sTemp(pValues[i].Name);
            sTemp += '\t';
            String sSource = *(OUString*)pValues[i].Value.getValue();
            sTemp += sSource;
            SvLBoxEntry* pEntry = aSelectionLB.InsertEntry(sTemp);
//          String sTempPath = INetURLObject(sSource).PathToFileName();
            sal_Bool bCaseSensitive = lcl_IsCaseSensitive(sSource);
//          sal_Bool bCaseSensitive = DirEntry(sTempPath).IsCaseSensitive();
            if((bCaseSensitive && sActiveURL == sSource)||
                !bCaseSensitive && COMPARE_EQUAL == sActiveURL.CompareTo(sSource))
            {
                aSelectionLB.Select(pEntry);
            }
        }
        aSelectionLB.GetModel()->Resort();

    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
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
IMPL_LINK(DBChangeDialog_Impl, EndDragHdl, HeaderBar*, pHB)
{
    long nTabs[3];
    nTabs[0] = 2;// Number of Tabs
    nTabs[1] = 0;
    nTabs[2] = pHB->GetItemSize( 1 );
    aSelectionLB.SetTabs( &nTabs[0], MAP_PIXEL );
    return 0;
};

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
        sRet = aSelectionLB.GetEntryText(pEntry, 1);
    }
    return sRet;
}

//-----------------------------------------------------------------------------
#define STR_UID "uid"
rtl::OUString gGridName(C2U("theGrid"));
rtl::OUString gViewName(C2U("theView"));
rtl::OUString gGlobalName(C2U("theGlobals"));
rtl::OUString gBeamerSize(C2U("theBeamerSize"));
rtl::OUString gViewSize(C2U("theViewSize"));

BibDataManager::BibDataManager(BibRegistry * pRegistry) :
    pToolbar(0),
    pGridWin(0)
{

//  nEditMode = DatabaseRecordMode_STANDARD;
    xRegistry = pRegistry;
    xGlobalProps=createGlobalProperties();

}
/* --------------------------------------------------

 --------------------------------------------------*/
BibDataManager::~BibDataManager()
{
    Reference< XLoadable >  xLoad(xForm, UNO_QUERY);
    Reference< lang::XComponent >  xComp(xForm, UNO_QUERY);
    if (xForm.is())
    {
        RemoveMeAsUidListener();
        if (xLoad.is())
            xLoad->unload();
        if (xComp.is())
            xComp->dispose();
        xForm = NULL;
    }
}
//------------------------------------------------------------------------
void BibDataManager::InsertFields(const Reference< XFormComponent > & xGrid)
{
    if (!xGrid.is())
        return;
try
{
    Reference< XNameAccess >  xFields = getColumns(xForm);
    if (!xFields.is())
        return;

    Reference< XGridColumnFactory >  xColFactory(xGrid, UNO_QUERY);
    Reference< XNameContainer >  xColContainer(xGrid, UNO_QUERY);

    Reference< XPropertySet >  xField;

    Sequence<rtl::OUString> aFields(xFields->getElementNames());
    const rtl::OUString* pFields = aFields.getConstArray();
    sal_Int32 nCount=aFields.getLength();
    Any aElement;
    for( sal_Int32 i=0; i<nCount; i++ )
    {
        rtl::OUString aCurrentColName= pFields[i];

        aElement = xFields->getByName(aCurrentColName);
        xField = *(Reference< XPropertySet > *)aElement.getValue();

        sal_Int32 nFormatKey = *(sal_Int32*)xField->getPropertyValue(FM_PROP_FORMATKEY).getValue();

        Reference< XPropertySet >  xCurrentCol = xColFactory->createColumn(getControlName(nFormatKey));

        Any aColName; aColName <<= aCurrentColName;
        xCurrentCol->setPropertyValue(FM_PROP_CONTROLSOURCE,    aColName);
        xCurrentCol->setPropertyValue(FM_PROP_LABEL, aColName);

        xColContainer->insertByName(aCurrentColName, Any(&xCurrentCol, ::getCppuType((Reference<XPropertySet>*)0)));
    }
}
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
{
    DBG_ERROR("Exception in BibDataManager::InsertFields")
}


}
/* --------------------------------------------------

 --------------------------------------------------*/
Reference< awt::XControlModel >  BibDataManager::createGridModel()
{
    return createGridModel(xForm);
}
/* --------------------------------------------------

 --------------------------------------------------*/
Reference< awt::XControlModel >  BibDataManager::createGridModel(const Reference< XForm > & xDbForm)
{
    try
    {
        Reference< XPropertySet >  aFormPropSet( xDbForm, UNO_QUERY );
        Any aDbSource=aFormPropSet->getPropertyValue(C2U("Command"));
        rtl::OUString aName=*(OUString*)aDbSource.getValue();

        //xGridModel=loadGridModel(aName);
          xGridModel=loadGridModel(gGridName);

        Reference< XNameContainer >  xNameCont(xDbForm, UNO_QUERY);

        if(xNameCont->hasByName(aName))
        {
            xNameCont->removeByName(aName);
        }

        xNameCont->insertByName(aName, Any(&xGridModel, ::getCppuType((Reference<XFormComponent>*)0)));

    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::createGridModel: something went wrong !");
    }


    return xGridModel;
}
/* --------------------------------------------------

 --------------------------------------------------*/
Reference< XForm >  BibDataManager::getDatabaseForm()
{
    return xForm;
}
/* --------------------------------------------------

 --------------------------------------------------*/
Reference< XForm >  BibDataManager::createDatabaseForm(BibDBDescriptor& rDesc)
{
    Reference< XForm >  xResult;
    try
    {
        Reference< lang::XMultiServiceFactory >  xMgr = utl::getProcessServiceFactory();
        xForm = Reference< XForm > (xMgr->createInstance( C2U("com.sun.star.form.component.Form") ),
                                                                            UNO_QUERY );

        Reference< XPropertySet >  aPropertySet(xForm, UNO_QUERY );

        aDataSourceURL = rDesc.sDataSource;
        if(aPropertySet.is())
        {
            Any aVal; aVal <<= rDesc.sDataSource;
            aPropertySet->setPropertyValue(C2U("DataSourceName"), aVal);
            aVal <<= (sal_Int32)sdbc::ResultSetType::SCROLL_INSENSITIVE;
            aPropertySet->setPropertyValue(C2U("ResultSetType"),aVal );
            aVal <<= (sal_Int32)sdbc::ResultSetConcurrency::READ_ONLY;
            aPropertySet->setPropertyValue(C2U("ResultSetConcurrency"), aVal);

            //Caching for Performance
            aVal <<= (sal_Int32)50;
            aPropertySet->setPropertyValue(C2U("FetchSize"), aVal);

            Reference< sdbc::XConnection >  xConnection = getConnection(rDesc.sDataSource);
            Reference< sdbcx::XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
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

                Reference< registry::XRegistryKey >  xRoot = xRegistry->getRootKey();
                Reference< registry::XRegistryKey >  xKey = xRoot->openKey(aActiveDataTable);
                if(!xKey.is())
                    xKey = xRoot->createKey(aActiveDataTable);

                xSourceProps=Reference< XPropertySet > (xKey, UNO_QUERY);

                aVal <<= aActiveDataTable;
                aPropertySet->setPropertyValue(C2U("Command"), aVal);
                aVal <<= rDesc.nCommandType;
                aPropertySet->setPropertyValue(C2U("CommandType"), aVal);


                Reference< sdbc::XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                aQuoteChar = xMetaData->getIdentifierQuoteString();

                Reference< sdb::XSQLQueryComposerFactory >  xFactory(xConnection, UNO_QUERY);
                xParser = xFactory->createQueryComposer();

                rtl::OUString aString(C2U("SELECT * FROM "));
                aString += aQuoteChar;
                aString += aActiveDataTable;
                aString += aQuoteChar;
                xParser->setQuery(aString);
                setQueryField(getQueryField());
                startQueryWith(getQueryString());

                xResult = xForm;
            }
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
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
        Reference< sdbcx::XTablesSupplier >  xSupplyTables(getConnection(xForm), UNO_QUERY);
        Reference< XNameAccess >  xTables;
        if (xSupplyTables.is())
            xTables = xSupplyTables->getTables();
        if (xTables.is())
            aTableNameSeq = xTables->getElementNames();
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
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
Reference< XSQLQueryComposer >  BibDataManager::getParser()
{
    return xParser;
}
//------------------------------------------------------------------------
void BibDataManager::setFilter(const rtl::OUString& rQuery)
{
    try
    {
        xParser->setFilter(rQuery);
        rtl::OUString aQuery=xParser->getFilter();
        Reference< XPropertySet >  aPropertySet(xForm, UNO_QUERY );
        Any aVal; aVal <<= aQuery;
        aPropertySet->setPropertyValue(C2U("Filter"), aVal);
        BOOL bVal = sal_True;
        aVal.setValue(&bVal, ::getBooleanCppuType());
        aPropertySet->setPropertyValue(C2U("ApplyFilter"), aVal);
        Reference< XLoadable >  xLoadable(xForm, UNO_QUERY );
        xLoadable->reload();
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::setFilterOnActiveDataSource: something went wrong !");
    }


}
//------------------------------------------------------------------------
rtl::OUString BibDataManager::getFilter()
{

    rtl::OUString aQueryString;
    try
    {
        Reference< XPropertySet >  aPropertySet(xForm, UNO_QUERY );
        Any aQuery=aPropertySet->getPropertyValue(C2U("Filter"));

        if(aQuery.getValueType() == ::getCppuType((OUString*)0))
        {
            aQueryString=*(OUString*)aQuery.getValue();
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::getFilterOnActiveDataSource: something went wrong !");
    }


    return aQueryString;

}
//------------------------------------------------------------------------
Sequence<rtl::OUString> BibDataManager::getQueryFields()
{
    Sequence<rtl::OUString> aFieldSeq;
    Reference< XNameAccess >  xFields = getColumns(xForm);
    if (xFields.is())
        aFieldSeq = xFields->getElementNames();
    return aFieldSeq;
}
//------------------------------------------------------------------------
void BibDataManager::setQueryField(const rtl::OUString& rQuery)
{
    try
    {
        Any aQuery; aQuery <<= rQuery;
        xSourceProps->setPropertyValue(C2U("QueryField"),aQuery);

    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("Exception in BibDataManager::setQueryField")
    }

}
//------------------------------------------------------------------------
rtl::OUString BibDataManager::getQueryField()
{
    rtl::OUString aFieldString;
    try
    {
        Any aField=xSourceProps->getPropertyValue(C2U("QueryField"));

        if(aField.getValueType() == ::getVoidCppuType())
        {
            Sequence<rtl::OUString> aSeq=getQueryFields();
            const rtl::OUString* pFields = aSeq.getConstArray();
            if(aSeq.getLength()>0)
            {
                aFieldString=pFields[0];
            }
        }
        else
        {
            aFieldString = *(OUString*)aField.getValue();
        }

    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("Exception in BibDataManager::getQueryField")
    }


    return aFieldString;
}
//------------------------------------------------------------------------
void BibDataManager::startQueryWith(const OUString& rQuery)
{
    try
    {
        Any aQuery;
        aQuery <<= rQuery;
        xSourceProps->setPropertyValue(C2U("QueryText"), aQuery);

        rtl::OUString aQueryString;
        if(rQuery.len()>0)
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
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("Exception in BibDataManager::StartQueryWith")
    }

}
//------------------------------------------------------------------------
rtl::OUString BibDataManager::getQueryString()
{
    rtl::OUString aQueryString;
    try
    {
        Any aQuery=xSourceProps->getPropertyValue(C2U("QueryText"));
        if(aQuery.getValueType() == ::getCppuType((OUString*)0))
        {
            aQueryString=*(OUString*)aQuery.getValue();
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("Exception in BibDataManager::getQueryString")
    }


    return aQueryString;
}
/* -----------------03.12.99 15:05-------------------

 --------------------------------------------------*/
void BibDataManager::setActiveDataSource(const rtl::OUString& rURL)
{
    //unloadDatabase();
    rtl::OUString uTable;
    aDataSourceURL = rURL;
    Reference< XPropertySet >  aPropertySet(xForm, UNO_QUERY );
    if(aPropertySet.is())
    {
        Any aVal; aVal <<= rURL;
        aPropertySet->setPropertyValue(C2U("DataSourceName"), aVal);
        Sequence<rtl::OUString> aTableNameSeq;
        Reference< sdbc::XConnection >  xConnection = getConnection(xForm);
        Reference< sdbcx::XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
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
            rtl::OUString aString(C2U("SELECT * FROM "));
            aString+=aQuoteChar;
            aString+=aActiveDataTable;
            aString+=aQuoteChar;
            xParser->setQuery(aString);
            setQueryField(getQueryField());
            startQueryWith(getQueryString());
            setActiveDataTable(aActiveDataTable);
        }
        FeatureStateEvent aEvent;
        util::URL aURL;
        aEvent.IsEnabled  = sal_True;
        aEvent.Requery    = sal_False;
        //aEvent.Source     = (XDispatch *) this;
        aEvent.FeatureDescriptor = getActiveDataTable();

        Sequence<rtl::OUString> aStringSeq = getDataSources();
        aEvent.State.setValue(&aStringSeq, ::getCppuType((Sequence<rtl::OUString>*)0));

        if(pToolbar)
        {
            aURL.Complete =C2U(".uno:Bib/source");
            aEvent.FeatureURL = aURL;
            pToolbar->statusChanged( aEvent );
        }
        if(pGridWin)
        {
            aURL.Complete =C2U(".uno:Bib/newGridModel");
            aEvent.FeatureURL = aURL;
            Reference< awt::XControlModel >  xModel = createGridModel();
            aEvent.State.setValue(&xModel, ::getCppuType((Reference<awt::XControlModel>*)0));
            pGridWin->statusChanged( aEvent );
        }
    }
}
/* --------------------------------------------------

 --------------------------------------------------*/
void BibDataManager::setActiveDataTable(const rtl::OUString& rTable)
{
    ResetIdentifierMapping();
    try
    {
        sal_Bool bFlag=sal_False;
        Reference< XPropertySet >  aPropertySet(xForm, UNO_QUERY );

        if(aPropertySet.is())
        {
            Reference< sdbc::XConnection >  xConnection = getConnection(xForm);
            Reference< sdbcx::XTablesSupplier >  xSupplyTables(xConnection, UNO_QUERY);
            Reference< XNameAccess >  xAccess = xSupplyTables->getTables();
            Sequence<rtl::OUString> aTableNameSeq = xAccess->getElementNames();
            sal_uInt32 nCount = aTableNameSeq.getLength();
            const rtl::OUString* pTableNames = aTableNameSeq.getConstArray();

            for(sal_Int32 i=0;i<nCount;i++)
            {
                if(rTable == pTableNames[i])
                {
                    aActiveDataTable=rTable;
                    Any aVal; aVal <<= rTable;
                    aPropertySet->setPropertyValue(C2U("Command"), aVal);
                    bFlag=sal_True;
                    break;
                }
            }
            if(bFlag)
            {
/*              if(xGridModel.is())
                {
                    saveGridModel(xGridModel);
                }*/

                Reference< registry::XRegistryKey >  xRoot = xRegistry->getRootKey();
                Reference< registry::XRegistryKey >  xKey = xRoot->openKey(aActiveDataTable);

                if(!xKey.is())
                    xKey = xRoot->createKey(aActiveDataTable);

                xSourceProps=Reference< XPropertySet > (xKey, UNO_QUERY);

                Reference< sdbc::XDatabaseMetaData >  xMetaData = xConnection->getMetaData();
                aQuoteChar = xMetaData->getIdentifierQuoteString();

                Reference< sdb::XSQLQueryComposerFactory >  xFactory(xConnection, UNO_QUERY);
                xParser = xFactory->createQueryComposer();

                rtl::OUString aString(C2U("SELECT * FROM "));
                aString+=aQuoteChar;
                aString+=aActiveDataTable;
                aString+=aQuoteChar;
                xParser->setQuery(aString);

                setQueryField(getQueryField());
                startQueryWith(getQueryString());

                BibDBDescriptor aDesc;
                aDesc.sDataSource = aDataSourceURL;
                aDesc.sTableOrQuery = aActiveDataTable;
                aDesc.nCommandType = CommandType::TABLE;
                BibModul::GetConfig()->SetBibliographyURL(aDesc);
            }
            if(pBibView)
                pBibView->UpdatePages();
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::setActiveDataTable: something went wrong !");
    }

}
//------------------------------------------------------------------------
void BibDataManager::loadDatabase()
{
    if(xForm.is())
    {
        Reference< XLoadable >  xFormAsLoadable(xForm, UNO_QUERY );
        DBG_ASSERT(xFormAsLoadable.is(), "BibDataManager::loadDatabase : invalid form !");
        xFormAsLoadable->load();
        xFormAsLoadable->reload();  // why ? may be very expensive !

        SetMeAsUidListener();
    }
}
//------------------------------------------------------------------------
void BibDataManager::unloadDatabase()
{
    if(xForm.is())
    {
        RemoveMeAsUidListener();
        Reference< XLoadable >  xFormAsLoadable(xForm, UNO_QUERY );
        DBG_ASSERT(xFormAsLoadable.is(), "BibDataManager::unloadDatabase : invalid form !");
        xFormAsLoadable->unload();
    }
}
//------------------------------------------------------------------------
Reference< awt::XControlModel >  BibDataManager::loadGridModel(const rtl::OUString& rName)
{
    Reference< awt::XControlModel >  xModel;

    try
    {
    Reference< lang::XMultiServiceFactory >  xMgr = utl::getProcessServiceFactory();
        Reference< XInterface >  xObject = xMgr->createInstance(C2U("com.sun.star.form.component.GridControl"));
        xModel=Reference< awt::XControlModel > ( xObject, UNO_QUERY );
        Reference< XPropertySet >  xPropSet( xModel, UNO_QUERY );
        Any aDbSource; aDbSource <<= rName;
        xPropSet->setPropertyValue( C2U("Name"),aDbSource);
        rtl::OUString aControlName(C2U("com.sun.star.form.control.ExtendedGridControl"));
        Any aAny; aAny <<= aControlName;
        xPropSet->setPropertyValue( C2U("DefaultControl"),aAny );

        Reference< XFormComponent >  aFormComp(xModel,UNO_QUERY );
        InsertFields(aFormComp);

        rtl::OUString uProp(C2U("HelpURL"));
        Reference< XPropertySetInfo >  xPropInfo = xPropSet->getPropertySetInfo();
        if(xPropInfo->hasPropertyByName(uProp))
        {
            String sId(C2S("HID:"));
            sId += HID_BIB_DB_GRIDCTRL;
            Any aVal; aVal <<= OUString(sId);
            xPropSet->setPropertyValue(uProp, aVal);
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::loadGridModel: something went wrong !");
    }


    return xModel;
}
//------------------------------------------------------------------------
/*void BibDataManager::saveGridModel(const Reference< awt::XControlModel > & rGridModel)
{
    try
    {
        Reference< XPersistObject >  aPersistObject(rGridModel, UNO_QUERY );

        Any aGridModel(&aPersistObject, ::getCppuType((const XPersistObject*)0));

        xSourceProps->setPropertyValue(gGridName,aGridModel);
    }
    catch(Exception& rEx)
    {
        DBG_ERROR("::saveGridModelData: something went wrong !");
    }

}*/
//------------------------------------------------------------------------
Reference< XPropertySet >   BibDataManager::createGlobalProperties()
{
    try
    {
        Reference< registry::XRegistryKey >  xRoot = xRegistry->getRootKey();
        Reference< registry::XRegistryKey >  xKey = xRoot->openKey(gGlobalName);
        if(!xKey.is())
            xKey = xRoot->createKey(gGlobalName);

        return Reference< XPropertySet > (xKey, UNO_QUERY);
    }

#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::getViewProperties: something went wrong !");
    }

    return Reference< XPropertySet > ();
}
//------------------------------------------------------------------------
void BibDataManager::setViewSize(long nSize)
{
    try
    {
        Any aViewSize; aViewSize <<= (sal_Int32) nSize;
        xGlobalProps->setPropertyValue(gViewSize,aViewSize);
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::setViewSize: something went wrong !");
    }

}
//------------------------------------------------------------------------
long BibDataManager::getViewSize()
{
    long nSize=0;
    try
    {
        Any aViewSize=xGlobalProps->getPropertyValue(gViewSize);

        if(aViewSize.getValueType() != ::getVoidCppuType())
        {
            aViewSize >>= nSize;
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::getViewSize: something went wrong !");
    }
    return nSize;
}
//------------------------------------------------------------------------
void BibDataManager::setBeamerSize(long nSize)
{
    try
    {
        Any aBeamerSize; aBeamerSize <<= (sal_Int32) nSize;
        xGlobalProps->setPropertyValue(gBeamerSize,aBeamerSize);
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::setBeamerSize: something went wrong !");
    }

}
//------------------------------------------------------------------------
long BibDataManager::getBeamerSize()
{
    long nSize=0;
    try
    {
        Any aBeamerSize=xGlobalProps->getPropertyValue(gBeamerSize);

        if(aBeamerSize.getValueType() != ::getVoidCppuType())
        {
            aBeamerSize >>= nSize;
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::getBeamerSize: something went wrong !");
    }
    return nSize;
}
//------------------------------------------------------------------------
Reference< XPropertySet >   BibDataManager::getViewProperties()
{
    try
    {
        Reference< registry::XRegistryKey >  xKey = Reference< registry::XRegistryKey > (xSourceProps, UNO_QUERY);
        Reference< registry::XRegistryKey >  xViewKey=xKey->openKey(gViewName);
        if(!xViewKey.is())
        {
            xViewKey=xKey->createKey(gViewName);
        }
        return Reference< XPropertySet > (xViewKey, UNO_QUERY);
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::getViewProperties: something went wrong !");
    }
    return Reference< XPropertySet > ();
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
Reference< awt::XControlModel >  BibDataManager::loadControlModel(const rtl::OUString& rName, sal_Bool bForceListBox)
{
    Reference< awt::XControlModel >  xModel;

    rtl::OUString aName(C2U("View_"));
    aName+=rName;

    try
    {
        Reference< io::XPersistObject >  aObject;
        Any aControlModel=xSourceProps->getPropertyValue(aName);
        if(aControlModel.getValueType() == ::getCppuType((Reference<io::XPersistObject>*)0))
        {
            aObject=*(Reference< io::XPersistObject > *)aControlModel.getValue();
        }

        if(!aObject.is() )
        {
            Reference< XNameAccess >  xFields = getColumns(xForm);
            if (!xFields.is())
                return xModel;
            Reference< XPropertySet >  xField;

            Any aElement;

            if(xFields->hasByName(rName))
            {
                aElement = xFields->getByName(rName);
                xField = *(Reference< XPropertySet > *)aElement.getValue();
                Reference< XPropertySetInfo >  xInfo = xField.is() ? xField->getPropertySetInfo() : Reference< XPropertySetInfo > ();
                sal_Int32 nFormatKey = 0;
                if (xInfo.is() && xInfo->hasPropertyByName(FM_PROP_FORMATKEY))
                    xField->getPropertyValue(FM_PROP_FORMATKEY) >>= nFormatKey;

                rtl::OUString aInstanceName(C2U("com.sun.star.form.component."));

                if (bForceListBox)
                    aInstanceName += C2U("ListBox");
                else
                    aInstanceName += getControlName(nFormatKey);

                Reference< lang::XMultiServiceFactory >  xMgr = utl::getProcessServiceFactory();
                Reference< XInterface >  xObject = xMgr->createInstance(aInstanceName);
                xModel=Reference< awt::XControlModel > ( xObject, UNO_QUERY );
                Reference< XPropertySet >  xPropSet( xModel, UNO_QUERY );
                Any aFieldName; aFieldName <<= aName;
                xPropSet->setPropertyValue( FM_PROP_NAME,aFieldName);

                Any aDbSource; aDbSource <<= rName;
                xPropSet->setPropertyValue(FM_PROP_CONTROLSOURCE,aDbSource);

                Reference< XFormComponent >  aFormComp(xModel,UNO_QUERY );

                Reference< XNameContainer >  xNameCont(xForm, UNO_QUERY);
                xNameCont->insertByName(aName, Any(&xModel, ::getCppuType((Reference<XFormComponent>*)0)));
            }
        }
        else
        {
            xModel=Reference< awt::XControlModel > (aObject,UNO_QUERY );
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::loadControlModel: something went wrong !");
    }
    return xModel;
}
//------------------------------------------------------------------------
void BibDataManager::saveCtrModel(const rtl::OUString& rName,const Reference< awt::XControlModel > & rCtrModel)
{
    try
    {
        rtl::OUString aName(C2U("View_"));
        aName+=rName;

        Reference< io::XPersistObject >  aPersistObject(rCtrModel, UNO_QUERY );

        Any aModel(&aPersistObject, ::getCppuType((Reference<io::XPersistObject>*)0));

        xSourceProps->setPropertyValue(aName,aModel);
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::saveCtrModel: something went wrong !");
    }

}
//------------------------------------------------------------------------
void BibDataManager::disposing( const lang::EventObject& Source )
{
    try
    {
//      if ( xForm.is())
//      {
//          Reference< XPropertySet >  aPropSet(xForm, UNO_QUERY );
//          aPropSet->removePropertyChangeListener(FM_PROP_EDITMODE, this);
//      }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::propertyChange: something went wrong !");
    }



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

            Reference< sdbcx::XRowLocate >  xLocate(xBibCursor, UNO_QUERY);
            DBG_ASSERT(xLocate.is(), "BibDataManager::propertyChange : invalid cursor !");
            bFlag = xLocate->moveToBookmark(aUID);
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
        DBG_ERROR("::propertyChange: something went wrong !");
    }


}
//------------------------------------------------------------------------
void BibDataManager::SetMeAsUidListener()
{
try
{
    Reference< XNameAccess >  xFields = getColumns(xForm);
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

    if(theFieldName.len()>0)
    {
        Reference< XPropertySet >  xPropSet;
        Any aElement;

        aElement = xFields->getByName(theFieldName);
        xPropSet = *(Reference< XPropertySet > *)aElement.getValue();

        xPropSet->addPropertyChangeListener(FM_PROP_VALUE, this);
    }

}
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
{
    DBG_ERROR("Exception in BibDataManager::SetMeAsUidListener")
}


}
//------------------------------------------------------------------------
void BibDataManager::RemoveMeAsUidListener()
{
try
{
    Reference< XNameAccess >  xFields = getColumns(xForm);
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

    if(theFieldName.len()>0)
    {
        Reference< XPropertySet >  xPropSet;
        Any aElement;

        aElement = xFields->getByName(theFieldName);
        xPropSet = *(Reference< XPropertySet > *)aElement.getValue();

        xPropSet->removePropertyChangeListener(FM_PROP_VALUE, this);
    }

}
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
{
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
        Reference< XResultSet >  xCursor(xForm, UNO_QUERY);
        Reference< XResultSetUpdate >  xCursorUpdate(xForm, UNO_QUERY);
        if (xCursor.is() && xCursorUpdate.is())
        {
            bRet = xCursor->relative(nCount);
            if (xCursor->isAfterLast())
                xCursorUpdate->moveToInsertRow();
        }
    }
#ifdef DBG_UTIL
        catch(Exception& e )
#else
        catch(Exception&)
#endif
    {
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
        unloadDatabase();
        pBibView->UpdatePages();
        loadDatabase();
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


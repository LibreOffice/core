/*************************************************************************
 *
 *  $RCSfile: unodatbr.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-07 08:19:43 $
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

#ifndef _SVX_GRIDCTRL_HXX
#include <svx/gridctrl.hxx>
#endif
#ifndef _SBA_UNODATBR_HXX_
#include "unodatbr.hxx"
#endif
#ifndef _SBA_GRID_HXX
#include "sbagrid.hxx"
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SV_MULTISEL_HXX //autogen
#include <tools/multisel.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XWARNINGSSUPPLIER_HPP_
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_TEXTALIGN_HPP_
#include <com/sun/star/awt/TextAlign.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _SVX_ALGITEM_HXX //autogen
#include <svx/algitem.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBAUI_DBTREEMODEL_HXX
#include "dbtreemodel.hxx"
#endif
#ifndef DBACCESS_UI_DBTREEVIEW_HXX
#include "dbtreeview.hxx"
#endif
#ifndef _SVLBOXITM_HXX
#include <svtools/svlbitm.hxx>
#endif
#ifndef _SV_SPLIT_HXX
#include <vcl/split.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBACCESS_SBA_GRID_HRC
#include "sbagrid.hrc"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef DBAUI_DBTREELISTBOX_HXX
#include "dbtreelistbox.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _DBAUI_QUERYDESIGNACCESS_HXX_
#include "querydesignaccess.hxx"
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef DBAUI_TOKENWRITER_HXX
#include "TokenWriter.hxx"
#endif
#ifndef DBAUI_DBEXCHANGE_HXX
#include "dbexchange.hxx"
#endif
#ifndef DBAUI_WIZ_COPYTABLEDIALOG_HXX
#include "WCopyTable.hxx"
#endif
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#include "WExtendPages.hxx"
#endif
#ifndef DBAUI_WIZ_NAMEMATCHING_HXX
#include "WNameMatch.hxx"
#endif
#ifndef DBAUI_WIZ_COLUMNSELECT_HXX
#include "WColumnSelect.hxx"
#endif
#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef DBAUI_WIZARD_CPAGE_HXX
#include "WCPage.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_RTFREADER_HXX
#include "RtfReader.hxx"
#endif
#ifndef DBAUI_HTMLREADER_HXX
#include "HtmlReader.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#include <sot/storage.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::io;
using namespace ::dbtools;

// .........................................................................
namespace dbaui
{
// .........................................................................
//==================================================================
//= SbaTableQueryBrowser
//==================================================================
// -------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_OBrowser()
{
    static OMultiInstanceAutoRegistration< SbaTableQueryBrowser > aAutoRegistration;
}

// -------------------------------------------------------------------------
void SafeAddPropertyListener(const Reference< XPropertySet > & xSet, const ::rtl::OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->addPropertyChangeListener(rPropName, pListener);
}

// -------------------------------------------------------------------------
void SafeRemovePropertyListener(const Reference< XPropertySet > & xSet, const ::rtl::OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->removePropertyChangeListener(rPropName, pListener);
}
//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaTableQueryBrowser::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}
//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL SbaTableQueryBrowser::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
::rtl::OUString SbaTableQueryBrowser::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.ODatasourceBrowser");
}
//-------------------------------------------------------------------------
::comphelper::StringSequence SbaTableQueryBrowser::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.DataSourceBrowser");
    return aSupported;
}
//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL SbaTableQueryBrowser::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    return *(new SbaTableQueryBrowser(_rxFactory));
}
//------------------------------------------------------------------------------
SbaTableQueryBrowser::SbaTableQueryBrowser(const Reference< XMultiServiceFactory >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_pTreeModel(NULL)
    ,m_pTreeView(NULL)
    ,m_pSplitter(NULL)
    ,m_pCurrentlyDisplayed(NULL)
{
    // calc the title for the load stopper
//  sal_uInt32 nTitleResId;
//  switch (m_xDefinition->GetKind())
//  {
//      case dbTable : nTitleResId = STR_TBL_TITLE; break;
//      case dbQuery : nTitleResId = STR_QRY_TITLE; break;
//      default : DBG_ERROR("OpenDataObjectThread::run : invalid object !");
//  }
//  String sTemp = String(ModuleRes(nTitleResId));
//  sTemp.SearchAndReplace('#', m_xDefinition->Name());
//  m_sLoadStopperCaption = String(ModuleRes(RID_STR_OPEN_OBJECT));
//  m_sLoadStopperCaption += ' ';
//  m_sLoadStopperCaption += sTemp;
}

//------------------------------------------------------------------------------
SbaTableQueryBrowser::~SbaTableQueryBrowser()
{
}

//------------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::disposing()
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    // reset the content's tree view: it holds a reference to our model which is to be deleted immediately,
    // and it will live longer than we do.
    if (getBrowserView())
        getBrowserView()->setTreeView(NULL);

    // clear the user data of the tree model
    SvLBoxEntry* pEntryLoop = m_pTreeModel->First();
    while (pEntryLoop)
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntryLoop->GetUserData());
        if(pData)
        {
            Reference<XConnection> xCon(pData->xObject,UNO_QUERY);
            if(xCon.is())
            {
                Reference< XComponent >  xComponent(xCon, UNO_QUERY);
                if (xComponent.is())
                {
                    Reference< ::com::sun::star::lang::XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
                    xComponent->removeEventListener(xEvtL);
                }
                ::comphelper::disposeComponent(pData->xObject);
            }
            delete pData;
        }
        pEntryLoop = m_pTreeModel->Next(pEntryLoop);
    }
    m_pCurrentlyDisplayed = NULL;
    // clear the tree model
    delete m_pTreeModel;
    m_pTreeModel = NULL;

    // remove ourself as status listener
    implRemoveStatusListeners();

    // remove the container listener from the database context
    Reference< XContainer > xDatasourceContainer(m_xDatabaseContext, UNO_QUERY);
    if (xDatasourceContainer.is())
        xDatasourceContainer->removeContainerListener(this);

    SbaXDataBrowserController::disposing();
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::Construct(Window* pParent)
{
    if (!SbaXDataBrowserController::Construct(pParent))
        return sal_False;

    try
    {
        Reference< XContainer > xDatasourceContainer(m_xDatabaseContext, UNO_QUERY);
        if (xDatasourceContainer.is())
            xDatasourceContainer->addContainerListener(this);
        else
            DBG_ERROR("SbaTableQueryBrowser::Construct: the DatabaseContext should allow us to be a listener!");
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::Construct: could not create (or start listening at) the database context!");
    }
    // some help ids
    if (getBrowserView() && getBrowserView()->getVclControl())
    {

        // create controls and set sizes
        const long  nFrameWidth = getBrowserView()->LogicToPixel( Size( 3, 0 ), MAP_APPFONT ).Width();

        m_pSplitter = new Splitter(getBrowserView(),WB_HSCROLL);
        m_pSplitter->SetPosSizePixel( Point(0,0), Size(nFrameWidth,0) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
        m_pSplitter->Show();

        m_pTreeView = new DBTreeView(getBrowserView(),m_xMultiServiceFacatory, WB_TABSTOP);
        m_pTreeView->Show();
        m_pTreeView->SetPreExpandHandler(LINK(this, SbaTableQueryBrowser, OnExpandEntry));
        m_pTreeView->SetHelpId(HID_CTL_TREEVIEW);

        m_pTreeView->SetContextMenuHandler(LINK(this, SbaTableQueryBrowser, OnListContextMenu));
        // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
        m_pSplitter->SetSplitPosPixel( getBrowserView()->LogicToPixel( Size( 80, 0 ), MAP_APPFONT ).Width() );

        getBrowserView()->setSplitter(m_pSplitter);
        getBrowserView()->setTreeView(m_pTreeView);

        // fill view with data
        m_pTreeModel = new DBTreeListModel;
        m_pTreeView->setModel(m_pTreeModel);
        m_pTreeView->setSelectHdl(LINK(this, SbaTableQueryBrowser, OnSelectEntry));
        initializeTreeModel();

        // TODO
        getBrowserView()->getVclControl()->GetDataWindow().SetUniqueId(UID_DATABROWSE_DATAWINDOW);
        getBrowserView()->getVclControl()->SetHelpId(HID_CTL_TABBROWSER);
        getBrowserView()->SetUniqueId(UID_CTL_CONTENT);
        if (getBrowserView()->getVclControl()->GetHeaderBar())
            getBrowserView()->getVclControl()->GetHeaderBar()->SetHelpId(HID_DATABROWSE_HEADER);
        InvalidateFeature(ID_BROWSER_EXPLORER);
    }

    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::InitializeForm(const Reference< ::com::sun::star::sdbc::XRowSet > & _rxForm)
{
    if(!m_pCurrentlyDisplayed)
        return sal_True;
    // this method set all format settings from the orignal table or query
    try
    {
        // we send all properties at once, maybe the implementation is clever enough to handle one big PropertiesChanged
        // more effective than many small PropertyChanged ;)
        Sequence< ::rtl::OUString> aProperties(3);
        Sequence< Any> aValues(3);

        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
        OSL_ENSURE(pData,"No user data set at the currently displayed entry!");
        Reference<XPropertySet> xTableProp(pData->xObject,UNO_QUERY);
        OSL_ENSURE(xTableProp.is(),"No table available!");

        // is the filter intially applied ?
        aProperties.getArray()[0]   = PROPERTY_APPLYFILTER;
        aValues.getArray()[0]       = xTableProp->getPropertyValue(PROPERTY_APPLYFILTER);

        // the initial filter
        aProperties.getArray()[1]   = PROPERTY_FILTER;
        aValues.getArray()[1]       = xTableProp->getPropertyValue(PROPERTY_FILTER);

        // the initial ordering
        aProperties.getArray()[2]   = PROPERTY_ORDER;
        aValues.getArray()[2]       = xTableProp->getPropertyValue(PROPERTY_ORDER);

        Reference< XMultiPropertySet >  xFormMultiSet(_rxForm, UNO_QUERY);
        xFormMultiSet->setPropertyValues(aProperties, aValues);
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::InitializeForm : something went wrong !");
        return sal_False;
    }


    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::InitializeGridModel(const Reference< ::com::sun::star::form::XFormComponent > & xGrid)
{
    try
    {

        Reference< ::com::sun::star::form::XGridColumnFactory >  xColFactory(xGrid, UNO_QUERY);
        Reference< XNameContainer >  xColContainer(xGrid, UNO_QUERY);
        // first we have to clear the grid
        {
            Sequence< ::rtl::OUString > aNames = xColContainer->getElementNames();
            const ::rtl::OUString* pBegin   = aNames.getConstArray();
            const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
            for (; pBegin != pEnd;++pBegin)
                xColContainer->removeByName(*pBegin);
        }

        // set the formats from the table
        if(m_pCurrentlyDisplayed)
        {
            Sequence< ::rtl::OUString> aProperties(3);
            Sequence< Any> aValues(3);

            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
            Reference<XPropertySet> xTableProp(pData->xObject,UNO_QUERY);
            OSL_ENSURE(xTableProp.is(),"No table available!");

            aProperties.getArray()[0]   = PROPERTY_FONT;
            aValues.getArray()[0]       = xTableProp->getPropertyValue(PROPERTY_FONT);
            aProperties.getArray()[1]   = PROPERTY_ROW_HEIGHT;
            aValues.getArray()[1]       = xTableProp->getPropertyValue(PROPERTY_ROW_HEIGHT);
            aProperties.getArray()[2]   = PROPERTY_TEXTCOLOR;
            aValues.getArray()[2]       = xTableProp->getPropertyValue(PROPERTY_TEXTCOLOR);

            Reference< XMultiPropertySet >  xFormMultiSet(xGrid, UNO_QUERY);
            xFormMultiSet->setPropertyValues(aProperties, aValues);
        }


        // get the formats supplier of the database we're working with
        Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = getNumberFormatter()->getNumberFormatsSupplier();

        Reference<XConnection> xConnection;
        Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
        ::cppu::extractInterface(xConnection,xProp->getPropertyValue(PROPERTY_ACTIVECONNECTION));
        OSL_ENSHURE(xConnection.is(),"A ActiveConnection should normaly exists!");

        Reference<XChild> xChild(xConnection,UNO_QUERY);
        Reference<XPropertySet> xDataSourceProp(xChild->getParent(),UNO_QUERY);
        sal_Bool bSupress = ::cppu::any2bool(xDataSourceProp->getPropertyValue(PROPERTY_SUPPRESSVERSIONCL));

        // insert the column into the gridcontrol so that we see something :-)
        ::rtl::OUString aCurrentModelType;
        Reference<XColumnsSupplier> xSupCols(getRowSet(),UNO_QUERY);
        Reference<XNameAccess> xColumns     = xSupCols->getColumns();
        Sequence< ::rtl::OUString> aNames   = xColumns->getElementNames();
        const ::rtl::OUString* pBegin       = aNames.getConstArray();
        const ::rtl::OUString* pEnd         = pBegin + aNames.getLength();

        Reference<XPropertySet> xColumn;
        for (sal_uInt16 i=0; pBegin != pEnd; ++i,++pBegin)
        {
            // Typ
            // first get type to determine wich control we need
            ::cppu::extractInterface(xColumn,xColumns->getByName(*pBegin));

            // ignore the column when it is a rowversion one
            if(bSupress && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ISROWVERSION)
                        && ::cppu::any2bool(xColumn->getPropertyValue(PROPERTY_ISROWVERSION)))
                continue;

            sal_Bool bIsFormatted           = sal_False;
            sal_Bool bFormattedIsNumeric    = sal_True;
            sal_Int32 nType = comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_TYPE));
            switch(nType)
            {
                // TODO : die Strings fuer die Column-Typen irgendwo richtig platzieren
                case DataType::BIT:
                    aCurrentModelType = ::rtl::OUString::createFromAscii("CheckBox");
                    break;

                case DataType::BINARY:
                case DataType::VARBINARY:
                case DataType::LONGVARBINARY:
                    aCurrentModelType = ::rtl::OUString::createFromAscii("TextField");
                    break;

                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                case DataType::CHAR:
                    bFormattedIsNumeric = sal_False;
                    // _NO_ break !
                default:
                    aCurrentModelType = ::rtl::OUString::createFromAscii("FormattedField");
                    bIsFormatted = sal_True;
                    break;
            }

            Reference< XPropertySet >  xCurrentCol = xColFactory->createColumn(aCurrentModelType);
            xCurrentCol->setPropertyValue(PROPERTY_CONTROLSOURCE, makeAny(*pBegin));
            xCurrentCol->setPropertyValue(PROPERTY_LABEL, makeAny(*pBegin));
            if (bIsFormatted)
            {
                if (xSupplier.is())
                    xCurrentCol->setPropertyValue(::rtl::OUString::createFromAscii("FormatsSupplier"), makeAny(xSupplier));
                xCurrentCol->setPropertyValue(PROPERTY_FORMATKEY, xColumn->getPropertyValue(PROPERTY_FORMATKEY));
                xCurrentCol->setPropertyValue(::rtl::OUString::createFromAscii("TreatAsNumber"), ::cppu::bool2any(bFormattedIsNumeric));
            }

            // default value
            if (nType == DataType::BIT)
            {
                Any aDefault; aDefault <<= ((sal_Int16)STATE_DONTKNOW);
                if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE))
                    aDefault <<= (comphelper::getString(xColumn->getPropertyValue(PROPERTY_DEFAULTVALUE)).toInt32() == 0) ? (sal_Int16)STATE_NOCHECK : (sal_Int16)STATE_CHECK;
                xCurrentCol->setPropertyValue(PROPERTY_DEFAULTSTATE, aDefault);
            }

            // transfer properties from the definition to the UNO-model :
            // ... the hidden flag
            xCurrentCol->setPropertyValue(PROPERTY_HIDDEN, xColumn->getPropertyValue(PROPERTY_HIDDEN));

            // ... the initial colum width
            xCurrentCol->setPropertyValue(PROPERTY_WIDTH, xColumn->getPropertyValue(PROPERTY_WIDTH));

            // ... horizontal justify
            sal_Int32 nAlign;
            xColumn->getPropertyValue(PROPERTY_ALIGN) >>= nAlign;
            xCurrentCol->setPropertyValue(PROPERTY_ALIGN, makeAny(sal_Int16(nAlign)));

            // ... the 'comment' property as helptext (will usually be shown as header-tooltip)

            Any aDescription; aDescription <<= ::rtl::OUString();
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION))
                aDescription <<= comphelper::getString(xColumn->getPropertyValue(PROPERTY_DESCRIPTION));
            xCurrentCol->setPropertyValue(PROPERTY_HELPTEXT, xColumn->getPropertyValue(PROPERTY_DESCRIPTION));

            xColContainer->insertByName(*pBegin, makeAny(xCurrentCol));
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::InitializeGridModel : something went wrong !");
        return sal_False;
    }


    return sal_True;
}

//------------------------------------------------------------------------------
ToolBox* SbaTableQueryBrowser::CreateToolBox(Window* _pParent)
{
    ToolBox* pTB = NULL;
    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
    if(xProp.is())
    {
        sal_Int32 nType;
        xProp->getPropertyValue(::rtl::OUString::createFromAscii("CommandType")) >>= nType;

        sal_uInt16 nResId = 0;
        switch (nType)
        {
            case CommandType::TABLE :   nResId = RID_BRW_TAB_TOOLBOX; break;
            case CommandType::QUERY :   nResId = RID_BRW_QRY_TOOLBOX; break;
            case CommandType::COMMAND:  nResId = RID_BRW_QRY_TOOLBOX; break;
            default : return NULL;
        }

        pTB = new ToolBox(_pParent, ModuleRes(nResId));
        if (!pTB)
            return NULL;
    }
    return pTB;
}
// -----------------------------------------------------------------------------
Reference<XPropertySet> getColumnHelper(SvLBoxEntry* _pCurrentlyDisplayed,const Reference<XPropertySet>& _rxSource)
{
    Reference<XPropertySet> xRet;
    if(_pCurrentlyDisplayed)
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pCurrentlyDisplayed->GetUserData());
        Reference<XColumnsSupplier> xColumnsSup(pData->xObject,UNO_QUERY);
        Reference<XNameAccess> xNames = xColumnsSup->getColumns();
        ::rtl::OUString aName;
        _rxSource->getPropertyValue(PROPERTY_NAME) >>= aName;
        if(xNames.is() && xNames->hasByName(aName))
            ::cppu::extractInterface(xRet,xNames->getByName(aName));
    }
    return xRet;
}
// -----------------------------------------------------------------------
void SbaTableQueryBrowser::propertyChange(const PropertyChangeEvent& evt)
{
    SbaXDataBrowserController::propertyChange(evt);

    try
    {
        Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
        if (!xSource.is())
            return;

        // one of the many properties which require us to update the definition ?
        // a column's width ?
        else if (evt.PropertyName.equals(PROPERTY_WIDTH))
        {   // a column width has changed -> update the model
            // (the update of the view is done elsewhere)
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
            {
                if(!evt.NewValue.hasValue())
                    xProp->setPropertyValue(PROPERTY_WIDTH,makeAny((sal_Int32)227));
                else
                    xProp->setPropertyValue(PROPERTY_WIDTH,evt.NewValue);
            }
        }

        // a column's 'visible' state ?
        else if (evt.PropertyName.equals(PROPERTY_HIDDEN))
        {
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_HIDDEN,evt.NewValue);
        }

        // a columns alignment ?
        else if (evt.PropertyName.equals(PROPERTY_ALIGN))
        {
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            try
            {
                if(xProp.is())
                {
                    if(evt.NewValue.hasValue())
                        xProp->setPropertyValue(PROPERTY_ALIGN,evt.NewValue);
                    else
                        xProp->setPropertyValue(PROPERTY_ALIGN,makeAny((sal_Int32)0));
                }
            }
            catch(Exception&)
            {
                OSL_ASSERT(0);
            }
        }

        // a column's format ?
        else if (   (evt.PropertyName.equals(PROPERTY_FORMATKEY))
            &&  (TypeClass_LONG == evt.NewValue.getValueTypeClass())
            )
        {
            // update the model (means the definition object)
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_FORMATKEY,evt.NewValue);
        }

        // some table definition properties ?
        // the height of the rows in the grid ?
        else if (evt.PropertyName.equals(PROPERTY_ROW_HEIGHT))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                OSL_ENSHURE(xProp.is(),"No table available!");

                sal_Bool bDefault = !evt.NewValue.hasValue();
                if (bDefault)
                    xProp->setPropertyValue(PROPERTY_ROW_HEIGHT,makeAny((sal_Int32)45));
                else
                    xProp->setPropertyValue(PROPERTY_ROW_HEIGHT,evt.NewValue);
            }
        }

    //  // the font of the grid ?
        else if (evt.PropertyName.equals(PROPERTY_FONT))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                OSL_ENSHURE(xProp.is(),"No table available!");
                xProp->setPropertyValue(PROPERTY_FONT,evt.NewValue);
            }
        }

    //  // the text color of the grid ?
        else if (evt.PropertyName.equals(PROPERTY_TEXTCOLOR))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                OSL_ENSHURE(xProp.is(),"No table available!");
                xProp->setPropertyValue(PROPERTY_TEXTCOLOR,evt.NewValue);
            }
        }

    //  // the filter ?
        else if (evt.PropertyName.equals(PROPERTY_FILTER))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                OSL_ENSHURE(xProp.is(),"No table available!");
                xProp->setPropertyValue(PROPERTY_FILTER,evt.NewValue);
            }
        }

        // the sort ?
        else if (evt.PropertyName.equals(PROPERTY_ORDER))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                OSL_ENSHURE(xProp.is(),"No table available!");
                xProp->setPropertyValue(PROPERTY_ORDER,evt.NewValue);
            }
        }
        // the appliance of the filter ?
        else if (evt.PropertyName.equals(PROPERTY_APPLYFILTER))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                Reference<XPropertySet> xProp(pData->xObject,UNO_QUERY);
                OSL_ENSHURE(xProp.is(),"No table available!");
                xProp->setPropertyValue(PROPERTY_APPLYFILTER,evt.NewValue);
            }
        }
    }
    catch(Exception& e)
    {
        OSL_ASSERT(0);
    }
}

// -----------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    if (!SbaXDataBrowserController::suspend(bSuspend))
        return sal_False;

    if(m_pCurrentlyDisplayed)
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
        if(pData)
        {
            try
            {
                Reference<XFlushable> xFlush(pData->xObject,UNO_QUERY);
                if(xFlush.is())
                    xFlush->flush();
            }
            catch(DisposedException&)
            {
                OSL_ENSURE(0,"Object already disposed!");
            }
            catch(Exception&)
            {
            }
        }
    }

    return sal_True;
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::statusChanged( const FeatureStateEvent& _rEvent ) throw(RuntimeException)
{
    // search the external dispatcher causing this call
    Reference< XDispatch > xSource(_rEvent.Source, UNO_QUERY);
    for (   SpecialSlotDispatchersIterator aLoop = m_aDispatchers.begin();
            aLoop != m_aDispatchers.end();
            ++aLoop
        )
    {
        if (_rEvent.FeatureURL.Complete == getURLForId(aLoop->first).Complete)
        {
            DBG_ASSERT(xSource.get() == aLoop->second.get(), "SbaTableQueryBrowser::statusChanged: inconsistent!");
            m_aDispatchStates[aLoop->first] = _rEvent.IsEnabled;
            implCheckExternalSlot(aLoop->first);
            break;
        }
    }
    DBG_ASSERT(aLoop != m_aDispatchers.end(), "SbaTableQueryBrowser::statusChanged: don't know who sent this!");
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::implCheckExternalSlot(sal_Int32 _nId)
{
    // check if we have to hide this item from the toolbox
    ToolBox* pTB = getBrowserView()->getToolBox();
    if (pTB)
    {
        sal_Bool bHaveDispatcher = m_aDispatchers[_nId].is();
        if (bHaveDispatcher != pTB->IsItemVisible(_nId))
            bHaveDispatcher ? pTB->ShowItem(_nId) : pTB->HideItem(_nId);
    }

    // and invalidate this feature in general
    InvalidateFeature(_nId);
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::disposing( const EventObject& _rSource ) throw(RuntimeException)
{
    // search the external dispatcher causing this call in our map
    Reference< XDispatch > xSource(_rSource.Source, UNO_QUERY);
    if(xSource.is())
    {
        for (   SpecialSlotDispatchersIterator aLoop = m_aDispatchers.begin();
                aLoop != m_aDispatchers.end();
                ++aLoop
            )
        {
            if (aLoop->second.get() == xSource.get())
            {
                SpecialSlotDispatchersIterator aPrevious = aLoop;
                --aPrevious;

                // remove it
                m_aDispatchers.erase(aLoop);
                m_aDispatchStates.erase(aLoop->first);

                // maybe update the UI
                implCheckExternalSlot(aLoop->first);

                // continue, the same XDispatch may be resposible for more than one URL
                aLoop = aPrevious;
            }
        }
    }
    else
    {
        Reference<XConnection> xCon(_rSource.Source, UNO_QUERY);
        if(xCon.is())
        {   // our connection is in dispose so we have to find the entry equal with this connection
            // and close it what means to collapse the entry
            // get the top-level representing the removed data source
            SvLBoxEntry* pDSLoop = m_pTreeView->getListBox()->FirstChild(NULL);
            while (pDSLoop)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pDSLoop->GetUserData());
                if(pData && pData->xObject == xCon)
                {
                    // we set the conenction to null to avoid a second disposing of the connection
                    pData->xObject = NULL;
                    closeConnection(pDSLoop,sal_False);
                    break;
                }

                pDSLoop = m_pTreeView->getListBox()->NextSibling(pDSLoop);
            }
        }
        else
            SbaXDataBrowserController::disposing(_rSource);
    }
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::implRemoveStatusListeners()
{
    // clear all old dispatches
    for (   ConstSpecialSlotDispatchersIterator aLoop = m_aDispatchers.begin();
            aLoop != m_aDispatchers.end();
            ++aLoop
        )
    {
        if (aLoop->second.is())
        {
            try
            {
                aLoop->second->removeStatusListener(this, getURLForId(aLoop->first));
            }
            catch (Exception&)
            {
                DBG_ERROR("SbaTableQueryBrowser::attachFrame: could not remove a status listener!");
            }
        }
    }
    m_aDispatchers.clear();
    m_aDispatchStates.clear();
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & _xFrame) throw( RuntimeException )
{
    implRemoveStatusListeners();

    SbaXDataBrowserController::attachFrame(_xFrame);

    // get the dispatchers for the external slots
    Reference< XDispatchProvider >  xProvider(m_xCurrentFrame, UNO_QUERY);
    DBG_ASSERT(xProvider.is(), "SbaTableQueryBrowser::attachFrame: no DispatchPprovider !");
    if (xProvider.is())
    {
        sal_Int32 nExternalIds[] = { ID_BROWSER_FORMLETTER, ID_BROWSER_INSERTCOLUMNS, ID_BROWSER_INSERTCONTENT };
        for (sal_Int32 i=0; i<sizeof(nExternalIds)/sizeof(nExternalIds[0]); ++i)
        {
            URL aURL = getURLForId(nExternalIds[i]);
            m_aDispatchers[nExternalIds[i]] = xProvider->queryDispatch(aURL, ::rtl::OUString::createFromAscii("_parent"), FrameSearchFlag::PARENT);
            if (m_aDispatchers[nExternalIds[i]].get() == static_cast< XDispatch* >(this))
                // as the URL is one of our "supported features", we may answer the request ourself if nobody out there
                // is interested in.
                m_aDispatchers[nExternalIds[i]].clear();

            // assume te general availability of the feature. This is overruled if there is no dispatcher for the URL
            m_aDispatchStates[nExternalIds[i]] = sal_True;

            if (m_aDispatchers[nExternalIds[i]].is())
            {
                try
                {
                    m_aDispatchers[nExternalIds[i]]->addStatusListener(this, aURL);
                }
                catch(DisposedException&)
                {
                    OSL_ENSURE(0,"Object already disposed!");
                }
                catch(Exception&)
                {
                    DBG_ERROR("SbaTableQueryBrowser::attachFrame: could not attach a status listener!");
                }
            }

            implCheckExternalSlot(nExternalIds[i]);
        }
    }
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::addModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    SbaXDataBrowserController::addModelListeners(_xGridControlModel);
    Reference< XPropertySet >  xCols(_xGridControlModel, UNO_QUERY);
    if (xCols.is())
    {
        xCols->addPropertyChangeListener(PROPERTY_ROW_HEIGHT, (XPropertyChangeListener*)this);
        xCols->addPropertyChangeListener(PROPERTY_FONT, (XPropertyChangeListener*)this);
        xCols->addPropertyChangeListener(PROPERTY_TEXTCOLOR, (XPropertyChangeListener*)this);
    }

}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::removeModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    SbaXDataBrowserController::removeModelListeners(_xGridControlModel);
    Reference< XPropertySet >  xSourceSet(_xGridControlModel, UNO_QUERY);
    if (xSourceSet.is())
    {
        xSourceSet->removePropertyChangeListener(PROPERTY_ROW_HEIGHT, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_FONT, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTCOLOR, (XPropertyChangeListener*)this);
    }
}
// -------------------------------------------------------------------------
String SbaTableQueryBrowser::getURL() const
{
    return String();
}
// -----------------------------------------------------------------------
void SbaTableQueryBrowser::InvalidateFeature(sal_uInt16 nId, const Reference< ::com::sun::star::frame::XStatusListener > & xListener)
{
    SbaXDataBrowserController::InvalidateFeature(nId, xListener);
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::AddColumnListener(const Reference< XPropertySet > & xCol)
{
    SbaXDataBrowserController::AddColumnListener(xCol);
    SafeAddPropertyListener(xCol, PROPERTY_WIDTH, (XPropertyChangeListener*)this);
    SafeAddPropertyListener(xCol, PROPERTY_HIDDEN, (XPropertyChangeListener*)this);
    SafeAddPropertyListener(xCol, PROPERTY_ALIGN, (XPropertyChangeListener*)this);
    SafeAddPropertyListener(xCol, PROPERTY_FORMATKEY, (XPropertyChangeListener*)this);
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::RemoveColumnListener(const Reference< XPropertySet > & xCol)
{
    SbaXDataBrowserController::RemoveColumnListener(xCol);
    SafeRemovePropertyListener(xCol, PROPERTY_WIDTH, (XPropertyChangeListener*)this);
    SafeRemovePropertyListener(xCol, PROPERTY_HIDDEN, (XPropertyChangeListener*)this);
    SafeRemovePropertyListener(xCol, PROPERTY_ALIGN, (XPropertyChangeListener*)this);
    SafeRemovePropertyListener(xCol, PROPERTY_FORMATKEY, (XPropertyChangeListener*)this);
}
//------------------------------------------------------------------------------
void SbaTableQueryBrowser::AddSupportedFeatures()
{
    SbaXDataBrowserController::AddSupportedFeatures();

    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Title")]               = ID_BROWSER_TITLE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/FormLetter")]    = ID_BROWSER_FORMLETTER;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/InsertColumns")] = ID_BROWSER_INSERTCOLUMNS;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/InsertContent")] = ID_BROWSER_INSERTCONTENT;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DataSourceBrowser/ToggleExplore")] = ID_BROWSER_EXPLORER;

            // TODO reenable our own code if we really have a handling for the formslots
//      ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToFirst"),        SID_FM_RECORD_FIRST     ),
//      ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToLast"),     SID_FM_RECORD_LAST      ),
//      ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToNew"),      SID_FM_RECORD_NEW       ),
//      ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToNext"),     SID_FM_RECORD_NEXT      ),
//      ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToPrev"),     SID_FM_RECORD_PREV      )

}

//------------------------------------------------------------------------------
FeatureState SbaTableQueryBrowser::GetState(sal_uInt16 nId)
{
    FeatureState aReturn;
        // (disabled automatically)

    if (ID_BROWSER_EXPLORER == nId)
    {       // this slot is available even if no form is loaded
        aReturn.bEnabled = sal_True;
        aReturn.aState = ::cppu::bool2any(haveExplorer());
        return aReturn;
    }
    try
    {
        // no chance without a view
        if (!getBrowserView() || !getBrowserView()->getVclControl())
            return aReturn;
        // no chance without valid models
        if (isValid() && !isValidCursor() && nId != ID_BROWSER_CLOSE) // the close button should always be enabled
            return aReturn;
        // no chance while loading the form
        if (PendingLoad())
            return aReturn;

        switch (nId)
        {
            case ID_BROWSER_INSERTCOLUMNS:
            case ID_BROWSER_INSERTCONTENT:
            case ID_BROWSER_FORMLETTER:
            {
                // the slot is enabled if we have an external dispatcher able to handle it,
                // and the dispatcher must have enabled the slot in general
                if (m_aDispatchers[nId].is())
                    aReturn.bEnabled = m_aDispatchStates[nId];
                else
                    aReturn.bEnabled = sal_False;

                // for the Insert* slots, we need at least one selected row
                if (ID_BROWSER_FORMLETTER != nId)
                    aReturn.bEnabled = aReturn.bEnabled && getBrowserView()->getVclControl()->GetSelectRowCount();

                // disabled for native queries which are not saved within the database
                // 67706 - 23.08.99 - FS
                Reference< XPropertySet >  xDataSource(getRowSet(), UNO_QUERY);
                try
                {
                    aReturn.bEnabled = aReturn.bEnabled && xDataSource.is();

                    if (xDataSource.is())
                    {
                        sal_Int32 nType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMANDTYPE));
                        aReturn.bEnabled = aReturn.bEnabled && ((::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING)) || (nType == ::com::sun::star::sdb::CommandType::QUERY)));
                    }
                }
                catch(DisposedException&)
                {
                    OSL_ENSURE(0,"Object already disposed!");
                }
                catch(Exception&)
                {
                }

            }
            break;
            case ID_BROWSER_TITLE:
                {
                    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
                    sal_Int32 nCommandType = CommandType::TABLE;
                    xProp->getPropertyValue(PROPERTY_COMMANDTYPE) >>= nCommandType;
                    String sTitle;
                    switch (nCommandType)
                    {
                        case CommandType::TABLE:
                            sTitle = String(ModuleRes(STR_TBL_TITLE)); break;
                        case CommandType::QUERY:
                        case CommandType::COMMAND:
                            sTitle = String(ModuleRes(STR_QRY_TITLE)); break;
                        default:
                            DBG_ASSERT(0,"Unbekannte DBDef Art");
                    }
                    ::rtl::OUString aName;
                    xProp->getPropertyValue(PROPERTY_COMMAND) >>= aName;
                    String sObject(aName.getStr());

                    sTitle.SearchAndReplace('#',sObject);
                    aReturn.aState <<= ::rtl::OUString(sTitle);
                    aReturn.bEnabled = sal_True;
                }
                break;
            case ID_BROWSER_TABLEATTR:
            case ID_BROWSER_ROWHEIGHT:
            case ID_BROWSER_COLATTRSET:
            case ID_BROWSER_COLWIDTH:
                aReturn.bEnabled = getBrowserView() && getBrowserView()->getVclControl() && isValid() && isValidCursor();
                //  aReturn.bEnabled &= getDefinition() && !getDefinition()->GetDatabase()->IsReadOnly();
                break;

            case ID_BROWSER_EDITDOC:
                aReturn = SbaXDataBrowserController::GetState(nId);
                //  aReturn.bEnabled &= !getDefinition()->IsLocked();
                    // somebody is modifying the definition -> no edit mode
                break;

            case ID_BROWSER_CLOSE:
                aReturn.bEnabled = sal_True;
                break;
            default:
                return SbaXDataBrowserController::GetState(nId);
        }
    }
    catch(Exception& e)
    {
#if DBG_UTIL
        String sMessage("SbaXDataBrowserController::GetState(", RTL_TEXTENCODING_ASCII_US);
        sMessage += String::CreateFromInt32(nId);
        sMessage.AppendAscii(") : catched an exception ! message : ");
        sMessage += (const sal_Unicode*)e.Message;
        DBG_ERROR(ByteString(sMessage, gsl_getSystemTextEncoding()).GetBuffer());
#else
        e;  // make compiler happy
#endif
    }
    ;

    return aReturn;

}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::Execute(sal_uInt16 nId)
{
    switch (nId)
    {
        case ID_BROWSER_EXPLORER:
            toggleExplorer();
            break;
        case ID_BROWSER_EDITDOC:
            SbaXDataBrowserController::Execute(nId);
            break;

        case ID_BROWSER_INSERTCOLUMNS:
        case ID_BROWSER_INSERTCONTENT:
        case ID_BROWSER_FORMLETTER:
            if (getBrowserView() && isValidCursor())
            {
                // the URL the slot id is assigned to
                URL aParentUrl = getURLForId(nId);

                // let the dispatcher execute the slot
                Reference< XDispatch > xDispatch(m_aDispatchers[nId]);
                if (xDispatch.is())
                {
                    // set the properties for the dispatch

                    // first fill the selection
                    SbaGridControl* pGrid = getBrowserView()->getVclControl();
                    MultiSelection* pSelection = (MultiSelection*)pGrid->GetSelection();
                    Sequence< sal_Int32 > aSelection;
                    if (pSelection != NULL)
                    {
                        aSelection.realloc(pSelection->GetSelectCount());
                        long nIdx = pSelection->FirstSelected();
                        sal_Int32 i = 0;
                        while (nIdx >= 0)
                        {
                            aSelection[i++] = nIdx+1;
                            nIdx = pSelection->NextSelected();
                        }
                    }

                    Reference< XResultSet > xCursorClone;
                    try
                    {
                        Reference< XResultSetAccess > xResultSetAccess(getRowSet(),UNO_QUERY);
                        if (xResultSetAccess.is())
                            xCursorClone = xResultSetAccess->createResultSet();
                    }
                    catch(DisposedException&)
                    {
                        OSL_ENSURE(0,"Object already disposed!");
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("SbaTableQueryBrowser::Execute(ID_BROWSER_?): could not clone the cursor!");
                    }

                    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);

                    try
                    {
                        Sequence< PropertyValue> aProps(5);
                        aProps[0] = PropertyValue(PROPERTY_DATASOURCENAME, -1, xProp->getPropertyValue(PROPERTY_DATASOURCENAME), PropertyState_DIRECT_VALUE);
                        aProps[1] = PropertyValue(PROPERTY_COMMAND, -1, xProp->getPropertyValue(PROPERTY_COMMAND), PropertyState_DIRECT_VALUE);
                        aProps[2] = PropertyValue(PROPERTY_COMMANDTYPE, -1, xProp->getPropertyValue(PROPERTY_COMMANDTYPE), PropertyState_DIRECT_VALUE);
                        aProps[3] = PropertyValue(::rtl::OUString::createFromAscii("Selection"), -1, makeAny(aSelection), PropertyState_DIRECT_VALUE);
                        aProps[4] = PropertyValue(::rtl::OUString::createFromAscii("Cursor"), -1, makeAny(xCursorClone), PropertyState_DIRECT_VALUE);

                        xDispatch->dispatch(aParentUrl, aProps);
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("SbaTableQueryBrowser::Execute(ID_BROWSER_?): could not dispatch the slot (caught an exception)!");
                    }
                }
            }
            break;

        case ID_BROWSER_CLOSE:
            {
                Reference<XComponent> xComp(m_xCurrentFrame,UNO_QUERY);
                ::comphelper::disposeComponent(xComp);
            }
            break;
        default:
            SbaXDataBrowserController::Execute(nId);
            break;
    }
}
// -------------------------------------------------------------------------
void SbaTableQueryBrowser::implAddDatasource(const String& _rDbName, Image& _rDbImage,
        String& _rQueryName, Image& _rQueryImage, String& _rTableName, Image& _rTableImage)
{
    // initialize the names/images if necessary
    if (!_rQueryName.Len())
        _rQueryName = String(ModuleRes(RID_STR_QUERIES_CONTAINER));
    if (!_rTableName.Len())
        _rTableName = String(ModuleRes(RID_STR_TABLES_CONTAINER));

    if (!_rQueryImage)
        _rQueryImage = Image(ModuleRes(QUERYFOLDER_TREE_ICON));
    if (!_rTableImage)
        _rTableImage = Image(ModuleRes(TABLEFOLDER_TREE_ICON));

    if (!_rDbImage)
        _rDbImage = Image(ModuleRes(IMG_DATABASE));

    // add the entry for the data source
    SvLBoxEntry* pDatasourceEntry = m_pTreeView->getListBox()->InsertEntry(_rDbName, _rDbImage, _rDbImage, NULL, sal_False);
    pDatasourceEntry->SetUserData(new DBTreeListModel::DBTreeListUserData);

    // the child for the queries container
    SvLBoxEntry* pQueries = m_pTreeView->getListBox()->InsertEntry(_rQueryName, _rQueryImage, _rQueryImage, pDatasourceEntry, sal_True);
    DBTreeListModel::DBTreeListUserData* pQueriesData = new DBTreeListModel::DBTreeListUserData;
    pQueriesData->bTable = sal_False;
    pQueries->SetUserData(pQueriesData);

    // the child for the tables container
    SvLBoxEntry* pTables = m_pTreeView->getListBox()->InsertEntry(_rTableName, _rTableImage, _rTableImage, pDatasourceEntry, sal_True);
    DBTreeListModel::DBTreeListUserData* pTablesData = new DBTreeListModel::DBTreeListUserData;
    pTablesData->bTable = sal_True;
    pTables->SetUserData(pTablesData);
}
// -------------------------------------------------------------------------
void SbaTableQueryBrowser::initializeTreeModel()
{
    if (m_xDatabaseContext.is())
    {
        Image aDBImage, aQueriesImage, aTablesImage;
        String sQueriesName, sTablesName;

        // fill the model with the names of the registered datasources
        Sequence< ::rtl::OUString > aDatasources = m_xDatabaseContext->getElementNames();
        const ::rtl::OUString* pBegin   = aDatasources.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aDatasources.getLength();
        for (; pBegin != pEnd; ++pBegin)
            implAddDatasource(*pBegin, aDBImage, sQueriesName, aQueriesImage, sTablesName, aTablesImage);
    }
}
// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::populateTree(const Reference<XNameAccess>& _xNameAccess, SvLBoxEntry* _pParent, const Image& _rImage)
{
    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pParent->GetUserData());
    if(pData)
        pData->xObject = _xNameAccess;

    try
    {
        Sequence< ::rtl::OUString > aNames = _xNameAccess->getElementNames();
        const ::rtl::OUString* pBegin   = aNames.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
        for (; pBegin != pEnd; ++pBegin)
            m_pTreeView->getListBox()->InsertEntry(*pBegin, _rImage, _rImage, _pParent, sal_False);
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaTableQueryBrowser::populateTree: could not fill the tree");
        return sal_False;
    }
    return sal_True;
}
//------------------------------------------------------------------------------
IMPL_LINK(SbaTableQueryBrowser, OnListContextMenu, const CommandEvent*, _pEvent)
{
    PopupMenu aContextMenu(ModuleRes(MENU_BROWSERTREE_CONTEXT));
    Point aPosition;
    SvLBoxEntry* pEntry = NULL;
    SvLBoxEntry* pOldSelection = NULL;
    if (_pEvent->IsMouseEvent())
    {
        aPosition = _pEvent->GetMousePosPixel();
        // ensure that the entry which the user clicked at is selected
        pEntry = m_pTreeView->getListBox()->GetEntry(aPosition);
        OSL_ENSURE(pEntry,"No current entry!");
        if (pEntry && !m_pTreeView->getListBox()->IsSelected(pEntry))
        {
            pOldSelection = m_pTreeView->getListBox()->FirstSelected();
            m_pTreeView->getListBox()->lockAutoSelect();
            m_pTreeView->getListBox()->Select(pEntry);
            m_pTreeView->getListBox()->unlockAutoSelect();
        }
    }
    else
    {
        // use the center of the current entry
        pEntry = m_pTreeView->getListBox()->GetCurEntry();
        OSL_ENSURE(pEntry,"No current entry!");
        aPosition = m_pTreeView->getListBox()->GetEntryPos(pEntry);
        aPosition.X() += m_pTreeView->getListBox()->GetOutputSizePixel().Width() / 2;
        aPosition.Y() += m_pTreeView->getListBox()->GetEntryHeight() / 2;
    }

    // disable entries according to the currently selected entry

    // does the datasource which the selected entry belongs to has an open connection ?
    SvLBoxEntry* pDSEntry = m_pTreeView->getListBox()->GetRootLevelParent(pEntry);
    DBTreeListModel::DBTreeListUserData* pDSData =
                pDSEntry
            ?   static_cast<DBTreeListModel::DBTreeListUserData*>(pDSEntry->GetUserData())
            :   NULL;
    if (!pDSData || !pDSData->xObject.is())
    {   // no -> disable the connection-related menu entries
        aContextMenu.EnableItem(ID_TREE_CLOSE_CONN, sal_False);
        aContextMenu.EnableItem(ID_TREE_REBUILD_CONN, sal_False);
    }
    // enable menu entries
    if(pEntry)
    {
        SvLBoxEntry* pTemp      = m_pTreeView->getListBox()->GetParent(pEntry);

        // 1. for tables
        SvLBoxEntry* pTables    = m_pTreeView->getListBox()->GetEntry(pDSEntry,1);

        aContextMenu.EnableItem(ID_TREE_TABLE_CREATE_DESIGN, (pTables == pEntry || pTables == pTemp));
        aContextMenu.EnableItem(ID_TREE_RELATION_DESIGN,     (pTables == pEntry || pTables == pTemp));

        sal_Bool bPasteAble = (pTables == pEntry || pTables == pTemp);
        if(bPasteAble)
        {
            TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard());
            bPasteAble = aTransferData.HasFormat(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE) ||
                         aTransferData.HasFormat(SOT_FORMAT_RTF) ||
                         aTransferData.HasFormat(SOT_FORMATSTR_ID_HTML);
        }
        aContextMenu.EnableItem(ID_TREE_TABLE_PASTE, bPasteAble);

        aContextMenu.EnableItem(ID_TREE_TABLE_EDIT,     (pTables != pEntry && pTables == pTemp));
        aContextMenu.EnableItem(ID_TREE_TABLE_DELETE,   (pTables != pEntry && pTables == pTemp));
        aContextMenu.EnableItem(ID_TREE_TABLE_COPY,     (pTables != pEntry && pTables == pTemp));

        // 2. for queries
        SvLBoxEntry* pQueries   = m_pTreeView->getListBox()->GetEntry(pDSEntry,0);
        aContextMenu.EnableItem(ID_TREE_QUERY_CREATE_DESIGN, (pQueries == pEntry || pQueries == pTemp));
        aContextMenu.EnableItem(ID_TREE_QUERY_CREATE_TEXT, (pQueries == pEntry || pQueries == pTemp));
        aContextMenu.EnableItem(ID_TREE_QUERY_EDIT,     (pQueries != pEntry && pQueries == pTemp));
        aContextMenu.EnableItem(ID_TREE_QUERY_DELETE,   (pQueries != pEntry && pQueries == pTemp));
        aContextMenu.EnableItem(ID_TREE_QUERY_COPY,     (pQueries != pEntry && pQueries == pTemp));
    }

    // rebuild conn not implemented yet
    aContextMenu.EnableItem(ID_TREE_REBUILD_CONN, sal_False);

    if (!m_xMultiServiceFacatory.is())
        // no ORB -> no administration dialog
        aContextMenu.EnableItem(ID_TREE_ADMINISTRATE, sal_False);

    // no disabled entries
    aContextMenu.RemoveDisabledEntries();

    sal_Bool bReopenConn = sal_False;
    USHORT nPos = aContextMenu.Execute(m_pTreeView->getListBox(), aPosition);

    // restore the old selection
    if (pOldSelection)
    {
        m_pTreeView->getListBox()->lockAutoSelect();
        m_pTreeView->getListBox()->Select(pOldSelection);
        m_pTreeView->getListBox()->unlockAutoSelect();
    }

    switch (nPos)
    {
        case ID_TREE_ADMINISTRATE:
            try
            {
                // the parameters:
                Sequence< Any > aArgs(2);
                // the parent window
                aArgs[0] <<= PropertyValue(
                    ::rtl::OUString::createFromAscii("ParentWindow"), 0,
                    makeAny(VCLUnoHelper::GetInterface(m_pTreeView->getListBox()->Window::GetParent())), PropertyState_DIRECT_VALUE);
                // the initial selection
                SvLBoxEntry* pTopLevelSelected = pEntry;
                while (pTopLevelSelected && m_pTreeView->getListBox()->GetParent(pTopLevelSelected))
                    pTopLevelSelected = m_pTreeView->getListBox()->GetParent(pTopLevelSelected);
                ::rtl::OUString sInitialSelection;
                if (pTopLevelSelected)
                    sInitialSelection = m_pTreeView->getListBox()->GetEntryText(pTopLevelSelected);
                aArgs[1] <<= PropertyValue(
                    ::rtl::OUString::createFromAscii("InitialSelection"), 0,
                    makeAny(sInitialSelection), PropertyState_DIRECT_VALUE);

                // create the dialog
                Reference< XExecutableDialog > xAdminDialog;
                xAdminDialog = Reference< XExecutableDialog >(
                    m_xMultiServiceFacatory->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.sdb.DatasourceAdministrationDialog"),
                        aArgs), UNO_QUERY);

                // execute it
                if (xAdminDialog.is())
                    xAdminDialog->execute();
            }
            catch(::com::sun::star::uno::Exception&)
            {
            }
            break;
        case ID_TREE_REBUILD_CONN:
            bReopenConn = sal_True;
        case ID_TREE_CLOSE_CONN:
        {
            closeConnection(pDSEntry);
        }
            break;
        case ID_TREE_RELATION_DESIGN:
        case ID_TREE_TABLE_CREATE_DESIGN:
        case ID_TREE_QUERY_CREATE_DESIGN:
        case ID_TREE_QUERY_CREATE_TEXT:
        case ID_TREE_QUERY_EDIT:
        case ID_TREE_TABLE_EDIT:
            try
            {
                ::osl::MutexGuard aGuard(m_aEntryMutex);

                // get all needed properties for design
                Reference<XConnection> xConnection;  // supports the service sdb::connection
                if(!secureConnection(pDSEntry,pDSData,xConnection))
                    break;

                ::rtl::OUString sCurrentObject;
                if ((ID_TREE_QUERY_EDIT == nPos || ID_TREE_TABLE_EDIT == nPos) && pEntry)
                {
                    // get the name of the query
                    SvLBoxItem* pQueryTextItem = pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                    if (pQueryTextItem)
                        sCurrentObject = static_cast<SvLBoxString*>(pQueryTextItem)->GetText();

                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntry->GetUserData());
                    if(!pData)
                    {
                        // the query has not been accessed before -> create it's user data
                        pData = new DBTreeListModel::DBTreeListUserData;
                        pData->bTable = sal_False;

                        Reference<XNameAccess> xNameAccess;
                        if(ID_TREE_TABLE_EDIT == nPos)
                        {
                            Reference<XTablesSupplier> xSup(xConnection,UNO_QUERY);
                            if(xSup.is())
                                xNameAccess = xSup->getTables();
                        }
                        else
                        {
                            Reference<XQueriesSupplier> xSup(xConnection,UNO_QUERY);
                            if(xSup.is())
                                xNameAccess = xSup->getQueries();
                        }

                        SvLBoxItem* pTextItem = pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                        if (pTextItem)
                            sCurrentObject = static_cast<SvLBoxString*>(pTextItem)->GetText();

                        if(xNameAccess.is() && xNameAccess->hasByName(sCurrentObject) &&
                            ::cppu::extractInterface(pData->xObject,xNameAccess->getByName(sCurrentObject))) // remember the table or query object
                            pEntry->SetUserData(pData);
                        else
                        {
                            delete pData;
                            pData = NULL;
                        }
                    }
                }

                ODesignAccess* pDispatcher = NULL;
                sal_Bool bEdit = sal_False;
                switch(nPos)
                {
                    case ID_TREE_RELATION_DESIGN:
                        pDispatcher = new ORelationDesignAccess(m_xMultiServiceFacatory) ;
                        break;
                    case ID_TREE_TABLE_EDIT:
                        bEdit = sal_True; // run through
                    case ID_TREE_TABLE_CREATE_DESIGN:
                        pDispatcher = new OTableDesignAccess(m_xMultiServiceFacatory) ;
                        break;
                    case ID_TREE_QUERY_EDIT:
                        bEdit = sal_True; // run through
                    case ID_TREE_QUERY_CREATE_DESIGN:
                    case ID_TREE_QUERY_CREATE_TEXT:
                        pDispatcher = new OQueryDesignAccess(m_xMultiServiceFacatory) ;
                        break;
                }
                ::rtl::OUString aDSName;
                SvLBoxItem* pTextItem = pDSEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                if (pTextItem)
                    aDSName = static_cast<SvLBoxString*>(pTextItem)->GetText();

                if (bEdit)
                    pDispatcher->edit(aDSName, sCurrentObject, xConnection);
                else
                    pDispatcher->create(aDSName, xConnection,nPos == ID_TREE_QUERY_CREATE_DESIGN);
            }
            catch(SQLException& e)
            {
                showError(SQLExceptionInfo(e));
            }
            catch(Exception&)
            {
                OSL_ASSERT(0);
            }
            break;
        case ID_TREE_QUERY_DELETE:
            if(pDSEntry)
            {
                String sDsName;
                SvLBoxItem* pTextItem = pDSEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                if (pTextItem)
                    sDsName = static_cast<SvLBoxString*>(pTextItem)->GetText();
                if(!sDsName.Len()) // we have no name
                    break;

                String sName;
                pTextItem = pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                if (pTextItem)
                    sName = static_cast<SvLBoxString*>(pTextItem)->GetText();
                if(!sName.Len())
                    break;

                String aMsg(ModuleRes(STR_QUERY_DELETE_QUERY));
                aMsg.SearchAndReplace(String::CreateFromAscii("%1"),String(sName));
                OSQLMessageBox aDlg(getBrowserView()->getVclControl(),String(ModuleRes(STR_TITLE_CONFIRM_DELETION )),aMsg,WB_YES_NO | WB_DEF_YES,OSQLMessageBox::Query);
                if(aDlg.Execute() == RET_YES)
                {
                    Reference<XQueryDefinitionsSupplier> xSet;
                    try
                    {
                        if(m_xDatabaseContext->hasByName(sDsName))
                            m_xDatabaseContext->getByName(sDsName) >>= xSet;
                    }
                    catch(Exception&)
                    { }
                    if(xSet.is())
                    {
                        Reference<XNameContainer> xNames(xSet->getQueryDefinitions(),UNO_QUERY);
                        if(xNames.is())
                        {
                            try
                            {
                                xNames->removeByName(sName);
                            }
                            catch(SQLException& e)
                            {
                                showError(SQLExceptionInfo(e));
                            }
                            catch(Exception&)
                            {
                            }
                        }
                    }
                }
            }
            break;
        case ID_TREE_TABLE_DELETE:
            {
                ::osl::MutexGuard aGuard(m_aEntryMutex);
                Reference<XConnection> xConnection;  // supports the service sdb::connection
                if(!secureConnection(pDSEntry,pDSData,xConnection))
                    break;
                // get all needed properties for design

                Reference<XTablesSupplier> xSup(xConnection,UNO_QUERY);
                OSL_ENSURE(xSup.is(),"NO XTablesSuppier!");
                if(!xSup.is())
                    break;
                ::rtl::OUString aName;
                SvLBoxItem* pTextItem = pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                if (pTextItem)
                    aName = static_cast<SvLBoxString*>(pTextItem)->GetText();
                Reference<XNameAccess> xTables = xSup->getTables();

                Reference<XDrop> xDrop(xTables,UNO_QUERY);
                if(xDrop.is())
                {
                    String aMsg(ModuleRes(STR_QUERY_DELETE_TABLE));
                    aMsg.SearchAndReplace(String::CreateFromAscii("%1"),String(aName));
                    OSQLMessageBox aDlg(getBrowserView()->getVclControl(),String(ModuleRes(STR_TITLE_CONFIRM_DELETION )),aMsg,WB_YES_NO | WB_DEF_YES,OSQLMessageBox::Query);
                    if(aDlg.Execute() == RET_YES)
                    {
                        try
                        {
                            if(aName.getLength())
                                xDrop->dropByName(aName);
                        }
                        catch(SQLException& e)
                        {
                            showError(SQLExceptionInfo(e));
                        }
                        catch(Exception&)
                        {
                        }
                    }
                }
            }
            break;
        case ID_TREE_QUERY_COPY:
        case ID_TREE_TABLE_COPY:
            try
            {
                ::osl::MutexGuard aGuard(m_aEntryMutex);
                Reference<XConnection> xConnection;  // supports the service sdb::connection
                if(!secureConnection(pDSEntry,pDSData,xConnection))
                    break;
                Reference<XQueriesSupplier> xSup(xConnection,UNO_QUERY);
                OSL_ENSURE(xSup.is(),"NO XQueriesSupplier!");
                ::rtl::OUString aName;
                SvLBoxItem* pTextItem = pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                if (pTextItem)
                    aName = static_cast<SvLBoxString*>(pTextItem)->GetText();

                ::rtl::OUString sDs;
                pTextItem = pDSEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                if (pTextItem)
                    sDs = static_cast<SvLBoxString*>(pTextItem)->GetText();

                Sequence<PropertyValue> aSeq(4);
                aSeq[0].Name    = PROPERTY_DATASOURCENAME;
                aSeq[0].Value <<= sDs;
                aSeq[1].Name    = PROPERTY_ACTIVECONNECTION;;
                aSeq[1].Value <<= xConnection;
                aSeq[2].Name    = PROPERTY_COMMANDTYPE;
                aSeq[2].Value <<= (ID_TREE_QUERY_COPY == nPos ) ? CommandType::QUERY : CommandType::TABLE;
                aSeq[3].Name    = PROPERTY_NAME;
                aSeq[3].Value <<= aName;

                // the rtf format
                ORTFImportExport* pRtf = new ORTFImportExport(aSeq,getORB(),getNumberFormatter());
                // the html format
                OHTMLImportExport* pHtml = new OHTMLImportExport(aSeq,getORB(),getNumberFormatter());
                // the sdbc format
                // the owner ship goes to ODataClipboard
                ODataClipboard* pData = new ODataClipboard(aSeq,pHtml,pRtf);
                Reference< ::com::sun::star::datatransfer::XTransferable> xRef = pData;
                pData->CopyToClipboard();
            }
            catch(SQLException& e)
            {
                showError(SQLExceptionInfo(e));
            }
            catch(Exception&)
            {
            }
            break;
        case ID_TREE_TABLE_PASTE:
            try
            {
                // paste into the tables
                TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard());
                if(aTransferData.HasFormat(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE))
                {
                    ::com::sun::star::datatransfer::DataFlavor aFlavor;
                    SotExchange::GetFormatDataFlavor(SOT_FORMATSTR_ID_SBA_DATAEXCHANGE,aFlavor);
                    Sequence<PropertyValue> aSeq;
                    aTransferData.GetAny(aFlavor) >>= aSeq;
                    if(aSeq.getLength())
                    {
                        ::rtl::OUString sDs;
                        SvLBoxItem* pTextItem = pDSEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                        if (pTextItem)
                            sDs = static_cast<SvLBoxString*>(pTextItem)->GetText();
                        const PropertyValue* pBegin = aSeq.getConstArray();
                        const PropertyValue* pEnd   = pBegin + aSeq.getLength();

                        // first get the dest connection
                        Reference<XConnection> xDestConnection;  // supports the service sdb::connection
                        if(!secureConnection(pDSEntry,pDSData,xDestConnection))
                            break;

                        Reference<XConnection> xSrcConnection;
                        ::rtl::OUString sName,sSrcDataSourceName;
                        sal_Int32 nCommandType = CommandType::TABLE;
                        for(;pBegin != pEnd;++pBegin)
                        {
                            if(pBegin->Name == PROPERTY_DATASOURCENAME)
                                pBegin->Value >>= sSrcDataSourceName;
                            else if(pBegin->Name == PROPERTY_COMMANDTYPE)
                                pBegin->Value >>= nCommandType;
                            else if(pBegin->Name == PROPERTY_NAME)
                                pBegin->Value >>= sName;
                            else if(pBegin->Name == PROPERTY_ACTIVECONNECTION)
                                pBegin->Value >>= xSrcConnection;
                        }

                        // get the source connection

                        sal_Bool bDispose = sal_False;
                        if(sSrcDataSourceName == sDs)
                            xSrcConnection = xDestConnection;
                        else if(!xSrcConnection.is())
                        {
                            Reference< XEventListener> xEvt((::cppu::OWeakObject*)this,UNO_QUERY);
                            showError(::dbaui::createConnection(sSrcDataSourceName,m_xDatabaseContext,getORB(),xEvt,xSrcConnection));
                            bDispose = sal_True;
                        }
                        Reference<XNameAccess> xNameAccess;
                        switch(nCommandType)
                        {
                            case CommandType::TABLE:
                                {
                                    // only for tables
                                    Reference<XTablesSupplier> xSup(xSrcConnection,UNO_QUERY);
                                    if(xSup.is())
                                        xNameAccess = xSup->getTables();
                                }
                                break;
                            case CommandType::QUERY:
                                {
                                    Reference<XQueriesSupplier> xSup(xSrcConnection,UNO_QUERY);
                                    if(xSup.is())
                                        xNameAccess = xSup->getQueries();
                                }
                                break;
                        }

                        // check if this name really exists in the name access
                        if(xNameAccess.is() && xNameAccess->hasByName(sName))
                        {
                            Reference<XPropertySet> xSourceObject;
                            xNameAccess->getByName(sName) >>= xSourceObject;
                            OCopyTableWizard aWizard(getView(),
                                                     xSourceObject,
                                                     xDestConnection,
                                                     getNumberFormatter(),
                                                     getORB());
                            OCopyTable*         pPage1 = new OCopyTable(&aWizard,COPY, sal_False,OCopyTableWizard::WIZARD_DEF_DATA);
                            OWizNameMatching*   pPage2 = new OWizNameMatching(&aWizard);
                            OWizColumnSelect*   pPage3 = new OWizColumnSelect(&aWizard);
                            OWizNormalExtend*   pPage4 = new OWizNormalExtend(&aWizard);

                            aWizard.AddWizardPage(pPage1);
                            aWizard.AddWizardPage(pPage2);
                            aWizard.AddWizardPage(pPage3);
                            aWizard.AddWizardPage(pPage4);
                            aWizard.ActivatePage();

                            if (aWizard.Execute())
                            {
                                Reference<XPropertySet> xTable;
                                switch(aWizard.GetCreateStyle())
                                {
                                    case OCopyTableWizard::WIZARD_DEF:
                                    case OCopyTableWizard::WIZARD_DEF_DATA:
                                        {
                                            xTable = aWizard.createTable();
                                            if(!xTable.is())
                                                break;
                                            if(OCopyTableWizard::WIZARD_DEF == aWizard.GetCreateStyle())
                                                break;
                                        }
                                    case OCopyTableWizard::WIZARD_APPEND_DATA:
                                        {
                                            Reference<XStatement> xStmt = xSrcConnection->createStatement();
                                            if(!xStmt.is())
                                                break;
                                            ::rtl::OUString sSql,sDestName;
                                            ::dbaui::composeTableName(xDestConnection->getMetaData(),xTable,sDestName,sal_False);
                                            // create the sql stmt
                                            if(nCommandType == CommandType::TABLE)
                                            {
                                                sSql = ::rtl::OUString::createFromAscii("SELECT * FROM ");
                                                ::rtl::OUString sComposedName;
                                                ::dbaui::composeTableName(xSrcConnection->getMetaData(),xSourceObject,sComposedName,sal_True);
                                                sSql += sComposedName;
                                            }
                                            else
                                                xSourceObject->getPropertyValue(PROPERTY_COMMAND) >>= sSql;

                                            Reference<XResultSet> xSrcRs = xStmt->executeQuery(sSql);
                                            Reference<XRow> xRow(xSrcRs,UNO_QUERY);
                                            if(!xSrcRs.is() || !xRow.is())
                                                break;

                                            Reference<XResultSet> xDestSet = Reference< XResultSet >(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdb.RowSet")),UNO_QUERY);
                                            Reference<XPropertySet > xProp(xDestSet,UNO_QUERY);
                                            if(xProp.is())
                                            {
                                                xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(xDestConnection));
                                                xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(CommandType::TABLE));
                                                xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(sDestName));
                                                xProp->setPropertyValue(PROPERTY_IGNORERESULT,::cppu::bool2any(sal_True));
                                                Reference<XRowSet> xRowSet(xProp,UNO_QUERY);
                                                xRowSet->execute();
                                            }
                                            Reference< XResultSetUpdate> xDestRsUpd(xDestSet,UNO_QUERY);
                                            Reference< XRowUpdate>       xDestRowUpd(xDestSet,UNO_QUERY);

                                            Reference< XResultSetMetaDataSupplier> xSrcMetaSup(xSrcRs,UNO_QUERY);
                                            Reference< XResultSetMetaData> xMeta = xSrcMetaSup->getMetaData();
                                            sal_Int32 nCount = xMeta->getColumnCount();

                                            sal_Bool bIsAutoIncrement           = aWizard.SetAutoincrement();
                                            ::std::vector<sal_Int32> vColumns   = aWizard.GetColumnPositions();
                                            OSL_ENSURE(sal_Int32(vColumns.size()) == nCount,"Column count isn't correct!");

                                            sal_Int32 nRowCount = 0;
                                            while(xSrcRs->next())
                                            {
                                                ++nRowCount;
                                                xDestRsUpd->moveToInsertRow();
                                                for(sal_Int32 i=1;i<=nCount;++i)
                                                {
                                                    sal_Int32 nPos = vColumns[i-1];
                                                    if(nPos == CONTAINER_ENTRY_NOTFOUND)
                                                        continue;
                                                    if(i == 1 && bIsAutoIncrement)
                                                    {
                                                        xDestRowUpd->updateInt(1,nRowCount);
                                                        continue;
                                                    }

                                                    switch(xMeta->getColumnType(i))
                                                    {
                                                        case DataType::CHAR:
                                                        case DataType::VARCHAR:
                                                            xDestRowUpd->updateString(vColumns[i-1],xRow->getString(i));
                                                            break;
                                                        case DataType::DECIMAL:
                                                        case DataType::NUMERIC:
                                                        case DataType::BIGINT:
                                                            xDestRowUpd->updateDouble(vColumns[i-1],xRow->getDouble(i));
                                                            break;
                                                        case DataType::FLOAT:
                                                            xDestRowUpd->updateFloat(vColumns[i-1],xRow->getFloat(i));
                                                            break;
                                                        case DataType::DOUBLE:
                                                            xDestRowUpd->updateDouble(vColumns[i-1],xRow->getDouble(i));
                                                            break;
                                                        case DataType::LONGVARCHAR:
                                                            xDestRowUpd->updateString(vColumns[i-1],xRow->getString(i));
                                                            break;
                                                        case DataType::LONGVARBINARY:
                                                            xDestRowUpd->updateBytes(vColumns[i-1],xRow->getBytes(i));
                                                            break;
                                                        case DataType::DATE:
                                                            xDestRowUpd->updateDate(vColumns[i-1],xRow->getDate(i));
                                                            break;
                                                        case DataType::TIME:
                                                            xDestRowUpd->updateTime(vColumns[i-1],xRow->getTime(i));
                                                            break;
                                                        case DataType::TIMESTAMP:
                                                            xDestRowUpd->updateTimestamp(vColumns[i-1],xRow->getTimestamp(i));
                                                            break;
                                                        case DataType::BIT:
                                                            xDestRowUpd->updateBoolean(vColumns[i-1],xRow->getBoolean(i));
                                                            break;
                                                        case DataType::TINYINT:
                                                            xDestRowUpd->updateByte(vColumns[i-1],xRow->getByte(i));
                                                            break;
                                                        case DataType::SMALLINT:
                                                            xDestRowUpd->updateShort(vColumns[i-1],xRow->getShort(i));
                                                            break;
                                                        case DataType::INTEGER:
                                                            xDestRowUpd->updateInt(vColumns[i-1],xRow->getInt(i));
                                                            break;
                                                        case DataType::REAL:
                                                            xDestRowUpd->updateDouble(vColumns[i-1],xRow->getDouble(i));
                                                            break;
                                                        case DataType::BINARY:
                                                        case DataType::VARBINARY:
                                                            xDestRowUpd->updateBytes(vColumns[i-1],xRow->getBytes(i));
                                                            break;
                                                        default:
                                                            OSL_ENSURE(0,"Unknown type");
                                                    }
                                                    if(xRow->wasNull())
                                                        xDestRowUpd->updateNull(vColumns[i-1]);
                                                }
                                                xDestRsUpd->insertRow();
                                            }
                                            ::comphelper::disposeComponent(xDestRsUpd);
                                        }
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                        if(bDispose)
                            ::comphelper::disposeComponent(xSrcConnection);
                    }
                }
                else if(aTransferData.HasFormat(SOT_FORMATSTR_ID_HTML) || aTransferData.HasFormat(SOT_FORMAT_RTF))
                {
                    // first get the dest connection
                    Reference<XConnection> xDestConnection;  // supports the service sdb::connection
                    if(!secureConnection(pDSEntry,pDSData,xDestConnection))
                        break;

                    SotStorageStreamRef aStream;
                    Reference<XEventListener> xEvt;
                    ODatabaseImportExport* pImport = NULL;
                    if(aTransferData.HasFormat(SOT_FORMATSTR_ID_HTML))
                    {
                        aTransferData.GetSotStorageStream(SOT_FORMATSTR_ID_HTML,aStream);

                        pImport = new OHTMLImportExport(xDestConnection,getNumberFormatter(),getORB());
                    }
                    else
                    {
                        aTransferData.GetSotStorageStream(SOT_FORMAT_RTF,aStream);
                        pImport = new ORTFImportExport(xDestConnection,getNumberFormatter(),getORB());
                    }
                    xEvt = pImport;
                    SvStream* pStream = (SvStream*)(SotStorageStream*)aStream;
                    pImport->setStream(pStream);
                    pImport->Read();
                }
            }
            catch(SQLException& e)
            {
                showError(SQLExceptionInfo(e));
            }
            catch(Exception& )
            {
                OSL_ASSERT(0);
            }
            break;
    }

    return 1L;  // handled
}
//------------------------------------------------------------------------------
IMPL_LINK(SbaTableQueryBrowser, OnExpandEntry, SvLBoxEntry*, _pParent)
{
    if (_pParent->HasChilds())
        // nothing to to ...
        return 1L;

    ::osl::MutexGuard aGuard(m_aEntryMutex);

    SvLBoxEntry* pFirstParent = m_pTreeView->getListBox()->GetRootLevelParent(_pParent);
    OSL_ENSHURE(pFirstParent,"SbaTableQueryBrowser::OnExpandEntry: No rootlevelparent!");

    DBTreeListModel::DBTreeListUserData* pData = static_cast< DBTreeListModel::DBTreeListUserData* >(_pParent->GetUserData());
    OSL_ENSHURE(pData,"SbaTableQueryBrowser::OnExpandEntry: No user data!");
    SvLBoxString* pString = static_cast<SvLBoxString*>(pFirstParent->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
    OSL_ENSHURE(pString,"SbaTableQueryBrowser::OnExpandEntry: No string item!");

    if(pData->bTable)
    {
        // it could be that we already have a connection
        DBTreeListModel::DBTreeListUserData* pFirstData = static_cast<DBTreeListModel::DBTreeListUserData*>(pFirstParent->GetUserData());
        Reference<XConnection> xConnection(pFirstData->xObject,UNO_QUERY);
        WaitObject aWaitCursor(getBrowserView());
        if(!pFirstData->xObject.is())
        {
            xConnection = connect(pString->GetText());
            pFirstData->xObject = xConnection;
        }
        if(xConnection.is())
        {
            Reference< XWarningsSupplier > xWarnings(xConnection, UNO_QUERY);
            if (xWarnings.is())
                xWarnings->clearWarnings();

            Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
            if(xTabSup.is())
            {
                Image aImage(ModuleRes(TABLE_TREE_ICON));
                populateTree(xTabSup->getTables(),_pParent,aImage);
                Reference<XContainer> xCont(xTabSup->getTables(),UNO_QUERY);
                if(xCont.is())
                    // add as listener to know when elements are inserted or removed
                    xCont->addContainerListener(this);
            }

            Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
            if(xViewSup.is())
            {
                Image aImage(ModuleRes(VIEW_TREE_ICON));
                populateTree(xViewSup->getViews(),_pParent,aImage);
                Reference<XContainer> xCont(xViewSup->getViews(),UNO_QUERY);
                if(xCont.is())
                    // add as listener to get notified if elements are inserted or removed
                    xCont->addContainerListener(this);
            }

            if (xWarnings.is())
            {
                SQLExceptionInfo aInfo(xWarnings->getWarnings());
                if (aInfo.isValid() && sal_False)
                {
                    SQLContext aContext;
                    aContext.Message = String(ModuleRes(STR_OPENTABLES_WARNINGS));
                    aContext.Details = String(ModuleRes(STR_OPENTABLES_WARNINGS_DETAILS));
                    aContext.NextException = aInfo.get();
                    aInfo = aContext;
                    showError(aInfo);
                }
                // TODO: we need a better concept for these warnings:
                // something like "don't show any warnings for this datasource, again" would be nice
                // But this requires an extension of the InteractionHandler and an additional property on the data source
            }
        }
        else
            return 0L;
                // 0 indicates that an error occured
    }
    else // we have to expand the queries
    {
        Any aValue;
        try
        {
            aValue = m_xDatabaseContext->getByName(pString->GetText());
        }
        catch(Exception&)
        { }
        Reference<XQueryDefinitionsSupplier> xQuerySup;
        if(::cppu::extractInterface(xQuerySup,aValue) && xQuerySup.is())
        {
            Image aImage(ModuleRes(QUERY_TREE_ICON));
            Reference<XNameAccess> xNames = xQuerySup->getQueryDefinitions();
            Reference<XContainer> xCont(xNames,UNO_QUERY);
            if(xCont.is())
            {
                // add as listener to know when elements are inserted or removed
                xCont->addContainerListener(this);
                // remember the the nameaccess for elementRemoved or elementInserted
                if(!pData->xObject.is())
                    pData->xObject = xCont;
            }
            populateTree(xNames,_pParent,aImage);
        }
    }
    return 1L;
}
//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::isSelected(SvLBoxEntry* _pEntry) const
{
    SvLBoxItem* pTextItem = _pEntry ? _pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING) : NULL;
    if (pTextItem)
        return static_cast<OBoldListboxString*>(pTextItem)->isEmphasized();
    else
        DBG_ERROR("SbaTableQueryBrowser::isSelected: invalid entry!");
    return sal_False;
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::select(SvLBoxEntry* _pEntry, sal_Bool _bSelect)
{
    SvLBoxItem* pTextItem = _pEntry ? _pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING) : NULL;
    if (pTextItem)
    {
        static_cast<OBoldListboxString*>(pTextItem)->emphasize(_bSelect);
        m_pTreeModel->InvalidateEntry(_pEntry);
    }
    else
        DBG_ERROR("SbaTableQueryBrowser::select: invalid entry!");
}

//------------------------------------------------------------------------------
void SbaTableQueryBrowser::selectPath(SvLBoxEntry* _pEntry, sal_Bool _bSelect)
{
    while (_pEntry)
    {
        select(_pEntry, _bSelect);
        _pEntry = m_pTreeModel->GetParent(_pEntry);
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaTableQueryBrowser, OnSelectEntry, SvLBoxEntry*, _pEntry)
{
    ::osl::MutexGuard aGuard(m_aEntryMutex);
    // reinitialize the rowset
    // but first check if it is necessary
    // get all old properties
    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
    ::rtl::OUString aOldName;
    xProp->getPropertyValue(PROPERTY_COMMAND) >>= aOldName;
    sal_Int32 nOldType;
    xProp->getPropertyValue(PROPERTY_COMMANDTYPE) >>= nOldType;
    Reference<XConnection> xOldConnection;
    ::cppu::extractInterface(xOldConnection,xProp->getPropertyValue(PROPERTY_ACTIVECONNECTION));
    // the name of the table or query
    SvLBoxString* pString = (SvLBoxString*)_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
    OSL_ENSURE(pString,"There must be a string item!");
    ::rtl::OUString aName(pString->GetText().GetBuffer());

    // get the entry for the tables or queries
    SvLBoxEntry* pTables = m_pTreeModel->GetParent(_pEntry);
    DBTreeListModel::DBTreeListUserData* pTablesData = static_cast<DBTreeListModel::DBTreeListUserData*>(pTables->GetUserData());

    // get the entry for the datasource
    SvLBoxEntry* pConnection = m_pTreeModel->GetParent(pTables);
    DBTreeListModel::DBTreeListUserData* pConData = static_cast<DBTreeListModel::DBTreeListUserData*>(pConnection->GetUserData());

    Reference<XConnection> xConnection(pConData->xObject,UNO_QUERY);
    sal_Int32 nCommandType = pTablesData->bTable ? CommandType::TABLE : CommandType::QUERY;

    // check if need to rebuild the rowset
    sal_Bool bRebuild = xOldConnection != xConnection || nOldType != nCommandType || aName != aOldName;

    Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(),UNO_QUERY);
    bRebuild |= !xLoadable->isLoaded();
    if(bRebuild)
    {
        try
        {
            // if table was selected before flush it
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                if(pData) // can be null because the first load can be failed @see below
                {
                    Reference<XFlushable> xFlush(pData->xObject,UNO_QUERY);
                    if(xFlush.is())
                        xFlush->flush();
                }
            }
        }
        catch(Exception&)
        {
            OSL_ENSURE(0,"Object can not be flushed!");
        }
        try
        {
            WaitObject aWaitCursor(getBrowserView());

            // tell the old entry it has been deselected
            selectPath(m_pCurrentlyDisplayed, sal_False);
            m_pCurrentlyDisplayed = _pEntry;
            // tell the new entry it has been selected
            selectPath(m_pCurrentlyDisplayed, sal_True);

            // get the name of the data source currently selected
            ::rtl::OUString sDataSourceName;
            SvLBoxEntry* pEntry = m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed);

            if (pEntry)
            {
                SvLBoxItem* pTextItem = pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                if (pTextItem)
                    sDataSourceName = static_cast<SvLBoxString*>(pTextItem)->GetText();
            }
            if(!xConnection.is())
            {
                xConnection = connect(sDataSourceName);
                pConData->xObject = xConnection;
            }
            if(!xConnection.is())
            {
                unloadForm(sal_False,sal_False);
                return 0L;
            }

            Reference<XNameAccess> xNameAccess;
            switch(nCommandType)
            {
                case CommandType::TABLE:
                    {
                        // only for tables
                        if(!pTablesData->xObject.is())
                        {
                            Reference<XTablesSupplier> xSup(xConnection,UNO_QUERY);
                            if(xSup.is())
                                xNameAccess = xSup->getTables();

                            pTablesData->xObject = xNameAccess;
                        }
                        else
                            xNameAccess = Reference<XNameAccess>(pTablesData->xObject,UNO_QUERY);
                    }
                    break;
                case CommandType::QUERY:
                    {
                        Reference<XQueriesSupplier> xSup(xConnection,UNO_QUERY);
                        if(xSup.is())
                            xNameAccess = xSup->getQueries();
                    }
                    break;
            }


            if(xNameAccess.is() && xNameAccess->hasByName(aName))
            {
                DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                if(!pData)
                {
                    DBTreeListModel::DBTreeListUserData* pTableData = new DBTreeListModel::DBTreeListUserData;
                    pTableData->bTable = pTablesData->bTable;
                    if(xNameAccess->getByName(aName) >>= pTableData->xObject) // remember the table or query object
                        m_pCurrentlyDisplayed->SetUserData(pTableData);
                    else
                        delete pTableData; // do you see the data can be null or not set
                }
            }
            // the values allowing the RowSet to re-execute
            xProp->setPropertyValue(PROPERTY_DATASOURCENAME,makeAny(sDataSourceName));
                // set this _before_ setting the connection, else the rowset would rebuild it ...
            xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(xConnection));
            xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(nCommandType));
            xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(aName));
            // the formatter depends on the data source we're working on, so rebuild it here ...
            initFormatter();

            // switch the grid to design mode while loading
            getBrowserView()->getGridControl()->setDesignMode(sal_True);
            InitializeForm(getRowSet());

            // load the row set
            {
                FormErrorHelper aNoticeErrors(this);
                if (xLoadable->isLoaded())
                    // reload does not work if not already loaded
                    xLoadable->reload();
                else
                    xLoadable->load();

                sal_Bool bLoadSuccess = !errorOccured();
                // initialize the model
                InitializeGridModel(getFormComponent());
                // reload ...
                // TODO: why this reload ... me thinks the GridModel can't handle beeing initialized when the form
                // is already loaded, but I'm not sure ...
                // have to change this, reloading is much too expensive ...
                if (xLoadable->isLoaded() && bLoadSuccess)
                    xLoadable->reload();
                FormLoaded(sal_True);
            }
        }
        catch(SQLException& e)
        {
            showError(SQLExceptionInfo(e));
            // reset the values
            xProp->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,Any());
        }
        catch(Exception&)
        {
            // reset the values
            xProp->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,Any());
        }
    }
    return 0L;
}
// -----------------------------------------------------------------------------
SvLBoxEntry* SbaTableQueryBrowser::getNameAccessFromEntry(const Reference<XNameAccess>& _rxNameAccess)
{
    SvLBoxEntry* pDSLoop = m_pTreeView->getListBox()->FirstChild(NULL);
    SvLBoxEntry* pContainer = NULL;
    while (pDSLoop)
    {
        pContainer  = m_pTreeView->getListBox()->GetEntry(pDSLoop,0);
        DBTreeListModel::DBTreeListUserData* pQueriesData = static_cast<DBTreeListModel::DBTreeListUserData*>(pContainer->GetUserData());
        if(pQueriesData && pQueriesData->xObject.get() == _rxNameAccess.get())
            break;

        pContainer  = m_pTreeView->getListBox()->GetEntry(pDSLoop,1);
        DBTreeListModel::DBTreeListUserData* pTablesData = static_cast<DBTreeListModel::DBTreeListUserData*>(pContainer->GetUserData());
        if(pTablesData && pTablesData->xObject.get() == _rxNameAccess.get())
            break;

        pDSLoop     = m_pTreeView->getListBox()->NextSibling(pDSLoop);
        pContainer  = NULL;
    }
    return pContainer;
}
// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    // first search for a definition container where we can insert this element

    SvLBoxEntry* pEntry = getNameAccessFromEntry(xNames);
    if(pEntry)  // found one
    {
        // insert the new entry into the tree
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntry->GetUserData());
        OSL_ENSURE(pData,"elementInserted: There must be user data for this type!");
        Image aImage(ModuleRes(pData->bTable ? TABLE_TREE_ICON : QUERY_TREE_ICON));
        SvLBoxEntry* pNewEntry = m_pTreeView->getListBox()->InsertEntry(::comphelper::getString(_rEvent.Accessor),
            aImage, aImage, pEntry, sal_False);

        if(pData->bTable)
        { // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
            DBTreeListModel::DBTreeListUserData* pNewData = new DBTreeListModel::DBTreeListUserData;
            pNewData->bTable  = pData->bTable;
            ::cppu::extractInterface(pNewData->xObject,_rEvent.Element);// remember the new element
            pNewEntry->SetUserData(pNewData);
        }
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   // a new datasource has been added to the context
        // the name of the new ds
        ::rtl::OUString sNewDS;
        _rEvent.Accessor >>= sNewDS;

        // add new entries to the list box model
        Image a, b, c;  // not interested in  reusing them
        String d, e;
        implAddDatasource(sNewDS, a, d, b, e, c);
    }
    else
        SbaXDataBrowserController::elementInserted(_rEvent);
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    // get the top-level representing the removed data source
    // and search for the queries and tables
    SvLBoxEntry* pEntry = getNameAccessFromEntry(xNames);
    if (pEntry)
    { // a query or table has been removed
        String aName = ::comphelper::getString(_rEvent.Accessor).getStr();
        if (m_pCurrentlyDisplayed && m_pTreeView->getListBox()->GetEntryText(m_pCurrentlyDisplayed) == aName)
        {
            // we need to remember the old value
            SvLBoxEntry* pTemp = m_pCurrentlyDisplayed;
            // unload
            unloadForm(sal_False, sal_False); // don't dispose the connection, don't flush
            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pTemp->GetUserData());
            delete pData; // the data could be null because we have a table which isn't correct
            m_pTreeModel->Remove(pTemp);
        }
        else
        {
            // remove the entry from the model
            SvLBoxEntry* pChild = m_pTreeModel->FirstChild(pEntry);
            while(pChild)
            {
                if (m_pTreeView->getListBox()->GetEntryText(pChild) == aName)
                {
                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pChild->GetUserData());
                    delete pData;
                    m_pTreeModel->Remove(pChild);
                    break;
                }
                pChild = m_pTreeModel->NextSibling(pChild);
            }
        }
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   // a datasource has been removed from the context

        // the name
        ::rtl::OUString sNewDS;
        _rEvent.Accessor >>= sNewDS;
        String sNewDatasource = sNewDS;

        // get the top-level representing the removed data source
        SvLBoxEntry* pDSLoop = m_pTreeView->getListBox()->FirstChild(NULL);
        while (pDSLoop)
        {
            if (m_pTreeView->getListBox()->GetEntryText(pDSLoop) == sNewDatasource)
                break;

            pDSLoop = m_pTreeView->getListBox()->NextSibling(pDSLoop);
        }

        if (pDSLoop)
        {
            if (isSelected(pDSLoop))
            {   // a table or query belonging to the deleted data source is currently beeing displayed.
                OSL_ENSURE(m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed) == pDSLoop, "SbaTableQueryBrowser::elementRemoved: inconsistence (1)!");
                unloadForm();
            }
            else
                OSL_ENSURE(
                        (NULL == m_pCurrentlyDisplayed)
                    ||  (m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed) != pDSLoop), "SbaTableQueryBrowser::elementRemoved: inconsistence (2)!");

            // look for user data to delete
            SvTreeEntryList* pList = m_pTreeModel->GetChildList(pDSLoop);
            if(pList)
            {
                SvLBoxEntry* pEntryLoop = static_cast<SvLBoxEntry*>(pList->First());
                while (pEntryLoop)
                {
                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntryLoop->GetUserData());
                    delete pData;
                    pEntryLoop = static_cast<SvLBoxEntry*>(pList->Next());
                }
            }
            // remove the entry. This should remove all children, too.
            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pDSLoop->GetUserData());
            delete pData;
            m_pTreeModel->Remove(pDSLoop);
        }
        else
            DBG_ERROR("SbaTableQueryBrowser::elementRemoved: unknown datasource name!");
    }
    else
        SbaXDataBrowserController::elementRemoved(_rEvent);
}

// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    SvLBoxEntry* pEntry = getNameAccessFromEntry(xNames);
    if (pEntry)
    {    // a table or query as been replaced
        String aName = ::comphelper::getString(_rEvent.Accessor).getStr();
        if (m_pCurrentlyDisplayed && m_pTreeView->getListBox()->GetEntryText(m_pCurrentlyDisplayed) == aName)
        {
            // we need to remember the old value
            SvLBoxEntry* pTemp = m_pCurrentlyDisplayed;
            unloadForm(sal_False); // don't dispose the connection

            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pTemp->GetUserData());
            OSL_ENSURE(pData,"elementReplaced: There must be user data!");
            if(pData->bTable)
            { // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                ::cppu::extractInterface(pData->xObject,_rEvent.Element);// remember the new element
            }
            else
            {
                delete pData;
                pTemp->SetUserData(NULL);
            }
        }
        else
        {
            // find the entry for this name
            SvLBoxEntry* pChild = m_pTreeModel->FirstChild(pEntry);
            while(pChild)
            {
                if (m_pTreeView->getListBox()->GetEntryText(pChild) == aName)
                {
                    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pChild->GetUserData());
                    OSL_ENSURE(pData,"elementReplaced: There must be user data!");
                    if(pData->bTable)
                    { // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                        ::cppu::extractInterface(pData->xObject,_rEvent.Element);// remember the new element
                    }
                    else
                    {
                        delete pData;
                        pChild->SetUserData(NULL);
                    }
                    break;
                }
                pChild = m_pTreeModel->NextSibling(pChild);
            }
        }
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   // a datasource has been replaced in the context
        DBG_ERROR("SbaTableQueryBrowser::elementReplaced: no support for replaced data sources!");
            // very suspicious: the database context should not allow to replace data source, only to register
            // and revoke them
    }
    else
        SbaXDataBrowserController::elementReplaced(_rEvent);
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::closeConnection(SvLBoxEntry* _pDSEntry,sal_Bool _bDisposeConnection)
{
    DBG_ASSERT(_pDSEntry, "SbaTableQueryBrowser::closeConnection: invalid entry (NULL)!");
    OSL_ENSHURE(m_pTreeView->getListBox()->GetRootLevelParent(_pDSEntry) == _pDSEntry, "SbaTableQueryBrowser::closeConnection: invalid entry (not top-level)!");

    // if one of the entries of the given DS is displayed currently, unload the form
    if (m_pCurrentlyDisplayed && (m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed) == _pDSEntry))
        unloadForm(_bDisposeConnection);

    // collapse the query/table container
    for (SvLBoxEntry* pContainers = m_pTreeModel->FirstChild(_pDSEntry); pContainers; pContainers= m_pTreeModel->NextSibling(pContainers))
    {
        m_pTreeView->getListBox()->Collapse(pContainers);
        m_pTreeView->getListBox()->EnableExpandHandler(pContainers);
        // and delete their children (they are connection-relative)
        for (SvLBoxEntry* pElements = m_pTreeModel->FirstChild(pContainers); pElements; )
        {
            SvLBoxEntry* pRemove = pElements;
            pElements= m_pTreeModel->NextSibling(pElements);
            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pRemove->GetUserData());
            delete pData;
            m_pTreeModel->Remove(pRemove);
        }
    }
    // collapse the entry itself
    m_pTreeView->getListBox()->Collapse(_pDSEntry);

    // get the connection
    DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(_pDSEntry->GetUserData());
    if(_bDisposeConnection) // and dispose/reset it
    {
        Reference< XComponent >  xComponent(pData->xObject, UNO_QUERY);
        if (xComponent.is())
        {
            Reference< ::com::sun::star::lang::XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
            xComponent->removeEventListener(xEvtL);
        }
        ::comphelper::disposeComponent(pData->xObject);
    }
    pData->xObject.clear();
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::unloadForm(sal_Bool _bDisposeConnection, sal_Bool _bFlushData)
{
    if (!m_pCurrentlyDisplayed)
        // nothing to do
        return;

    SvLBoxEntry* pDSEntry = m_pTreeView->getListBox()->GetRootLevelParent(m_pCurrentlyDisplayed);

    // de-select the path for the currently displayed table/query
    if (m_pCurrentlyDisplayed)
    {
        if (_bFlushData)
        {
            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
            try
            {
                if(pData)
                {
                    Reference<XFlushable> xFlush(pData->xObject, UNO_QUERY);
                    if(xFlush.is())
                        xFlush->flush();
                }
            }
            catch (RuntimeException&)
            {
                OSL_ENSURE(sal_False, "SbaTableQueryBrowser::unloadForm: could not flush the data (caught a RuntimeException)!");
            }
        }
        selectPath(m_pCurrentlyDisplayed, sal_False);
    }
    m_pCurrentlyDisplayed = NULL;

    try
    {
        // get the active connection. We need to dispose it.
        Reference< XPropertySet > xProp(getRowSet(),UNO_QUERY);
        Reference< XConnection > xConn;
        ::cppu::extractInterface(xConn, xProp->getPropertyValue(PROPERTY_ACTIVECONNECTION));
#ifdef DEBUG
        {
            Reference< XComponent > xComp;
            ::cppu::extractInterface(xComp, xProp->getPropertyValue(PROPERTY_ACTIVECONNECTION));
        }
#endif

        // unload the form
        Reference< XLoadable > xLoadable(getRowSet(), UNO_QUERY);
        xLoadable->unload();

        // clear the grid control
        Reference< XNameContainer >  xColContainer(getControlModel(), UNO_QUERY);
        // first we have to clear the grid
        {
            Sequence< ::rtl::OUString > aNames = xColContainer->getElementNames();
            const ::rtl::OUString* pBegin   = aNames.getConstArray();
            const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
            for (; pBegin != pEnd;++pBegin)
                xColContainer->removeByName(*pBegin);
        }

        // dispose the connection
        if(_bDisposeConnection)
        {
            DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pDSEntry->GetUserData());
            if(pData)
            {
                Reference< XComponent >  xComponent(pData->xObject, UNO_QUERY);
                if (xComponent.is())
                {
                    Reference< ::com::sun::star::lang::XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
                    xComponent->removeEventListener(xEvtL);
                }
                pData->xObject = NULL;
            }
            ::comphelper::disposeComponent(xConn);
        }
    }
    catch(SQLException& e)
    {
        showError(SQLExceptionInfo(e));
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "SbaTableQueryBrowser::unloadForm: could not reset the form");
    }
}
// -------------------------------------------------------------------------
void SAL_CALL SbaTableQueryBrowser::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    // first initialize the parent
    SbaXDataBrowserController::initialize( aArguments );

    Reference<XConnection> xConnection;
    PropertyValue aValue;
    const Any* pBegin   = aArguments.getConstArray();
    const Any* pEnd     = pBegin + aArguments.getLength();

    ::rtl::OUString aTableName,aCatalogName,aSchemaName;
    sal_Bool bEsacpeProcessing = sal_True;
    m_nDefaultCommandType = -1;
    for(;pBegin != pEnd;++pBegin)
    {
        if((*pBegin >>= aValue) && aValue.Name == PROPERTY_DATASOURCENAME)
            aValue.Value >>= m_sDefaultDataSourceName;
        else if(aValue.Name == PROPERTY_COMMANDTYPE)
            aValue.Value >>= m_nDefaultCommandType;
        else if(aValue.Name == PROPERTY_COMMAND)
            aValue.Value >>= m_sDefaultCommand;
        else if(aValue.Name == PROPERTY_ACTIVECONNECTION)
            ::cppu::extractInterface(xConnection,aValue.Value);
        else if(aValue.Name == PROPERTY_UPDATE_CATALOGNAME)
            aValue.Value >>= aCatalogName;
        else if(aValue.Name == PROPERTY_UPDATE_SCHEMANAME)
            aValue.Value >>= aSchemaName;
        else if(aValue.Name == PROPERTY_UPDATE_TABLENAME)
            aValue.Value >>= aTableName;
        else if(aValue.Name == PROPERTY_USE_ESCAPE_PROCESSING)
            bEsacpeProcessing = ::cppu::any2bool(aValue.Value);
        else if(aValue.Name == PROPERTY_SHOWTREEVIEW)
        {
            try
            {
                if(::cppu::any2bool(aValue.Value))
                    showExplorer();
                else
                    hideExplorer();
            }
            catch(Exception&)
            {
            }
        }
        else if(aValue.Name == PROPERTY_SHOWTREEVIEWBUTTON)
        {
            try
            {
                if(!::cppu::any2bool(aValue.Value) && getView())
                {
                    // hide the explorer and the separator
                    getView()->getToolBox()->HideItem(ID_BROWSER_EXPLORER);
                    getView()->getToolBox()->HideItem(getView()->getToolBox()->GetItemId(getView()->getToolBox()->GetItemPos(ID_BROWSER_EXPLORER)+1));
                    getView()->getToolBox()->ShowItem(ID_BROWSER_CLOSE);
                }
            }
            catch(Exception&)
            {
            }
        }
    }

    if(m_sDefaultDataSourceName.getLength() && m_sDefaultCommand.getLength() && m_nDefaultCommandType != -1)
    {
        SvLBoxEntry* pDataSource = m_pTreeView->getListBox()->GetEntryPosByName(m_sDefaultDataSourceName,NULL);
        if(pDataSource)
        {
            m_pTreeView->getListBox()->Expand(pDataSource);
            SvLBoxEntry* pCommandType = NULL;
            if(CommandType::TABLE == m_nDefaultCommandType)
                pCommandType = m_pTreeView->getListBox()->GetModel()->GetEntry(pDataSource,1);
            else if(CommandType::QUERY == m_nDefaultCommandType)
                pCommandType = m_pTreeView->getListBox()->GetModel()->GetEntry(pDataSource,0);
            if(pCommandType)
            {
                // we need to expand the command
                m_pTreeView->getListBox()->Expand(pCommandType);
                SvLBoxEntry* pCommand = m_pTreeView->getListBox()->GetEntryPosByName(m_sDefaultCommand,pCommandType);
                if(pCommand)
                   m_pTreeView->getListBox()->Select(pCommand);
            }
            else // we have a command and need to display this in the rowset
            {
                Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
                if(xProp.is())
                {
                    Reference< ::com::sun::star::form::XLoadable >  xLoadable(xProp,UNO_QUERY);
                    try
                    {
                        // the values allowing the RowSet to re-execute
                        xProp->setPropertyValue(PROPERTY_DATASOURCENAME,makeAny(m_sDefaultDataSourceName));
                            // set this _before_ setting the connection, else the rowset would rebuild it ...
                        if(xConnection.is())
                            xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(xConnection));
                        xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(m_nDefaultCommandType));
                        xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(m_sDefaultCommand));

                        xProp->setPropertyValue(PROPERTY_UPDATE_CATALOGNAME,makeAny(aCatalogName));
                        xProp->setPropertyValue(PROPERTY_UPDATE_SCHEMANAME,makeAny(aSchemaName));
                        xProp->setPropertyValue(PROPERTY_UPDATE_TABLENAME,makeAny(aTableName));
                        xProp->setPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING,::cppu::bool2any(bEsacpeProcessing));
                        // the formatter depends on the data source we're working on, so rebuild it here ...
                        initFormatter();
                        // switch the grid to design mode while loading
                        getBrowserView()->getGridControl()->setDesignMode(sal_True);
                        InitializeForm(getRowSet());
                        {
                            FormErrorHelper aHelper(this);
                            // load the row set
                            if (xLoadable->isLoaded())
                                // reload does not work if not already loaded
                                xLoadable->reload();
                            else
                                xLoadable->load();
                            // initialize the model
                            InitializeGridModel(getFormComponent());
                            Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(),UNO_QUERY);
                            if (xLoadable->isLoaded() && !errorOccured())
                                xLoadable->reload();
                        }

                        FormLoaded(sal_True);
                    }
                    catch(SQLException& e)
                    {
                        showError(SQLExceptionInfo(e));
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
        }
    }
}
// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::haveExplorer() const
{
    return m_pTreeView && m_pTreeView->IsVisible();
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::hideExplorer()
{
    if (!haveExplorer())
        return;
    if (!getBrowserView())
        return;

    m_pTreeView->Hide();
    m_pSplitter->Hide();
    getBrowserView()->Resize();

    InvalidateFeature(ID_BROWSER_EXPLORER);
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::showExplorer()
{
    if (haveExplorer())
        return;

    if (!getBrowserView())
        return;

    m_pTreeView->Show();
    m_pSplitter->Show();
    getBrowserView()->Resize();

    InvalidateFeature(ID_BROWSER_EXPLORER);
}
// -----------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::secureConnection(SvLBoxEntry* _pDSEntry,void* pDSData,Reference<XConnection>& _xConnection)
{
    if(_pDSEntry)
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pDSData);
        ::rtl::OUString aDSName;
        SvLBoxItem* pTextItem = _pDSEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
        if (pTextItem)
            aDSName = static_cast<SvLBoxString*>(pTextItem)->GetText();

        if (pData)
            _xConnection = Reference<XConnection>(pData->xObject,UNO_QUERY);
        if(!_xConnection.is() && pData)
        {
            _xConnection = connect(aDSName);
            pData->xObject = _xConnection; // share the conenction with the querydesign
        }
    }

    return _xConnection.is();
}
// .........................................................................
}   // namespace dbaui
// .........................................................................



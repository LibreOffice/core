/*************************************************************************
 *
 *  $RCSfile: unodatbr.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-09 07:34:47 $
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

//#ifndef _DLGSIZE_HXX
//#include "dlgsize.hxx"
//#endif
//#ifndef _SBA_SBATTRDLG_HXX
//#include "dlgattr.hxx"
//#endif
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

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
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

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
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
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XRESULTSETACCESS_HPP_
#include <com/sun/star/sdb/XResultSetAccess.hpp>
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
#ifndef _DBA_REGISTRATION_HELPER_HXX_
#include "registrationhelper.hxx"
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
    return ::rtl::OUString::createFromAscii("org.openoffice.dbaccess.ODatabaseBrowser");
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
    return *(new SbaTableQueryBrowser(_rxFactory));
}
//------------------------------------------------------------------------------
SbaTableQueryBrowser::SbaTableQueryBrowser(const Reference< XMultiServiceFactory >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_pTreeModel(NULL)
    ,m_pTreeView(NULL)
    ,m_pSplitter(NULL)
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
void SAL_CALL SbaTableQueryBrowser::dispose()
{
    // reset the content's tree view: it holds a reference to our model which is to be deleted immediately,
    // and it will live longer than we do.
    if (getContent())
        getContent()->setTreeView(NULL);

    // clear the user data of the tree model
    SvLBoxEntry* pEntryLoop = m_pTreeModel->First();
    while (pEntryLoop)
    {
        DBTreeListModel::DBTreeListUserData* pData = static_cast<DBTreeListModel::DBTreeListUserData*>(pEntryLoop->GetUserData());
         delete pData;
         pEntryLoop = m_pTreeModel->Next(pEntryLoop);
    }
    // clear the tree model
    delete m_pTreeModel;
    m_pTreeModel = NULL;

    SbaXDataBrowserController::dispose();
}

//------------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::Construct(Window* pParent)
{
    if (!SbaXDataBrowserController::Construct(pParent))
        return sal_False;

    // some help ids
    if (getContent() && getContent()->getVclControl())
    {

        // create controls and set sizes
        const long  nFrameWidth = getContent()->LogicToPixel( Size( 3, 0 ), MAP_APPFONT ).Width();

        m_pSplitter = new Splitter(getContent(),WB_HSCROLL);
        m_pSplitter->SetPosSizePixel( Point(0,0), Size(nFrameWidth,0) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );
        m_pSplitter->Show();

        m_pTreeView = new DBTreeView(getContent(), WB_TABSTOP);
        m_pTreeView->Show();
        m_pTreeView->SetPreExpandHandler(LINK(this, SbaTableQueryBrowser, OnExpandEntry));

        // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
        m_pSplitter->SetSplitPosPixel( getContent()->LogicToPixel( Size( 80, 0 ), MAP_APPFONT ).Width() );

        getContent()->setSplitter(m_pSplitter);
        getContent()->setTreeView(m_pTreeView);

        // fill view with data
        m_pTreeModel = new DBTreeListModel;
        m_pTreeView->setModel(m_pTreeModel);
        m_pTreeView->setSelectHdl(LINK(this, SbaTableQueryBrowser, OnSelectEntry));
        initializeTreeModel();

        // TODO
        //  getContent()->getVclControl()->GetDataWindow().SetUniqueId(UID_DATABROWSE_DATAWINDOW);
        //  getContent()->getVclControl()->SetHelpId(HID_CTL_TABBROWSER);
        //  if (getContent()->getVclControl()->GetHeaderBar())
            //  getContent()->getVclControl()->GetHeaderBar()->SetHelpId(HID_DATABROWSE_HEADER);
    }

    return sal_True;
}
// -------------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::InitializeForm(const Reference< ::com::sun::star::sdbc::XRowSet > & _rxForm)
{
    try
    {
        // set the parent of the ::com::sun::star::form::Form
        Reference< ::com::sun::star::form::XForm >  xForm(_rxForm, UNO_QUERY);
//      {
//          DbEnvironmentAccess aEnv;
//          if (aEnv.getEnv().is())
//          {
//              Reference< ::com::sun::star::sdb::XDatabaseAccess >  xDatabase = aEnv.getEnv()->getDatabaseAccess(m_xDatabase->Name());
//              if (xDatabase.is())
//                  xForm->setParent(xDatabase->getConnection(::rtl::OUString(),::rtl::OUString()));
//          }
//      }

        // we send all properties at once, maybe the implementation is clever enough to handle one big PropertiesChanged
        // more effective than many small PropertyChanged ;)
        Sequence< ::rtl::OUString> aProperties(6);
        Sequence< Any> aValues(6);

        // is the filter intially applied ?
//      const SfxBoolItem* pFilterApplied = (const SfxBoolItem*)getDefinition()->GetObjAttrs()->GetItem(SBA_DEF_FILTERAPPLIED);
//      aProperties.getArray()[0] = PROPERTY_APPLYFILTER;
//      aValues.getArray()[0] = ::utl::makeBoolAny((sal_Bool)(pFilterApplied ? pFilterApplied->GetValue() : sal_False));
//
//      // cursor source
//      aProperties.getArray()[1] = PROPERTY_COMMAND;
//      aValues.getArray()[1] <<= ::rtl::OUString(m_xDefinition->Name());
//          // may be replaced with a sql statement below
//
//      // cursor source type
//      aProperties.getArray()[2] = PROPERTY_COMMANDTYPE;
//
//      sal_Int32 nType = ::com::sun::star::sdb::CommandType::COMMAND;
//      sal_Bool bEscapeProcessing = sal_True;
//      switch (m_xDefinition->GetKind())
//      {
//          case dbTable : nType = ::com::sun::star::sdb::CommandType::TABLE; break;
//          case dbQuery :
//              if (m_xDefinition->Name().Len())
//                  nType = ::com::sun::star::sdb::CommandType::QUERY;
//              else
//              {   // it is a temporary query object
//                  SbaQueryDef* pQuery = PTR_CAST(SbaQueryDef, &m_xDefinition);
//                  DBG_ASSERT(pQuery, "SbaTableQueryBrowser::InitializeForm : the object is of kind dbQuery but no query def ?");
//
//                  if (pQuery->IsNative())
//                      bEscapeProcessing = sal_False;
//
//                  aValues.getArray()[1] <<= ::rtl::OUString(pQuery->Statement());
//              }
//              break;
//
//          default : DBG_ERROR("SbaTableQueryBrowser::InitializeForm : the definition has to be a table or a query !");
//      }
//      aValues.getArray()[2] <<= nType;
//
//      // escape processing ?
//      aProperties.getArray()[3] = PROPERTY_USE_ESCAPE_PROCESSING;
//      aValues.getArray()[3].setValue(&bEscapeProcessing,::getBooleanCppuType());
//
//      // the initial filter
//      const SfxStringItem* pFilterSort = (const SfxStringItem*)getDefinition()->GetObjAttrs()->GetItem(SBA_DEF_FILTER);
//      aProperties.getArray()[4] = PROPERTY_FILTER;
//      aValues.getArray()[4] <<= ::rtl::OUString(pFilterSort ? ::rtl::OUString(pFilterSort->GetValue()) : ::rtl::OUString());
//
//      // the initial ordering
//      pFilterSort = (const SfxStringItem*)getDefinition()->GetObjAttrs()->GetItem(SBA_DEF_SORT);
//      aProperties.getArray()[5] = PROPERTY_ORDER;
//      aValues.getArray()[5] <<= ::rtl::OUString(pFilterSort ? ::rtl::OUString(pFilterSort->GetValue()) : ::rtl::OUString());
//
//      Reference< XMultiPropertySet >  xFormMultiSet(_rxForm, UNO_QUERY);
//      xFormMultiSet->setPropertyValues(aProperties, aValues);
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

    // get the formats supplier of the database we're working with
    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = getNumberFormatter()->getNumberFormatsSupplier();



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
        xColumns->getByName(*pBegin) >>= xColumn;

        sal_Bool bIsFormatted = sal_False;
        sal_Bool bFormattedIsNumeric = sal_True;
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
        xCurrentCol->setPropertyValue(PROPERTY_ALIGN, xColumn->getPropertyValue(PROPERTY_ALIGN));

        // ... the 'comment' property as helptext (will usually be shown as header-tooltip)

        Any aDescription; aDescription <<= ::rtl::OUString();
        if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION))
            aDescription <<= comphelper::getString(xColumn->getPropertyValue(PROPERTY_DESCRIPTION));
        xCurrentCol->setPropertyValue(PROPERTY_HELPTEXT, xColumn->getPropertyValue(PROPERTY_DESCRIPTION));

        xColContainer->insertByName(*pBegin, makeAny(xCurrentCol));
    }

//  SFX_ITEMSET_GET(*pDef->GetObjAttrs(), pRowHeight, SbaColRowSizeItem, SBA_DEF_ROWHEIGHT, sal_True);
//  SFX_ITEMSET_GET(*pDef->GetObjAttrs(), pRowHeightDefault, SfxBoolItem, SBA_DEF_ROWHEIGHT_DEFAULT, sal_True);
//  if (xGridSet.is() && pRowHeight && pRowHeightDefault)
//  {
//      Any aHeight;
//      if (pRowHeightDefault->GetValue())
//      {
//          Reference< XPropertyState >  xPropState(xGridSet, UNO_QUERY);
//          if (xPropState.is())
//          {
//              try { aHeight = xPropState->getPropertyDefault(PROPERTY_ROW_HEIGHT); } catch(Exception&) { } ;
//          }
//      }
//      else
//          aHeight <<= pRowHeight->GetLogicValue();
//      xGridSet->setPropertyValue(PROPERTY_ROW_HEIGHT, aHeight);
//  }
//
//  if (xGridSet.is())
//  {
//      ::com::sun::star::awt::FontDescriptor aFont = BuildFontFromItems(pDef->GetObjAttrs(), Application::GetDefaultDevice()->GetFont());
//      xGridSet->setPropertyValue(PROPERTY_FONT, Any(&aFont, ::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0)));
//
//      SFX_ITEMSET_GET(*pDef->GetObjAttrs(), pColor, SvxColorItem, SBA_DEF_FONTCOLOR, sal_True);
//      xGridSet->setPropertyValue(PROPERTY_TEXTCOLOR, makeAny((sal_Int32)pColor->GetValue().GetColor()));
//  }
//
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

// -----------------------------------------------------------------------
void SbaTableQueryBrowser::propertyChange(const PropertyChangeEvent& evt)
{
    SbaXDataBrowserController::propertyChange(evt);

//  Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
//  if (!xSource.is())
//      return;
//
//  // one of the many properties which require us to update the definition ?
//  // a column's width ?
//  if (evt.PropertyName.equals(PROPERTY_WIDTH))
//  {   // a column width has changed -> update the model
//      // (the update of the view is done elsewhere)
//      SbaColumn* pAffectedCol = GetDefinitionCol(xSource);
//      if (pAffectedCol)
//      {
//          SbaColRowSizeItem aNewWidth(SBA_DEF_FLTCOLWIDTH);
//          sal_Bool bDefault = !evt.NewValue.hasValue();
//          SfxBoolItem aDefault(SBA_DEF_FLTCOLWIDTH_DEFAULT, bDefault);
//          if (bDefault)
//              // this is for older versions which don't know the default flag
//              aNewWidth.PutLogicValue((sal_uInt16)227);
//          else
//              aNewWidth.PutLogicValue((sal_uInt16)::utl::getINT32(evt.NewValue));
//
//          pAffectedCol->Put(aNewWidth);
//          pAffectedCol->Put(aDefault);
//      }
//  }
//
//  // a column's 'visible' state ?
//  if (evt.PropertyName.equals(PROPERTY_HIDDEN))
//  {
//      SbaColumn* pAffectedCol = GetDefinitionCol(xSource);
//      if (pAffectedCol)
//          pAffectedCol->Put(SfxBoolItem(SBA_DEF_FLTHIDE, ::utl::getBOOL(evt.NewValue)));
//  }
//
//  // a columns alignment ?
//  if (evt.PropertyName.equals(PROPERTY_ALIGN))
//  {
//      SbaColumn* pAffectedCol = GetDefinitionCol(xSource);
//      if (pAffectedCol)
//      {
//          SvxCellHorJustify eHorJustify = SVX_HOR_JUSTIFY_STANDARD;
//          if (evt.NewValue.hasValue())
//              switch (::utl::getINT16(evt.NewValue))
//              {
//                  case ::com::sun::star::awt::TextAlign::LEFT:
//                      eHorJustify = SVX_HOR_JUSTIFY_LEFT;
//                      break;
//                  case ::com::sun::star::awt::TextAlign::CENTER:
//                      eHorJustify = SVX_HOR_JUSTIFY_CENTER;
//                      break;
//                  case ::com::sun::star::awt::TextAlign::RIGHT:
//                      eHorJustify = SVX_HOR_JUSTIFY_RIGHT;
//                      break;
//              }
//
//          pAffectedCol->Put(SvxHorJustifyItem(eHorJustify, SBA_ATTR_ALIGN_HOR_JUSTIFY));
//      }
//  }
//
//  // a column's format ?
//  if  (   (evt.PropertyName.equals(PROPERTY_FORMATKEY))
//      &&  (TypeClass_LONG == evt.NewValue.getValueTypeClass())
//      )
//  {
//      // update the model (means the definition object)
//      SbaColumn* pAffectedCol = GetDefinitionCol(xSource);
//      if (pAffectedCol)
//          pAffectedCol->Put(SfxUInt32Item(SBA_DEF_FMTVALUE, ::utl::getINT32(evt.NewValue)));
//  }
//
//  // some table definition properties ?
//  // the height of the rows in the grid ?
//  if (evt.PropertyName.equals(PROPERTY_ROW_HEIGHT))
//  {
//      if (getDefinition())
//      {
//          SbaColRowSizeItem aNewHeight(SBA_DEF_ROWHEIGHT);
//          sal_Bool bDefault = !evt.NewValue.hasValue();
//          SfxBoolItem aDefault(SBA_DEF_FLTCOLWIDTH_DEFAULT, bDefault);
//
//          if (bDefault)
//              aNewHeight.PutLogicValue((sal_uInt16)45);
//          else
//              aNewHeight.PutLogicValue((sal_uInt16)::utl::getINT32(evt.NewValue));
//
//          getDefinition()->GetObjAttrs()->Put(aNewHeight);
//          getDefinition()->GetObjAttrs()->Put(aDefault);
//          getDefinition()->PutObjAttrs(*(getDefinition()->GetObjAttrs()));
//              // this sets the modified flagd
//      }
//  }
//
//  // the font of the grid ?
//  if (evt.PropertyName.equals(PROPERTY_FONT))
//  {
//      if (getDefinition())
//      {
//          BuildItemsFromFont(getDefinition()->GetObjAttrs(), *(::com::sun::star::awt::FontDescriptor*)evt.NewValue.getValue());
//          getDefinition()->PutObjAttrs(*(getDefinition()->GetObjAttrs()));
//              // this sets the modified flagd
//      }
//  }
//
//  // the text color of the grid ?
//  if (evt.PropertyName.equals(PROPERTY_TEXTCOLOR))
//  {
//      if (getDefinition())
//      {
//          SvxColorItem aColor(Color(::utl::getINT32(evt.NewValue)), SBA_DEF_FONTCOLOR);
//          getDefinition()->GetObjAttrs()->Put(aColor);
//          getDefinition()->PutObjAttrs(*(getDefinition()->GetObjAttrs()));
//              // this sets the modified flagd
//      }
//  }
//
//  // the filter ?
//  if (evt.PropertyName.equals(PROPERTY_FILTER))
//  {
//      if (getDefinition())
//      {
//          SfxStringItem aFilter(SBA_DEF_FILTER, ::utl::getString(evt.NewValue));
//          getDefinition()->GetObjAttrs()->Put(aFilter);
//          getDefinition()->PutObjAttrs(*(getDefinition()->GetObjAttrs()));
//              // this sets the modified flagd
//      }
//  }
//
//  // the sort ?
//  if (evt.PropertyName.equals(PROPERTY_ORDER))
//  {
//      if (getDefinition())
//      {
//          SfxStringItem aSort(SBA_DEF_SORT, ::utl::getString(evt.NewValue));
//          getDefinition()->GetObjAttrs()->Put(aSort);
//          getDefinition()->PutObjAttrs(*(getDefinition()->GetObjAttrs()));
//              // this sets the modified flagd
//      }
//  }
//
//  // the appliance of the filter ?
//  if (evt.PropertyName.equals(PROPERTY_APPLYFILTER))
//  {
//      if (getDefinition())
//      {
//          SfxBoolItem aApplied(SBA_DEF_FILTERAPPLIED, ::utl::getBOOL(evt.NewValue));
//          getDefinition()->GetObjAttrs()->Put(aApplied);
//          getDefinition()->PutObjAttrs(*(getDefinition()->GetObjAttrs()));
//              // this sets the modified flagd
//      }
//  }
}

// -----------------------------------------------------------------------
sal_Bool SbaTableQueryBrowser::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    if (!SbaXDataBrowserController::suspend(bSuspend))
        return sal_False;

//  if (getDefinition())
//  {
//      // let the definition save its UI settings
//      SbaDBDataDef* pDef = getDefinition();
//      if (pDef && !pDef->GetDatabase()->IsReadOnly() && pDef->IsModified())
//          pDef->Update();
//
//      // we aren't using the definition anymore
//      if (m_bUsingDefinition)
//      {
//          pDef->ReleaseUsage();
//          m_xDefinition = NULL;
//          m_bUsingDefinition = false;
//      }
//
//  }
//
//  // we don't need to listen at the definition and its database anymore
    //  EndListening(*SBA_MOD());

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
    ToolBox* pTB = getContent()->getToolBox();
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

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & _xFrame) throw( RuntimeException )
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

    try
    {
        // no chance without a view
        if (!getContent() || !getContent()->getVclControl())
            return aReturn;
        // no chance without valid models
        if (isValid() && !isValidCursor())
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
                    aReturn.bEnabled = aReturn.bEnabled && getContent()->getVclControl()->GetSelectRowCount();

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
                aReturn.bEnabled = getContent() && getContent()->getVclControl() && isValid() && isValidCursor();
                //  aReturn.bEnabled &= getDefinition() && !getDefinition()->GetDatabase()->IsReadOnly();
                break;

            case ID_BROWSER_EDITDOC:
                aReturn = SbaXDataBrowserController::GetState(nId);
                //  aReturn.bEnabled &= !getDefinition()->IsLocked();
                    // somebody is modifying the definition -> no edit mode
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
        case ID_BROWSER_EDITDOC:
            SbaXDataBrowserController::Execute(nId);
            break;

        case ID_BROWSER_INSERTCOLUMNS:
        case ID_BROWSER_INSERTCONTENT:
        case ID_BROWSER_FORMLETTER:
            if (getContent() && isValidCursor())
            {
                // the URL the slot id is assigned to
                URL aParentUrl = getURLForId(nId);

                // let the dispatcher execute the slot
                Reference< XDispatch > xDispatch(m_aDispatchers[nId]);
                if (xDispatch.is())
                {
                    // set the properties for the dispatch

                    // first fill the selection
                    SbaGridControl* pGrid = getContent()->getVclControl();
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

        default:
            SbaXDataBrowserController::Execute(nId);
            break;
    }
}

// -------------------------------------------------------------------------
void SbaTableQueryBrowser::initializeTreeModel()
{
    DBG_ASSERT(m_xMultiServiceFacatory.is(), "DBTreeListModel::DBTreeListModel : need a service factory !");
    try
    {
        m_xDatabaseContext = Reference< XNameAccess >(m_xMultiServiceFacatory->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    }
    catch(Exception&)
    {
    }

    if (m_xDatabaseContext.is())
    {
        String aNames[2];
        aNames[0] = String(ModuleRes(STR_QRY_TITLE)).GetToken(0,' ');
        aNames[1] = String(ModuleRes(STR_TBL_TITLE)).GetToken(0,' ');

        // images for the table and query folders
        Image aFolderImage[2] = {
                    Image(ModuleRes(QUERYFOLDER_TREE_ICON)),
                    Image(ModuleRes(TABLEFOLDER_TREE_ICON))
                };

        Image aDBImage(ModuleRes(IMG_DATABASE));
        // fill the model with the names of the registered datasources
        Sequence< ::rtl::OUString > aDatasources = m_xDatabaseContext->getElementNames();
        const ::rtl::OUString* pBegin   = aDatasources.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aDatasources.getLength();
        for (; pBegin != pEnd; ++pBegin)
        {
            // add items
            SvLBoxEntry* pEntry = m_pTreeView->getListBox()->InsertEntry(*pBegin, aDBImage, aDBImage, NULL, sal_False);
            pEntry->SetUserData(new DBTreeListModel::DBTreeListUserData);



            for(sal_Int32 i=0;i<2;++i)
            {
                SvLBoxEntry* pChild = m_pTreeView->getListBox()->InsertEntry(aNames[i], aFolderImage[i], aFolderImage[i], pEntry, sal_True);
                DBTreeListModel::DBTreeListUserData* pData = new DBTreeListModel::DBTreeListUserData;
                pData->bTable = (i != 0);
                pChild->SetUserData(pData);
            }
        }
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
IMPL_LINK(SbaTableQueryBrowser, OnExpandEntry, SvLBoxEntry*, _pParent)
{
    if (_pParent->HasChilds())
        // nothing to to ...
        return 1L;

    SvLBoxEntry* pFirstParent = m_pTreeView->getListBox()->GetRootLevelParent(_pParent);
    OSL_ENSHURE(pFirstParent,"SbaTableQueryBrowser::OnExpandEntry: No rootlevelparent!");

    DBTreeListModel::DBTreeListUserData* pData = static_cast< DBTreeListModel::DBTreeListUserData* >(_pParent->GetUserData());
    OSL_ENSHURE(pData,"SbaTableQueryBrowser::OnExpandEntry: No user data!");
    SvLBoxString* pString = static_cast<SvLBoxString*>(pFirstParent->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    OSL_ENSHURE(pString,"SbaTableQueryBrowser::OnExpandEntry: No string item!");
    String aName(pString->GetText());
    Any aValue(m_xDatabaseContext->getByName(aName));
    if(pData->bTable)
    {
        Reference<XPropertySet> xProp;
        aValue >>= xProp;
        ::rtl::OUString sPwd, sUser;
        sal_Bool bPwdReq = sal_False;
        try
        {
            xProp->getPropertyValue(PROPERTY_PASSWORD) >>= sPwd;
            bPwdReq = cppu::any2bool(xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED));
            xProp->getPropertyValue(PROPERTY_USER) >>= sUser;
        }
        catch(Exception&)
        {
            DBG_ERROR("SbaTableQueryBrowser::OnExpandEntry: error while retrieving data source properties!");
        }

        SQLExceptionInfo aInfo;
        try
        {

            Reference<XConnection> xConnection;  // supports the service sdb::connection
            if(bPwdReq && !sPwd.getLength())
            {   // password required, but empty -> connect using an interaction handler
                Reference<XCompletedConnection> xConnectionCompletion(xProp, UNO_QUERY);
                if (!xConnectionCompletion.is())
                {
                    DBG_ERROR("SbaTableQueryBrowser::OnExpandEntry: missing an interface ... need an error message here!");
                }
                else
                {   // instantiate the default SDB interaction handler
                    Reference< XInteractionHandler > xHandler(m_xMultiServiceFacatory->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
                    if (!xHandler.is())
                    {
                        ShowServiceNotAvailableError(getContent(), String(SERVICE_SDB_INTERACTION_HANDLER), sal_True);
                            // TODO: a real parent!
                    }
                    else
                    {
                        xConnection = xConnectionCompletion->connectWithCompletion(xHandler);
                    }
                }
            }
            else
            {
                Reference<XDataSource> xDataSource(xProp,UNO_QUERY);
                xConnection = xDataSource->getConnection(sUser, sPwd);
            }

            if(xConnection.is())
            {
                DBTreeListModel::DBTreeListUserData* pFirstData = (DBTreeListModel::DBTreeListUserData*)pFirstParent->GetUserData();
                if(!pFirstData->xObject.is())
                    pFirstData->xObject = xConnection;

                Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
                if(xTabSup.is())
                {
                    Image aImage(ModuleRes(TABLE_TREE_ICON));
                    populateTree(xTabSup->getTables(),_pParent,aImage);
                }

                Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
                if(xViewSup.is())
                {
                    Image aImage(ModuleRes(VIEW_TREE_ICON));
                    populateTree(xViewSup->getViews(),_pParent,aImage);
                }
            }
        }
        catch(SQLException& e) { aInfo = SQLExceptionInfo(e); }
        catch(SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
        catch(SQLContext& e) { aInfo = SQLExceptionInfo(e); }
        catch(Exception&) { DBG_ERROR("SbaTableQueryBrowser::OnExpandEntry: could not connect - unknown exception!"); }

        if (aInfo.isValid())
        {
            try
            {
                Sequence< Any > aArgs(1);
                aArgs[0] <<= PropertyValue(PROPERTY_SQLEXCEPTION, 0, aInfo.get(), PropertyState_DIRECT_VALUE);
                Reference< XExecutableDialog > xErrorDialog(
                    m_xMultiServiceFacatory->createInstanceWithArguments(::rtl::OUString::createFromAscii("com.sun.star.sdb.ErrorMessageDialog"), aArgs), UNO_QUERY);
                if (xErrorDialog.is())
                    xErrorDialog->execute();
            }
            catch(Exception&)
            {
                DBG_ERROR("SbaTableQueryBrowser::OnExpandEntry: could not display the error message!");
            }
            return 0L;
                // 0 indicates that an error occured
        }
    }
    else // we have to expand the queries
    {
        Reference<XQueryDefinitionsSupplier> xQuerySup;
        if(aValue >>= xQuerySup)
        {
            Image aImage(ModuleRes(QUERY_TREE_ICON));
            populateTree(xQuerySup->getQueryDefinitions(),_pParent,aImage);
        }
    }

    return 1L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaTableQueryBrowser, OnSelectEntry, SvLBoxEntry*, _pEntry)
{
    // reinitialize the rowset
    // but first check if it is necessary
    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
    ::rtl::OUString aOldName;
    xProp->getPropertyValue(PROPERTY_COMMAND) >>= aOldName;
    sal_Int32 nOldType;
    xProp->getPropertyValue(PROPERTY_COMMANDTYPE) >>= nOldType;
    Reference<XConnection> xOldConnection;
    xProp->getPropertyValue(PROPERTY_ACTIVECONNECTION) >>= xOldConnection;
    // the name of the table or query
    SvLBoxString* pString = (SvLBoxString*)_pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING);
    ::rtl::OUString aName(pString->GetText().GetBuffer());

    SvLBoxEntry* pTables = m_pTreeModel->GetParent(_pEntry);
    DBTreeListModel::DBTreeListUserData* pData = (DBTreeListModel::DBTreeListUserData*)pTables->GetUserData();
    SvLBoxEntry* pConnection = m_pTreeModel->GetParent(pTables);
    DBTreeListModel::DBTreeListUserData* pConData = (DBTreeListModel::DBTreeListUserData*)pConnection->GetUserData();

    Reference<XConnection> xConnection(pConData->xObject,UNO_QUERY);
    sal_Int32 nCommandType = pData->bTable ? CommandType::TABLE : CommandType::QUERY;

    sal_Bool bRebuild = xOldConnection != xConnection || nOldType != nCommandType || aName != aOldName;

    if(bRebuild)
    {
        // the values allowing the RowSet to re-execute
        xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(xConnection));
        xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(nCommandType));
        xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(aName));

        // the formatter depends on the data source we're working on, so rebuild it here ...
        initFormatter();

        // switch the grid to design mode while loading
        getContent()->getGridControl()->setDesignMode(sal_True);
        // reload the row set
        getRowSet()->execute();
        // initialize the model
        InitializeGridModel(getFormComponent());
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(),UNO_QUERY);
        xLoadable->reload();
        FormLoaded(sal_True);
    }
    return 0L;
}
// -------------------------------------------------------------------------

// .........................................................................
}   // namespace dbaui
// .........................................................................



/*************************************************************************
 *
 *  $RCSfile: unodatbr.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-06 17:40:34 $
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
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _SVX_ALGITEM_HXX //autogen
#include <svx/algitem.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#include "dbtreemodel.hxx"
#include "dbtreeview.hxx"
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

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbaui;
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
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.dialogs.DatabaseBrowser");
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

        // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
        m_pSplitter->SetSplitPosPixel( getContent()->LogicToPixel( Size( 80, 0 ), MAP_APPFONT ).Width() );

        getContent()->setSplitter(m_pSplitter);
        getContent()->setTreeView(m_pTreeView);

        // fill view with data
        m_pTreeModel = new DBTreeListModel(m_xMultiServiceFacatory);
        m_pTreeView->setModel(m_pTreeModel);
        m_pTreeView->setSelectHdl(LINK(this, SbaTableQueryBrowser, OnSelectEntry));

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
//              try { aHeight = xPropState->getPropertyDefault(PROPERTY_ROW_HEIGHT); } catch(...) { } ;
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
    catch(...)
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

        if (pTB->GetItemPos(ID_BROWSER_EXPL_PREVLEVEL) != TOOLBOX_ITEM_NOTFOUND)
        {   // if we display the grid in the beamer we don't want a "prev level" button
            //  if (getFrame().is() && (getFrame()->getName().equals(::rtl::OUString::createFromAscii("_beamer"))))
                pTB->HideItem(ID_BROWSER_EXPL_PREVLEVEL);
        }
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
void SbaTableQueryBrowser::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & _xFrame) throw( RuntimeException )
{
    SbaXDataBrowserController::attachFrame(_xFrame);

    sal_Bool bIsBeamer = (getFrame().is() && (getFrame()->getName().equals(::rtl::OUString::createFromAscii("_beamer"))));
    Reference< XPropertySet >  xGridSet(getFormComponent(), UNO_QUERY);
    ToolBox* pTB = getContent() ? getContent()->getToolBox() : NULL;

    if (bIsBeamer)
    {
        // hide the "move a level up"-button
        sal_uInt16 nPos = pTB ? pTB->GetItemPos(ID_BROWSER_EXPL_PREVLEVEL) : TOOLBOX_ITEM_NOTFOUND;
        if ((nPos != TOOLBOX_ITEM_NOTFOUND) && pTB->IsItemVisible(ID_BROWSER_EXPL_PREVLEVEL))
        {
            DBG_ASSERT(nPos == 0, "SbaTableQueryBrowser::attachFrame : can't handle a prev-level-slot which isn't the first one within the TB !");
                // we check all slots behind the prev-level slot and don't deal with the ones before
            pTB->HideItem(ID_BROWSER_EXPL_PREVLEVEL);
            ++nPos;
            if (pTB->GetItemType(nPos) == TOOLBOXITEM_SEPARATOR)
                pTB->RemoveItem(nPos);
        }
    }
    else
    {
        // show the "move a level up"-button
        sal_uInt16 nPos = pTB ? pTB->GetItemPos(ID_BROWSER_EXPL_PREVLEVEL) : TOOLBOX_ITEM_NOTFOUND;
        if ((nPos != TOOLBOX_ITEM_NOTFOUND) && !pTB->IsItemVisible(ID_BROWSER_EXPL_PREVLEVEL))
        {
            DBG_ASSERT(nPos == 0, "SbaTableQueryBrowser::attachFrame : can't handle a prev-level-slot which isn't the first one within the TB !");
                // we check all slots behind the prev-level slot and don't deal with the ones before
            pTB->ShowItem(ID_BROWSER_EXPL_PREVLEVEL);
            ++nPos;
            if (pTB->GetItemType(nPos) != TOOLBOXITEM_SEPARATOR)
                pTB->InsertSeparator(nPos);
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
//  switch (nId)
//  {
//      case ID_BROWSER_INSERT:
//      case ID_BROWSER_UPDATE:
//      case ID_BROWSER_MERGE:
//          GetBindings().Invalidate(nId, sal_True, sal_False);
//          break;
//      default:
//          SbaXDataBrowserController::InvalidateFeature(nId, xListener);
//  }
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
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:WriterDB/MailInsert")] = ID_BROWSER_INSERT;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:WriterDB/MailMerge")]  = ID_BROWSER_MERGE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:WriterDB/MailUpdate")] = ID_BROWSER_UPDATE;

            // TODO reenable our own code if we really have a handling for the formslots
//          ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToFirst"),        SID_FM_RECORD_FIRST     ),
//          ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToLast"),     SID_FM_RECORD_LAST      ),
//          ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToNew"),      SID_FM_RECORD_NEW       ),
//          ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToNext"),     SID_FM_RECORD_NEXT      ),
//          ControllerFeature( ::rtl::OUString::createFromAscii("private:FormSlot/moveToPrev"),     SID_FM_RECORD_PREV      )

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
            case ID_BROWSER_INSERT:
            case ID_BROWSER_UPDATE:
            case ID_BROWSER_MERGE:
            {
                // TODO
//              SfxPoolItem* pState = NULL;
//              SfxItemState eState = GetBindings().QueryState(nId, pState);
//              if (pState)
//                  delete pState;  // not interested in
//              aReturn.bEnabled = (eState >= SFX_ITEM_AVAILABLE);

                if (ID_BROWSER_MERGE != nId)
                    aReturn.bEnabled = aReturn.bEnabled && getContent()->getVclControl()->GetSelectRowCount();

                Reference< XPropertySet >  xDataSource(getRowSet(), UNO_QUERY);
                try
                {
                    aReturn.bEnabled = aReturn.bEnabled && xDataSource.is();

                    // disable them for native queries which are not saved within the database
                    // 67706 - 23.08.99 - FS
                    if (xDataSource.is())
                    {
                        sal_Int32 nType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMANDTYPE));
                        aReturn.bEnabled = aReturn.bEnabled && ((::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING)) || (nType == ::com::sun::star::sdb::CommandType::QUERY)));
                    }
                }
                catch(...)
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

            case ID_BROWSER_EXPL_PREVLEVEL:
                aReturn.bEnabled = sal_True;
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

        case ID_BROWSER_INSERT:
        case ID_BROWSER_UPDATE:
        case ID_BROWSER_MERGE:
            // TODO use dispatch
            if (getContent() && isValidCursor())
            {
                ::com::sun::star::util::URL aParentUrl;
                // is faster as to search in the map
                switch(nId)
                {
                    case ID_BROWSER_INSERT:
                        aParentUrl.Complete = ::rtl::OUString::createFromAscii(".uno:WriterDB/MailInsert");
                        break;
                    case ID_BROWSER_UPDATE:
                        aParentUrl.Complete = ::rtl::OUString::createFromAscii(".uno:WriterDB/MailUpdate");
                        break;
                    case ID_BROWSER_MERGE:
                        aParentUrl.Complete = ::rtl::OUString::createFromAscii(".uno:WriterDB/MailMerge");
                        break;
                }

                Reference< ::com::sun::star::frame::XDispatchProvider >  xProvider(m_xCurrentFrame, UNO_QUERY);
                DBG_ASSERT(xProvider.is(), "SbaTableQueryBrowser::Execute(ID_BROWSER_EXPL_PREVLEVEL) : no DispatchPprovider !");
                Reference< ::com::sun::star::frame::XDispatch >  xDispatcher = xProvider->queryDispatch(aParentUrl, m_xCurrentFrame->getName(), 0);
                if (xDispatcher.is())
                {
                    // set the properties for the dispatch

                    // first fill the selection
                    SbaGridControl* pGrid = getContent()->getVclControl();
                    MultiSelection* pSelection = (MultiSelection*)pGrid->GetSelection();
                    Sequence< sal_Int32> aSelection;
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

                    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);

                    Sequence< PropertyValue> aProps(4);
                    aProps[0] = PropertyValue(PROPERTY_DATASOURCENAME,-1,xProp->getPropertyValue(PROPERTY_DATASOURCENAME),PropertyState_DIRECT_VALUE);
                    aProps[1] = PropertyValue(PROPERTY_COMMAND,-1,xProp->getPropertyValue(PROPERTY_COMMAND),PropertyState_DIRECT_VALUE);
                    aProps[2] = PropertyValue(PROPERTY_COMMANDTYPE,-1,xProp->getPropertyValue(PROPERTY_COMMANDTYPE),PropertyState_DIRECT_VALUE);
                    aProps[3] = PropertyValue(PROPERTY_ROWCOUNT,-1,makeAny(aSelection),PropertyState_DIRECT_VALUE);

                    xDispatcher->dispatch(aParentUrl, aProps);
                }
            }
            //  SendInfo(nId);
            break;

        case ID_BROWSER_EXPL_PREVLEVEL:
        {
            INetURLObject aURL(getURL());
            DBG_ASSERT(aURL.HasMark(), "SbaTableQueryBrowser::Execute : invalid ::com::sun::star::util::URL !");
            String sMark = aURL.GetMark();
            xub_StrLen nSep = sMark.SearchBackward(';');
            DBG_ASSERT(nSep != STRING_NOTFOUND, "SbaTableQueryBrowser::Execute : invalid ::com::sun::star::util::URL !");
            String sParentContainerUrl; sParentContainerUrl.AssignAscii("file:///");
            sParentContainerUrl += aURL.GetURLPath();
            sParentContainerUrl += '#';
            sParentContainerUrl += sMark.Copy(0, nSep);

            ::com::sun::star::util::URL aParentUrl;
            aParentUrl.Complete = sParentContainerUrl;

            Reference< ::com::sun::star::frame::XDispatchProvider >  xProvider(m_xCurrentFrame, UNO_QUERY);
            DBG_ASSERT(xProvider.is(), "SbaTableQueryBrowser::Execute(ID_BROWSER_EXPL_PREVLEVEL) : no DispatchPprovider !");
            Reference< ::com::sun::star::frame::XDispatch >  xDispatcher = xProvider->queryDispatch(aParentUrl, m_xCurrentFrame->getName(), 0);
            if (xDispatcher.is())
                xDispatcher->dispatch(aParentUrl, Sequence< PropertyValue>());
        }
        break;

        default:
            SbaXDataBrowserController::Execute(nId);
            break;
    }
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

    Reference<XConnection> xConnection(pConData->xInterface,UNO_QUERY);
    sal_Int32 nCommandType = pData->bTable ? CommandType::TABLE : CommandType::QUERY;

    sal_Bool bRebuild = xOldConnection != xConnection || nOldType != nCommandType || aName != aOldName;

    if(bRebuild)
    {
        xProp->setPropertyValue(PROPERTY_ACTIVECONNECTION,makeAny(xConnection));
        xProp->setPropertyValue(PROPERTY_COMMANDTYPE,makeAny(nCommandType));
        xProp->setPropertyValue(PROPERTY_COMMAND,makeAny(aName));

        getContent()->getGridControl()->setDesignMode(sal_True);
        getRowSet()->execute();
        InitializeGridModel(getFormComponent());
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(),UNO_QUERY);
        xLoadable->reload();
        FormLoaded(sal_True);
    }
    return 0L;
}
// -------------------------------------------------------------------------



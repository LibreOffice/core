/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "browserids.hxx"
#include "dbaccess_helpid.hrc"
#include "dbexchange.hxx"
#include "dbtreelistbox.hxx"
#include "dbtreemodel.hxx"
#include "dbtreeview.hxx"
#include "dbu_brw.hrc"
#include "dbu_reghelper.hxx"
#include "dbustrings.hrc"
#include "dlgsave.hxx"
#include "HtmlReader.hxx"
#include "imageprovider.hxx"
#include "listviewitems.hxx"
#include "QEnumTypes.hxx"
#include "RtfReader.hxx"
#include "sbagrid.hrc"
#include "sbagrid.hxx"
#include "sqlmessage.hxx"
#include "TokenWriter.hxx"
#include "UITools.hxx"
#include "unodatbr.hxx"
#include "WColumnSelect.hxx"
#include "WCopyTable.hxx"
#include "WCPage.hxx"
#include "WExtendPages.hxx"
#include "WNameMatch.hxx"

#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdb/XResultSetAccess.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sot/storage.hxx>
#include <svl/filenotation.hxx>
#include <svl/intitem.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistbox.hxx>
#include "svtools/treelistentry.hxx"
#include <svx/algitem.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/databaseregistrationui.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <tools/multisel.hxx>
#include <tools/urlobj.hxx>
#include <unotools/confignode.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/split.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>

#include <memory>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdb::application;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::ui;
using namespace ::dbtools;
using namespace ::comphelper;
using namespace ::svx;

namespace dbaui
{

namespace DatabaseObject = ::com::sun::star::sdb::application::DatabaseObject;
namespace DatabaseObjectContainer = ::com::sun::star::sdb::application::DatabaseObjectContainer;


extern "C" void SAL_CALL createRegistryInfo_OBrowser()
{
    static OMultiInstanceAutoRegistration< SbaTableQueryBrowser > aAutoRegistration;
}

void SafeAddPropertyListener(const Reference< XPropertySet > & xSet, const OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->addPropertyChangeListener(rPropName, pListener);
}

void SafeRemovePropertyListener(const Reference< XPropertySet > & xSet, const OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->removePropertyChangeListener(rPropName, pListener);
}

OUString SAL_CALL SbaTableQueryBrowser::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

::comphelper::StringSequence SAL_CALL SbaTableQueryBrowser::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

OUString SbaTableQueryBrowser::getImplementationName_Static() throw(RuntimeException)
{
    return OUString("org.openoffice.comp.dbu.ODatasourceBrowser");
}

::comphelper::StringSequence SbaTableQueryBrowser::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported[0] = "com.sun.star.sdb.DataSourceBrowser";
    return aSupported;
}

Reference< XInterface > SAL_CALL SbaTableQueryBrowser::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    SolarMutexGuard aGuard;
    return *(new SbaTableQueryBrowser(comphelper::getComponentContext(_rxFactory)));
}

SbaTableQueryBrowser::SbaTableQueryBrowser(const Reference< XComponentContext >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_aSelectionListeners( getMutex() )
    ,m_aContextMenuInterceptors( getMutex() )
    ,m_aTableCopyHelper(this)
    ,m_pTreeView(NULL)
    ,m_pSplitter(NULL)
    ,m_pTreeModel(NULL)
    ,m_pCurrentlyDisplayed(NULL)
    ,m_nAsyncDrop(0)
    ,m_nBorder(1)
    ,m_bQueryEscapeProcessing( sal_False )
    ,m_bShowMenu(sal_False)
    ,m_bInSuspend(sal_False)
    ,m_bEnableBrowser(sal_True)
{
}

SbaTableQueryBrowser::~SbaTableQueryBrowser()
{
    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        SAL_WARN("dbaccess.ui", "Please check who doesn't dispose this component!");
        
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

Any SAL_CALL SbaTableQueryBrowser::queryInterface(const Type& _rType) throw (RuntimeException)
{
    if ( _rType.equals( cppu::UnoType<XScriptInvocationContext>::get() ) )
    {
        OSL_PRECOND( !!m_aDocScriptSupport, "SbaTableQueryBrowser::queryInterface: did not initialize this, yet!" );
        if ( !!m_aDocScriptSupport && *m_aDocScriptSupport )
            return makeAny( Reference< XScriptInvocationContext >( this ) );
        return Any();
    }

    Any aReturn = SbaXDataBrowserController::queryInterface(_rType);
    if (!aReturn.hasValue())
        aReturn = SbaTableQueryBrowser_Base::queryInterface(_rType);
    return aReturn;
}

Sequence< Type > SAL_CALL SbaTableQueryBrowser::getTypes(  ) throw (RuntimeException)
{
    Sequence< Type > aTypes( ::comphelper::concatSequences(
        SbaXDataBrowserController::getTypes(),
        SbaTableQueryBrowser_Base::getTypes()
    ) );

    OSL_PRECOND( !!m_aDocScriptSupport, "SbaTableQueryBrowser::getTypes: did not initialize this, yet!" );
    if ( !m_aDocScriptSupport || !*m_aDocScriptSupport )
    {
        Sequence< Type > aStrippedTypes( aTypes.getLength() - 1 );
        ::std::remove_copy_if(
            aTypes.getConstArray(),
            aTypes.getConstArray() + aTypes.getLength(),
            aStrippedTypes.getArray(),
            ::std::bind2nd( ::std::equal_to< Type >(), cppu::UnoType<XScriptInvocationContext>::get() )
        );
        aTypes = aStrippedTypes;
    }
    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL SbaTableQueryBrowser::getImplementationId(  ) throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

void SAL_CALL SbaTableQueryBrowser::disposing()
{
    SolarMutexGuard aGuard;
        

    
    com::sun::star::lang::EventObject aEvt(*this);
    m_aSelectionListeners.disposeAndClear(aEvt);
    m_aContextMenuInterceptors.disposeAndClear(aEvt);

    
    
    if (getBrowserView())
        getBrowserView()->setTreeView(NULL);

    clearTreeModel();
    
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SvTreeList> aTemp(m_pTreeModel);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pTreeModel = NULL;
    }

    
    implRemoveStatusListeners();

    
    try
    {
        Reference< XDatabaseRegistrations > xDatabaseRegistrations( m_xDatabaseContext, UNO_QUERY_THROW );
        xDatabaseRegistrations->removeDatabaseRegistrationsListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    
    
    if (m_xCurrentFrameParent.is())
        m_xCurrentFrameParent->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
    SbaXDataBrowserController::disposing();
}

sal_Bool SbaTableQueryBrowser::Construct(Window* pParent)
{
    if ( !SbaXDataBrowserController::Construct( pParent ) )
        return sal_False;

    try
    {
        Reference< XDatabaseRegistrations > xDatabaseRegistrations( m_xDatabaseContext, UNO_QUERY_THROW );
        xDatabaseRegistrations->addDatabaseRegistrationsListener( this );

        
        m_xCollator = Collator::create( getORB() );
        m_xCollator->loadDefaultCollator( Application::GetSettings().GetLanguageTag().getLocale(), 0 );
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::Construct: could not create (or start listening at) the database context!");
    }
    
    if (getBrowserView() && getBrowserView()->getVclControl())
    {

        
        const long  nFrameWidth = getBrowserView()->LogicToPixel( ::Size( 3, 0 ), MAP_APPFONT ).Width();

        m_pSplitter = new Splitter(getBrowserView(),WB_HSCROLL);
        m_pSplitter->SetPosSizePixel( ::Point(0,0), ::Size(nFrameWidth,0) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );

        m_pTreeView = new DBTreeView(getBrowserView(), WB_TABSTOP | WB_BORDER);
        m_pTreeView->SetPreExpandHandler(LINK(this, SbaTableQueryBrowser, OnExpandEntry));

        m_pTreeView->setCopyHandler(LINK(this, SbaTableQueryBrowser, OnCopyEntry));

        m_pTreeView->getListBox().setContextMenuProvider( this );
        m_pTreeView->getListBox().setControlActionListener( this );
        m_pTreeView->SetHelpId(HID_CTL_TREEVIEW);

        
        m_pSplitter->SetSplitPosPixel( getBrowserView()->LogicToPixel( ::Size( 80, 0 ), MAP_APPFONT ).Width() );

        getBrowserView()->setSplitter(m_pSplitter);
        getBrowserView()->setTreeView(m_pTreeView);

        
        m_pTreeModel = new SvTreeList;
        m_pTreeModel->SetSortMode(SortAscending);
        m_pTreeModel->SetCompareHdl(LINK(this, SbaTableQueryBrowser, OnTreeEntryCompare));
        m_pTreeView->setModel(m_pTreeModel);
        m_pTreeView->setSelChangeHdl( LINK( this, SbaTableQueryBrowser, OnSelectionChange ) );

        
        getBrowserView()->getVclControl()->GetDataWindow().SetUniqueId(UID_DATABROWSE_DATAWINDOW);
        getBrowserView()->getVclControl()->SetHelpId(HID_CTL_TABBROWSER);
        getBrowserView()->SetUniqueId(UID_CTL_CONTENT);
        if (getBrowserView()->getVclControl()->GetHeaderBar())
            getBrowserView()->getVclControl()->GetHeaderBar()->SetHelpId(HID_DATABROWSE_HEADER);
        InvalidateFeature(ID_BROWSER_EXPLORER);
    }

    return sal_True;
}

namespace
{
    struct SelectValueByName : public ::std::unary_function< OUString, Any >
    {
        const Any& operator()( OUString const& i_name ) const
        {
            return m_rCollection.get( i_name );
        }

        SelectValueByName( ::comphelper::NamedValueCollection const& i_collection )
            :m_rCollection( i_collection )
        {
        }

        ::comphelper::NamedValueCollection const&   m_rCollection;
    };
}

void SbaTableQueryBrowser::impl_sanitizeRowSetClauses_nothrow()
{
    try
    {
        Reference< XPropertySet > xRowSetProps( getRowSet(), UNO_QUERY_THROW );
        sal_Bool bEscapeProcessing = sal_False;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bEscapeProcessing );
        if ( !bEscapeProcessing )
            
            return;

        Reference< XSingleSelectQueryComposer > xComposer( createParser_nothrow() );
        if ( !xComposer.is() )
            
            return;

        
        const Reference< XTablesSupplier > xSuppTables( xComposer, UNO_QUERY_THROW );
        const Reference< XNameAccess > xTableNames( xSuppTables->getTables(), UNO_QUERY_THROW );

        
        const Reference< XColumnsSupplier > xSuppColumns( xComposer, UNO_QUERY_THROW );
        const Reference< XNameAccess > xColumnNames( xSuppColumns->getColumns(), UNO_QUERY_THROW );

        
        const Reference< XIndexAccess > xOrderColumns( xComposer->getOrderColumns(), UNO_SET_THROW );
        const sal_Int32 nOrderColumns( xOrderColumns->getCount() );
        bool invalidColumn = nOrderColumns == 0;
        for ( sal_Int32 c=0; ( c < nOrderColumns ) && !invalidColumn; ++c )
        {
            const Reference< XPropertySet > xOrderColumn( xOrderColumns->getByIndex(c), UNO_QUERY_THROW );
            OUString sTableName;
            OSL_VERIFY( xOrderColumn->getPropertyValue( PROPERTY_TABLENAME ) >>= sTableName );
            OUString sColumnName;
            OSL_VERIFY( xOrderColumn->getPropertyValue( PROPERTY_NAME ) >>= sColumnName );

            if ( sTableName.isEmpty() )
            {
                if ( !xColumnNames->hasByName( sColumnName ) )
                {
                    invalidColumn = true;
                    break;
                }
            }
            else
            {
                if ( !xTableNames->hasByName( sTableName ) )
                {
                    invalidColumn = true;
                    break;
                }

                const Reference< XColumnsSupplier > xSuppTableColumns( xTableNames->getByName( sTableName ), UNO_QUERY_THROW );
                const Reference< XNameAccess > xTableColumnNames( xSuppTableColumns->getColumns(), UNO_QUERY_THROW );
                if ( !xTableColumnNames->hasByName( sColumnName ) )
                {
                    invalidColumn = true;
                    break;
                }
            }
        }

        if ( invalidColumn )
        {
            
            const OUString sEmptyOrder;
            xRowSetProps->setPropertyValue( PROPERTY_ORDER, makeAny( sEmptyOrder ) );
            xComposer->setOrder( sEmptyOrder );
        }

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

sal_Bool SbaTableQueryBrowser::InitializeForm( const Reference< XPropertySet > & i_formProperties )
{
    if(!m_pCurrentlyDisplayed)
        return sal_True;

    
    try
    {
        DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
        ENSURE_OR_RETURN_FALSE( pData, "SbaTableQueryBrowser::InitializeForm: No user data set at the currently displayed entry!" );
        ENSURE_OR_RETURN_FALSE( pData->xObjectProperties.is(), "SbaTableQueryBrowser::InitializeForm: No table available!" );

        Reference< XPropertySetInfo > xPSI( pData->xObjectProperties->getPropertySetInfo(), UNO_SET_THROW );

        ::comphelper::NamedValueCollection aPropertyValues;

        const OUString aTransferProperties[] =
        {
            OUString(PROPERTY_APPLYFILTER),
            OUString(PROPERTY_FILTER),
            OUString(PROPERTY_HAVING_CLAUSE),
            OUString(PROPERTY_ORDER)
        };
        for (size_t i = 0; i < SAL_N_ELEMENTS(aTransferProperties); ++i)
        {
            if ( !xPSI->hasPropertyByName( aTransferProperties[i] ) )
                continue;
            aPropertyValues.put( aTransferProperties[i], pData->xObjectProperties->getPropertyValue( aTransferProperties[i] ) );
        }

        ::std::vector< OUString > aNames( aPropertyValues.getNames() );
        ::std::sort(aNames.begin(), aNames.end());
        Sequence< OUString > aPropNames( aNames.size() );
        ::std::copy( aNames.begin(), aNames.end(), aPropNames.getArray() );

        Sequence< Any > aPropValues( aNames.size() );
        ::std::transform( aNames.begin(), aNames.end(), aPropValues.getArray(), SelectValueByName( aPropertyValues ) );

        Reference< XMultiPropertySet > xFormMultiSet( i_formProperties, UNO_QUERY_THROW );
        xFormMultiSet->setPropertyValues( aPropNames, aPropValues );

        impl_sanitizeRowSetClauses_nothrow();
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        return sal_False;
    }

    return sal_True;
}

void SbaTableQueryBrowser::initializePreviewMode()
{
    if ( getBrowserView() && getBrowserView()->getVclControl() )
    {
        getBrowserView()->getVclControl()->AlwaysEnableInput( false );
        getBrowserView()->getVclControl()->EnableInput( false );
        getBrowserView()->getVclControl()->ForceHideScrollbars( sal_True );
    }
    Reference< XPropertySet >  xDataSourceSet(getRowSet(), UNO_QUERY);
    if ( xDataSourceSet.is() )
    {
        xDataSourceSet->setPropertyValue("AllowInserts",makeAny(sal_False));
        xDataSourceSet->setPropertyValue("AllowUpdates",makeAny(sal_False));
        xDataSourceSet->setPropertyValue("AllowDeletes",makeAny(sal_False));
    }
}

sal_Bool SbaTableQueryBrowser::InitializeGridModel(const Reference< ::com::sun::star::form::XFormComponent > & xGrid)
{
    try
    {
        Reference< ::com::sun::star::form::XGridColumnFactory >  xColFactory(xGrid, UNO_QUERY);
        Reference< XNameContainer >  xColContainer(xGrid, UNO_QUERY);
        clearGridColumns( xColContainer );

        Reference< XChild > xGridAsChild(xGrid, UNO_QUERY);
        Reference< XLoadable > xFormAsLoadable;
        if (xGridAsChild.is())
            xFormAsLoadable = xFormAsLoadable.query(xGridAsChild->getParent());
        if (xFormAsLoadable.is() && xFormAsLoadable->isLoaded())
        {
            
            if(m_pCurrentlyDisplayed)
            {
                Sequence< OUString> aProperties(6 + ( m_bPreview ? 5 : 0 ));
                Sequence< Any> aValues(7 + ( m_bPreview ? 5 : 0 ));

                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                OSL_ENSURE( pData->xObjectProperties.is(), "SbaTableQueryBrowser::InitializeGridModel: No table available!" );
                if ( !pData->xObjectProperties.is() )
                    return sal_False;

                OUString* pStringIter = aProperties.getArray();
                Any* pValueIter = aValues.getArray();
                if ( m_bPreview )
                {
                    *pStringIter++  = "AlwaysShowCursor";
                    *pValueIter++   <<= sal_False;
                    *pStringIter++  = PROPERTY_BORDER;
                    *pValueIter++   <<= sal_Int16(0);
                }

                *pStringIter++  = PROPERTY_FONT;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_FONT);
                *pStringIter++  = PROPERTY_TEXTEMPHASIS;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTEMPHASIS);
                *pStringIter++  = PROPERTY_TEXTRELIEF;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTRELIEF);
                if ( m_bPreview )
                {
                    *pStringIter++  = "HasNavigationBar";
                    *pValueIter++       <<= sal_False;
                    *pStringIter++  = "HasRecordMarker";
                    *pValueIter++       <<= sal_False;
                }
                *pStringIter++  = PROPERTY_ROW_HEIGHT;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_ROW_HEIGHT);
                if ( m_bPreview )
                {
                    *pStringIter++  = "Tabstop";
                    *pValueIter++       <<= sal_False;
                }
                *pStringIter++  = PROPERTY_TEXTCOLOR;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTCOLOR);
                *pStringIter++  = PROPERTY_TEXTLINECOLOR;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTLINECOLOR);

                Reference< XMultiPropertySet >  xFormMultiSet(xGrid, UNO_QUERY);
                xFormMultiSet->setPropertyValues(aProperties, aValues);
            }

            
            Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = getNumberFormatter()->getNumberFormatsSupplier();

            Reference<XConnection> xConnection;
            Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
            xRowSetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;
            OSL_ENSURE(xConnection.is(),"A ActiveConnection should normaly exists!");

            Reference<XChild> xChild(xConnection,UNO_QUERY);
            Reference<XPropertySet> xDataSourceProp(xChild->getParent(),UNO_QUERY);
            sal_Bool bSuppressVersionCol = sal_False;
            OSL_VERIFY( xDataSourceProp->getPropertyValue( PROPERTY_SUPPRESSVERSIONCL ) >>= bSuppressVersionCol );

            
            OUString aCurrentModelType;
            Reference<XColumnsSupplier> xSupCols(getRowSet(),UNO_QUERY);
            Reference<XNameAccess> xColumns     = xSupCols->getColumns();
            Sequence< OUString> aNames   = xColumns->getElementNames();
            const OUString* pIter        = aNames.getConstArray();
            const OUString* pEnd         = pIter + aNames.getLength();

            OUString sDefaultProperty;
            Reference< XPropertySet > xColumn;
            Reference< XPropertySetInfo > xColPSI;
            for (sal_uInt16 i=0; pIter != pEnd; ++i,++pIter)
            {
                xColumn.set( xColumns->getByName( *pIter ), UNO_QUERY_THROW );
                xColPSI.set( xColumn->getPropertySetInfo(), UNO_SET_THROW );

                
                if  (   bSuppressVersionCol
                    &&  xColPSI->hasPropertyByName( PROPERTY_ISROWVERSION )
                    &&  ::cppu::any2bool( xColumn->getPropertyValue( PROPERTY_ISROWVERSION ) )
                    )
                    continue;

                
                sal_Bool bFormattedIsNumeric    = sal_True;
                sal_Int32 nType = ::comphelper::getINT32( xColumn->getPropertyValue( PROPERTY_TYPE ) );

                ::std::vector< NamedValue > aInitialValues;
                ::std::vector< OUString > aCopyProperties;
                Any aDefault;

                switch(nType)
                {
                    case DataType::BIT:
                    case DataType::BOOLEAN:
                    {
                        aCurrentModelType = "CheckBox";
                        aInitialValues.push_back( NamedValue( OUString( "VisualEffect" ), makeAny( VisualEffect::FLAT ) ) );
                        sDefaultProperty = PROPERTY_DEFAULTSTATE;

                        sal_Int32 nNullable = ColumnValue::NULLABLE_UNKNOWN;
                        OSL_VERIFY( xColumn->getPropertyValue( PROPERTY_ISNULLABLE ) >>= nNullable );
                        aInitialValues.push_back( NamedValue(
                            OUString( "TriState" ),
                            makeAny( sal_Bool( ColumnValue::NO_NULLS != nNullable ) )
                        ) );
                        if ( ColumnValue::NO_NULLS == nNullable )
                            aDefault <<= (sal_Int16)STATE_NOCHECK;
                    }
                    break;

                    case DataType::LONGVARCHAR:
                    case DataType::CLOB:
                        aInitialValues.push_back( NamedValue( OUString( "MultiLine" ), makeAny( (sal_Bool)sal_True ) ) );
                        
                    case DataType::BINARY:
                    case DataType::VARBINARY:
                    case DataType::LONGVARBINARY:
                        aCurrentModelType = "TextField";
                        sDefaultProperty = PROPERTY_DEFAULTTEXT;
                        break;

                    case DataType::VARCHAR:
                    case DataType::CHAR:
                        bFormattedIsNumeric = sal_False;
                        
                    default:
                        aCurrentModelType = "FormattedField";
                        sDefaultProperty = PROPERTY_EFFECTIVEDEFAULT;

                        if ( xSupplier.is() )
                            aInitialValues.push_back( NamedValue( OUString("FormatsSupplier"), makeAny( xSupplier ) ) );
                        aInitialValues.push_back( NamedValue( OUString("TreatAsNumber"), makeAny( (sal_Bool)bFormattedIsNumeric ) ) );
                        aCopyProperties.push_back( static_cast<const OUString&>(PROPERTY_FORMATKEY) );
                        break;
                }

                aInitialValues.push_back( NamedValue( PROPERTY_CONTROLSOURCE, makeAny( *pIter ) ) );
                OUString sLabel;
                xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
                if ( !sLabel.isEmpty() )
                    aInitialValues.push_back( NamedValue( PROPERTY_LABEL, makeAny( sLabel ) ) );
                else
                    aInitialValues.push_back( NamedValue( PROPERTY_LABEL, makeAny( *pIter ) ) );

                Reference< XPropertySet > xGridCol( xColFactory->createColumn( aCurrentModelType ), UNO_SET_THROW );
                Reference< XPropertySetInfo > xGridColPSI( xGridCol->getPropertySetInfo(), UNO_SET_THROW );

                
                if ( xGridColPSI->hasPropertyByName( PROPERTY_CONTROLDEFAULT ) )
                {
                    aDefault = xColumn->getPropertyValue( PROPERTY_CONTROLDEFAULT );
                    
                    if ( nType == DataType::BIT || nType == DataType::BOOLEAN )
                    {
                        if ( aDefault.hasValue() )
                            aDefault <<= (comphelper::getString(aDefault).toInt32() == 0) ? (sal_Int16)STATE_NOCHECK : (sal_Int16)STATE_CHECK;
                        else
                            aDefault <<= ((sal_Int16)STATE_DONTKNOW);
                    }
                }

                if ( aDefault.hasValue() )
                    aInitialValues.push_back( NamedValue( sDefaultProperty, aDefault ) );

                
                aCopyProperties.push_back( static_cast<const OUString&>(PROPERTY_HIDDEN) );
                aCopyProperties.push_back( static_cast<const OUString&>(PROPERTY_WIDTH) );

                
                Any aDescription;
                if ( xColPSI->hasPropertyByName( PROPERTY_HELPTEXT ) )
                    aDescription = xColumn->getPropertyValue( PROPERTY_HELPTEXT );
                OUString sTemp;
                aDescription >>= sTemp;
                if ( sTemp.isEmpty() )
                    xColumn->getPropertyValue( PROPERTY_DESCRIPTION ) >>= sTemp;

                aDescription <<= sTemp;
                aInitialValues.push_back( NamedValue( PROPERTY_HELPTEXT, aDescription ) );

                
                Any aAlign; aAlign <<= sal_Int16( 0 );
                Any aColAlign( xColumn->getPropertyValue( PROPERTY_ALIGN ) );
                if ( aColAlign.hasValue() )
                    aAlign <<= sal_Int16( ::comphelper::getINT32( aColAlign ) );
                aInitialValues.push_back( NamedValue( PROPERTY_ALIGN, aAlign ) );

                
                if ( xGridColPSI->hasPropertyByName( PROPERTY_MOUSE_WHEEL_BEHAVIOR ) )
                    aInitialValues.push_back( NamedValue( PROPERTY_MOUSE_WHEEL_BEHAVIOR, makeAny( MouseWheelBehavior::SCROLL_DISABLED ) ) );

                
                for ( ::std::vector< NamedValue >::const_iterator property = aInitialValues.begin();
                      property != aInitialValues.end();
                      ++property
                    )
                {
                    xGridCol->setPropertyValue( property->Name, property->Value );
                }
                for ( ::std::vector< OUString >::const_iterator copyPropertyName = aCopyProperties.begin();
                      copyPropertyName != aCopyProperties.end();
                      ++copyPropertyName
                    )
                    xGridCol->setPropertyValue( *copyPropertyName, xColumn->getPropertyValue( *copyPropertyName ) );

                xColContainer->insertByName(*pIter, makeAny(xGridCol));
            }
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
        return sal_False;
    }

    return sal_True;
}

Reference<XPropertySet> getColumnHelper(SvTreeListEntry* _pCurrentlyDisplayed,const Reference<XPropertySet>& _rxSource)
{
    Reference<XPropertySet> xRet;
    if(_pCurrentlyDisplayed)
    {
        DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(_pCurrentlyDisplayed->GetUserData());
        Reference<XColumnsSupplier> xColumnsSup(pData->xObjectProperties,UNO_QUERY);
        Reference<XNameAccess> xNames = xColumnsSup->getColumns();
        OUString aName;
        _rxSource->getPropertyValue(PROPERTY_NAME) >>= aName;
        if(xNames.is() && xNames->hasByName(aName))
            xRet.set(xNames->getByName(aName),UNO_QUERY);
    }
    return xRet;
}

void SbaTableQueryBrowser::transferChangedControlProperty(const OUString& _rProperty, const Any& _rNewValue)
{
    if(m_pCurrentlyDisplayed)
    {
        DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
        Reference< XPropertySet > xObjectProps(pData->xObjectProperties, UNO_QUERY);
        OSL_ENSURE(xObjectProps.is(),"SbaTableQueryBrowser::transferChangedControlProperty: no table/query object!");
        if (xObjectProps.is())
            xObjectProps->setPropertyValue(_rProperty, _rNewValue);
    }
}

void SbaTableQueryBrowser::propertyChange(const PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException)
{
    SbaXDataBrowserController::propertyChange(evt);

    try
    {
        Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
        if (!xSource.is())
            return;

        
        
        else if (evt.PropertyName.equals(PROPERTY_WIDTH))
        {   
            
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
            {
                if(!evt.NewValue.hasValue())
                    xProp->setPropertyValue(PROPERTY_WIDTH,makeAny((sal_Int32)227));
                else
                    xProp->setPropertyValue(PROPERTY_WIDTH,evt.NewValue);
            }
        }

        
        else if (evt.PropertyName.equals(PROPERTY_HIDDEN))
        {
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_HIDDEN,evt.NewValue);
        }

        
        else if (evt.PropertyName.equals(PROPERTY_ALIGN))
        {
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            try
            {
                if(xProp.is())
                {
                    if(evt.NewValue.hasValue())
                    {
                        sal_Int16 nAlign = 0;
                        if(evt.NewValue >>= nAlign)
                            xProp->setPropertyValue(PROPERTY_ALIGN,makeAny(sal_Int32(nAlign)));
                        else
                            xProp->setPropertyValue(PROPERTY_ALIGN,evt.NewValue);
                    }
                    else
                        xProp->setPropertyValue(PROPERTY_ALIGN,makeAny(::com::sun::star::awt::TextAlign::LEFT));
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }

        
        else if (   (evt.PropertyName.equals(PROPERTY_FORMATKEY))
            &&  (TypeClass_LONG == evt.NewValue.getValueTypeClass())
            )
        {
            
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_FORMATKEY,evt.NewValue);
        }

        
        
        else if (evt.PropertyName.equals(PROPERTY_ROW_HEIGHT))
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
                OSL_ENSURE( pData->xObjectProperties.is(), "No table available!" );

                sal_Bool bDefault = !evt.NewValue.hasValue();
                if (bDefault)
                    pData->xObjectProperties->setPropertyValue(PROPERTY_ROW_HEIGHT,makeAny((sal_Int32)45));
                else
                    pData->xObjectProperties->setPropertyValue(PROPERTY_ROW_HEIGHT,evt.NewValue);
            }
        }

        else if (   evt.PropertyName.equals(PROPERTY_FONT)          
                ||  evt.PropertyName.equals(PROPERTY_TEXTCOLOR)     
                ||  evt.PropertyName.equals(PROPERTY_FILTER)        
                ||  evt.PropertyName.equals(PROPERTY_HAVING_CLAUSE) 
                ||  evt.PropertyName.equals(PROPERTY_ORDER)         
                ||  evt.PropertyName.equals(PROPERTY_APPLYFILTER)   
                ||  evt.PropertyName.equals(PROPERTY_TEXTLINECOLOR) 
                ||  evt.PropertyName.equals(PROPERTY_TEXTEMPHASIS)  
                ||  evt.PropertyName.equals(PROPERTY_TEXTRELIEF)    
                )
        {
            transferChangedControlProperty(evt.PropertyName, evt.NewValue);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

sal_Bool SbaTableQueryBrowser::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    if ( getView() && getView()->IsInModalMode() )
        return sal_False;
    sal_Bool bRet = sal_False;
    if ( !m_bInSuspend )
    {
        m_bInSuspend = sal_True;
        if ( rBHelper.bDisposed )
            throw DisposedException( OUString(), *this );

        bRet = SbaXDataBrowserController::suspend(bSuspend);
        if ( bRet && getView() )
            getView()->Hide();

        m_bInSuspend = sal_False;
    }

    return bRet;
}

void SAL_CALL SbaTableQueryBrowser::statusChanged( const FeatureStateEvent& _rEvent ) throw(RuntimeException)
{
    
    Reference< XDispatch > xSource(_rEvent.Source, UNO_QUERY);
    ExternalFeaturesMap::iterator aLoop;
    for ( aLoop = m_aExternalFeatures.begin();
          aLoop != m_aExternalFeatures.end();
          ++aLoop
        )
    {
        if ( _rEvent.FeatureURL.Complete == aLoop->second.aURL.Complete)
        {
            OSL_ENSURE( xSource.get() == aLoop->second.xDispatcher.get(), "SbaTableQueryBrowser::statusChanged: inconsistent!" );
            
            aLoop->second.bEnabled = _rEvent.IsEnabled;

            switch ( aLoop->first )
            {
                case ID_BROWSER_DOCUMENT_DATASOURCE:
                {
                    
                    Sequence< PropertyValue > aDescriptor;
    #if OSL_DEBUG_LEVEL > 0
                    sal_Bool bProperFormat =
    #endif
                    _rEvent.State >>= aDescriptor;
                    OSL_ENSURE(bProperFormat, "SbaTableQueryBrowser::statusChanged: need a data access descriptor here!");
                    m_aDocumentDataSource.initializeFrom(aDescriptor);

                    OSL_ENSURE( (   m_aDocumentDataSource.has(daDataSource)
                                ||  m_aDocumentDataSource.has(daDatabaseLocation)
                                )
                                &&  m_aDocumentDataSource.has(daCommand)
                                &&  m_aDocumentDataSource.has(daCommandType),
                        "SbaTableQueryBrowser::statusChanged: incomplete descriptor!");

                    
                    checkDocumentDataSource();
                }
                break;

                default:
                    
                    implCheckExternalSlot( aLoop->first );
                    break;
            }
            break;
        }
    }

    OSL_ENSURE(aLoop != m_aExternalFeatures.end(), "SbaTableQueryBrowser::statusChanged: don't know who sent this!");
}

void SbaTableQueryBrowser::checkDocumentDataSource()
{
    SvTreeListEntry* pDataSourceEntry = NULL;
    SvTreeListEntry* pContainerEntry = NULL;
    SvTreeListEntry* pObjectEntry = getObjectEntry( m_aDocumentDataSource, &pDataSourceEntry, &pContainerEntry, sal_False );
    sal_Bool bKnownDocDataSource = (NULL != pObjectEntry);
    if (!bKnownDocDataSource)
    {
        if (NULL != pDataSourceEntry)
        {   
            if (NULL != pContainerEntry)
                bKnownDocDataSource = sal_True; 
                
            else
            {
                if ((NULL == pObjectEntry) && m_aDocumentDataSource.has(daCommandType) && m_aDocumentDataSource.has(daCommand))
                {   
                    sal_Int32 nCommandType = CommandType::TABLE;
                    m_aDocumentDataSource[daCommandType] >>= nCommandType;

                    OUString sCommand;
                    m_aDocumentDataSource[daCommand] >>= sCommand;

                    bKnownDocDataSource = (CommandType::COMMAND == nCommandType) && (!sCommand.isEmpty());
                }
            }
        }
    }

    if ( !bKnownDocDataSource )
        m_aExternalFeatures[ ID_BROWSER_DOCUMENT_DATASOURCE ].bEnabled = sal_False;

    
    implCheckExternalSlot(ID_BROWSER_DOCUMENT_DATASOURCE);
}

void SbaTableQueryBrowser::extractDescriptorProps(const ::svx::ODataAccessDescriptor& _rDescriptor, OUString& _rDataSource, OUString& _rCommand, sal_Int32& _rCommandType, sal_Bool& _rEscapeProcessing)
{
    _rDataSource = _rDescriptor.getDataSource();
    if ( _rDescriptor.has(daCommand) )
        _rDescriptor[daCommand] >>= _rCommand;
    if ( _rDescriptor.has(daCommandType) )
        _rDescriptor[daCommandType] >>= _rCommandType;

    
    _rEscapeProcessing = sal_True;
    if (_rDescriptor.has(daEscapeProcessing))
        _rEscapeProcessing = ::cppu::any2bool(_rDescriptor[daEscapeProcessing]);
}

namespace
{
    bool getDataSourceDisplayName_isURL( const OUString& _rDS, OUString& _rDisplayName, OUString& _rUniqueId )
    {
        INetURLObject aURL( _rDS );
        if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        {
            _rDisplayName = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DECODE_WITH_CHARSET);
            _rUniqueId = aURL.GetMainURL( INetURLObject::NO_DECODE );
            return true;
        }
        _rDisplayName = _rDS;
        _rUniqueId = "";
        return false;
    }

    struct FilterByEntryDataId : public IEntryFilter
    {
        OUString sId;
        FilterByEntryDataId( const OUString& _rId ) : sId( _rId ) { }

        virtual ~FilterByEntryDataId() {}

        virtual bool    includeEntry( SvTreeListEntry* _pEntry ) const;
    };

    bool FilterByEntryDataId::includeEntry( SvTreeListEntry* _pEntry ) const
    {
        DBTreeListUserData* pData = static_cast< DBTreeListUserData* >( _pEntry->GetUserData() );
        return ( !pData || ( pData->sAccessor == sId ) );
    }
}

OUString SbaTableQueryBrowser::getDataSourceAcessor( SvTreeListEntry* _pDataSourceEntry ) const
{
    OSL_ENSURE( _pDataSourceEntry, "SbaTableQueryBrowser::getDataSourceAcessor: invalid entry!" );

    DBTreeListUserData* pData = static_cast< DBTreeListUserData* >( _pDataSourceEntry->GetUserData() );
    OSL_ENSURE( pData, "SbaTableQueryBrowser::getDataSourceAcessor: invalid entry data!" );
    OSL_ENSURE( pData->eType == etDatasource, "SbaTableQueryBrowser::getDataSourceAcessor: entry does not denote a data source!" );
    return !pData->sAccessor.isEmpty() ? OUString(pData->sAccessor) : GetEntryText( _pDataSourceEntry );
}

SvTreeListEntry* SbaTableQueryBrowser::getObjectEntry(const OUString& _rDataSource, const OUString& _rCommand, sal_Int32 _nCommandType,
        SvTreeListEntry** _ppDataSourceEntry, SvTreeListEntry** _ppContainerEntry, sal_Bool _bExpandAncestors,
        const SharedConnection& _rxConnection )
{
    if (_ppDataSourceEntry)
        *_ppDataSourceEntry = NULL;
    if (_ppContainerEntry)
        *_ppContainerEntry = NULL;

    SvTreeListEntry* pObject = NULL;
    if ( m_pTreeView )
    {
        
        OUString sDisplayName, sDataSourceId;
        bool bIsDataSourceURL = getDataSourceDisplayName_isURL( _rDataSource, sDisplayName, sDataSourceId );
            
            

        FilterByEntryDataId aFilter( sDataSourceId );
        SvTreeListEntry* pDataSource = m_pTreeView->getListBox().GetEntryPosByName( sDisplayName, NULL, &aFilter );
        if ( !pDataSource ) 
        {
            if ( bIsDataSourceURL )
            {
                
                
                implAddDatasource( _rDataSource, _rxConnection );
                pDataSource = m_pTreeView->getListBox().GetEntryPosByName( sDisplayName, NULL, &aFilter );
                OSL_ENSURE( pDataSource, "SbaTableQueryBrowser::getObjectEntry: hmm - did not find it again!" );
            }
        }
        if (_ppDataSourceEntry)
            
            *_ppDataSourceEntry = pDataSource;

        if (pDataSource)
        {
            
            if (_bExpandAncestors)
                m_pTreeView->getListBox().Expand(pDataSource);

            
            SvTreeListEntry* pCommandType = NULL;
            switch (_nCommandType)
            {
                case CommandType::TABLE:
                    pCommandType = m_pTreeView->getListBox().GetModel()->GetEntry(pDataSource, CONTAINER_TABLES);
                    break;

                case CommandType::QUERY:
                    pCommandType = m_pTreeView->getListBox().GetModel()->GetEntry(pDataSource, CONTAINER_QUERIES);
                    break;
            }

            if (_ppContainerEntry)
                *_ppContainerEntry = pCommandType;

            if (pCommandType)
            {
                
                if (_bExpandAncestors)
                {
                    m_pTreeView->getListBox().Expand(pCommandType);
                }

                
                OUString sCommand = _rCommand;
                sal_Int32 nIndex = 0;
                do
                {
                    OUString sPath;
                    switch (_nCommandType)
                    {
                    case CommandType::TABLE:
                        sPath = sCommand;
                        nIndex = -1;
                        break;

                    default:
                        assert(false);
                        
                    case CommandType::QUERY:
                        sPath = sCommand.getToken( 0, '/', nIndex );
                        break;
                    }
                    pObject = m_pTreeView->getListBox().GetEntryPosByName(sPath, pCommandType);
                    pCommandType = pObject;
                    if ( nIndex >= 0 )
                    {
                        if (ensureEntryObject(pObject))
                        {
                            DBTreeListUserData* pParentData = static_cast< DBTreeListUserData* >( pObject->GetUserData() );
                            Reference< XNameAccess > xCollection( pParentData->xContainer, UNO_QUERY );
                            sal_Int32 nIndex2 = nIndex;
                            sPath = sCommand.getToken( 0, '/', nIndex2 );
                            try
                            {
                                if ( xCollection->hasByName(sPath) )
                                {
                                    if(!m_pTreeView->getListBox().GetEntryPosByName(sPath,pObject))
                                    {
                                        Reference<XNameAccess> xChild(xCollection->getByName(sPath),UNO_QUERY);
                                        DBTreeListUserData* pEntryData = new DBTreeListUserData;
                                        pEntryData->eType = etQuery;
                                        if ( xChild.is() )
                                        {
                                            pEntryData->eType = etQueryContainer;
                                        }
                                        implAppendEntry( pObject, sPath, pEntryData, pEntryData->eType );
                                    }
                                }
                            }
                            catch(const Exception&)
                            {
                                SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::populateTree: could not fill the tree");
                            }
                        }
                    }
                }
                while ( nIndex >= 0 );
            }
        }
    }
    return pObject;
}

SvTreeListEntry* SbaTableQueryBrowser::getObjectEntry(const ::svx::ODataAccessDescriptor& _rDescriptor,
        SvTreeListEntry** _ppDataSourceEntry, SvTreeListEntry** _ppContainerEntry,
        sal_Bool _bExpandAncestors)
{
    
    OUString sDataSource;
    OUString sCommand;
    sal_Int32 nCommandType = CommandType::COMMAND;
    sal_Bool bEscapeProcessing = sal_True;
    extractDescriptorProps(_rDescriptor, sDataSource, sCommand, nCommandType, bEscapeProcessing);

    return getObjectEntry( sDataSource, sCommand, nCommandType, _ppDataSourceEntry, _ppContainerEntry, _bExpandAncestors, SharedConnection() );
}

void SbaTableQueryBrowser::connectExternalDispatches()
{
    Reference< XDispatchProvider >  xProvider( getFrame(), UNO_QUERY );
    OSL_ENSURE(xProvider.is(), "SbaTableQueryBrowser::connectExternalDispatches: no DispatchProvider !");
    if (xProvider.is())
    {
        if ( m_aExternalFeatures.empty() )
        {
            const sal_Char* pURLs[] = {
                ".uno:DataSourceBrowser/DocumentDataSource",
                ".uno:DataSourceBrowser/FormLetter",
                ".uno:DataSourceBrowser/InsertColumns",
                ".uno:DataSourceBrowser/InsertContent",
            };
            const sal_uInt16 nIds[] = {
                ID_BROWSER_DOCUMENT_DATASOURCE,
                ID_BROWSER_FORMLETTER,
                ID_BROWSER_INSERTCOLUMNS,
                ID_BROWSER_INSERTCONTENT
            };

            for ( size_t i=0; i < sizeof( pURLs ) / sizeof( pURLs[0] ); ++i )
            {
                URL aURL;
                aURL.Complete = OUString::createFromAscii( pURLs[i] );
                if ( m_xUrlTransformer.is() )
                    m_xUrlTransformer->parseStrict( aURL );
                m_aExternalFeatures[ nIds[ i ] ] = ExternalFeature( aURL );
            }
        }

        for ( ExternalFeaturesMap::iterator feature = m_aExternalFeatures.begin();
              feature != m_aExternalFeatures.end();
              ++feature
            )
        {
            feature->second.xDispatcher = xProvider->queryDispatch(
                feature->second.aURL, OUString("_parent"), FrameSearchFlag::PARENT
            );

            if ( feature->second.xDispatcher.get() == static_cast< XDispatch* >( this ) )
            {
                SAL_WARN("dbaccess.ui",  "SbaTableQueryBrowser::connectExternalDispatches: this should not happen anymore!" );
                    
                    
                feature->second.xDispatcher.clear();
            }

            if ( feature->second.xDispatcher.is() )
            {
                try
                {
                    feature->second.xDispatcher->addStatusListener( this, feature->second.aURL );
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }

            implCheckExternalSlot( feature->first );
        }
    }
}

void SbaTableQueryBrowser::implCheckExternalSlot( sal_uInt16 _nId )
{
    if ( !m_xMainToolbar.is() )
        return;

    Window* pToolboxWindow = VCLUnoHelper::GetWindow( m_xMainToolbar );
    ToolBox* pToolbox = dynamic_cast< ToolBox* >( pToolboxWindow );
    OSL_ENSURE( pToolbox, "SbaTableQueryBrowser::implCheckExternalSlot: cannot obtain the toolbox window!" );

    
    if ( pToolbox )
    {
        bool bHaveDispatcher = m_aExternalFeatures[ _nId ].xDispatcher.is();
        if ( bHaveDispatcher != pToolbox->IsItemVisible( _nId ) )
            bHaveDispatcher ? pToolbox->ShowItem( _nId ) : pToolbox->HideItem( _nId );
    }

    
    InvalidateFeature( _nId );
}

void SAL_CALL SbaTableQueryBrowser::disposing( const com::sun::star::lang::EventObject& _rSource ) throw(RuntimeException)
{
    
    Reference< ::com::sun::star::frame::XFrame >  xSourceFrame(_rSource.Source, UNO_QUERY);
    if (m_xCurrentFrameParent.is() && (xSourceFrame == m_xCurrentFrameParent))
        m_xCurrentFrameParent->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
    else
    {
        
        Reference< XDispatch > xSource(_rSource.Source, UNO_QUERY);
        if(xSource.is())
        {
            ExternalFeaturesMap::iterator aLoop = m_aExternalFeatures.begin();
            ExternalFeaturesMap::iterator aEnd = m_aExternalFeatures.end();
            while (aLoop != aEnd)
            {
                ExternalFeaturesMap::iterator aI = aLoop++;
                if ( aI->second.xDispatcher.get() == xSource.get() )
                {
                    sal_uInt16 nSlot = aI->first;

                    
                    m_aExternalFeatures.erase(aI);

                    
                    implCheckExternalSlot(nSlot);

                    
                }
            }
        }
        else
        {
            Reference<XConnection> xCon(_rSource.Source, UNO_QUERY);
            if ( xCon.is() && m_pTreeView )
            {   
                
                
                SvTreeListEntry* pDSLoop = m_pTreeView->getListBox().FirstChild(NULL);
                while (pDSLoop)
                {
                    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pDSLoop->GetUserData());
                    if ( pData && pData->xConnection == xCon )
                    {
                        
                        pData->xConnection.clear();
                        closeConnection(pDSLoop,sal_False);
                        break;
                    }

                    pDSLoop = m_pTreeView->getListBox().NextSibling(pDSLoop);
                }
            }
            else
                SbaXDataBrowserController::disposing(_rSource);
        }
    }
}

void SbaTableQueryBrowser::implRemoveStatusListeners()
{
    
    for ( ExternalFeaturesMap::const_iterator aLoop = m_aExternalFeatures.begin();
          aLoop != m_aExternalFeatures.end();
          ++aLoop
        )
    {
        if ( aLoop->second.xDispatcher.is() )
        {
            try
            {
                aLoop->second.xDispatcher->removeStatusListener( this, aLoop->second.aURL );
            }
            catch (Exception&)
            {
                SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::implRemoveStatusListeners: could not remove a status listener!");
            }
        }
    }
    m_aExternalFeatures.clear();
}

sal_Bool SAL_CALL SbaTableQueryBrowser::select( const Any& _rSelection ) throw (IllegalArgumentException, RuntimeException)
{
    SolarMutexGuard aGuard;
        

    Sequence< PropertyValue > aDescriptorSequence;
    if (!(_rSelection >>= aDescriptorSequence))
        throw IllegalArgumentException(OUString(), *this, 1);
        

    ODataAccessDescriptor aDescriptor;
    try
    {
        aDescriptor = ODataAccessDescriptor(aDescriptorSequence);
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::select: could not extract the descriptor!");
    }

    
    if ( !(aDescriptor.has(daDataSource) || aDescriptor.has(daDatabaseLocation)) || !aDescriptor.has(daCommand) || !aDescriptor.has(daCommandType))
        throw IllegalArgumentException(OUString(), *this, 1);
        

    return implSelect(aDescriptor,sal_True);
}

Any SAL_CALL SbaTableQueryBrowser::getSelection(  ) throw (RuntimeException)
{
    Any aReturn;

    try
    {
        Reference< XLoadable > xLoadable(getRowSet(), UNO_QUERY);
        if (xLoadable.is() && xLoadable->isLoaded())
        {
            Reference< XPropertySet > aFormProps(getRowSet(), UNO_QUERY);
            ODataAccessDescriptor aDescriptor(aFormProps);
            
            aDescriptor.erase(daConnection);
            aDescriptor.erase(daCursor);

            aReturn <<= aDescriptor.createPropertyValueSequence();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aReturn;
}

void SAL_CALL SbaTableQueryBrowser::addSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener ) throw (RuntimeException)
{
    m_aSelectionListeners.addInterface(_rxListener);
}

void SAL_CALL SbaTableQueryBrowser::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener ) throw (RuntimeException)
{
    m_aSelectionListeners.removeInterface(_rxListener);
}

void SbaTableQueryBrowser::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & _xFrame) throw( RuntimeException )
{
    implRemoveStatusListeners();

    if (m_xCurrentFrameParent.is())
        m_xCurrentFrameParent->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);

    SbaXDataBrowserController::attachFrame(_xFrame);

    Reference< XFrame > xCurrentFrame( getFrame() );
    if ( xCurrentFrame.is() )
    {
        m_xCurrentFrameParent = xCurrentFrame->findFrame(OUString("_parent"),FrameSearchFlag::PARENT);
        if ( m_xCurrentFrameParent.is() )
            m_xCurrentFrameParent->addFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);

        
        try
        {
            Reference< XPropertySet > xFrameProps( m_aCurrentFrame.getFrame(), UNO_QUERY_THROW );
            Reference< XLayoutManager > xLayouter(
                xFrameProps->getPropertyValue("LayoutManager"),
                UNO_QUERY );

            if ( xLayouter.is() )
            {
                Reference< XUIElement > xUI(
                    xLayouter->getElement( OUString( "private:resource/toolbar/toolbar" ) ),
                    UNO_SET_THROW );
                m_xMainToolbar = m_xMainToolbar.query( xUI->getRealInterface() );
                OSL_ENSURE( m_xMainToolbar.is(), "SbaTableQueryBrowser::attachFrame: where's my toolbox?" );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    
    connectExternalDispatches();
}

void SbaTableQueryBrowser::addModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    SbaXDataBrowserController::addModelListeners(_xGridControlModel);
    Reference< XPropertySet >  xSourceSet(_xGridControlModel, UNO_QUERY);
    if (xSourceSet.is())
    {
        xSourceSet->addPropertyChangeListener(PROPERTY_ROW_HEIGHT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_FONT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTCOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTLINECOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTEMPHASIS, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->addPropertyChangeListener(PROPERTY_TEXTRELIEF, static_cast<XPropertyChangeListener*>(this));
    }

}

void SbaTableQueryBrowser::removeModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    SbaXDataBrowserController::removeModelListeners(_xGridControlModel);
    Reference< XPropertySet >  xSourceSet(_xGridControlModel, UNO_QUERY);
    if (xSourceSet.is())
    {
        xSourceSet->removePropertyChangeListener(PROPERTY_ROW_HEIGHT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_FONT, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTCOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTLINECOLOR, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTEMPHASIS, static_cast<XPropertyChangeListener*>(this));
        xSourceSet->removePropertyChangeListener(PROPERTY_TEXTRELIEF, static_cast<XPropertyChangeListener*>(this));
    }
}

void SbaTableQueryBrowser::RowChanged()
{
    if(getBrowserView())
    {
        SbaGridControl* pControl = getBrowserView()->getVclControl();
        if (!pControl->IsEditing())
            InvalidateFeature(ID_BROWSER_COPY);
    }
    SbaXDataBrowserController::RowChanged();
}

void SbaTableQueryBrowser::ColumnChanged()
{
    if(getBrowserView())
    {
        SbaGridControl* pControl = getBrowserView()->getVclControl();
        if (!pControl->IsEditing())
            InvalidateFeature(ID_BROWSER_COPY);
    }
    SbaXDataBrowserController::ColumnChanged();
}

void SbaTableQueryBrowser::AddColumnListener(const Reference< XPropertySet > & xCol)
{
    SbaXDataBrowserController::AddColumnListener(xCol);
    SafeAddPropertyListener(xCol, PROPERTY_WIDTH, static_cast<XPropertyChangeListener*>(this));
    SafeAddPropertyListener(xCol, PROPERTY_HIDDEN, static_cast<XPropertyChangeListener*>(this));
    SafeAddPropertyListener(xCol, PROPERTY_ALIGN, static_cast<XPropertyChangeListener*>(this));
    SafeAddPropertyListener(xCol, PROPERTY_FORMATKEY, static_cast<XPropertyChangeListener*>(this));
}

void SbaTableQueryBrowser::RemoveColumnListener(const Reference< XPropertySet > & xCol)
{
    SbaXDataBrowserController::RemoveColumnListener(xCol);
    SafeRemovePropertyListener(xCol, PROPERTY_WIDTH, static_cast<XPropertyChangeListener*>(this));
    SafeRemovePropertyListener(xCol, PROPERTY_HIDDEN, static_cast<XPropertyChangeListener*>(this));
    SafeRemovePropertyListener(xCol, PROPERTY_ALIGN, static_cast<XPropertyChangeListener*>(this));
    SafeRemovePropertyListener(xCol, PROPERTY_FORMATKEY, static_cast<XPropertyChangeListener*>(this));
}

void SbaTableQueryBrowser::criticalFail()
{
    SbaXDataBrowserController::criticalFail();
    unloadAndCleanup( sal_False );
}

void SbaTableQueryBrowser::LoadFinished(sal_Bool _bWasSynch)
{
    SbaXDataBrowserController::LoadFinished(_bWasSynch);

    m_sQueryCommand = "";
    m_bQueryEscapeProcessing = sal_False;

    if (isValid() && !loadingCancelled())
    {
        
        sal_Bool bTemporary;    
        if ( implGetQuerySignature( m_sQueryCommand, bTemporary ) )
            m_bQueryEscapeProcessing = bTemporary;
    }

    
    com::sun::star::lang::EventObject aEvent( *this );
    m_aSelectionListeners.notifyEach( &XSelectionChangeListener::selectionChanged, aEvent );
}

sal_Bool SbaTableQueryBrowser::getExternalSlotState( sal_uInt16 _nId ) const
{
    sal_Bool bEnabled = sal_False;
    ExternalFeaturesMap::const_iterator aPos = m_aExternalFeatures.find( _nId );
    if ( ( m_aExternalFeatures.end() != aPos ) && aPos->second.xDispatcher.is() )
        bEnabled = aPos->second.bEnabled;
    return bEnabled;
}

FeatureState SbaTableQueryBrowser::GetState(sal_uInt16 nId) const
{
    FeatureState aReturn;
        

    
    if (!getBrowserView() || !getBrowserView()->getVclControl())
        return aReturn;

    switch ( nId )
    {
        case ID_TREE_ADMINISTRATE:
            aReturn.bEnabled = true;
            return aReturn;

        case ID_BROWSER_CLOSE:
            
            aReturn.bEnabled = !m_bEnableBrowser;
            return aReturn;

            
        case ID_BROWSER_EXPLORER:
            aReturn.bEnabled = m_bEnableBrowser;
            aReturn.bChecked = haveExplorer();
            return aReturn;

        case ID_BROWSER_REMOVEFILTER:
            return SbaXDataBrowserController::GetState( nId );

        case ID_BROWSER_COPY:
            if ( !m_pTreeView->HasChildPathFocus() )
                
                break;
            
        case ID_TREE_CLOSE_CONN:
        case ID_TREE_EDIT_DATABASE:
        {
            SvTreeListEntry* pCurrentEntry( m_pTreeView->getListBox().GetCurEntry() );
            EntryType eType = getEntryType( pCurrentEntry );
            if ( eType == etUnknown )
                return aReturn;

            SvTreeListEntry* pDataSourceEntry = m_pTreeView->getListBox().GetRootLevelParent( pCurrentEntry );
            DBTreeListUserData* pDSData
                =   pDataSourceEntry
                ?   static_cast< DBTreeListUserData* >( pDataSourceEntry->GetUserData() )
                :   NULL;

            if ( nId == ID_TREE_CLOSE_CONN )
            {
                aReturn.bEnabled = ( pDSData != NULL ) && pDSData->xConnection.is();
            }
            else if ( nId == ID_TREE_EDIT_DATABASE )
            {
                ::utl::OConfigurationTreeRoot aConfig( ::utl::OConfigurationTreeRoot::createWithComponentContext( getORB(),
                    OUString( "/org.openoffice.Office.DataAccess/Policies/Features/Common" ) ) );
                sal_Bool bHaveEditDatabase( sal_True );
                OSL_VERIFY( aConfig.getNodeValue( "EditDatabaseFromDataSourceView" ) >>= bHaveEditDatabase );
                aReturn.bEnabled = getORB().is() && ( pDataSourceEntry != NULL ) && bHaveEditDatabase;
            }
            else if ( nId == ID_BROWSER_COPY )
            {
                aReturn.bEnabled = isEntryCopyAllowed( pCurrentEntry );
            }

            return aReturn;
        }
    }

    
    if (!isLoaded())
        return aReturn;

    try
    {
        sal_Bool bHandled = sal_False;
        switch (nId)
        {
            case ID_BROWSER_DOCUMENT_DATASOURCE:
                
                
                aReturn.bEnabled = getExternalSlotState( ID_BROWSER_DOCUMENT_DATASOURCE );
                bHandled = sal_True;
                break;
            case ID_BROWSER_REFRESH:
                aReturn.bEnabled = sal_True;
                bHandled = sal_True;
                break;
        }

        if (bHandled)
            return aReturn;

        
        if (isValid() && !isValidCursor() && nId != ID_BROWSER_CLOSE)
            return aReturn;

        switch (nId)
        {
            case ID_BROWSER_INSERTCOLUMNS:
            case ID_BROWSER_INSERTCONTENT:
            case ID_BROWSER_FORMLETTER:
            {
                
                
                aReturn.bEnabled = getExternalSlotState( nId );

                
                if (ID_BROWSER_FORMLETTER != nId)
                    aReturn.bEnabled = aReturn.bEnabled && getBrowserView()->getVclControl()->GetSelectRowCount();

                
                Reference< XPropertySet >  xDataSource(getRowSet(), UNO_QUERY);
                try
                {
                    aReturn.bEnabled = aReturn.bEnabled && xDataSource.is();

                    if (xDataSource.is())
                    {
                        sal_Int32 nType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMAND_TYPE));
                        aReturn.bEnabled = aReturn.bEnabled && ((::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)) || (nType == ::com::sun::star::sdb::CommandType::QUERY)));
                    }
                }
                catch(DisposedException&)
                {
                    SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::GetState: object already disposed!");
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION();
                }
            }
            break;

            case ID_BROWSER_TITLE:
                {
                    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
                    sal_Int32 nCommandType = CommandType::TABLE;
                    xProp->getPropertyValue(PROPERTY_COMMAND_TYPE) >>= nCommandType;
                    OUString sTitle;
                    switch (nCommandType)
                    {
                        case CommandType::TABLE:
                            sTitle = OUString(ModuleRes(STR_TBL_TITLE)); break;
                        case CommandType::QUERY:
                        case CommandType::COMMAND:
                            sTitle = OUString(ModuleRes(STR_QRY_TITLE)); break;
                        default:
                            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::GetState: unknown command type!");
                    }
                    OUString aName;
                    xProp->getPropertyValue(PROPERTY_COMMAND) >>= aName;
                    OUString sObject(aName);

                    aReturn.sTitle = sTitle.replaceFirst(OUString('#'), sObject);
                    aReturn.bEnabled = sal_True;
                }
                break;
            case ID_BROWSER_TABLEATTR:
            case ID_BROWSER_ROWHEIGHT:
            case ID_BROWSER_COLATTRSET:
            case ID_BROWSER_COLWIDTH:
                aReturn.bEnabled = getBrowserView() && getBrowserView()->getVclControl() && isValid() && isValidCursor();
                
                break;

            case ID_BROWSER_COPY:
                OSL_ENSURE( !m_pTreeView->HasChildPathFocus(), "SbaTableQueryBrowser::GetState( ID_BROWSER_COPY ): this should have been handled above!" );
                if (getBrowserView() && getBrowserView()->getVclControl() && !getBrowserView()->getVclControl()->IsEditing())
                {
                    SbaGridControl* pControl = getBrowserView()->getVclControl();
                    if ( pControl->GetSelectRowCount() )
                    {
                        aReturn.bEnabled = m_aCurrentFrame.isActive();
                        break;
                    }
                    else
                        aReturn.bEnabled = pControl->canCopyCellText(pControl->GetCurRow(), pControl->GetCurColumnId());
                    break;
                }
                
            default:
                return SbaXDataBrowserController::GetState(nId);
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return aReturn;

}

void SbaTableQueryBrowser::Execute(sal_uInt16 nId, const Sequence< PropertyValue >& aArgs)
{
    switch (nId)
    {
        default:
            SbaXDataBrowserController::Execute(nId,aArgs);
            break;

        case ID_TREE_EDIT_DATABASE:
            implAdministrate( m_pTreeView->getListBox().GetCurEntry() );
            break;

        case ID_TREE_CLOSE_CONN:
            openHelpAgent( OString( HID_DSBROWSER_DISCONNECTING ));
            closeConnection( m_pTreeView->getListBox().GetRootLevelParent( m_pTreeView->getListBox().GetCurEntry() ) );
            break;

        case ID_TREE_ADMINISTRATE:
            ::svx::administrateDatabaseRegistration( getView() );
            break;

        case ID_BROWSER_REFRESH:
        {
            if ( !SaveModified( ) )
                
                break;

            sal_Bool bFullReinit = sal_False;
            
            if ( !m_sQueryCommand.isEmpty() )
            {
                OUString sNewQueryCommand;
                sal_Bool bNewQueryEP;

#if OSL_DEBUG_LEVEL > 0
                sal_Bool bIsQuery =
#endif
                implGetQuerySignature( sNewQueryCommand, bNewQueryEP );
                OSL_ENSURE( bIsQuery, "SbaTableQueryBrowser::Execute: was a query before, but is not anymore?" );

                bFullReinit = ( sNewQueryCommand != m_sQueryCommand ) || ( m_bQueryEscapeProcessing != bNewQueryEP );
            }
            if ( !bFullReinit )
            {
                
                SbaXDataBrowserController::Execute(nId,aArgs);
                break;
            }
            
        }

        case ID_BROWSER_REFRESH_REBUILD:
        {
            if ( !SaveModified() )
                
                break;

            SvTreeListEntry* pSelected = m_pCurrentlyDisplayed;
            
            unloadAndCleanup( sal_False );

            
            if ( pSelected )
            {
                implSelect( pSelected );
            }
            else
            {
                Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
                implSelect(::svx::ODataAccessDescriptor(xProp));
            }
        }
        break;

        case ID_BROWSER_EXPLORER:
            toggleExplorer();
            break;

        case ID_BROWSER_DOCUMENT_DATASOURCE:
            implSelect(m_aDocumentDataSource);
            break;

        case ID_BROWSER_INSERTCOLUMNS:
        case ID_BROWSER_INSERTCONTENT:
        case ID_BROWSER_FORMLETTER:
            if (getBrowserView() && isValidCursor())
            {
                
                OSL_ENSURE( m_aExternalFeatures.find( nId ) != m_aExternalFeatures.end(),
                    "SbaTableQueryBrowser::Execute( ID_BROWSER_?): how could this ever be enabled?" );
                URL aParentUrl = m_aExternalFeatures[ nId ].aURL;

                
                Reference< XDispatch > xDispatch( m_aExternalFeatures[ nId ].xDispatcher );
                if (xDispatch.is())
                {
                    

                    
                    SbaGridControl* pGrid = getBrowserView()->getVclControl();
                    MultiSelection* pSelection = (MultiSelection*)pGrid->GetSelection();
                    Sequence< Any > aSelection;
                    if ( !pGrid->IsAllSelected() )
                    {   
                        
                        
                        if (pSelection != NULL)
                        {
                            aSelection.realloc(pSelection->GetSelectCount());
                            long nIdx = pSelection->FirstSelected();
                            Any* pSelectionNos = aSelection.getArray();
                            while (nIdx >= 0)
                            {
                                *pSelectionNos++ <<= (sal_Int32)(nIdx + 1);
                                nIdx = pSelection->NextSelected();
                            }
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
                        SAL_WARN("dbaccess.ui", "Object already disposed!");
                    }
                    catch(const Exception&)
                    {
                        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::Execute(ID_BROWSER_?): could not clone the cursor!");
                    }

                    Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);

                    try
                    {
                        ODataAccessDescriptor aDescriptor;
                        OUString sDataSourceName;
                        xProp->getPropertyValue(PROPERTY_DATASOURCENAME) >>= sDataSourceName;

                        aDescriptor.setDataSource(sDataSourceName);
                        aDescriptor[daCommand]      =   xProp->getPropertyValue(PROPERTY_COMMAND);
                        aDescriptor[daCommandType]  =   xProp->getPropertyValue(PROPERTY_COMMAND_TYPE);
                        aDescriptor[daConnection]   =   xProp->getPropertyValue(PROPERTY_ACTIVE_CONNECTION);
                        aDescriptor[daCursor]       <<= xCursorClone;
                        if ( aSelection.getLength() )
                        {
                            aDescriptor[daSelection]            <<= aSelection;
                            aDescriptor[daBookmarkSelection]    <<= sal_False;
                                
                                
                                
                        }

                        xDispatch->dispatch(aParentUrl, aDescriptor.createPropertyValueSequence());
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
            break;

        case ID_BROWSER_CLOSE:
            closeTask();
            
            break;

        case ID_BROWSER_COPY:
            if(m_pTreeView->HasChildPathFocus())
            {
                copyEntry(m_pTreeView->getListBox().GetCurEntry());
            }
            else if (getBrowserView() && getBrowserView()->getVclControl() && !getBrowserView()->getVclControl()->IsEditing() && getBrowserView()->getVclControl()->GetSelectRowCount() < 1)
            {
                SbaGridControl* pControl = getBrowserView()->getVclControl();
                pControl->copyCellText(pControl->GetCurRow(), pControl->GetCurColumnId());
            }
            else
                SbaXDataBrowserController::Execute(nId,aArgs);
            break;
    }
}

void SbaTableQueryBrowser::implAddDatasource( const OUString& _rDataSourceName, const SharedConnection& _rxConnection )
{
    Image a, b, c;
    OUString d, e;
    implAddDatasource( _rDataSourceName, a, d, b, e, c, _rxConnection );
}

void SbaTableQueryBrowser::implAddDatasource(const OUString& _rDbName, Image& _rDbImage,
        OUString& _rQueryName, Image& _rQueryImage, OUString& _rTableName, Image& _rTableImage,
        const SharedConnection& _rxConnection)
{
    SolarMutexGuard aGuard;
    
    if (_rQueryName.isEmpty())
        _rQueryName = OUString(ModuleRes(RID_STR_QUERIES_CONTAINER));
    if (_rTableName.isEmpty())
        _rTableName = OUString(ModuleRes(RID_STR_TABLES_CONTAINER));

    ImageProvider aImageProvider;
    if (!_rQueryImage)
        _rQueryImage = aImageProvider.getFolderImage( DatabaseObject::QUERY );
    if (!_rTableImage)
        _rTableImage = aImageProvider.getFolderImage( DatabaseObject::TABLE );

    if (!_rDbImage)
        _rDbImage = aImageProvider.getDatabaseImage();

    
    
    
    OUString sDSDisplayName, sDataSourceId;
    getDataSourceDisplayName_isURL( _rDbName, sDSDisplayName, sDataSourceId );

    SvTreeListEntry* pDatasourceEntry = m_pTreeView->getListBox().InsertEntry( sDSDisplayName, _rDbImage, _rDbImage, NULL, sal_False );
    DBTreeListUserData* pDSData = new DBTreeListUserData;
    pDSData->eType = etDatasource;
    pDSData->sAccessor = sDataSourceId;
    pDSData->xConnection = _rxConnection;
    pDatasourceEntry->SetUserData(pDSData);

    
    {
        DBTreeListUserData* pQueriesData = new DBTreeListUserData;
        pQueriesData->eType = etQueryContainer;

        m_pTreeView->getListBox().InsertEntry(
            _rQueryName, _rQueryImage, _rQueryImage, pDatasourceEntry,
            sal_True /*ChildrenOnDemand*/, LIST_APPEND, pQueriesData );
    }

    
    {
        DBTreeListUserData* pTablesData = new DBTreeListUserData;
        pTablesData->eType = etTableContainer;

        m_pTreeView->getListBox().InsertEntry(
            _rTableName, _rTableImage, _rTableImage, pDatasourceEntry,
            sal_True /*ChildrenOnDemand*/, LIST_APPEND, pTablesData );
    }

}

void SbaTableQueryBrowser::initializeTreeModel()
{
    if (m_xDatabaseContext.is())
    {
        Image aDBImage, aQueriesImage, aTablesImage;
        OUString sQueriesName, sTablesName;

        
        Sequence< OUString > aDatasources = m_xDatabaseContext->getElementNames();
        const OUString* pIter    = aDatasources.getConstArray();
        const OUString* pEnd     = pIter + aDatasources.getLength();
        for (; pIter != pEnd; ++pIter)
            implAddDatasource( *pIter, aDBImage, sQueriesName, aQueriesImage, sTablesName, aTablesImage, SharedConnection() );
    }
}

void SbaTableQueryBrowser::populateTree(const Reference<XNameAccess>& _xNameAccess,
                                            SvTreeListEntry* _pParent,
                                            EntryType _eEntryType)
{
    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(_pParent->GetUserData());
    if(pData) 
        pData->xContainer = _xNameAccess;

    try
    {
        Sequence< OUString > aNames = _xNameAccess->getElementNames();
        const OUString* pIter    = aNames.getConstArray();
        const OUString* pEnd     = pIter + aNames.getLength();
        for (; pIter != pEnd; ++pIter)
        {
            if( !m_pTreeView->getListBox().GetEntryPosByName(*pIter,_pParent))
            {
                DBTreeListUserData* pEntryData = new DBTreeListUserData;
                pEntryData->eType = _eEntryType;
                if ( _eEntryType == etQuery )
                {
                    Reference<XNameAccess> xChild(_xNameAccess->getByName(*pIter),UNO_QUERY);
                    if ( xChild.is() )
                        pEntryData->eType = etQueryContainer;
                }
                implAppendEntry( _pParent, *pIter, pEntryData, pEntryData->eType );
            }
        }
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::populateTree: could not fill the tree");
    }
}

SvTreeListEntry* SbaTableQueryBrowser::implAppendEntry( SvTreeListEntry* _pParent, const OUString& _rName, void* _pUserData, EntryType _eEntryType )
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ImageProvider > pImageProvider( getImageProviderFor( _pParent ) );
    SAL_WNODEPRECATED_DECLARATIONS_POP

    Image aImage;
    pImageProvider->getImages( _rName, getDatabaseObjectType( _eEntryType ), aImage );

    SvTreeListEntry* pNewEntry = m_pTreeView->getListBox().InsertEntry( _rName, _pParent, _eEntryType == etQueryContainer , LIST_APPEND, _pUserData );

    m_pTreeView->getListBox().SetExpandedEntryBmp(  pNewEntry, aImage );
    m_pTreeView->getListBox().SetCollapsedEntryBmp( pNewEntry, aImage );

    return pNewEntry;
}

IMPL_LINK(SbaTableQueryBrowser, OnExpandEntry, SvTreeListEntry*, _pParent)
{
    if (_pParent->HasChildren())
        
        return 1L;

    SvTreeListEntry* pFirstParent = m_pTreeView->getListBox().GetRootLevelParent(_pParent);
    OSL_ENSURE(pFirstParent,"SbaTableQueryBrowser::OnExpandEntry: No rootlevelparent!");

    DBTreeListUserData* pData = static_cast< DBTreeListUserData* >(_pParent->GetUserData());
    OSL_ENSURE(pData,"SbaTableQueryBrowser::OnExpandEntry: No user data!");
#if OSL_DEBUG_LEVEL > 0
    SvLBoxString* pString = static_cast<SvLBoxString*>(pFirstParent->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING));
    OSL_ENSURE(pString,"SbaTableQueryBrowser::OnExpandEntry: No string item!");
#endif

    if (etTableContainer == pData->eType)
    {
        WaitObject aWaitCursor(getBrowserView());

        
        SharedConnection xConnection;
        ensureConnection( pFirstParent, xConnection );

        if ( xConnection.is() )
        {
            SQLExceptionInfo aInfo;
            try
            {
                Reference< XWarningsSupplier > xWarnings(xConnection, UNO_QUERY);
                if (xWarnings.is())
                    xWarnings->clearWarnings();

                
                
                
                Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
                if(xViewSup.is())
                    populateTree( xViewSup->getViews(), _pParent, etTableOrView );

                Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
                if(xTabSup.is())
                {
                    populateTree( xTabSup->getTables(), _pParent, etTableOrView );
                    Reference<XContainer> xCont(xTabSup->getTables(),UNO_QUERY);
                    if(xCont.is())
                        
                        xCont->addContainerListener(this);
                }

                if (xWarnings.is())
                {
                    SQLExceptionInfo aWarnings(xWarnings->getWarnings());
#if 0





                    if (aWarnings.isValid() && sal_False)
                    {
                        SQLContext aContext;
                        aContext.Message = String(ModuleRes(STR_OPENTABLES_WARNINGS));
                        aContext.Details = String(ModuleRes(STR_OPENTABLES_WARNINGS_DETAILS));
                        aContext.NextException = aWarnings.get();
                        aWarnings = aContext;
                        showError(aWarnings);
                    }
#endif
                    
                    
                    
                }
            }
            catch(const SQLContext& e) { aInfo = e; }
            catch(const SQLWarning& e) { aInfo = e; }
            catch(const SQLException& e) { aInfo = e; }
            catch(const WrappedTargetException& e)
            {
                SQLException aSql;
                if(e.TargetException >>= aSql)
                    aInfo = aSql;
                else
                    SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::OnExpandEntry: something strange happended!");
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            if (aInfo.isValid())
                showError(aInfo);
        }
        else
            return 0L;
                
    }
    else
    {   
        if (ensureEntryObject(_pParent))
        {
            DBTreeListUserData* pParentData = static_cast< DBTreeListUserData* >( _pParent->GetUserData() );
            Reference< XNameAccess > xCollection( pParentData->xContainer, UNO_QUERY );
            populateTree( xCollection, _pParent, etQuery );
        }
    }
    return 1L;
}

sal_Bool SbaTableQueryBrowser::ensureEntryObject( SvTreeListEntry* _pEntry )
{
    OSL_ENSURE(_pEntry, "SbaTableQueryBrowser::ensureEntryObject: invalid argument!");
    if (!_pEntry)
        return sal_False;

    EntryType eType = getEntryType( _pEntry );

    
    DBTreeListUserData* pEntryData = static_cast<DBTreeListUserData*>(_pEntry->GetUserData());
    OSL_ENSURE(pEntryData,"ensureEntryObject: user data should already be set!");

    SvTreeListEntry* pDataSourceEntry = m_pTreeView->getListBox().GetRootLevelParent(_pEntry);

    sal_Bool bSuccess = sal_False;
    switch (eType)
    {
        case etQueryContainer:
            if ( pEntryData->xContainer.is() )
            {
                
                bSuccess = sal_True;
                break;
            }

            {
                SvTreeListEntry* pParent = m_pTreeView->getListBox().GetParent(_pEntry);
                if ( pParent != pDataSourceEntry )
                {
                    SvLBoxString* pString = (SvLBoxString*)_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
                    OSL_ENSURE(pString,"There must be a string item!");
                    OUString aName(pString->GetText());
                    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pParent->GetUserData());
                    try
                    {
                        Reference< XNameAccess > xNameAccess(pData->xContainer,UNO_QUERY);
                        if ( xNameAccess.is() )
                            pEntryData->xContainer.set(xNameAccess->getByName(aName),UNO_QUERY);
                    }
                    catch(const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }

                    bSuccess = pEntryData->xContainer.is();
                }
                else
                {
                    try
                    {
                        Reference< XQueryDefinitionsSupplier > xQuerySup;
                        m_xDatabaseContext->getByName( getDataSourceAcessor( pDataSourceEntry ) ) >>= xQuerySup;
                        if (xQuerySup.is())
                        {
                            Reference< XNameAccess > xQueryDefs = xQuerySup->getQueryDefinitions();
                            Reference< XContainer > xCont(xQueryDefs, UNO_QUERY);
                            if (xCont.is())
                                
                                xCont->addContainerListener(this);

                            pEntryData->xContainer = xQueryDefs;
                            bSuccess = pEntryData->xContainer.is();
                        }
                        else {
                            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::ensureEntryObject: no XQueryDefinitionsSupplier interface!");
                        }
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
            break;

        default:
            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::ensureEntryObject: ooops ... missing some implementation here!");
            
            break;
    }

    return bSuccess;
}

sal_Bool SbaTableQueryBrowser::implSelect(const ::svx::ODataAccessDescriptor& _rDescriptor,sal_Bool _bSelectDirect)
{
    
    OUString sDataSource;
    OUString sCommand;
    sal_Int32 nCommandType = CommandType::COMMAND;
    sal_Bool bEscapeProcessing = sal_True;
    extractDescriptorProps(_rDescriptor, sDataSource, sCommand, nCommandType, bEscapeProcessing);

    
    return implSelect( sDataSource, sCommand, nCommandType, bEscapeProcessing, SharedConnection(), _bSelectDirect );
}

sal_Bool SbaTableQueryBrowser::implLoadAnything(const OUString& _rDataSourceName, const OUString& _rCommand,
    const sal_Int32 _nCommandType, const sal_Bool _bEscapeProcessing, const SharedConnection& _rxConnection)
{
    try
    {
        Reference<XPropertySet> xProp( getRowSet(), UNO_QUERY_THROW );
        Reference< XLoadable >  xLoadable( xProp, UNO_QUERY_THROW );
        
        xProp->setPropertyValue(PROPERTY_DATASOURCENAME, makeAny(_rDataSourceName));
        if(_rxConnection.is())
            xProp->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( _rxConnection.getTyped() ) );

            
        xProp->setPropertyValue(PROPERTY_COMMAND_TYPE, makeAny(_nCommandType));
        xProp->setPropertyValue(PROPERTY_COMMAND, makeAny(_rCommand));
        xProp->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, ::cppu::bool2any(_bEscapeProcessing));
        if ( m_bPreview )
        {
            xProp->setPropertyValue(PROPERTY_FETCHDIRECTION, makeAny(FetchDirection::FORWARD));
        }

        
        initFormatter();

        
        getBrowserView()->getGridControl()->setDesignMode(sal_True);
        InitializeForm( xProp );

        sal_Bool bSuccess = sal_True;

        {
            {
                Reference< XNameContainer >  xColContainer(getFormComponent(), UNO_QUERY);
                
                clearGridColumns(xColContainer);
            }
            FormErrorHelper aHelper(this);
            
            bSuccess = reloadForm(xLoadable);

            
            InitializeGridModel(getFormComponent());

            Any aVal = xProp->getPropertyValue(PROPERTY_ISNEW);
            if (aVal.hasValue() && ::comphelper::getBOOL(aVal))
            {
                
                Reference< XReset> xReset(xProp, UNO_QUERY);
                xReset->reset();
            }

            if ( m_bPreview )
                initializePreviewMode();

            LoadFinished(sal_True);
        }

        InvalidateAll();
        return bSuccess;
    }
    catch( const SQLException& )
    {
        Any aException( ::cppu::getCaughtException() );
        showError( SQLExceptionInfo( aException ) );
    }
    catch( const WrappedTargetException& e )
    {
        SQLException aSql;
        if  ( e.TargetException.isExtractableTo( ::cppu::UnoType< SQLException >::get() ) )
            showError( SQLExceptionInfo( e.TargetException ) );
        else
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    InvalidateAll();
    return sal_False;
}

sal_Bool SbaTableQueryBrowser::implSelect(const OUString& _rDataSourceName, const OUString& _rCommand,
                                      const sal_Int32 _nCommandType, const sal_Bool _bEscapeProcessing,
                                      const SharedConnection& _rxConnection
                                      ,sal_Bool _bSelectDirect)
{
    if (_rDataSourceName.getLength() && _rCommand.getLength() && (-1 != _nCommandType))
    {
        SvTreeListEntry* pDataSource = NULL;
        SvTreeListEntry* pCommandType = NULL;
        SvTreeListEntry* pCommand = getObjectEntry( _rDataSourceName, _rCommand, _nCommandType, &pDataSource, &pCommandType, sal_True, _rxConnection );

        if (pCommand)
        {
            bool bSuccess = true;
            if ( _bSelectDirect )
            {
                bSuccess = implSelect( pCommand );
            }
            else
            {
                m_pTreeView->getListBox().Select( pCommand );
            }

            if ( bSuccess )
            {
                m_pTreeView->getListBox().MakeVisible(pCommand);
                m_pTreeView->getListBox().SetCursor(pCommand);
            }
        }
        else if (!pCommandType)
        {
            if ( m_pCurrentlyDisplayed )
            {   
                selectPath(m_pCurrentlyDisplayed, sal_False);
                m_pCurrentlyDisplayed = NULL;
            }

            
            return implLoadAnything(_rDataSourceName, _rCommand, _nCommandType, _bEscapeProcessing, _rxConnection);
        }
    }
    return sal_False;
}

IMPL_LINK(SbaTableQueryBrowser, OnSelectionChange, void*, /*NOINTERESTEDIN*/)
{
    return implSelect( m_pTreeView->getListBox().FirstSelected() ) ? 1L : 0L;
}

SvTreeListEntry* SbaTableQueryBrowser::implGetConnectionEntry(SvTreeListEntry* _pEntry) const
{
    SvTreeListEntry* pCurrentEntry = _pEntry;
    DBTreeListUserData* pEntryData = static_cast< DBTreeListUserData* >( pCurrentEntry->GetUserData() );
    while(pEntryData->eType != etDatasource )
    {
        pCurrentEntry = m_pTreeModel->GetParent(pCurrentEntry);
        pEntryData = static_cast< DBTreeListUserData* >( pCurrentEntry->GetUserData() );
    }
    return pCurrentEntry;
}

bool SbaTableQueryBrowser::implSelect( SvTreeListEntry* _pEntry )
{
    if ( !_pEntry )
        return false;

    DBTreeListUserData* pEntryData = static_cast< DBTreeListUserData* >( _pEntry->GetUserData() );
    switch (pEntryData->eType)
    {
        case etTableOrView:
        case etQuery:
            break;
        default:
            
            return false;
    }

    OSL_ENSURE(m_pTreeModel->HasParent(_pEntry), "SbaTableQueryBrowser::implSelect: invalid entry (1)!");
    OSL_ENSURE(m_pTreeModel->HasParent(m_pTreeModel->GetParent(_pEntry)), "SbaTableQueryBrowser::implSelect: invalid entry (2)!");

    
    SvTreeListEntry* pContainer = m_pTreeModel->GetParent(_pEntry);
    DBTreeListUserData* pContainerData = static_cast<DBTreeListUserData*>(pContainer->GetUserData());

    
    SvTreeListEntry* pConnection = implGetConnectionEntry(pContainer);
    DBTreeListUserData* pConData = static_cast<DBTreeListUserData*>(pConnection->GetUserData());

    
    
    
    Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
    OUString aOldName;
    xRowSetProps->getPropertyValue(PROPERTY_COMMAND) >>= aOldName;
    sal_Int32 nOldType = 0;
    xRowSetProps->getPropertyValue(PROPERTY_COMMAND_TYPE) >>= nOldType;
    Reference<XConnection> xOldConnection(xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);

    
    SvLBoxString* pString = (SvLBoxString*)_pEntry->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
    OSL_ENSURE(pString,"There must be a string item!");
    const OUString sSimpleName = pString->GetText();
    OUStringBuffer sNameBuffer(sSimpleName);
    if ( etQueryContainer == pContainerData->eType )
    {
        SvTreeListEntry* pTemp = pContainer;
        while( m_pTreeModel->GetParent(pTemp) != pConnection )
        {
            sNameBuffer.insert(0,'/');
            pString = (SvLBoxString*)pTemp->GetFirstItem(SV_ITEM_ID_BOLDLBSTRING);
            OSL_ENSURE(pString,"There must be a string item!");
            sNameBuffer.insert(0,pString->GetText());
            pTemp = m_pTreeModel->GetParent(pTemp);
        }
    }
    OUString aName = sNameBuffer.makeStringAndClear();

    sal_Int32 nCommandType =    ( etTableContainer == pContainerData->eType)
                            ?   CommandType::TABLE
                            :   CommandType::QUERY;

    
    bool bRebuild = ( xOldConnection != pConData->xConnection )
                     || ( nOldType != nCommandType )
                     || ( aName != aOldName );

    Reference< ::com::sun::star::form::XLoadable >  xLoadable = getLoadable();
    bRebuild |= !xLoadable->isLoaded();
    bool bSuccess = true;
    if ( bRebuild )
    {
        try
        {
            WaitObject aWaitCursor(getBrowserView());

            
            selectPath(m_pCurrentlyDisplayed, sal_False);
            m_pCurrentlyDisplayed = NULL;

            
            m_pCurrentlyDisplayed = _pEntry;
            
            selectPath(m_pCurrentlyDisplayed, sal_True);

            
            ensureConnection( m_pCurrentlyDisplayed, pConData->xConnection );

            if ( !pConData->xConnection.is() )
            {
                unloadAndCleanup( sal_False );
                return false;
            }

            Reference<XNameAccess> xNameAccess;
            switch(nCommandType)
            {
                case CommandType::TABLE:
                    {
                        
                        if ( !pContainerData->xContainer.is() )
                        {
                            Reference<XTablesSupplier> xSup( pConData->xConnection, UNO_QUERY );
                            if(xSup.is())
                                xNameAccess = xSup->getTables();

                            pContainerData->xContainer = xNameAccess;
                        }
                        else
                            xNameAccess.set( pContainerData->xContainer, UNO_QUERY );
                    }
                    break;
                case CommandType::QUERY:
                    {
                        if ( pContainerData->xContainer.is() )
                            xNameAccess.set( pContainerData->xContainer, UNO_QUERY );
                        else
                        {
                            Reference<XQueriesSupplier> xSup( pConData->xConnection, UNO_QUERY );
                            if(xSup.is())
                                xNameAccess = xSup->getQueries();
                        }
                    }
                    break;
            }
            OUString sStatus(ModuleRes( CommandType::TABLE == nCommandType ? STR_LOADING_TABLE : STR_LOADING_QUERY ));
            sStatus = sStatus.replaceFirst("$name$", aName);
            BrowserViewStatusDisplay aShowStatus(static_cast<UnoDataBrowserView*>(getView()), sStatus);

            sal_Bool bEscapeProcessing = sal_True;
            if(xNameAccess.is() && xNameAccess->hasByName(sSimpleName))
            {
                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(_pEntry->GetUserData());
                if ( !pData->xObjectProperties.is() )
                {
                    Reference<XInterface> xObject;
                    if(xNameAccess->getByName(sSimpleName) >>= xObject) 
                    {
                        pData->xObjectProperties = pData->xObjectProperties.query( xObject );
                        
                        if ( nCommandType == CommandType::QUERY && xObject.is() )
                        {
                            Reference<XPropertySet> xObjectProps(xObject,UNO_QUERY);
                            xObjectProps->getPropertyValue(PROPERTY_ESCAPE_PROCESSING) >>= bEscapeProcessing;
                            if ( m_bPreview )
                            {
                                OUString sSql;
                                xObjectProps->getPropertyValue(PROPERTY_COMMAND) >>= sSql;
                                Reference< XMultiServiceFactory >  xFactory( pConData->xConnection, UNO_QUERY );
                                if (xFactory.is())
                                {
                                    try
                                    {
                                        Reference<XSingleSelectQueryAnalyzer> xAnalyzer(xFactory->createInstance(SERVICE_NAME_SINGLESELECTQUERYCOMPOSER),UNO_QUERY);
                                        if ( xAnalyzer.is() )
                                        {
                                            xAnalyzer->setQuery(sSql);
                                            Reference<XParametersSupplier> xParSup(xAnalyzer,UNO_QUERY);
                                            if ( xParSup->getParameters()->getCount() > 0 )
                                            {
                                                OUString sFilter = " WHERE ";
                                                sFilter = sFilter + xAnalyzer->getFilter();
                                                OUString sReplace(sSql);
                                                sReplace = sReplace.replaceFirst(sFilter,OUString());
                                                xAnalyzer->setQuery(sReplace);
                                                Reference<XSingleSelectQueryComposer> xComposer(xAnalyzer,UNO_QUERY);
                                                xComposer->setFilter(OUString("0=1"));
                                                aName = xAnalyzer->getQuery();
                                                nCommandType = CommandType::COMMAND;
                                            }
                                        }
                                    }
                                    catch (Exception&)
                                    {
                                        DBG_UNHANDLED_EXCEPTION();
                                    }
                                }
                            }
                        }
                    }
                }
            }

            OUString sDataSourceName( getDataSourceAcessor( pConnection ) );
            bSuccess = implLoadAnything( sDataSourceName, aName, nCommandType, bEscapeProcessing, pConData->xConnection );
            if ( !bSuccess )
            {   
                criticalFail();
            }
        }
        catch(const SQLException& e)
        {
            showError(SQLExceptionInfo(e));
            
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
        }
        catch(WrappedTargetException& e)
        {
            SQLException aSql;
            if(e.TargetException >>= aSql)
                showError(SQLExceptionInfo(aSql));
            else
                SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::implSelect: something strange happended!");
            
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
        }
        catch(const Exception&)
        {
            
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
        }
    }
    return bSuccess;
}

SvTreeListEntry* SbaTableQueryBrowser::getEntryFromContainer(const Reference<XNameAccess>& _rxNameAccess)
{
    DBTreeListBox& rListBox = m_pTreeView->getListBox();
    SvTreeListEntry* pContainer = NULL;
    SvTreeListEntry* pDSLoop = rListBox.FirstChild(NULL);
    while (pDSLoop)
    {
        pContainer  = rListBox.GetEntry(pDSLoop, CONTAINER_QUERIES);
        DBTreeListUserData* pQueriesData = static_cast<DBTreeListUserData*>(pContainer->GetUserData());
        if ( pQueriesData && pQueriesData->xContainer == _rxNameAccess )
            break;

        pContainer  = rListBox.GetEntry(pDSLoop, CONTAINER_TABLES);
        DBTreeListUserData* pTablesData = static_cast<DBTreeListUserData*>(pContainer->GetUserData());
        if ( pTablesData && pTablesData->xContainer == _rxNameAccess )
            break;

        pDSLoop     = rListBox.NextSibling(pDSLoop);
        pContainer  = NULL;
    }
    return pContainer;
}

void SAL_CALL SbaTableQueryBrowser::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    

    SvTreeListEntry* pEntry = getEntryFromContainer(xNames);
    if(pEntry)  
    {
        
        DBTreeListUserData* pContainerData = static_cast<DBTreeListUserData*>(pEntry->GetUserData());
        OSL_ENSURE(pContainerData, "elementInserted: There must be user data for this type!");

        DBTreeListUserData* pNewData = new DBTreeListUserData;
        sal_Bool bIsTable = etTableContainer == pContainerData->eType;
        if ( bIsTable )
        {
            _rEvent.Element >>= pNewData->xObjectProperties;
            pNewData->eType = etTableOrView;
        }
        else
        {
            if ((sal_Int32)m_pTreeView->getListBox().GetChildCount(pEntry) < ( xNames->getElementNames().getLength() - 1 ) )
            {
                
                
                populateTree(xNames, pEntry, etQuery );
            }
            pNewData->eType = etQuery;
        }
        implAppendEntry( pEntry, ::comphelper::getString( _rEvent.Accessor ), pNewData, pNewData->eType );
    }
    else
        SbaXDataBrowserController::elementInserted(_rEvent);
}

sal_Bool SbaTableQueryBrowser::isCurrentlyDisplayedChanged(const OUString& _sName,SvTreeListEntry* _pContainer)
{
    return m_pCurrentlyDisplayed
            &&  getEntryType(m_pCurrentlyDisplayed) == getChildType(_pContainer)
            &&  m_pTreeView->getListBox().GetParent(m_pCurrentlyDisplayed) == _pContainer
            &&  m_pTreeView->getListBox().GetEntryText(m_pCurrentlyDisplayed) == _sName;
}

void SAL_CALL SbaTableQueryBrowser::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    
    
    SvTreeListEntry* pContainer = getEntryFromContainer(xNames);
    if ( pContainer )
    { 
        OUString aName = ::comphelper::getString(_rEvent.Accessor);

        if ( isCurrentlyDisplayedChanged( aName, pContainer) )
        {   

            
            SvTreeListEntry* pTemp = m_pCurrentlyDisplayed;

            
            unloadAndCleanup( sal_False ); 

            DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pTemp->GetUserData());
            pTemp->SetUserData(NULL);
            delete pData;
                
            m_pTreeModel->Remove(pTemp);
        }
        else
        {
            
            SvTreeListEntry* pChild = m_pTreeModel->FirstChild(pContainer);
            while(pChild)
            {
                if (m_pTreeView->getListBox().GetEntryText(pChild) == aName)
                {
                    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pChild->GetUserData());
                    pChild->SetUserData(NULL);
                    delete pData;
                    m_pTreeModel->Remove(pChild);
                    break;
                }
                pChild = m_pTreeModel->NextSibling(pChild);
            }
        }

        
        checkDocumentDataSource();
    }
    else
        SbaXDataBrowserController::elementRemoved(_rEvent);
}

void SAL_CALL SbaTableQueryBrowser::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    SvTreeListEntry* pContainer = getEntryFromContainer(xNames);
    if ( pContainer )
    {    
        OUString aName = ::comphelper::getString(_rEvent.Accessor);

        if ( isCurrentlyDisplayedChanged( aName, pContainer) )
        {   

            
            SvTreeListEntry* pTemp = m_pCurrentlyDisplayed;
            unloadAndCleanup( sal_False ); 

            DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pTemp->GetUserData());
            if (pData)
            {
                if ( etTableOrView == pData->eType )
                { 
                     _rEvent.Element >>= pData->xObjectProperties;  
                }
                else
                {
                    pTemp->SetUserData(NULL);
                    delete pData;
                }
            }
        }
        else
        {
            
            SvTreeListEntry* pChild = m_pTreeModel->FirstChild(pContainer);
            while(pChild)
            {
                if (m_pTreeView->getListBox().GetEntryText(pChild) == aName)
                {
                    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pChild->GetUserData());
                    if (pData)
                    {
                        if ( etTableOrView == pData->eType )
                        { 
                            _rEvent.Element >>= pData->xObjectProperties;   
                        }
                        else
                        {
                            pChild->SetUserData(NULL);
                            delete pData;
                        }
                    }
                    break;
                }
                pChild = m_pTreeModel->NextSibling(pChild);
            }
        }

        
        checkDocumentDataSource();
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::elementReplaced: no support for replaced data sources!");
            
            
    }
    else
        SbaXDataBrowserController::elementReplaced(_rEvent);
}

void SbaTableQueryBrowser::impl_releaseConnection( SharedConnection& _rxConnection )
{
    
    Reference< XComponent > xComponent( _rxConnection, UNO_QUERY );
    if ( xComponent.is() )
    {
        Reference< XEventListener > xListener( static_cast< ::cppu::OWeakObject* >( this ), UNO_QUERY );
        xComponent->removeEventListener( xListener );
    }

    try
    {
        
        Reference< XFlushable > xFlush( _rxConnection, UNO_QUERY );
        if ( xFlush.is() )
            xFlush->flush();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    
    _rxConnection.clear();
        
}

void SbaTableQueryBrowser::disposeConnection( SvTreeListEntry* _pDSEntry )
{
    OSL_ENSURE( _pDSEntry, "SbaTableQueryBrowser::disposeConnection: invalid entry (NULL)!" );
    OSL_ENSURE( impl_isDataSourceEntry( _pDSEntry ), "SbaTableQueryBrowser::disposeConnection: invalid entry (not top-level)!" );

    if ( _pDSEntry )
    {
        DBTreeListUserData* pTreeListData = static_cast< DBTreeListUserData* >( _pDSEntry->GetUserData() );
        if ( pTreeListData )
            impl_releaseConnection( pTreeListData->xConnection );
    }
}

void SbaTableQueryBrowser::closeConnection(SvTreeListEntry* _pDSEntry,sal_Bool _bDisposeConnection)
{
    OSL_ENSURE(_pDSEntry, "SbaTableQueryBrowser::closeConnection: invalid entry (NULL)!");
    OSL_ENSURE( impl_isDataSourceEntry( _pDSEntry ), "SbaTableQueryBrowser::closeConnection: invalid entry (not top-level)!");

    
    if (m_pCurrentlyDisplayed && (m_pTreeView->getListBox().GetRootLevelParent(m_pCurrentlyDisplayed) == _pDSEntry))
        unloadAndCleanup(_bDisposeConnection);

    
    for (SvTreeListEntry* pContainers = m_pTreeModel->FirstChild(_pDSEntry); pContainers; pContainers= m_pTreeModel->NextSibling(pContainers))
    {
        SvTreeListEntry* pElements = m_pTreeModel->FirstChild(pContainers);
        if ( pElements )
            m_pTreeView->getListBox().Collapse(pContainers);
        m_pTreeView->getListBox().EnableExpandHandler(pContainers);
        
        for (; pElements; )
        {
            SvTreeListEntry* pRemove = pElements;
            pElements= m_pTreeModel->NextSibling(pElements);
            DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pRemove->GetUserData());
            pRemove->SetUserData(NULL);
            delete pData;
            m_pTreeModel->Remove(pRemove);
        }
    }
    
    m_pTreeView->getListBox().Collapse(_pDSEntry);

    
    if ( _bDisposeConnection )
        disposeConnection( _pDSEntry );
}

void SbaTableQueryBrowser::unloadAndCleanup( sal_Bool _bDisposeConnection )
{
    if (!m_pCurrentlyDisplayed)
        
        return;

    SvTreeListEntry* pDSEntry = m_pTreeView->getListBox().GetRootLevelParent(m_pCurrentlyDisplayed);

    
    if (m_pCurrentlyDisplayed)
    {
        selectPath(m_pCurrentlyDisplayed, sal_False);
    }
    m_pCurrentlyDisplayed = NULL;

    try
    {
        
        Reference< XPropertySet > xRowSetProps(getRowSet(),UNO_QUERY);
        Reference< XConnection > xConn;
        xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION) >>= xConn;
#if OSL_DEBUG_LEVEL > 1
        {
            Reference< XComponent > xComp;
            ::cppu::extractInterface(xComp, xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION));
        }
#endif

        
        Reference< XLoadable > xLoadable = getLoadable();
        if (xLoadable->isLoaded())
            xLoadable->unload();

        
        Reference< XNameContainer > xConta(getControlModel(),UNO_QUERY);
        clearGridColumns(xConta);

        
        if(_bDisposeConnection)
            disposeConnection( pDSEntry );
    }
    catch(SQLException& e)
    {
        showError(SQLExceptionInfo(e));
    }
    catch(WrappedTargetException& e)
    {
        SQLException aSql;
        if(e.TargetException >>= aSql)
            showError(SQLExceptionInfo(aSql));
        else
            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::unloadAndCleanup: something strange happended!");
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::unloadAndCleanup: could not reset the form");
    }
}

namespace
{
    Reference< XInterface > lcl_getDataSource( const Reference< XDatabaseContext >& _rxDatabaseContext,
        const OUString& _rDataSourceName, const Reference< XConnection >& _rxConnection )
    {
        Reference< XDataSource > xDataSource;
        try
        {
            if ( !_rDataSourceName.isEmpty() && _rxDatabaseContext->hasByName( _rDataSourceName ) )
                xDataSource.set( _rxDatabaseContext->getByName( _rDataSourceName ), UNO_QUERY_THROW );

            if ( !xDataSource.is() )
            {
                Reference< XChild > xConnAsChild( _rxConnection, UNO_QUERY );
                if ( xConnAsChild.is() )
                    xDataSource.set( xConnAsChild->getParent(), UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return xDataSource.get();
    }
}

void SbaTableQueryBrowser::impl_initialize()
{
    SolarMutexGuard aGuard;
        

    
    SbaXDataBrowserController::impl_initialize();

    Reference<XConnection> xForeignConnection;
    Reference< XFrame > xFrame;

    OUString aTableName, aCatalogName, aSchemaName;

    sal_Bool bEsacpeProcessing = sal_True;
    sal_Int32 nInitialDisplayCommandType = CommandType::COMMAND;
    OUString sInitialDataSourceName;
    OUString sInitialCommand;

    const NamedValueCollection& rArguments( getInitParams() );

    rArguments.get_ensureType( (OUString)PROPERTY_DATASOURCENAME, sInitialDataSourceName );
    rArguments.get_ensureType( (OUString)PROPERTY_COMMAND_TYPE, nInitialDisplayCommandType );
    rArguments.get_ensureType( (OUString)PROPERTY_COMMAND, sInitialCommand );
    rArguments.get_ensureType( (OUString)PROPERTY_ACTIVE_CONNECTION, xForeignConnection );
    rArguments.get_ensureType( (OUString)PROPERTY_UPDATE_CATALOGNAME, aCatalogName );
    rArguments.get_ensureType( (OUString)PROPERTY_UPDATE_SCHEMANAME, aSchemaName );
    rArguments.get_ensureType( (OUString)PROPERTY_UPDATE_TABLENAME, aTableName );
    rArguments.get_ensureType( (OUString)PROPERTY_ESCAPE_PROCESSING, bEsacpeProcessing );
    rArguments.get_ensureType( "Frame", xFrame );
    rArguments.get_ensureType( (OUString)PROPERTY_SHOWMENU, m_bShowMenu );

    
    
    sal_Bool bDisableBrowser =  ( sal_False == rArguments.getOrDefault( "ShowTreeViewButton", sal_True ) )   
                            ||  ( sal_False == rArguments.getOrDefault( (OUString)PROPERTY_ENABLE_BROWSER, sal_True ) );
    OSL_ENSURE( !rArguments.has( "ShowTreeViewButton" ),
        "SbaTableQueryBrowser::impl_initialize: ShowTreeViewButton is superseded by EnableBrowser!" );
    m_bEnableBrowser = !bDisableBrowser;

    
    sal_Bool bHideTreeView =    ( !m_bEnableBrowser )
                            ||  ( sal_False == rArguments.getOrDefault( "ShowTreeView", sal_True ) )  
                            ||  ( sal_False == rArguments.getOrDefault( (OUString)PROPERTY_SHOW_BROWSER, sal_True ) );
    OSL_ENSURE( !rArguments.has( "ShowTreeView" ),
        "SbaTableQueryBrowser::impl_initialize: ShowTreeView is superseded by ShowBrowser!" );

    if ( bHideTreeView )
        hideExplorer();
    else
        showExplorer();

    if ( m_bPreview )
    {
        try
        {
            Sequence< OUString> aProperties(5);
            Sequence< Any> aValues(5);

            OUString* pStringIter = aProperties.getArray();
            Any* pValueIter = aValues.getArray();
            *pStringIter++  = "AlwaysShowCursor";
            *pValueIter++   <<= sal_False;
            *pStringIter++  = PROPERTY_BORDER;
            *pValueIter++   <<= sal_Int16(0);

            *pStringIter++  = "HasNavigationBar";
            *pValueIter++   <<= sal_False;
            *pStringIter++  = "HasRecordMarker";
            *pValueIter++   <<= sal_False;

            *pStringIter++  = "Tabstop";
            *pValueIter++   <<= sal_False;

            Reference< XMultiPropertySet >  xFormMultiSet(getFormComponent(), UNO_QUERY);
            if ( xFormMultiSet.is() )
                xFormMultiSet->setPropertyValues(aProperties, aValues);
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    
    
    bool bSubFrameOfEmbeddedDocument = false;
    if ( xFrame.is() )
    {
        Reference<XFramesSupplier> xSup = xFrame->getCreator();
        Reference<XController> xCont = xSup.is() ? xSup->getController() : Reference<XController>();

        bSubFrameOfEmbeddedDocument = xCont.is() && ::dbtools::isEmbeddedInDatabase( xCont->getModel(), xForeignConnection );
    }

    
    
    SharedConnection xConnection( xForeignConnection, SharedConnection::NoTakeOwnership );

    
    
    bool bLimitedTreeEntries = false;
    
    
    bLimitedTreeEntries |= bSubFrameOfEmbeddedDocument;
    
    
    bLimitedTreeEntries |= !m_bEnableBrowser;

    if ( bLimitedTreeEntries )
    {
        if ( xConnection.is() )
        {
            startConnectionListening( xConnection );

            
            if ( sInitialDataSourceName.isEmpty() )
            {
                Reference< XChild > xChild( xConnection, UNO_QUERY );
                Reference< XPropertySet > xDataSourceProperties;
                if ( xChild.is() )
                    xDataSourceProperties = xDataSourceProperties.query( xChild->getParent() );
                if ( xDataSourceProperties.is() )
                {
                    try
                    {
                        OSL_VERIFY( xDataSourceProperties->getPropertyValue( PROPERTY_NAME ) >>= sInitialDataSourceName );
                    }
                    catch( const Exception& )
                    {
                        SAL_WARN("dbaccess.ui",  "SbaTableQueryBrowser::impl_initialize: a connection parent which does not have a 'Name'!??" );
                    }
                }
            }
        }

        implAddDatasource( sInitialDataSourceName, xConnection );
        m_pTreeView->getListBox().Expand( m_pTreeView->getListBox().First() );
    }
    else
        initializeTreeModel();

    if ( m_bEnableBrowser )
    {
        m_aDocScriptSupport = ::boost::optional< bool >( false );
    }
    else
    {
        
        
        Reference< XOfficeDatabaseDocument > xDocument( getDataSourceOrModel(
            lcl_getDataSource( m_xDatabaseContext, sInitialDataSourceName, xConnection ) ), UNO_QUERY );
        m_aDocScriptSupport = ::boost::optional< bool >( Reference< XEmbeddedScripts >( xDocument, UNO_QUERY ).is() );
    }

    if ( implSelect( sInitialDataSourceName, sInitialCommand, nInitialDisplayCommandType, bEsacpeProcessing, xConnection, sal_True ) )
    {
        try
        {
            Reference< XPropertySet > xRowSetProps(getRowSet(), UNO_QUERY);
            xRowSetProps->setPropertyValue(PROPERTY_UPDATE_CATALOGNAME,makeAny(aCatalogName));
            xRowSetProps->setPropertyValue(PROPERTY_UPDATE_SCHEMANAME,makeAny(aSchemaName));
            xRowSetProps->setPropertyValue(PROPERTY_UPDATE_TABLENAME,makeAny(aTableName));

        }
        catch(const Exception&)
        {
            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::impl_initialize: could not set the update related names!");
        }
    }

    InvalidateAll();
}

sal_Bool SbaTableQueryBrowser::haveExplorer() const
{
    return m_pTreeView && m_pTreeView->IsVisible();
}

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

sal_Bool SbaTableQueryBrowser::ensureConnection(SvTreeListEntry* _pAnyEntry, SharedConnection& _rConnection)
{
    SvTreeListEntry* pDSEntry = m_pTreeView->getListBox().GetRootLevelParent(_pAnyEntry);
    DBTreeListUserData* pDSData =
                pDSEntry
            ?   static_cast<DBTreeListUserData*>(pDSEntry->GetUserData())
            :   NULL;

    return ensureConnection( pDSEntry, pDSData, _rConnection );
}

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr< ImageProvider > SbaTableQueryBrowser::getImageProviderFor( SvTreeListEntry* _pAnyEntry )
{
    ::std::auto_ptr< ImageProvider > pImageProvider( new ImageProvider );
    SharedConnection xConnection;
    if ( getExistentConnectionFor( _pAnyEntry, xConnection ) )
        pImageProvider.reset( new ImageProvider( xConnection ) );
    return pImageProvider;
}

SAL_WNODEPRECATED_DECLARATIONS_POP

sal_Bool SbaTableQueryBrowser::getExistentConnectionFor( SvTreeListEntry* _pAnyEntry, SharedConnection& _rConnection )
{
    SvTreeListEntry* pDSEntry = m_pTreeView->getListBox().GetRootLevelParent( _pAnyEntry );
    DBTreeListUserData* pDSData =
                pDSEntry
            ?   static_cast< DBTreeListUserData* >( pDSEntry->GetUserData() )
            :   NULL;
    if ( pDSData )
        _rConnection = pDSData->xConnection;
    return _rConnection.is();
}

#if OSL_DEBUG_LEVEL > 0
bool SbaTableQueryBrowser::impl_isDataSourceEntry( SvTreeListEntry* _pEntry ) const
{
    return m_pTreeModel->GetRootLevelParent( _pEntry ) == _pEntry;
}

#endif

sal_Bool SbaTableQueryBrowser::ensureConnection( SvTreeListEntry* _pDSEntry, void* pDSData, SharedConnection& _rConnection )
{
    OSL_ENSURE( impl_isDataSourceEntry( _pDSEntry ), "SbaTableQueryBrowser::ensureConnection: this entry does not denote a data source!" );
    if(_pDSEntry)
    {
        DBTreeListUserData* pTreeListData = static_cast<DBTreeListUserData*>(pDSData);
        OUString aDSName = GetEntryText(_pDSEntry);

        if ( pTreeListData )
            _rConnection = pTreeListData->xConnection;

        if ( !_rConnection.is() && pTreeListData )
        {
            
            OUString sConnecting(ModuleRes(STR_CONNECTING_DATASOURCE));
            sConnecting = sConnecting.replaceFirst("$name$", aDSName);
            BrowserViewStatusDisplay aShowStatus(static_cast<UnoDataBrowserView*>(getView()), sConnecting);

            
            OUString sConnectingContext( ModuleRes( STR_COULDNOTCONNECT_DATASOURCE ) );
            sConnectingContext = sConnectingContext.replaceFirst("$name$", aDSName);

            
            _rConnection.reset(
                connect( getDataSourceAcessor( _pDSEntry ), sConnectingContext, NULL ),
                SharedConnection::TakeOwnership
            );

            
            pTreeListData->xConnection = _rConnection;
        }
    }

    return _rConnection.is();
}

IMPL_LINK( SbaTableQueryBrowser, OnTreeEntryCompare, const SvSortData*, _pSortData )
{
    const SvTreeListEntry* pLHS = static_cast<const SvTreeListEntry*>(_pSortData->pLeft);
    const SvTreeListEntry* pRHS = static_cast<const SvTreeListEntry*>(_pSortData->pRight);
    OSL_ENSURE(pLHS && pRHS, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid tree entries!");
    

    if (isContainer(pRHS))
    {
        
        

        const EntryType eRight = getEntryType(pRHS);
        if (etTableContainer == eRight)
            
            return -1;

        const OUString sLeft = m_pTreeView->getListBox().GetEntryText(const_cast<SvTreeListEntry*>(pLHS));

        EntryType eLeft = etTableContainer;
        if (OUString(ModuleRes(RID_STR_TABLES_CONTAINER)) == sLeft)
            eLeft = etTableContainer;
        else if (OUString(ModuleRes(RID_STR_QUERIES_CONTAINER)) == sLeft)
            eLeft = etQueryContainer;

        if ( eLeft == eRight )
            return 0;

        if ( ( eLeft == etTableContainer ) && ( eRight == etQueryContainer ) )
            return 1;

        if ( ( eLeft == etQueryContainer ) && ( eRight == etTableContainer ) )
            return -1;

        SAL_WARN("dbaccess.ui",  "SbaTableQueryBrowser::OnTreeEntryCompare: unexpected case!" );
        return 0;
    }

    const SvLBoxString* pLeftTextItem = static_cast<const SvLBoxString*>(pLHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    const SvLBoxString* pRightTextItem = static_cast<const SvLBoxString*>(pRHS->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    OSL_ENSURE(pLeftTextItem && pRightTextItem, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid text items!");

    OUString sLeftText = pLeftTextItem->GetText();
    OUString sRightText = pRightTextItem->GetText();

    sal_Int32 nCompareResult = 0;   

    if (m_xCollator.is())
    {
        try
        {
            nCompareResult = m_xCollator->compareString(sLeftText, sRightText);
        }
        catch(const Exception&)
        {
        }
    }
    else
        
        nCompareResult = sLeftText.compareTo(sRightText);

    return nCompareResult;
}

void SbaTableQueryBrowser::implAdministrate( SvTreeListEntry* _pApplyTo )
{
    OSL_PRECOND( _pApplyTo, "SbaTableQueryBrowser::implAdministrate: illegal entry!" );
    if ( !_pApplyTo )
        return;

    try
    {
        
        sal_Int32 nFrameSearchFlag = FrameSearchFlag::ALL | FrameSearchFlag::GLOBAL ;
        Reference< XDesktop2 > xFrameLoader = Desktop::create( getORB() );

        
        SvTreeListEntry* pTopLevelSelected = _pApplyTo;
        while (pTopLevelSelected && m_pTreeView->getListBox().GetParent(pTopLevelSelected))
            pTopLevelSelected = m_pTreeView->getListBox().GetParent(pTopLevelSelected);
        OUString sInitialSelection;
        if (pTopLevelSelected)
            sInitialSelection = getDataSourceAcessor( pTopLevelSelected );

        Reference< XDataSource > xDataSource( getDataSourceByName( sInitialSelection, getView(), getORB(), NULL ) );
        Reference< XModel > xDocumentModel( getDataSourceOrModel( xDataSource ), UNO_QUERY );

        if ( xDocumentModel.is() )
        {
            Reference< XInteractionHandler2 > xInteractionHandler(
                InteractionHandler::createWithParent(getORB(), 0) );

            ::comphelper::NamedValueCollection aLoadArgs;
            aLoadArgs.put( "Model", xDocumentModel );
            aLoadArgs.put( "InteractionHandler", xInteractionHandler );
            aLoadArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );

            Sequence< PropertyValue > aLoadArgPV;
            aLoadArgs >>= aLoadArgPV;

            xFrameLoader->loadComponentFromURL(
                xDocumentModel->getURL(),
                OUString("_default"),
                nFrameSearchFlag,
                aLoadArgPV
            );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

sal_Bool SbaTableQueryBrowser::requestQuickHelp( const SvTreeListEntry* _pEntry, OUString& _rText ) const
{
    const DBTreeListUserData* pData = static_cast< const DBTreeListUserData* >( _pEntry->GetUserData() );
    if ( ( pData->eType == etDatasource ) && !pData->sAccessor.isEmpty() )
    {
        _rText = ::svt::OFileNotation( pData->sAccessor ).get( ::svt::OFileNotation::N_SYSTEM );
        return sal_True;
    }
    return sal_False;
}

PopupMenu* SbaTableQueryBrowser::getContextMenu( Control& _rControl ) const
{
    OSL_PRECOND( &m_pTreeView->getListBox() == &_rControl,
        "SbaTableQueryBrowser::getContextMenu: where does this come from?" );
    if ( &m_pTreeView->getListBox() != &_rControl )
        return NULL;

    return new PopupMenu( ModuleRes( MENU_BROWSER_DEFAULTCONTEXT ) );
}

IController& SbaTableQueryBrowser::getCommandController()
{
    return *this;
}

::cppu::OInterfaceContainerHelper* SbaTableQueryBrowser::getContextMenuInterceptors()
{
    return &m_aContextMenuInterceptors;
}

Any SbaTableQueryBrowser::getCurrentSelection( Control& _rControl ) const
{
    OSL_PRECOND( &m_pTreeView->getListBox() == &_rControl,
        "SbaTableQueryBrowser::getCurrentSelection: where does this come from?" );

    if ( &m_pTreeView->getListBox() != &_rControl )
        return Any();

    SvTreeListEntry* pSelected = m_pTreeView->getListBox().FirstSelected();
    if ( !pSelected )
        return Any();

    OSL_ENSURE( m_pTreeView->getListBox().NextSelected( pSelected ) == NULL,
        "SbaTableQueryBrowser::getCurrentSelection: single-selection is expected here!" );

    NamedDatabaseObject aSelectedObject;
    DBTreeListUserData* pData = static_cast< DBTreeListUserData* >( pSelected->GetUserData() );
    aSelectedObject.Type = static_cast< sal_Int32 >( pData->eType );

    switch ( aSelectedObject.Type )
    {
    case DatabaseObject::QUERY:
    case DatabaseObject::TABLE:
        aSelectedObject.Name = m_pTreeView->getListBox().GetEntryText( pSelected );
        break;

    case DatabaseObjectContainer::DATA_SOURCE:
    case DatabaseObjectContainer::QUERIES:
    case DatabaseObjectContainer::TABLES:
        aSelectedObject.Name = getDataSourceAcessor( pSelected );
        break;

    default:
        SAL_WARN("dbaccess.ui",  "SbaTableQueryBrowser::getCurrentSelection: invalid (unexpected) object type!" );
        break;
    }

    return makeAny( aSelectedObject );
}

sal_Bool SbaTableQueryBrowser::implGetQuerySignature( OUString& _rCommand, sal_Bool& _bEscapeProcessing )
{
    _rCommand = "";
    _bEscapeProcessing = sal_False;

    try
    {
        
        OUString sDataSourceName;
        OUString sCommand;
        sal_Int32       nCommandType = CommandType::COMMAND;
        Reference< XPropertySet > xRowsetProps( getRowSet(), UNO_QUERY );
        ODataAccessDescriptor aDesc( xRowsetProps );
        sDataSourceName = aDesc.getDataSource();
        aDesc[ daCommand ]      >>= sCommand;
        aDesc[ daCommandType ]  >>= nCommandType;

        
        if ( CommandType::QUERY != nCommandType )
            return sal_False;

        
        Reference< XQueryDefinitionsSupplier > xSuppQueries;
        Reference< XNameAccess > xQueries;
        Reference< XPropertySet > xQuery;
        m_xDatabaseContext->getByName( sDataSourceName ) >>= xSuppQueries;
        if ( xSuppQueries.is() )
            xQueries = xSuppQueries->getQueryDefinitions();
        if ( xQueries.is() )
            xQueries->getByName( sCommand ) >>= xQuery;
        OSL_ENSURE( xQuery.is(), "SbaTableQueryBrowser::implGetQuerySignature: could not retrieve the query object!" );

        
        if ( xQuery.is() )
        {
            xQuery->getPropertyValue( PROPERTY_COMMAND ) >>= _rCommand;
            _bEscapeProcessing = ::cppu::any2bool( xQuery->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) );
            return sal_True;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return sal_False;
}

void SbaTableQueryBrowser::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( RuntimeException )
{
    if (aEvent.Frame == m_xCurrentFrameParent)
    {
        if(aEvent.Action == FrameAction_COMPONENT_DETACHING)
            implRemoveStatusListeners();
        else if (aEvent.Action == FrameAction_COMPONENT_REATTACHED)
            connectExternalDispatches();
    }
    else
        SbaXDataBrowserController::frameAction(aEvent);

}

void SbaTableQueryBrowser::clearGridColumns(const Reference< XNameContainer >& _xColContainer)
{
    
    Sequence< OUString > aNames = _xColContainer->getElementNames();
    const OUString* pIter    = aNames.getConstArray();
    const OUString* pEnd     = pIter + aNames.getLength();
    Reference< XInterface > xColumn;
    for (; pIter != pEnd;++pIter)
    {
        _xColContainer->getByName(*pIter) >>= xColumn;
        _xColContainer->removeByName(*pIter);
        ::comphelper::disposeComponent(xColumn);
    }
}

void SbaTableQueryBrowser::loadMenu(const Reference< XFrame >& _xFrame)
{
    if ( m_bShowMenu )
    {
        OGenericUnoController::loadMenu(_xFrame);
    }
    else if ( !m_bPreview )
    {
        Reference< ::com::sun::star::frame::XLayoutManager > xLayoutManager = getLayoutManager(_xFrame);

        if ( xLayoutManager.is() )
        {
            xLayoutManager->lock();
            xLayoutManager->createElement( OUString( "private:resource/toolbar/toolbar" ));
            xLayoutManager->unlock();
            xLayoutManager->doLayout();
        }
        onLoadedMenu( xLayoutManager );
    }
}

OUString SbaTableQueryBrowser::getPrivateTitle() const
{
    OUString sTitle;
    if ( m_pCurrentlyDisplayed )
    {
        SvTreeListEntry* pContainer = m_pTreeModel->GetParent(m_pCurrentlyDisplayed);
        
        SvTreeListEntry* pConnection = implGetConnectionEntry(pContainer);
        OUString sName = m_pTreeView->getListBox().GetEntryText(m_pCurrentlyDisplayed);
        sTitle = GetEntryText( pConnection );
        INetURLObject aURL(sTitle);
        if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
            sTitle = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DECODE_WITH_CHARSET);
        if ( !sName.isEmpty() )
        {
            sName += " - ";
            sName += sTitle;
            sTitle = sName;
        }
    }

    return sTitle;
}

sal_Bool SbaTableQueryBrowser::preReloadForm()
{
    sal_Bool bIni = sal_False;
    if ( !m_pCurrentlyDisplayed )
    {
        
        getBrowserView()->getGridControl()->setDesignMode(sal_True);
        
        Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
        ::svx::ODataAccessDescriptor aDesc(xRowSetProps);
        
        OUString sDataSource;
        OUString sCommand;
        sal_Int32 nCommandType = CommandType::COMMAND;
        sal_Bool bEscapeProcessing = sal_True;
        extractDescriptorProps(aDesc, sDataSource, sCommand, nCommandType, bEscapeProcessing);
        if ( !sDataSource.isEmpty() && !sCommand.isEmpty() && (-1 != nCommandType) )
        {
            SvTreeListEntry* pDataSource = NULL;
            SvTreeListEntry* pCommandType = NULL;
            m_pCurrentlyDisplayed = getObjectEntry( sDataSource, sCommand, nCommandType, &pDataSource, &pCommandType, sal_True, SharedConnection() );
            bIni = sal_True;
        }
    }
    return bIni;
}

void SbaTableQueryBrowser::postReloadForm()
{
    InitializeGridModel(getFormComponent());
    LoadFinished(sal_True);
}

Reference< XEmbeddedScripts > SAL_CALL SbaTableQueryBrowser::getScriptContainer() throw (RuntimeException)
{
    
    Reference< XModel > xDocument;
    try
    {
        Reference< XPropertySet > xCursorProps( getRowSet(), UNO_QUERY_THROW );
        Reference< XConnection > xConnection( xCursorProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ), UNO_QUERY );
        if ( xConnection.is() )
        {
            Reference< XChild > xChild( xConnection, UNO_QUERY_THROW );
            Reference< XDocumentDataSource > xDataSource( xChild->getParent(), UNO_QUERY_THROW );
            xDocument.set( xDataSource->getDatabaseDocument(), UNO_QUERY_THROW );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    Reference< XEmbeddedScripts > xScripts( xDocument, UNO_QUERY );
    OSL_ENSURE( xScripts.is() || !xDocument.is(),
        "SbaTableQueryBrowser::getScriptContainer: invalid database document!" );
    return xScripts;
}

void SAL_CALL SbaTableQueryBrowser::registerContextMenuInterceptor( const Reference< XContextMenuInterceptor >& _Interceptor ) throw (RuntimeException)
{
    if ( _Interceptor.is() )
        m_aContextMenuInterceptors.addInterface( _Interceptor );
}

void SAL_CALL SbaTableQueryBrowser::releaseContextMenuInterceptor( const Reference< XContextMenuInterceptor >& _Interceptor ) throw (RuntimeException)
{
    if ( _Interceptor.is() )
        m_aContextMenuInterceptors.removeInterface( _Interceptor );
}

void SAL_CALL SbaTableQueryBrowser::registeredDatabaseLocation( const DatabaseRegistrationEvent& _Event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    implAddDatasource( _Event.Name, SharedConnection() );
}

void SbaTableQueryBrowser::impl_cleanupDataSourceEntry( const OUString& _rDataSourceName )
{
    
    SvTreeListEntry* pDataSourceEntry = m_pTreeView->getListBox().FirstChild( NULL );
    while ( pDataSourceEntry )
    {
        if ( m_pTreeView->getListBox().GetEntryText( pDataSourceEntry ) == _rDataSourceName )
            break;

        pDataSourceEntry = m_pTreeView->getListBox().NextSibling( pDataSourceEntry );
    }

    OSL_ENSURE( pDataSourceEntry, "SbaTableQueryBrowser::impl_cleanupDataSourceEntry: do not know this data source!" );
    if ( !pDataSourceEntry )
        return;

    if ( isSelected( pDataSourceEntry ) )
    {   
        OSL_ENSURE( m_pTreeView->getListBox().GetRootLevelParent( m_pCurrentlyDisplayed ) == pDataSourceEntry,
            "SbaTableQueryBrowser::impl_cleanupDataSourceEntry: inconsistence (1)!" );
        unloadAndCleanup( sal_True );
    }
    else
        OSL_ENSURE(
                ( NULL == m_pCurrentlyDisplayed )
            ||  ( m_pTreeView->getListBox().GetRootLevelParent( m_pCurrentlyDisplayed ) != pDataSourceEntry ),
            "SbaTableQueryBrowser::impl_cleanupDataSourceEntry: inconsistence (2)!");

    
    std::pair<SvTreeListEntries::iterator, SvTreeListEntries::iterator> aIters =
        m_pTreeModel->GetChildIterators(pDataSourceEntry);

    SvTreeListEntries::iterator it = aIters.first, itEnd = aIters.second;

    for (; it != itEnd; ++it)
    {
        SvTreeListEntry* pEntry = &(*it);
        const DBTreeListUserData* pData = static_cast<const DBTreeListUserData*>(pEntry->GetUserData());
        pEntry->SetUserData(NULL);
        delete pData;
    }

    
    DBTreeListUserData* pData = static_cast< DBTreeListUserData* >( pDataSourceEntry->GetUserData() );
    pDataSourceEntry->SetUserData( NULL );
    delete pData;
    m_pTreeModel->Remove( pDataSourceEntry );
}

void SAL_CALL SbaTableQueryBrowser::revokedDatabaseLocation( const DatabaseRegistrationEvent& _Event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    impl_cleanupDataSourceEntry( _Event.Name );

    
    checkDocumentDataSource();
}

void SAL_CALL SbaTableQueryBrowser::changedDatabaseLocation( const DatabaseRegistrationEvent& _Event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    
    
    impl_cleanupDataSourceEntry( _Event.Name );
    implAddDatasource( _Event.Name, SharedConnection() );
}

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

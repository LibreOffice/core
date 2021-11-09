/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <browserids.hxx>
#include <core_resource.hxx>
#include <helpids.h>
#include <dbtreelistbox.hxx>
#include "dbtreemodel.hxx"
#include <strings.hrc>
#include <imageprovider.hxx>
#include <sbagrid.hxx>
#include <strings.hxx>
#include <UITools.hxx>
#include <unodatbr.hxx>

#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/Collator.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XDatabaseContext.hpp>
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
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>

#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <svl/filenotation.hxx>
#include <svx/dataaccessdescriptor.hxx>
#include <svx/databaseregistrationui.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/multisel.hxx>
#include <tools/urlobj.hxx>
#include <unotools/confignode.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>

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

// SbaTableQueryBrowser
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_dbu_ODatasourceBrowser_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    SolarMutexGuard aGuard;
    return cppu::acquire(new ::dbaui::SbaTableQueryBrowser(context));
}

namespace dbaui
{

namespace DatabaseObject = css::sdb::application::DatabaseObject;
namespace DatabaseObjectContainer = css::sdb::application::DatabaseObjectContainer;

static void SafeAddPropertyListener(const Reference< XPropertySet > & xSet, const OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->addPropertyChangeListener(rPropName, pListener);
}

static void SafeRemovePropertyListener(const Reference< XPropertySet > & xSet, const OUString& rPropName, XPropertyChangeListener* pListener)
{
    Reference< XPropertySetInfo >  xInfo = xSet->getPropertySetInfo();
    if (xInfo->hasPropertyByName(rPropName))
        xSet->removePropertyChangeListener(rPropName, pListener);
}

OUString SAL_CALL SbaTableQueryBrowser::getImplementationName()
{
    return "org.openoffice.comp.dbu.ODatasourceBrowser";
}

css::uno::Sequence<OUString> SAL_CALL SbaTableQueryBrowser::getSupportedServiceNames()
{
    return { "com.sun.star.sdb.DataSourceBrowser" };
}

SbaTableQueryBrowser::SbaTableQueryBrowser(const Reference< XComponentContext >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_aSelectionListeners( getMutex() )
    ,m_aContextMenuInterceptors( getMutex() )
    ,m_aTableCopyHelper(this)
    ,m_pTreeView(nullptr)
    ,m_pSplitter(nullptr)
    ,m_nAsyncDrop(nullptr)
    ,m_bQueryEscapeProcessing( false )
    ,m_bShowMenu(false)
    ,m_bInSuspend(false)
    ,m_bEnableBrowser(true)
{
}

SbaTableQueryBrowser::~SbaTableQueryBrowser()
{
    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        SAL_WARN("dbaccess.ui", "Please check who doesn't dispose this component!");
        // increment ref count to prevent double call of Dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
    SolarMutexGuard g;
    m_pTreeView.reset();
    m_pSplitter.reset();
}

Any SAL_CALL SbaTableQueryBrowser::queryInterface(const Type& _rType)
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

Sequence< Type > SAL_CALL SbaTableQueryBrowser::getTypes(  )
{
    Sequence< Type > aTypes( ::comphelper::concatSequences(
        SbaXDataBrowserController::getTypes(),
        SbaTableQueryBrowser_Base::getTypes()
    ) );

    OSL_PRECOND( !!m_aDocScriptSupport, "SbaTableQueryBrowser::getTypes: did not initialize this, yet!" );
    if ( !m_aDocScriptSupport || !*m_aDocScriptSupport )
    {
        auto [begin, end] = asNonConstRange(aTypes);
        auto newEnd = std::remove_if( begin, end,
                                      [](const Type& type)
                                      { return type == cppu::UnoType<XScriptInvocationContext>::get(); } );
        aTypes.realloc( std::distance(begin, newEnd) );
    }
    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL SbaTableQueryBrowser::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

void SAL_CALL SbaTableQueryBrowser::disposing()
{
    SolarMutexGuard aGuard;
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    // kiss our listeners goodbye
    css::lang::EventObject aEvt(*this);
    m_aSelectionListeners.disposeAndClear(aEvt);
    m_aContextMenuInterceptors.disposeAndClear(aEvt);

    if (getBrowserView())
    {
        // Need to do some cleanup of the data pointed to the tree view entries before we remove the treeview
        clearTreeModel();
        m_pTreeView = nullptr;
        getBrowserView()->setTreeView(nullptr);
    }

    // remove ourself as status listener
    implRemoveStatusListeners();

    // check out from all the objects we are listening
    // the frame
    if (m_xCurrentFrameParent.is())
        m_xCurrentFrameParent->removeFrameActionListener(static_cast<css::frame::XFrameActionListener*>(this));

    // remove the container listener from the database context
    try
    {
        Reference< XDatabaseRegistrations > xDatabaseRegistrations( m_xDatabaseContext, UNO_QUERY_THROW );
        xDatabaseRegistrations->removeDatabaseRegistrationsListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    SbaXDataBrowserController::disposing();
}

bool SbaTableQueryBrowser::Construct(vcl::Window* pParent)
{
    if ( !SbaXDataBrowserController::Construct( pParent ) )
        return false;

    try
    {
        Reference< XDatabaseRegistrations > xDatabaseRegistrations( m_xDatabaseContext, UNO_QUERY_THROW );
        xDatabaseRegistrations->addDatabaseRegistrationsListener( this );

        // the collator for the string compares
        m_xCollator = Collator::create( getORB() );
        m_xCollator->loadDefaultCollator( Application::GetSettings().GetLanguageTag().getLocale(), 0 );
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::Construct: could not create (or start listening at) the database context!");
    }
    // some help ids
    if (getBrowserView() && getBrowserView()->getVclControl())
    {

        // create controls and set sizes
        const tools::Long  nFrameWidth = getBrowserView()->LogicToPixel(::Size(3, 0), MapMode(MapUnit::MapAppFont)).Width();

        m_pSplitter = VclPtr<Splitter>::Create(getBrowserView(),WB_HSCROLL);
        m_pSplitter->SetPosSizePixel( ::Point(0,0), ::Size(nFrameWidth,0) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );

        m_pTreeView = VclPtr<InterimDBTreeListBox>::Create(getBrowserView());

        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        rTreeView.connect_expanding(LINK(this, SbaTableQueryBrowser, OnExpandEntry));

        m_pTreeView->setCopyHandler(LINK(this, SbaTableQueryBrowser, OnCopyEntry));

        m_pTreeView->setContextMenuProvider( this );
        m_pTreeView->setControlActionListener( this );
        m_pTreeView->SetHelpId(HID_CTL_TREEVIEW);

        // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
        m_pSplitter->SetSplitPosPixel(getBrowserView()->LogicToPixel(::Size(80, 0), MapMode(MapUnit::MapAppFont)).Width());

        getBrowserView()->setSplitter(m_pSplitter);
        getBrowserView()->setTreeView(m_pTreeView);

        // fill view with data
        rTreeView.set_sort_order(true);
        rTreeView.set_sort_func([this](const weld::TreeIter& rLeft, const weld::TreeIter& rRight){
            return OnTreeEntryCompare(rLeft, rRight);
        });
        rTreeView.make_sorted();
        m_pTreeView->SetSelChangeHdl(LINK(this, SbaTableQueryBrowser, OnSelectionChange));
        m_pTreeView->show_container();

        // TODO
        getBrowserView()->getVclControl()->SetHelpId(HID_CTL_TABBROWSER);
        if (getBrowserView()->getVclControl()->GetHeaderBar())
            getBrowserView()->getVclControl()->GetHeaderBar()->SetHelpId(HID_DATABROWSE_HEADER);
        InvalidateFeature(ID_BROWSER_EXPLORER);
    }

    return true;
}

namespace
{
    struct SelectValueByName
    {
        const Any& operator()( OUString const& i_name ) const
        {
            return m_rCollection.get( i_name );
        }

        explicit SelectValueByName( ::comphelper::NamedValueCollection const& i_collection )
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
        bool bEscapeProcessing = false;
        OSL_VERIFY( xRowSetProps->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) >>= bEscapeProcessing );
        if ( !bEscapeProcessing )
            // don't touch or interpret anything if escape processing is disabled
            return;

        Reference< XSingleSelectQueryComposer > xComposer( createParser_nothrow() );
        if ( !xComposer.is() )
            // can't do anything. Already reported via assertion in createParser_nothrow.
            return;

        // the tables participating in the statement
        const Reference< XTablesSupplier > xSuppTables( xComposer, UNO_QUERY_THROW );
        const Reference< XNameAccess > xTableNames( xSuppTables->getTables(), UNO_SET_THROW );

        // the columns participating in the statement
        const Reference< XColumnsSupplier > xSuppColumns( xComposer, UNO_QUERY_THROW );
        const Reference< XNameAccess > xColumnNames( xSuppColumns->getColumns(), UNO_SET_THROW );

        // check if the order columns apply to tables which really exist in the statement
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
                const Reference< XNameAccess > xTableColumnNames( xSuppTableColumns->getColumns(), UNO_SET_THROW );
                if ( !xTableColumnNames->hasByName( sColumnName ) )
                {
                    invalidColumn = true;
                    break;
                }
            }
        }

        if ( invalidColumn )
        {
            // reset the complete order statement at both the row set and the parser
            xRowSetProps->setPropertyValue( PROPERTY_ORDER, makeAny( OUString() ) );
            xComposer->setOrder( "" );
        }

        // check if the columns participating in the filter refer to existing tables
        // TODO: there's no API at all for this. The method which comes nearest to what we need is
        // "getStructuredFilter", but it returns pure column names only. That is, for a statement like
        // "SELECT * FROM <table> WHERE <other_table>.<column> = <value>", it will return "<column>". But
        // there's no API at all to retrieve the information about  "<other_table>" - which is what would
        // be needed here.
        // That'd be a chance to replace getStructuredFilter with something more reasonable.
        // So, what really would be handy, is some
        //   XNormalizedFilter getNormalizedFilter();
        // with
        //   interface XDisjunctiveFilterExpression
        //   {
        //     XConjunctiveFilterTerm getTerm( int index );
        //   }
        //   interface XConjunctiveFilterTerm
        //   {
        //     ComparisonPredicate getPredicate( int index );
        //   }
        //   struct ComparisonPredicate
        //   {
        //     XComparisonOperand   Lhs;
        //     SQLFilterOperator    Operator;
        //     XComparisonOperand   Rhs;
        //   }
        //   interface XComparisonOperand
        //   {
        //     SQLFilterOperand Type;
        //     XPropertySet     getColumn();
        //     string           getLiteral();
        //     ...
        //   }
        //   enum SQLFilterOperand { Column, Literal, ... }
        // ... or something like this...
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

bool SbaTableQueryBrowser::InitializeForm( const Reference< XPropertySet > & i_formProperties )
{
    if (!m_xCurrentlyDisplayed)
        return true;

    // this method set all format settings from the original table or query
    try
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*m_xCurrentlyDisplayed).toUInt64());
        ENSURE_OR_RETURN_FALSE( pData, "SbaTableQueryBrowser::InitializeForm: No user data set at the currently displayed entry!" );
        ENSURE_OR_RETURN_FALSE( pData->xObjectProperties.is(), "SbaTableQueryBrowser::InitializeForm: No table available!" );

        Reference< XPropertySetInfo > xPSI( pData->xObjectProperties->getPropertySetInfo(), UNO_SET_THROW );

        ::comphelper::NamedValueCollection aPropertyValues;

        static constexpr rtl::OUStringConstExpr aTransferProperties[]
        {
            PROPERTY_APPLYFILTER,
            PROPERTY_FILTER,
            PROPERTY_HAVING_CLAUSE,
            PROPERTY_ORDER
        };
        for (const auto & aTransferProperty : aTransferProperties)
        {
            if ( !xPSI->hasPropertyByName( aTransferProperty ) )
                continue;
            aPropertyValues.put( aTransferProperty, pData->xObjectProperties->getPropertyValue( aTransferProperty ) );
        }

        std::vector< OUString > aNames( aPropertyValues.getNames() );
        std::sort(aNames.begin(), aNames.end());
        Sequence< OUString > aPropNames( comphelper::containerToSequence(aNames) );

        Sequence< Any > aPropValues( aNames.size() );
        std::transform( aNames.begin(), aNames.end(), aPropValues.getArray(), SelectValueByName( aPropertyValues ) );

        Reference< XMultiPropertySet > xFormMultiSet( i_formProperties, UNO_QUERY_THROW );
        xFormMultiSet->setPropertyValues( aPropNames, aPropValues );

        impl_sanitizeRowSetClauses_nothrow();
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
        return false;
    }

    return true;
}

void SbaTableQueryBrowser::initializePreviewMode()
{
    if ( getBrowserView() && getBrowserView()->getVclControl() )
    {
        getBrowserView()->getVclControl()->AlwaysEnableInput( false );
        getBrowserView()->getVclControl()->EnableInput( false );
        getBrowserView()->getVclControl()->ForceHideScrollbars();
    }
    Reference< XPropertySet >  xDataSourceSet(getRowSet(), UNO_QUERY);
    if ( xDataSourceSet.is() )
    {
        xDataSourceSet->setPropertyValue("AllowInserts",makeAny(false));
        xDataSourceSet->setPropertyValue("AllowUpdates",makeAny(false));
        xDataSourceSet->setPropertyValue("AllowDeletes",makeAny(false));
    }
}

void SbaTableQueryBrowser::InitializeGridModel(const Reference< css::form::XFormComponent > & xGrid)
{
    try
    {
        Reference< css::form::XGridColumnFactory >  xColFactory(xGrid, UNO_QUERY);
        Reference< XNameContainer >  xColContainer(xGrid, UNO_QUERY);
        clearGridColumns( xColContainer );

        Reference< XLoadable > xFormAsLoadable;
        if (xGrid.is())
            xFormAsLoadable.set(xGrid->getParent(), css::uno::UNO_QUERY);
        if (xFormAsLoadable.is() && xFormAsLoadable->isLoaded())
        {
            // set the formats from the table
            if (m_xCurrentlyDisplayed)
            {
                Sequence< OUString> aProperties(6 + ( m_bPreview ? 5 : 0 ));
                Sequence< Any> aValues(7 + ( m_bPreview ? 5 : 0 ));

                weld::TreeView& rTreeView = m_pTreeView->GetWidget();
                DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*m_xCurrentlyDisplayed).toUInt64());
                OSL_ENSURE( pData->xObjectProperties.is(), "SbaTableQueryBrowser::InitializeGridModel: No table available!" );
                if ( !pData->xObjectProperties.is() )
                    return;

                OUString* pStringIter = aProperties.getArray();
                Any* pValueIter = aValues.getArray();
                if ( m_bPreview )
                {
                    *pStringIter++  = "AlwaysShowCursor";
                    *pValueIter++   <<= false;
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
                    *pValueIter++       <<= false;
                    *pStringIter++  = "HasRecordMarker";
                    *pValueIter++       <<= false;
                }
                *pStringIter++  = PROPERTY_ROW_HEIGHT;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_ROW_HEIGHT);
                if ( m_bPreview )
                {
                    *pStringIter++  = "Tabstop";
                    *pValueIter++       <<= false;
                }
                *pStringIter++  = PROPERTY_TEXTCOLOR;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTCOLOR);
                *pStringIter++  = PROPERTY_TEXTLINECOLOR;
                *pValueIter++   = pData->xObjectProperties->getPropertyValue(PROPERTY_TEXTLINECOLOR);

                Reference< XMultiPropertySet >  xFormMultiSet(xGrid, UNO_QUERY);
                xFormMultiSet->setPropertyValues(aProperties, aValues);
            }

            // get the formats supplier of the database we're working with
            Reference< css::util::XNumberFormatsSupplier >  xSupplier = getNumberFormatter()->getNumberFormatsSupplier();

            Reference<XConnection> xConnection;
            Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
            xRowSetProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;
            OSL_ENSURE(xConnection.is(),"A ActiveConnection should normally exists!");

            Reference<XChild> xChild(xConnection,UNO_QUERY);
            Reference<XPropertySet> xDataSourceProp(xChild->getParent(),UNO_QUERY);
            bool bSuppressVersionCol = false;
            OSL_VERIFY( xDataSourceProp->getPropertyValue( PROPERTY_SUPPRESSVERSIONCL ) >>= bSuppressVersionCol );

            // insert the column into the gridcontrol so that we see something :-)
            OUString aCurrentModelType;
            Reference<XColumnsSupplier> xSupCols(getRowSet(),UNO_QUERY);
            Reference<XNameAccess> xColumns     = xSupCols->getColumns();

            OUString sDefaultProperty;
            Reference< XPropertySet > xColumn;
            Reference< XPropertySetInfo > xColPSI;
            const Sequence<OUString> aColNames = xColumns->getElementNames();
            for (const OUString& rName : aColNames)
            {
                xColumn.set( xColumns->getByName( rName ), UNO_QUERY_THROW );
                xColPSI.set( xColumn->getPropertySetInfo(), UNO_SET_THROW );

                // ignore the column when it is a rowversion one
                if  (   bSuppressVersionCol
                    &&  xColPSI->hasPropertyByName( PROPERTY_ISROWVERSION )
                    &&  ::cppu::any2bool( xColumn->getPropertyValue( PROPERTY_ISROWVERSION ) )
                    )
                    continue;

                // use the result set column's type to determine the type of grid column to create
                bool bFormattedIsNumeric    = true;
                sal_Int32 nType = ::comphelper::getINT32( xColumn->getPropertyValue( PROPERTY_TYPE ) );

                std::vector< NamedValue > aInitialValues;
                std::vector< OUString > aCopyProperties;
                Any aDefault;

                switch(nType)
                {
                    case DataType::BIT:
                    case DataType::BOOLEAN:
                    {
                        aCurrentModelType = "CheckBox";
                        aInitialValues.emplace_back( "VisualEffect", makeAny( VisualEffect::FLAT ) );
                        sDefaultProperty = PROPERTY_DEFAULTSTATE;

                        sal_Int32 nNullable = ColumnValue::NULLABLE_UNKNOWN;
                        OSL_VERIFY( xColumn->getPropertyValue( PROPERTY_ISNULLABLE ) >>= nNullable );
                        aInitialValues.emplace_back(
                            "TriState",
                            makeAny( ColumnValue::NO_NULLS != nNullable )
                        );
                        if ( ColumnValue::NO_NULLS == nNullable )
                            aDefault <<= sal_Int16(TRISTATE_FALSE);
                    }
                    break;

                    case DataType::LONGVARCHAR:
                    case DataType::CLOB:
                        aInitialValues.emplace_back( "MultiLine", makeAny( true ) );
                        [[fallthrough]];
                    case DataType::BINARY:
                    case DataType::VARBINARY:
                    case DataType::LONGVARBINARY:
                        aCurrentModelType = "TextField";
                        sDefaultProperty = PROPERTY_DEFAULTTEXT;
                        break;

                    case DataType::VARCHAR:
                    case DataType::CHAR:
                        bFormattedIsNumeric = false;
                        [[fallthrough]];
                    default:
                        aCurrentModelType = "FormattedField";
                        sDefaultProperty = PROPERTY_EFFECTIVEDEFAULT;

                        if ( xSupplier.is() )
                            aInitialValues.emplace_back( "FormatsSupplier", makeAny( xSupplier ) );
                        aInitialValues.emplace_back( "TreatAsNumber", makeAny( bFormattedIsNumeric ) );
                        aCopyProperties.emplace_back(PROPERTY_FORMATKEY );
                        break;
                }

                aInitialValues.emplace_back( PROPERTY_CONTROLSOURCE, makeAny( rName ) );
                OUString sLabel;
                xColumn->getPropertyValue(PROPERTY_LABEL) >>= sLabel;
                if ( !sLabel.isEmpty() )
                    aInitialValues.emplace_back( PROPERTY_LABEL, makeAny( sLabel ) );
                else
                    aInitialValues.emplace_back( PROPERTY_LABEL, makeAny( rName ) );

                Reference< XPropertySet > xGridCol( xColFactory->createColumn( aCurrentModelType ), UNO_SET_THROW );
                Reference< XPropertySetInfo > xGridColPSI( xGridCol->getPropertySetInfo(), UNO_SET_THROW );

                // calculate the default
                if ( xGridColPSI->hasPropertyByName( PROPERTY_CONTROLDEFAULT ) )
                {
                    aDefault = xColumn->getPropertyValue( PROPERTY_CONTROLDEFAULT );
                    // default value
                    if ( nType == DataType::BIT || nType == DataType::BOOLEAN )
                    {
                        if ( aDefault.hasValue() )
                            aDefault <<= (comphelper::getString(aDefault).toInt32() == 0) ? sal_Int16(TRISTATE_FALSE) : sal_Int16(TRISTATE_TRUE);
                        else
                            aDefault <<= sal_Int16(TRISTATE_INDET);
                    }
                }

                if ( aDefault.hasValue() )
                    aInitialValues.emplace_back( sDefaultProperty, aDefault );

                // transfer properties from the definition to the UNO-model :
                aCopyProperties.emplace_back(PROPERTY_HIDDEN );
                aCopyProperties.emplace_back(PROPERTY_WIDTH );

                // help text to display for the column
                Any aDescription;
                if ( xColPSI->hasPropertyByName( PROPERTY_HELPTEXT ) )
                    aDescription = xColumn->getPropertyValue( PROPERTY_HELPTEXT );
                OUString sTemp;
                aDescription >>= sTemp;
                if ( sTemp.isEmpty() )
                    xColumn->getPropertyValue( PROPERTY_DESCRIPTION ) >>= sTemp;

                aDescription <<= sTemp;
                aInitialValues.emplace_back( PROPERTY_HELPTEXT, aDescription );

                // ... horizontal justify
                Any aAlign; aAlign <<= sal_Int16( 0 );
                Any aColAlign( xColumn->getPropertyValue( PROPERTY_ALIGN ) );
                if ( aColAlign.hasValue() )
                    aAlign <<= sal_Int16( ::comphelper::getINT32( aColAlign ) );
                aInitialValues.emplace_back( PROPERTY_ALIGN, aAlign );

                // don't allow the mouse to scroll in the cells
                if ( xGridColPSI->hasPropertyByName( PROPERTY_MOUSE_WHEEL_BEHAVIOR ) )
                    aInitialValues.emplace_back( PROPERTY_MOUSE_WHEEL_BEHAVIOR, makeAny( MouseWheelBehavior::SCROLL_DISABLED ) );

                // now set all those values
                for (auto const& property : aInitialValues)
                {
                    xGridCol->setPropertyValue( property.Name, property.Value );
                }
                for (auto const& copyPropertyName : aCopyProperties)
                    xGridCol->setPropertyValue( copyPropertyName, xColumn->getPropertyValue(copyPropertyName) );

                xColContainer->insertByName(rName, makeAny(xGridCol));
            }
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

static Reference<XPropertySet> getColumnHelper(const weld::TreeView& rTreeView,
                                               const weld::TreeIter* pCurrentlyDisplayed,
                                               const Reference<XPropertySet>& rxSource)
{
    Reference<XPropertySet> xRet;
    if (pCurrentlyDisplayed)
    {
        DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*pCurrentlyDisplayed).toUInt64());
        Reference<XColumnsSupplier> xColumnsSup(pData->xObjectProperties,UNO_QUERY);
        Reference<XNameAccess> xNames = xColumnsSup->getColumns();
        OUString aName;
        rxSource->getPropertyValue(PROPERTY_NAME) >>= aName;
        if(xNames.is() && xNames->hasByName(aName))
            xRet.set(xNames->getByName(aName),UNO_QUERY);
    }
    return xRet;
}

void SbaTableQueryBrowser::transferChangedControlProperty(const OUString& _rProperty, const Any& _rNewValue)
{
    if (m_xCurrentlyDisplayed)
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*m_xCurrentlyDisplayed).toUInt64());
        Reference< XPropertySet > xObjectProps = pData->xObjectProperties;
        OSL_ENSURE(xObjectProps.is(),"SbaTableQueryBrowser::transferChangedControlProperty: no table/query object!");
        if (xObjectProps.is())
            xObjectProps->setPropertyValue(_rProperty, _rNewValue);
    }
}

void SbaTableQueryBrowser::propertyChange(const PropertyChangeEvent& evt)
{
    SbaXDataBrowserController::propertyChange(evt);

    weld::TreeView& rTreeView = m_pTreeView->GetWidget();

    try
    {
        Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
        if (!xSource.is())
            return;
        // one of the many properties which require us to update the definition ?
        // a column's width ?
        else if (evt.PropertyName == PROPERTY_WIDTH)
        {   // a column width has changed -> update the model
            // (the update of the view is done elsewhere)
            Reference<XPropertySet> xProp = getColumnHelper(rTreeView, m_xCurrentlyDisplayed.get(), xSource);
            if(xProp.is())
            {
                if(!evt.NewValue.hasValue())
                    xProp->setPropertyValue(PROPERTY_WIDTH,makeAny(sal_Int32(227)));
                else
                    xProp->setPropertyValue(PROPERTY_WIDTH,evt.NewValue);
            }
        }

        // a column's 'visible' state ?
        else if (evt.PropertyName == PROPERTY_HIDDEN)
        {
            Reference<XPropertySet> xProp = getColumnHelper(rTreeView, m_xCurrentlyDisplayed.get(), xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_HIDDEN,evt.NewValue);
        }

        // a columns alignment ?
        else if (evt.PropertyName == PROPERTY_ALIGN)
        {
            Reference<XPropertySet> xProp = getColumnHelper(rTreeView, m_xCurrentlyDisplayed.get(), xSource);
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
                        xProp->setPropertyValue(PROPERTY_ALIGN,makeAny(css::awt::TextAlign::LEFT));
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }

        // a column's format ?
        else if (   evt.PropertyName == PROPERTY_FORMATKEY
            &&  (TypeClass_LONG == evt.NewValue.getValueTypeClass())
            )
        {
            // update the model (means the definition object)
            Reference<XPropertySet> xProp = getColumnHelper(rTreeView, m_xCurrentlyDisplayed.get(), xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_FORMATKEY,evt.NewValue);
        }

        // some table definition properties ?
        // the height of the rows in the grid ?
        else if (evt.PropertyName == PROPERTY_ROW_HEIGHT)
        {
            if (m_xCurrentlyDisplayed)
            {
                DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*m_xCurrentlyDisplayed).toUInt64());
                OSL_ENSURE( pData->xObjectProperties.is(), "No table available!" );

                bool bDefault = !evt.NewValue.hasValue();
                if (bDefault)
                    pData->xObjectProperties->setPropertyValue(PROPERTY_ROW_HEIGHT,makeAny(sal_Int32(45)));
                else
                    pData->xObjectProperties->setPropertyValue(PROPERTY_ROW_HEIGHT,evt.NewValue);
            }
        }

        else if (   evt.PropertyName == PROPERTY_FONT          // the font ?
                ||  evt.PropertyName == PROPERTY_TEXTCOLOR     // the text color ?
                ||  evt.PropertyName == PROPERTY_FILTER        // the filter ?
                ||  evt.PropertyName == PROPERTY_HAVING_CLAUSE // the having clause ?
                ||  evt.PropertyName == PROPERTY_ORDER         // the sort ?
                ||  evt.PropertyName == PROPERTY_APPLYFILTER   // the appliance of the filter ?
                ||  evt.PropertyName == PROPERTY_TEXTLINECOLOR // the text line color ?
                ||  evt.PropertyName == PROPERTY_TEXTEMPHASIS  // the text emphasis ?
                ||  evt.PropertyName == PROPERTY_TEXTRELIEF    // the text relief ?
                )
        {
            transferChangedControlProperty(evt.PropertyName, evt.NewValue);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

sal_Bool SbaTableQueryBrowser::suspend(sal_Bool bSuspend)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    if ( getView() && getView()->IsInModalMode() )
        return false;
    bool bRet = false;
    if ( !m_bInSuspend )
    {
        m_bInSuspend = true;
        if ( rBHelper.bDisposed )
            throw DisposedException( OUString(), *this );

        bRet = SbaXDataBrowserController::suspend(bSuspend);
        if ( bRet && getView() )
            getView()->Hide();

        m_bInSuspend = false;
    }

    return bRet;
}

void SAL_CALL SbaTableQueryBrowser::statusChanged( const FeatureStateEvent& _rEvent )
{
    // search the external dispatcher causing this call
    Reference< XDispatch > xSource(_rEvent.Source, UNO_QUERY);
    bool bFound = false;
    for (auto & externalFeature : m_aExternalFeatures)
    {
        if ( _rEvent.FeatureURL.Complete == externalFeature.second.aURL.Complete)
        {
            bFound = true;
            OSL_ENSURE( xSource.get() == externalFeature.second.xDispatcher.get(), "SbaTableQueryBrowser::statusChanged: inconsistent!" );
            // update the enabled state
            externalFeature.second.bEnabled = _rEvent.IsEnabled;

            switch ( externalFeature.first )
            {
                case ID_BROWSER_DOCUMENT_DATASOURCE:
                {
                    // if it's the slot for the document data source, remember the state
                    Sequence< PropertyValue > aDescriptor;
                    bool bProperFormat = _rEvent.State >>= aDescriptor;
                    OSL_ENSURE(bProperFormat, "SbaTableQueryBrowser::statusChanged: need a data access descriptor here!");
                    m_aDocumentDataSource.initializeFrom(aDescriptor);

                    OSL_ENSURE( (   m_aDocumentDataSource.has(DataAccessDescriptorProperty::DataSource)
                                ||  m_aDocumentDataSource.has(DataAccessDescriptorProperty::DatabaseLocation)
                                )
                                &&  m_aDocumentDataSource.has(DataAccessDescriptorProperty::Command)
                                &&  m_aDocumentDataSource.has(DataAccessDescriptorProperty::CommandType),
                        "SbaTableQueryBrowser::statusChanged: incomplete descriptor!");

                    // check if we know the object which is set as document data source
                    checkDocumentDataSource();
                }
                break;

                default:
                    // update the toolbox
                    implCheckExternalSlot( externalFeature.first );
                    break;
            }
            break;
        }
    }

    OSL_ENSURE(bFound, "SbaTableQueryBrowser::statusChanged: don't know who sent this!");
}

void SbaTableQueryBrowser::checkDocumentDataSource()
{
    std::unique_ptr<weld::TreeIter> xDataSourceEntry;
    std::unique_ptr<weld::TreeIter> xContainerEntry;
    std::unique_ptr<weld::TreeIter> xObjectEntry = getObjectEntry(m_aDocumentDataSource, &xDataSourceEntry, &xContainerEntry);
    bool bKnownDocDataSource = static_cast<bool>(xObjectEntry);
    if (!bKnownDocDataSource)
    {
        if (xDataSourceEntry)
        {
            // at least the data source is known
            if (xContainerEntry)
            {
                bKnownDocDataSource = true; // assume we know it.
                // TODO: should we expand the object container? This may be too expensive just for checking...
            }
            else
            {
                if (m_aDocumentDataSource.has(DataAccessDescriptorProperty::CommandType)
                    && m_aDocumentDataSource.has(DataAccessDescriptorProperty::Command))
                {   // maybe we have a command to be displayed ?
                    sal_Int32 nCommandType = CommandType::TABLE;
                    m_aDocumentDataSource[DataAccessDescriptorProperty::CommandType] >>= nCommandType;

                    OUString sCommand;
                    m_aDocumentDataSource[DataAccessDescriptorProperty::Command] >>= sCommand;

                    bKnownDocDataSource = (CommandType::COMMAND == nCommandType) && (!sCommand.isEmpty());
                }
            }
        }
    }

    if ( !bKnownDocDataSource )
        m_aExternalFeatures[ ID_BROWSER_DOCUMENT_DATASOURCE ].bEnabled = false;

    // update the toolbox
    implCheckExternalSlot(ID_BROWSER_DOCUMENT_DATASOURCE);
}

void SbaTableQueryBrowser::extractDescriptorProps(const svx::ODataAccessDescriptor& _rDescriptor, OUString& _rDataSource, OUString& _rCommand, sal_Int32& _rCommandType, bool& _rEscapeProcessing)
{
    _rDataSource = _rDescriptor.getDataSource();
    if ( _rDescriptor.has(DataAccessDescriptorProperty::Command) )
        _rDescriptor[DataAccessDescriptorProperty::Command] >>= _rCommand;
    if ( _rDescriptor.has(DataAccessDescriptorProperty::CommandType) )
        _rDescriptor[DataAccessDescriptorProperty::CommandType] >>= _rCommandType;

    // escape processing is the only one allowed not to be present
    _rEscapeProcessing = true;
    if (_rDescriptor.has(DataAccessDescriptorProperty::EscapeProcessing))
        _rEscapeProcessing = ::cppu::any2bool(_rDescriptor[DataAccessDescriptorProperty::EscapeProcessing]);
}

namespace
{
    bool getDataSourceDisplayName_isURL( const OUString& _rDS, OUString& _rDisplayName, OUString& _rUniqueId )
    {
        INetURLObject aURL( _rDS );
        if ( aURL.GetProtocol() != INetProtocol::NotValid )
        {
            _rDisplayName = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::WithCharset);
            _rUniqueId = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            return true;
        }
        _rDisplayName = _rDS;
        _rUniqueId.clear();
        return false;
    }

    struct FilterByEntryDataId : public IEntryFilter
    {
        OUString sId;
        explicit FilterByEntryDataId( const OUString& _rId ) : sId( _rId ) { }

        virtual ~FilterByEntryDataId() {}

        virtual bool    includeEntry(const void* pEntry) const override;
    };

    bool FilterByEntryDataId::includeEntry(const void* pUserData) const
    {
        const DBTreeListUserData* pData = static_cast<const DBTreeListUserData*>(pUserData);
        return ( !pData || ( pData->sAccessor == sId ) );
    }
}

OUString SbaTableQueryBrowser::getDataSourceAccessor(const weld::TreeIter& rDataSourceEntry) const
{
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(rDataSourceEntry).toUInt64());
    OSL_ENSURE( pData, "SbaTableQueryBrowser::getDataSourceAccessor: invalid entry data!" );
    OSL_ENSURE( pData->eType == etDatasource, "SbaTableQueryBrowser::getDataSourceAccessor: entry does not denote a data source!" );
    return !pData->sAccessor.isEmpty() ? pData->sAccessor : GetEntryText(rDataSourceEntry);
}

std::unique_ptr<weld::TreeIter> SbaTableQueryBrowser::getObjectEntry(const OUString& _rDataSource, const OUString& _rCommand, sal_Int32 nCommandType,
        std::unique_ptr<weld::TreeIter>* ppDataSourceEntry, std::unique_ptr<weld::TreeIter>* ppContainerEntry, bool bExpandAncestors,
        const SharedConnection& _rxConnection )
{
    if (ppDataSourceEntry)
        ppDataSourceEntry->reset();
    if (ppContainerEntry)
        ppContainerEntry->reset();

    std::unique_ptr<weld::TreeIter> xObject;
    if ( m_pTreeView )
    {
        // look for the data source entry
        OUString sDisplayName, sDataSourceId;
        bool bIsDataSourceURL = getDataSourceDisplayName_isURL( _rDataSource, sDisplayName, sDataSourceId );
            // the display name may differ from the URL for readability reasons
            // #i33699#

        FilterByEntryDataId aFilter( sDataSourceId );
        std::unique_ptr<weld::TreeIter> xDataSource = m_pTreeView->GetEntryPosByName( sDisplayName, nullptr, &aFilter );
        if (!xDataSource) // check if the data source name is a file location
        {
            if ( bIsDataSourceURL )
            {
                // special case, the data source is a URL
                // add new entries to the list box model
                implAddDatasource( _rDataSource, _rxConnection );
                xDataSource = m_pTreeView->GetEntryPosByName( sDisplayName, nullptr, &aFilter );
                OSL_ENSURE( xDataSource, "SbaTableQueryBrowser::getObjectEntry: hmm - did not find it again!" );
            }
        }

        weld::TreeView& rTreeView = m_pTreeView->GetWidget();

        if (xDataSource)
        {
            if (ppDataSourceEntry)
            {
                // (caller wants to have it...)
                *ppDataSourceEntry = rTreeView.make_iterator(xDataSource.get());
            }

            // expand if required so
            if (bExpandAncestors)
                rTreeView.expand_row(*xDataSource);

            // look for the object container
            std::unique_ptr<weld::TreeIter> xCommandType;
            if (nCommandType == CommandType::QUERY || nCommandType == CommandType::TABLE)
            {
                xCommandType = rTreeView.make_iterator(xDataSource.get());
                if (!rTreeView.iter_children(*xCommandType))
                    xCommandType.reset();
                else
                {
                    // 1st child is queries, so we're already done if looking for CommandType::QUERY

                    // 2nd child is tables
                    if (nCommandType == CommandType::TABLE && !rTreeView.iter_next_sibling(*xCommandType))
                        xCommandType.reset();
                }
            }

            if (xCommandType)
            {
                if (ppContainerEntry)
                {
                    // (caller wants to have it...)
                    *ppContainerEntry = rTreeView.make_iterator(xCommandType.get());
                }

                rTreeView.make_unsorted();

                // expand if required so
                if (bExpandAncestors)
                {
                    rTreeView.expand_row(*xCommandType);
                }

                // look for the object
                sal_Int32 nIndex = 0;
                do
                {
                    OUString sPath;
                    switch (nCommandType)
                    {
                    case CommandType::TABLE:
                        sPath = _rCommand;
                        nIndex = -1;
                        break;

                    case CommandType::QUERY:
                        sPath = _rCommand.getToken( 0, '/', nIndex );
                        break;

                    default:
                        assert(false);
                    }
                    xObject = m_pTreeView->GetEntryPosByName(sPath, xCommandType.get());
                    if (xObject)
                        rTreeView.copy_iterator(*xObject, *xCommandType);
                    else
                        xCommandType.reset();
                    if ( nIndex >= 0 )
                    {
                        if (ensureEntryObject(*xObject))
                        {
                            DBTreeListUserData* pParentData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xObject).toUInt64());
                            Reference< XNameAccess > xCollection( pParentData->xContainer, UNO_QUERY );
                            sal_Int32 nIndex2 = nIndex;
                            sPath = _rCommand.getToken( 0, '/', nIndex2 );
                            try
                            {
                                if ( xCollection->hasByName(sPath) )
                                {
                                    if(!m_pTreeView->GetEntryPosByName(sPath, xObject.get()))
                                    {
                                        Reference<XNameAccess> xChild(xCollection->getByName(sPath),UNO_QUERY);
                                        DBTreeListUserData* pEntryData = new DBTreeListUserData;
                                        pEntryData->eType = etQuery;
                                        if ( xChild.is() )
                                        {
                                            pEntryData->eType = etQueryContainer;
                                        }
                                        implAppendEntry(xObject.get(), sPath, pEntryData);
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

                rTreeView.make_sorted();
            }
        }
    }
    return xObject;
}

std::unique_ptr<weld::TreeIter> SbaTableQueryBrowser::getObjectEntry(const svx::ODataAccessDescriptor& rDescriptor,
        std::unique_ptr<weld::TreeIter>* ppDataSourceEntry, std::unique_ptr<weld::TreeIter>* ppContainerEntry)
{
    // extract the props from the descriptor
    OUString sDataSource;
    OUString sCommand;
    sal_Int32 nCommandType = CommandType::COMMAND;
    bool bEscapeProcessing = true;
    extractDescriptorProps(rDescriptor, sDataSource, sCommand, nCommandType, bEscapeProcessing);

    return getObjectEntry(sDataSource, sCommand, nCommandType, ppDataSourceEntry, ppContainerEntry, false /*bExpandAncestors*/);
}

void SbaTableQueryBrowser::connectExternalDispatches()
{
    Reference< XDispatchProvider >  xProvider( getFrame(), UNO_QUERY );
    OSL_ENSURE(xProvider.is(), "SbaTableQueryBrowser::connectExternalDispatches: no DispatchProvider !");
    if (!xProvider.is())
        return;

    if ( m_aExternalFeatures.empty() )
    {
        const char* pURLs[] = {
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

        for ( size_t i=0; i < SAL_N_ELEMENTS( pURLs ); ++i )
        {
            URL aURL;
            aURL.Complete = OUString::createFromAscii( pURLs[i] );
            if ( m_xUrlTransformer.is() )
                m_xUrlTransformer->parseStrict( aURL );
            m_aExternalFeatures[ nIds[ i ] ] = ExternalFeature( aURL );
        }
    }

    for (auto & externalFeature : m_aExternalFeatures)
    {
        externalFeature.second.xDispatcher = xProvider->queryDispatch(
            externalFeature.second.aURL, "_parent", FrameSearchFlag::PARENT
        );

        if ( externalFeature.second.xDispatcher.get() == static_cast< XDispatch* >( this ) )
        {
            SAL_WARN("dbaccess.ui",  "SbaTableQueryBrowser::connectExternalDispatches: this should not happen anymore!" );
                // (nowadays, the URLs aren't in our SupportedFeatures list anymore, so we should
                // not supply a dispatcher for this)
            externalFeature.second.xDispatcher.clear();
        }

        if ( externalFeature.second.xDispatcher.is() )
        {
            try
            {
                externalFeature.second.xDispatcher->addStatusListener( this, externalFeature.second.aURL );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
        }

        implCheckExternalSlot( externalFeature.first );
    }
}

void SbaTableQueryBrowser::implCheckExternalSlot( sal_uInt16 _nId )
{
    if ( !m_xMainToolbar.is() )
        return;

    VclPtr<vcl::Window> pToolboxWindow = VCLUnoHelper::GetWindow( m_xMainToolbar );
    ToolBox* pToolbox = dynamic_cast< ToolBox* >( pToolboxWindow.get() );
    OSL_ENSURE( pToolbox, "SbaTableQueryBrowser::implCheckExternalSlot: cannot obtain the toolbox window!" );

    // check if we have to hide this item from the toolbox
    if ( pToolbox )
    {
        bool bHaveDispatcher = m_aExternalFeatures[ _nId ].xDispatcher.is();
        if ( bHaveDispatcher != pToolbox->IsItemVisible( ToolBoxItemId(_nId) ) )
            bHaveDispatcher ? pToolbox->ShowItem( ToolBoxItemId(_nId) ) : pToolbox->HideItem( ToolBoxItemId(_nId) );
    }

    // and invalidate this feature in general
    InvalidateFeature( _nId );
}

void SAL_CALL SbaTableQueryBrowser::disposing( const css::lang::EventObject& _rSource )
{
    // our frame ?
    Reference< css::frame::XFrame >  xSourceFrame(_rSource.Source, UNO_QUERY);
    if (m_xCurrentFrameParent.is() && (xSourceFrame == m_xCurrentFrameParent))
        m_xCurrentFrameParent->removeFrameActionListener(static_cast<css::frame::XFrameActionListener*>(this));
    else
    {
        // search the external dispatcher causing this call in our map
        Reference< XDispatch > xSource(_rSource.Source, UNO_QUERY);
        if(xSource.is())
        {
            ExternalFeaturesMap::const_iterator aLoop = m_aExternalFeatures.begin();
            ExternalFeaturesMap::const_iterator aEnd = m_aExternalFeatures.end();
            while (aLoop != aEnd)
            {
                if ( aLoop->second.xDispatcher.get() == xSource.get() )
                {
                    sal_uInt16 nSlot = aLoop->first;

                    // remove it
                    aLoop = m_aExternalFeatures.erase(aLoop);

                    // maybe update the UI
                    implCheckExternalSlot(nSlot);

                    // continue, the same XDispatch may be responsible for more than one URL
                }
                ++aLoop;
            }
        }
        else
        {
            Reference<XConnection> xCon(_rSource.Source, UNO_QUERY);
            if ( xCon.is() && m_pTreeView )
            {
                // our connection is in dispose so we have to find the entry equal with this connection
                // and close it what means to collapse the entry
                // get the top-level representing the removed data source
                weld::TreeView& rTreeView = m_pTreeView->GetWidget();
                std::unique_ptr<weld::TreeIter> xDSLoop(rTreeView.make_iterator());
                if (rTreeView.get_iter_first(*xDSLoop))
                {
                    do
                    {
                        DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xDSLoop).toUInt64());
                        if ( pData && pData->xConnection == xCon )
                        {
                            // we set the connection to null to avoid a second disposing of the connection
                            pData->xConnection.clear();
                            closeConnection(*xDSLoop, false);
                            break;
                        }
                    }
                    while (rTreeView.iter_next_sibling(*xDSLoop));
                }
            }
            else
                SbaXDataBrowserController::disposing(_rSource);
        }
    }
}

void SbaTableQueryBrowser::implRemoveStatusListeners()
{
    // clear all old dispatches
    for (auto const& externalFeature : m_aExternalFeatures)
    {
        if ( externalFeature.second.xDispatcher.is() )
        {
            try
            {
                externalFeature.second.xDispatcher->removeStatusListener( this, externalFeature.second.aURL );
            }
            catch (Exception&)
            {
                SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::implRemoveStatusListeners: could not remove a status listener!");
            }
        }
    }
    m_aExternalFeatures.clear();
}

sal_Bool SAL_CALL SbaTableQueryBrowser::select( const Any& _rSelection )
{
    SolarMutexGuard aGuard;
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    Sequence< PropertyValue > aDescriptorSequence;
    if (!(_rSelection >>= aDescriptorSequence))
        throw IllegalArgumentException(OUString(), *this, 1);
        // TODO: error message

    ODataAccessDescriptor aDescriptor;
    try
    {
        aDescriptor = ODataAccessDescriptor(aDescriptorSequence);
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::select: could not extract the descriptor!");
    }

    // check the presence of the props we need
    if ( !(aDescriptor.has(DataAccessDescriptorProperty::DataSource) || aDescriptor.has(DataAccessDescriptorProperty::DatabaseLocation)) || !aDescriptor.has(DataAccessDescriptorProperty::Command) || !aDescriptor.has(DataAccessDescriptorProperty::CommandType))
        throw IllegalArgumentException(OUString(), *this, 1);
        // TODO: error message

    return implSelect(aDescriptor,true);
}

Any SAL_CALL SbaTableQueryBrowser::getSelection(  )
{
    Any aReturn;

    try
    {
        Reference< XLoadable > xLoadable(getRowSet(), UNO_QUERY);
        if (xLoadable.is() && xLoadable->isLoaded())
        {
            Reference< XPropertySet > aFormProps(getRowSet(), UNO_QUERY);
            ODataAccessDescriptor aDescriptor(aFormProps);
            // remove properties which are not part of our "selection"
            aDescriptor.erase(DataAccessDescriptorProperty::Connection);
            aDescriptor.erase(DataAccessDescriptorProperty::Cursor);

            aReturn <<= aDescriptor.createPropertyValueSequence();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    return aReturn;
}

void SAL_CALL SbaTableQueryBrowser::addSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener )
{
    m_aSelectionListeners.addInterface(_rxListener);
}

void SAL_CALL SbaTableQueryBrowser::removeSelectionChangeListener( const Reference< XSelectionChangeListener >& _rxListener )
{
    m_aSelectionListeners.removeInterface(_rxListener);
}

void SbaTableQueryBrowser::attachFrame(const Reference< css::frame::XFrame > & _xFrame)
{
    implRemoveStatusListeners();

    if (m_xCurrentFrameParent.is())
        m_xCurrentFrameParent->removeFrameActionListener(static_cast<css::frame::XFrameActionListener*>(this));

    SbaXDataBrowserController::attachFrame(_xFrame);

    Reference< XFrame > xCurrentFrame( getFrame() );
    if ( xCurrentFrame.is() )
    {
        m_xCurrentFrameParent = xCurrentFrame->findFrame("_parent",FrameSearchFlag::PARENT);
        if ( m_xCurrentFrameParent.is() )
            m_xCurrentFrameParent->addFrameActionListener(static_cast<css::frame::XFrameActionListener*>(this));

        // obtain our toolbox
        try
        {
            Reference< XPropertySet > xFrameProps( m_aCurrentFrame.getFrame(), UNO_QUERY_THROW );
            Reference< XLayoutManager > xLayouter(
                xFrameProps->getPropertyValue("LayoutManager"),
                UNO_QUERY );

            if ( xLayouter.is() )
            {
                Reference< XUIElement > xUI(
                    xLayouter->getElement( "private:resource/toolbar/toolbar" ),
                    UNO_SET_THROW );
                m_xMainToolbar.set(xUI->getRealInterface(), css::uno::UNO_QUERY);
                OSL_ENSURE( m_xMainToolbar.is(), "SbaTableQueryBrowser::attachFrame: where's my toolbox?" );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }

    // get the dispatchers for the external slots
    connectExternalDispatches();
}

void SbaTableQueryBrowser::addModelListeners(const Reference< css::awt::XControlModel > & _xGridControlModel)
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

void SbaTableQueryBrowser::removeModelListeners(const Reference< css::awt::XControlModel > & _xGridControlModel)
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
    unloadAndCleanup( false );
}

void SbaTableQueryBrowser::LoadFinished(bool _bWasSynch)
{
    SbaXDataBrowserController::LoadFinished(_bWasSynch);

    m_sQueryCommand.clear();
    m_bQueryEscapeProcessing = false;

    if (isValid() && !loadingCancelled())
    {
        // did we load a query?
        bool bTemporary;    // needed because we m_bQueryEscapeProcessing is only one bit wide (and we want to pass it by reference)
        if ( implGetQuerySignature( m_sQueryCommand, bTemporary ) )
            m_bQueryEscapeProcessing = bTemporary;
    }

    // if the form has been loaded, this means that our "selection" has changed
    css::lang::EventObject aEvent( *this );
    m_aSelectionListeners.notifyEach( &XSelectionChangeListener::selectionChanged, aEvent );
}

bool SbaTableQueryBrowser::getExternalSlotState( sal_uInt16 _nId ) const
{
    bool bEnabled = false;
    ExternalFeaturesMap::const_iterator aPos = m_aExternalFeatures.find( _nId );
    if ( ( m_aExternalFeatures.end() != aPos ) && aPos->second.xDispatcher.is() )
        bEnabled = aPos->second.bEnabled;
    return bEnabled;
}

FeatureState SbaTableQueryBrowser::GetState(sal_uInt16 nId) const
{
    FeatureState aReturn;
        // (disabled automatically)

    // no chance without a view
    if (!getBrowserView() || !getBrowserView()->getVclControl())
        return aReturn;

    switch ( nId )
    {
        case ID_TREE_ADMINISTRATE:
            aReturn.bEnabled = true;
            return aReturn;

        case ID_BROWSER_CLOSE:
            // the close button should always be enabled
            aReturn.bEnabled = !m_bEnableBrowser;
            return aReturn;

            // "toggle explorer" is always enabled (if we have an explorer)
        case ID_BROWSER_EXPLORER:
            aReturn.bEnabled = m_bEnableBrowser;
            aReturn.bChecked = haveExplorer();
            return aReturn;

        case ID_BROWSER_REMOVEFILTER:
            return SbaXDataBrowserController::GetState( nId );

        case ID_BROWSER_COPY:
            if ( !m_pTreeView->HasChildPathFocus() )
                // handled below
                break;
            [[fallthrough]];
        case ID_TREE_CLOSE_CONN:
        case ID_TREE_EDIT_DATABASE:
        {
            weld::TreeView& rTreeView = m_pTreeView->GetWidget();
            std::unique_ptr<weld::TreeIter> xCurrentEntry(rTreeView.make_iterator());
            if (!rTreeView.get_cursor(xCurrentEntry.get()))
                return aReturn;

            EntryType eType = getEntryType(*xCurrentEntry);
            if ( eType == etUnknown )
                return aReturn;

            std::unique_ptr<weld::TreeIter> xDataSourceEntry = m_pTreeView->GetRootLevelParent(xCurrentEntry.get());
            DBTreeListUserData* pDSData
                =   xDataSourceEntry
                ?   reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xDataSourceEntry).toUInt64())
                :   nullptr;

            if ( nId == ID_TREE_CLOSE_CONN )
            {
                aReturn.bEnabled = ( pDSData != nullptr ) && pDSData->xConnection.is();
            }
            else if ( nId == ID_TREE_EDIT_DATABASE )
            {
                ::utl::OConfigurationTreeRoot aConfig( ::utl::OConfigurationTreeRoot::createWithComponentContext( getORB(),
                    "/org.openoffice.Office.DataAccess/Policies/Features/Common" ) );
                bool bHaveEditDatabase( true );
                OSL_VERIFY( aConfig.getNodeValue( "EditDatabaseFromDataSourceView" ) >>= bHaveEditDatabase );
                aReturn.bEnabled = getORB().is() && xDataSourceEntry && bHaveEditDatabase;
            }
            else if ( nId == ID_BROWSER_COPY )
            {
                aReturn.bEnabled = isEntryCopyAllowed(*xCurrentEntry);
            }

            return aReturn;
        }
    }

    // all slots not handled above are not available if no form is loaded
    if (!isLoaded())
        return aReturn;

    try
    {
        bool bHandled = false;
        switch (nId)
        {
            case ID_BROWSER_DOCUMENT_DATASOURCE:
                // the slot is enabled if we have an external dispatcher able to handle it,
                // and the dispatcher must have enabled the slot in general
                aReturn.bEnabled = getExternalSlotState( ID_BROWSER_DOCUMENT_DATASOURCE );
                bHandled = true;
                break;
            case ID_BROWSER_REFRESH:
                aReturn.bEnabled = true;
                bHandled = true;
                break;
        }

        if (bHandled)
            return aReturn;

        // no chance without valid models
        if (isValid() && !isValidCursor() && nId != ID_BROWSER_CLOSE)
            return aReturn;

        switch (nId)
        {
            case ID_BROWSER_INSERTCOLUMNS:
            case ID_BROWSER_INSERTCONTENT:
            case ID_BROWSER_FORMLETTER:
            {
                // the slot is enabled if we have an external dispatcher able to handle it,
                // and the dispatcher must have enabled the slot in general
                aReturn.bEnabled = getExternalSlotState( nId );

                // for the Insert* slots, we need at least one selected row
                if (ID_BROWSER_FORMLETTER != nId)
                    aReturn.bEnabled = aReturn.bEnabled && getBrowserView()->getVclControl()->GetSelectRowCount();

                // disabled for native queries which are not saved within the database
                Reference< XPropertySet >  xDataSource(getRowSet(), UNO_QUERY);
                try
                {
                    aReturn.bEnabled = aReturn.bEnabled && xDataSource.is();

                    if (xDataSource.is())
                    {
                        sal_Int32 nType = ::comphelper::getINT32(xDataSource->getPropertyValue(PROPERTY_COMMAND_TYPE));
                        aReturn.bEnabled = aReturn.bEnabled &&
                                           ( ::comphelper::getBOOL(xDataSource->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)) ||
                                             (nType == css::sdb::CommandType::QUERY) );
                    }
                }
                catch(DisposedException&)
                {
                    SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::GetState: object already disposed!");
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("dbaccess");
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
                            sTitle = DBA_RES(STR_TBL_TITLE); break;
                        case CommandType::QUERY:
                        case CommandType::COMMAND:
                            sTitle = DBA_RES(STR_QRY_TITLE); break;
                        default:
                            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::GetState: unknown command type!");
                    }
                    OUString aName;
                    xProp->getPropertyValue(PROPERTY_COMMAND) >>= aName;
                    OUString sObject(aName);

                    aReturn.sTitle = sTitle.replaceFirst("#", sObject);
                    aReturn.bEnabled = true;
                }
                break;
            case ID_BROWSER_TABLEATTR:
            case ID_BROWSER_ROWHEIGHT:
            case ID_BROWSER_COLATTRSET:
            case ID_BROWSER_COLWIDTH:
                aReturn.bEnabled = getBrowserView()->getVclControl() && isValid() && isValidCursor();
                //  aReturn.bEnabled &= getDefinition() && !getDefinition()->GetDatabase()->IsReadOnly();
                break;

            case ID_BROWSER_COPY:
                OSL_ENSURE( !m_pTreeView->HasChildPathFocus(), "SbaTableQueryBrowser::GetState( ID_BROWSER_COPY ): this should have been handled above!" );
                if (getBrowserView()->getVclControl() && !getBrowserView()->getVclControl()->IsEditing())
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
                [[fallthrough]];
            default:
                return SbaXDataBrowserController::GetState(nId);
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
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
        {
            weld::TreeView& rTreeView = m_pTreeView->GetWidget();
            std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
            if (rTreeView.get_cursor(xIter.get()))
                implAdministrate(*xIter);
            break;
        }
        case ID_TREE_CLOSE_CONN:
        {
            weld::TreeView& rTreeView = m_pTreeView->GetWidget();
            std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
            if (rTreeView.get_cursor(xIter.get()))
            {
                xIter = m_pTreeView->GetRootLevelParent(xIter.get());
                closeConnection(*xIter);
            }
            break;
        }
        case ID_TREE_ADMINISTRATE:
            svx::administrateDatabaseRegistration( getFrameWeld() );
            break;

        case ID_BROWSER_REFRESH:
        {
            if ( !SaveModified( ) )
                // nothing to do
                break;

            bool bFullReinit = false;
            // check if the query signature (if the form is based on a query) has changed
            if ( !m_sQueryCommand.isEmpty() )
            {
                OUString sNewQueryCommand;
                bool bNewQueryEP;

                bool bIsQuery =
                    implGetQuerySignature( sNewQueryCommand, bNewQueryEP );
                OSL_ENSURE( bIsQuery, "SbaTableQueryBrowser::Execute: was a query before, but is not anymore?" );

                bFullReinit = ( sNewQueryCommand != m_sQueryCommand ) || ( m_bQueryEscapeProcessing != bNewQueryEP );
            }
            if ( !bFullReinit )
            {
                // let the base class do a simple reload
                SbaXDataBrowserController::Execute(nId,aArgs);
                break;
            }
            [[fallthrough]];
        }

        case ID_BROWSER_REFRESH_REBUILD:
        {
            if ( !SaveModified() )
                // nothing to do
                break;

            weld::TreeView& rTreeView = m_pTreeView->GetWidget();
            std::unique_ptr<weld::TreeIter> xSelected = m_xCurrentlyDisplayed ?
                rTreeView.make_iterator(m_xCurrentlyDisplayed.get()) : nullptr;

            // unload
            unloadAndCleanup( false );

            // reselect the entry
            if ( xSelected )
            {
                implSelect(xSelected.get());
            }
            else
            {
                Reference<XPropertySet> xProp(getRowSet(),UNO_QUERY);
                implSelect(svx::ODataAccessDescriptor(xProp));
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
                // the URL the slot id is assigned to
                OSL_ENSURE( m_aExternalFeatures.find( nId ) != m_aExternalFeatures.end(),
                    "SbaTableQueryBrowser::Execute( ID_BROWSER_?): how could this ever be enabled?" );
                URL aParentUrl = m_aExternalFeatures[ nId ].aURL;

                // let the dispatcher execute the slot
                Reference< XDispatch > xDispatch( m_aExternalFeatures[ nId ].xDispatcher );
                if (xDispatch.is())
                {
                    // set the properties for the dispatch

                    // first fill the selection
                    SbaGridControl* pGrid = getBrowserView()->getVclControl();
                    MultiSelection* pSelection = const_cast<MultiSelection*>(pGrid->GetSelection());
                    Sequence< Any > aSelection;
                    if ( !pGrid->IsAllSelected() )
                    {   // transfer the selected rows only if not all rows are selected
                        // (all rows means the whole table)
                        // #i3832#
                        if (pSelection != nullptr)
                        {
                            aSelection.realloc(pSelection->GetSelectCount());
                            tools::Long nIdx = pSelection->FirstSelected();
                            Any* pSelectionNos = aSelection.getArray();
                            while (nIdx != SFX_ENDOFSELECTION)
                            {
                                *pSelectionNos++ <<= static_cast<sal_Int32>(nIdx + 1);
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
                        aDescriptor[DataAccessDescriptorProperty::Command]      =   xProp->getPropertyValue(PROPERTY_COMMAND);
                        aDescriptor[DataAccessDescriptorProperty::CommandType]  =   xProp->getPropertyValue(PROPERTY_COMMAND_TYPE);
                        aDescriptor[DataAccessDescriptorProperty::Connection]   =   xProp->getPropertyValue(PROPERTY_ACTIVE_CONNECTION);
                        aDescriptor[DataAccessDescriptorProperty::Cursor]       <<= xCursorClone;
                        if ( aSelection.hasElements() )
                        {
                            aDescriptor[DataAccessDescriptorProperty::Selection]            <<= aSelection;
                            aDescriptor[DataAccessDescriptorProperty::BookmarkSelection]    <<= false;
                                // these are selection indices
                                // before we change this, all clients have to be adjusted
                                // so that they recognize the new BookmarkSelection property!
                        }

                        xDispatch->dispatch(aParentUrl, aDescriptor.createPropertyValueSequence());
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                }
            }
            break;

        case ID_BROWSER_CLOSE:
            closeTask();
            // if it's not 0, such an async close is already pending
            break;

        case ID_BROWSER_COPY:
            if(m_pTreeView->HasChildPathFocus())
            {
                weld::TreeView& rTreeView = m_pTreeView->GetWidget();
                std::unique_ptr<weld::TreeIter> xCursor(rTreeView.make_iterator());
                if (rTreeView.get_cursor(xCursor.get()))
                    copyEntry(*xCursor);
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
    OUString a, b, c, d, e;
    implAddDatasource( _rDataSourceName, a, d, b, e, c, _rxConnection );
}

void SbaTableQueryBrowser::implAddDatasource(const OUString& _rDbName, OUString& _rDbImage,
        OUString& _rQueryName, OUString& _rQueryImage, OUString& _rTableName, OUString& _rTableImage,
        const SharedConnection& _rxConnection)
{
    SolarMutexGuard aGuard;
    // initialize the names/images if necessary
    if (_rQueryName.isEmpty())
        _rQueryName = DBA_RES(RID_STR_QUERIES_CONTAINER);
    if (_rTableName.isEmpty())
        _rTableName = DBA_RES(RID_STR_TABLES_CONTAINER);

    if (_rQueryImage.isEmpty())
        _rQueryImage = ImageProvider::getFolderImageId(DatabaseObject::QUERY);
    if (_rTableImage.isEmpty())
        _rTableImage = ImageProvider::getFolderImageId(DatabaseObject::TABLE);

    if (_rDbImage.isEmpty())
        _rDbImage = ImageProvider::getDatabaseImage();

    // add the entry for the data source
    // special handling for data sources denoted by URLs - we do not want to display this ugly URL, do we?
    // #i33699#
    OUString sDSDisplayName, sDataSourceId;
    getDataSourceDisplayName_isURL( _rDbName, sDSDisplayName, sDataSourceId );

    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    DBTreeListUserData* pDSData = new DBTreeListUserData;
    pDSData->eType = etDatasource;
    pDSData->sAccessor = sDataSourceId;
    pDSData->xConnection = _rxConnection;
    OUString sId(OUString::number(reinterpret_cast<sal_uInt64>(pDSData)));

    std::unique_ptr<weld::TreeIter> xDatasourceEntry(rTreeView.make_iterator());
    rTreeView.insert(nullptr, -1, &sDSDisplayName, &sId, nullptr, nullptr, false, xDatasourceEntry.get());
    rTreeView.set_image(*xDatasourceEntry, _rDbImage);
    rTreeView.set_text_emphasis(*xDatasourceEntry, false, 0);

    // the child for the queries container
    {
        DBTreeListUserData* pQueriesData = new DBTreeListUserData;
        pQueriesData->eType = etQueryContainer;
        sId = OUString::number(reinterpret_cast<sal_uInt64>(pQueriesData));

        std::unique_ptr<weld::TreeIter> xRet(rTreeView.make_iterator());
        rTreeView.insert(xDatasourceEntry.get(), -1, &_rQueryName, &sId,
                         nullptr, nullptr, true /*ChildrenOnDemand*/, xRet.get());
        rTreeView.set_image(*xRet, _rQueryImage);
        rTreeView.set_text_emphasis(*xRet, false, 0);
    }

    // the child for the tables container
    {
        DBTreeListUserData* pTablesData = new DBTreeListUserData;
        pTablesData->eType = etTableContainer;
        sId = OUString::number(reinterpret_cast<sal_uInt64>(pTablesData));

        std::unique_ptr<weld::TreeIter> xRet(rTreeView.make_iterator());
        rTreeView.insert(xDatasourceEntry.get(), -1, &_rTableName, &sId,
                         nullptr, nullptr, true /*ChildrenOnDemand*/, xRet.get());
        rTreeView.set_image(*xRet, _rTableImage);
        rTreeView.set_text_emphasis(*xRet, false, 0);
    }
}

void SbaTableQueryBrowser::initializeTreeModel()
{
    if (m_xDatabaseContext.is())
    {
        OUString aDBImage, aQueriesImage, aTablesImage;
        OUString sQueriesName, sTablesName;

        // fill the model with the names of the registered datasources
        const Sequence<OUString> aDatasourceNames = m_xDatabaseContext->getElementNames();
        for (const OUString& rDatasource : aDatasourceNames)
            implAddDatasource( rDatasource, aDBImage, sQueriesName, aQueriesImage, sTablesName, aTablesImage, SharedConnection() );
    }
}

void SbaTableQueryBrowser::populateTree(const Reference<XNameAccess>& _xNameAccess,
                                        const weld::TreeIter& rParent,
                                        EntryType eEntryType)
{
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    rTreeView.make_unsorted();

    DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(rParent).toUInt64());
    if (pData) // don't ask if the nameaccess is already set see OnExpandEntry views and tables
        pData->xContainer = _xNameAccess;

    try
    {
        const Sequence<OUString> aNames = _xNameAccess->getElementNames();
        for (const OUString& rName : aNames)
        {
            if( !m_pTreeView->GetEntryPosByName(rName, &rParent))
            {
                DBTreeListUserData* pEntryData = new DBTreeListUserData;
                pEntryData->eType = eEntryType;
                if ( eEntryType == etQuery )
                {
                    Reference<XNameAccess> xChild(_xNameAccess->getByName(rName),UNO_QUERY);
                    if ( xChild.is() )
                        pEntryData->eType = etQueryContainer;
                }
                implAppendEntry(&rParent, rName, pEntryData);
            }
        }
    }
    catch(const Exception&)
    {
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::populateTree: could not fill the tree");
    }

    rTreeView.make_sorted();
}

std::unique_ptr<weld::TreeIter> SbaTableQueryBrowser::implAppendEntry(const weld::TreeIter* pParent, const OUString& rName, DBTreeListUserData* pUserData)
{
    EntryType eEntryType = pUserData->eType;

    std::unique_ptr<ImageProvider> xImageProvider(getImageProviderFor(pParent));

    OUString aImage = xImageProvider->getImageId(rName, getDatabaseObjectType(eEntryType));

    OUString sId(OUString::number(reinterpret_cast<sal_uInt64>(pUserData)));
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    std::unique_ptr<weld::TreeIter> xNewEntry(rTreeView.make_iterator());
    rTreeView.insert(pParent, -1, &rName, &sId, nullptr, nullptr, eEntryType == etQueryContainer, xNewEntry.get());
    rTreeView.set_image(*xNewEntry, aImage);
    rTreeView.set_text_emphasis(*xNewEntry, false, 0);

    return xNewEntry;
}

IMPL_LINK(SbaTableQueryBrowser, OnExpandEntry, const weld::TreeIter&, rParent, bool)
{
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    if (rTreeView.iter_has_child(rParent))
    {
        // nothing to do...
        return true;
    }

    std::unique_ptr<weld::TreeIter> xFirstParent = m_pTreeView->GetRootLevelParent(&rParent);
    OSL_ENSURE(xFirstParent,"SbaTableQueryBrowser::OnExpandEntry: No rootlevelparent!");

    DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(rParent).toInt64());
    assert(pData && "SbaTableQueryBrowser::OnExpandEntry: No user data!");

    if (etTableContainer == pData->eType)
    {
        weld::WaitObject aWaitCursor(getFrameWeld());

        // it could be that we already have a connection
        SharedConnection xConnection;
        ensureConnection(xFirstParent.get(), xConnection);

        if ( xConnection.is() )
        {
            SQLExceptionInfo aInfo;
            try
            {
                Reference< XWarningsSupplier > xWarnings(xConnection, UNO_QUERY);
                if (xWarnings.is())
                    xWarnings->clearWarnings();

                // first insert the views because the tables can also include
                // views but that time the bitmap is the wrong one
                // the nameaccess will be overwritten in populateTree
                Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
                if(xViewSup.is())
                    populateTree( xViewSup->getViews(), rParent, etTableOrView );

                Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
                if(xTabSup.is())
                {
                    populateTree( xTabSup->getTables(), rParent, etTableOrView );
                    Reference<XContainer> xCont(xTabSup->getTables(),UNO_QUERY);
                    if(xCont.is())
                        // add as listener to know when elements are inserted or removed
                        xCont->addContainerListener(this);
                }

                if (xWarnings.is())
                {
#if 0
                    SQLExceptionInfo aWarnings(xWarnings->getWarnings());
// Obviously this if test is always false. So to avoid a Clang warning
// "use of logical '&&' with constant operand" I put this in #if
// 0. Yeah, I know it is fairly likely nobody will ever read this
// comment and make a decision what to do here, so I could as well
// have just binned this...
                    if (aWarnings.isValid() && sal_False)
                    {
                        SQLContext aContext;
                        aContext.Message = DBA_RES(STR_OPENTABLES_WARNINGS);
                        aContext.Details = DBA_RES(STR_OPENTABLES_WARNINGS_DETAILS);
                        aContext.NextException = aWarnings.get();
                        aWarnings = aContext;
                        showError(aWarnings);
                    }
#endif
                    // TODO: we need a better concept for these warnings:
                    // something like "don't show any warnings for this datasource, again" would be nice
                    // But this requires an extension of the InteractionHandler and an additional property on the data source
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
                    SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::OnExpandEntry: something strange happened!");
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION("dbaccess");
            }
            if (aInfo.isValid())
                showError(aInfo);
        }
        else
            return false;
                // 0 indicates that an error occurred
    }
    else
    {
        // we have to expand the queries or bookmarks
        if (ensureEntryObject(rParent))
        {
            DBTreeListUserData* pParentData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(rParent).toUInt64());
            Reference< XNameAccess > xCollection( pParentData->xContainer, UNO_QUERY );
            populateTree(xCollection, rParent, etQuery);
        }
    }
    return true;
}

bool SbaTableQueryBrowser::ensureEntryObject(const weld::TreeIter& rEntry)
{
    EntryType eType = getEntryType(rEntry);

    // the user data of the entry
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    DBTreeListUserData* pEntryData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(rEntry).toUInt64());
    OSL_ENSURE(pEntryData,"ensureEntryObject: user data should already be set!");

    std::unique_ptr<weld::TreeIter> xDataSourceEntry = m_pTreeView->GetRootLevelParent(&rEntry);

    bool bSuccess = false;
    switch (eType)
    {
        case etQueryContainer:
        {
            if ( pEntryData->xContainer.is() )
            {
                // nothing to do
                bSuccess = true;
                break;
            }

            std::unique_ptr<weld::TreeIter> xParent(rTreeView.make_iterator(&rEntry));
            if (rTreeView.iter_parent(*xParent))
            {
                if (rTreeView.iter_compare(*xParent, *xDataSourceEntry) != 0)
                {
                    OUString aName(rTreeView.get_text(rEntry));
                    DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xParent).toUInt64());
                    try
                    {
                        Reference< XNameAccess > xNameAccess(pData->xContainer,UNO_QUERY);
                        if ( xNameAccess.is() )
                            pEntryData->xContainer.set(xNameAccess->getByName(aName),UNO_QUERY);
                    }
                    catch(const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }

                    bSuccess = pEntryData->xContainer.is();
                }
                else
                {
                    try
                    {
                        Reference< XQueryDefinitionsSupplier > xQuerySup;
                        m_xDatabaseContext->getByName(getDataSourceAccessor(*xDataSourceEntry)) >>= xQuerySup;
                        if (xQuerySup.is())
                        {
                            Reference< XNameAccess > xQueryDefs = xQuerySup->getQueryDefinitions();
                            Reference< XContainer > xCont(xQueryDefs, UNO_QUERY);
                            if (xCont.is())
                                // add as listener to get notified if elements are inserted or removed
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
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }
                }
            }
            break;
        }
        default:
            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::ensureEntryObject: ooops ... missing some implementation here!");
            // TODO ...
            break;
    }
    return bSuccess;
}

bool SbaTableQueryBrowser::implSelect(const svx::ODataAccessDescriptor& _rDescriptor, bool _bSelectDirect)
{
    // extract the props
    OUString sDataSource;
    OUString sCommand;
    sal_Int32 nCommandType = CommandType::COMMAND;
    bool bEscapeProcessing = true;
    extractDescriptorProps(_rDescriptor, sDataSource, sCommand, nCommandType, bEscapeProcessing);

    // select it
    return implSelect( sDataSource, sCommand, nCommandType, bEscapeProcessing, SharedConnection(), _bSelectDirect );
}

bool SbaTableQueryBrowser::implLoadAnything(const OUString& _rDataSourceName, const OUString& _rCommand,
    const sal_Int32 nCommandType, const bool _bEscapeProcessing, const SharedConnection& _rxConnection)
{
    try
    {
        Reference<XPropertySet> xProp( getRowSet(), UNO_QUERY_THROW );
        Reference< XLoadable >  xLoadable( xProp, UNO_QUERY_THROW );
        // the values allowing the RowSet to re-execute
        xProp->setPropertyValue(PROPERTY_DATASOURCENAME, makeAny(_rDataSourceName));
        if(_rxConnection.is())
            xProp->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, makeAny( _rxConnection.getTyped() ) );

            // set this _before_ setting the connection, else the rowset would rebuild it ...
        xProp->setPropertyValue(PROPERTY_COMMAND_TYPE, makeAny(nCommandType));
        xProp->setPropertyValue(PROPERTY_COMMAND, makeAny(_rCommand));
        xProp->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, css::uno::makeAny(_bEscapeProcessing));
        if ( m_bPreview )
        {
            xProp->setPropertyValue(PROPERTY_FETCHDIRECTION, makeAny(FetchDirection::FORWARD));
        }

        // the formatter depends on the data source we're working on, so rebuild it here ...
        initFormatter();

        // switch the grid to design mode while loading
        getBrowserView()->getGridControl()->setDesignMode(true);
        InitializeForm( xProp );

        bool bSuccess = true;

        {
            {
                Reference< XNameContainer >  xColContainer(getFormComponent(), UNO_QUERY);
                // first we have to clear the grid
                clearGridColumns(xColContainer);
            }
            FormErrorHelper aHelper(this);
            // load the form
            bSuccess = reloadForm(xLoadable);

            // initialize the model
            InitializeGridModel(getFormComponent());

            Any aVal = xProp->getPropertyValue(PROPERTY_ISNEW);
            if (aVal.hasValue() && ::comphelper::getBOOL(aVal))
            {
                // then set the default values and the parameters given from the parent
                Reference< XReset> xReset(xProp, UNO_QUERY);
                xReset->reset();
            }

            if ( m_bPreview )
                initializePreviewMode();

            LoadFinished(true);
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
        if  ( e.TargetException.isExtractableTo( ::cppu::UnoType< SQLException >::get() ) )
            showError( SQLExceptionInfo( e.TargetException ) );
        else
        {
            TOOLS_WARN_EXCEPTION("dbaccess", "");
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    InvalidateAll();
    return false;
}

bool SbaTableQueryBrowser::implSelect(const OUString& _rDataSourceName, const OUString& _rCommand,
                                      const sal_Int32 nCommandType, const bool _bEscapeProcessing,
                                      const SharedConnection& _rxConnection,
                                      bool _bSelectDirect)
{
    if (_rDataSourceName.getLength() && _rCommand.getLength() && (-1 != nCommandType))
    {
        std::unique_ptr<weld::TreeIter> xDataSource;
        std::unique_ptr<weld::TreeIter> xCommandType;
        std::unique_ptr<weld::TreeIter> xCommand = getObjectEntry( _rDataSourceName, _rCommand, nCommandType, &xDataSource, &xCommandType, true, _rxConnection );

        if (xCommand)
        {
            weld::TreeView& rTreeView = m_pTreeView->GetWidget();

            bool bSuccess = true;
            if ( _bSelectDirect )
            {
                bSuccess = implSelect(xCommand.get());
            }
            else
            {
                rTreeView.select(*xCommand);
            }

            if ( bSuccess )
            {
                rTreeView.scroll_to_row(*xCommand);
                rTreeView.set_cursor(*xCommand);
            }
        }
        else if (!xCommandType)
        {
            if (m_xCurrentlyDisplayed)
            {
                // tell the old entry (if any) it has been deselected
                selectPath(m_xCurrentlyDisplayed.get(), false);
                m_xCurrentlyDisplayed.reset();
            }

            // we have a command and need to display this in the rowset
            return implLoadAnything(_rDataSourceName, _rCommand, nCommandType, _bEscapeProcessing, _rxConnection);
        }
    }
    return false;
}

IMPL_LINK_NOARG(SbaTableQueryBrowser, OnSelectionChange, LinkParamNone*, void)
{
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    std::unique_ptr<weld::TreeIter> xSelection(rTreeView.make_iterator());
    if (!rTreeView.get_selected(xSelection.get()))
        xSelection.reset();
    implSelect(xSelection.get());
}

std::unique_ptr<weld::TreeIter> SbaTableQueryBrowser::implGetConnectionEntry(const weld::TreeIter& rEntry) const
{
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    std::unique_ptr<weld::TreeIter> xCurrentEntry(rTreeView.make_iterator(&rEntry));
    DBTreeListUserData* pEntryData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xCurrentEntry).toInt64());
    while (pEntryData->eType != etDatasource)
    {
        rTreeView.iter_parent(*xCurrentEntry);
        pEntryData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xCurrentEntry).toInt64());
    }
    return xCurrentEntry;
}

bool SbaTableQueryBrowser::implSelect(const weld::TreeIter* pEntry)
{
    if ( !pEntry )
        return false;

    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    DBTreeListUserData* pEntryData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*pEntry).toUInt64());
    switch (pEntryData->eType)
    {
        case etTableOrView:
        case etQuery:
            break;
        default:
            // nothing to do
            return false;
    }

    OSL_ENSURE(rTreeView.get_iter_depth(*pEntry) >= 2, "SbaTableQueryBrowser::implSelect: invalid entry!");

    // get the entry for the tables or queries
    std::unique_ptr<weld::TreeIter> xContainer = rTreeView.make_iterator(pEntry);
    rTreeView.iter_parent(*xContainer);
    DBTreeListUserData* pContainerData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xContainer).toUInt64());

    // get the entry for the datasource
    std::unique_ptr<weld::TreeIter> xConnection = implGetConnectionEntry(*xContainer);
    DBTreeListUserData* pConData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xConnection).toUInt64());

    // reinitialize the rowset
    // but first check if it is necessary
    // get all old properties
    Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
    OUString aOldName;
    xRowSetProps->getPropertyValue(PROPERTY_COMMAND) >>= aOldName;
    sal_Int32 nOldType = 0;
    xRowSetProps->getPropertyValue(PROPERTY_COMMAND_TYPE) >>= nOldType;
    Reference<XConnection> xOldConnection(xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),UNO_QUERY);

    // the name of the table or query
    const OUString sSimpleName = rTreeView.get_text(*pEntry);
    OUStringBuffer sNameBuffer(sSimpleName);
    if ( etQueryContainer == pContainerData->eType )
    {
        std::unique_ptr<weld::TreeIter> xTemp = rTreeView.make_iterator(xContainer.get());
        std::unique_ptr<weld::TreeIter> xNextTemp = rTreeView.make_iterator(xTemp.get());
        if (rTreeView.iter_parent(*xNextTemp))
        {
            while (rTreeView.iter_compare(*xNextTemp, *xConnection) != 0)
            {
                sNameBuffer.insert(0,'/');
                sNameBuffer.insert(0, rTreeView.get_text(*xTemp));
                rTreeView.copy_iterator(*xNextTemp, *xTemp);
                if (!rTreeView.iter_parent(*xNextTemp))
                    break;
            }
        }
    }
    OUString aName = sNameBuffer.makeStringAndClear();

    sal_Int32 nCommandType =    ( etTableContainer == pContainerData->eType)
                            ?   CommandType::TABLE
                            :   CommandType::QUERY;

    // check if need to rebuild the rowset
    bool bRebuild = ( xOldConnection != pConData->xConnection )
                     || ( nOldType != nCommandType )
                     || ( aName != aOldName );

    Reference< css::form::XLoadable >  xLoadable = getLoadable();
    bRebuild |= !xLoadable->isLoaded();
    bool bSuccess = true;
    if ( bRebuild )
    {
        try
        {
            weld::WaitObject aWaitCursor(getFrameWeld());

            // tell the old entry it has been deselected
            selectPath(m_xCurrentlyDisplayed.get(), false);
            m_xCurrentlyDisplayed.reset();

            // not really loaded
            m_xCurrentlyDisplayed = rTreeView.make_iterator(pEntry);
            // tell the new entry it has been selected
            selectPath(m_xCurrentlyDisplayed.get());

            // get the name of the data source currently selected
            ensureConnection(m_xCurrentlyDisplayed.get(), pConData->xConnection);

            if ( !pConData->xConnection.is() )
            {
                unloadAndCleanup( false );
                return false;
            }

            Reference<XNameAccess> xNameAccess;
            switch(nCommandType)
            {
                case CommandType::TABLE:
                    {
                        // only for tables
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
            OUString sStatus(DBA_RES(CommandType::TABLE == nCommandType ? STR_LOADING_TABLE : STR_LOADING_QUERY));
            sStatus = sStatus.replaceFirst("$name$", aName);
            BrowserViewStatusDisplay aShowStatus(static_cast<UnoDataBrowserView*>(getView()), sStatus);

            bool bEscapeProcessing = true;
            if(xNameAccess.is() && xNameAccess->hasByName(sSimpleName))
            {
                DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*pEntry).toUInt64());
                if ( !pData->xObjectProperties.is() )
                {
                    Reference<XInterface> xObject;
                    if(xNameAccess->getByName(sSimpleName) >>= xObject) // remember the table or query object
                    {
                        pData->xObjectProperties.set(xObject, css::uno::UNO_QUERY);
                        // if the query contains a parameterized statement and preview is enabled we won't get any data.
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
                                                OUString sFilter = " WHERE " + xAnalyzer->getFilter();
                                                OUString sReplace = sSql.replaceFirst(sFilter, "");
                                                xAnalyzer->setQuery(sReplace);
                                                Reference<XSingleSelectQueryComposer> xComposer(xAnalyzer,UNO_QUERY);
                                                xComposer->setFilter("0=1");
                                                aName = xAnalyzer->getQuery();
                                                nCommandType = CommandType::COMMAND;
                                            }
                                        }
                                    }
                                    catch (Exception&)
                                    {
                                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                                    }
                                }
                            }
                        }
                    }
                }
            }

            OUString sDataSourceName(getDataSourceAccessor(*xConnection));
            bSuccess = implLoadAnything( sDataSourceName, aName, nCommandType, bEscapeProcessing, pConData->xConnection );
            if ( !bSuccess )
            {   // clean up
                criticalFail();
            }
        }
        catch(const SQLException& e)
        {
            showError(SQLExceptionInfo(e));
            // reset the values
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
            bSuccess = false;
        }
        catch(WrappedTargetException& e)
        {
            SQLException aSql;
            if(e.TargetException >>= aSql)
                showError(SQLExceptionInfo(aSql));
            else
                SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::implSelect: something strange happened!");
            // reset the values
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
            bSuccess = false;
        }
        catch(const Exception&)
        {
            // reset the values
            xRowSetProps->setPropertyValue(PROPERTY_DATASOURCENAME,Any());
            xRowSetProps->setPropertyValue(PROPERTY_ACTIVE_CONNECTION,Any());
            bSuccess = false;
        }
    }
    return bSuccess;
}

std::unique_ptr<weld::TreeIter> SbaTableQueryBrowser::getEntryFromContainer(const Reference<XNameAccess>& rxNameAccess)
{
    std::unique_ptr<weld::TreeIter> xContainer;

    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    std::unique_ptr<weld::TreeIter> xDSLoop(rTreeView.make_iterator(xContainer.get()));
    if (rTreeView.get_iter_first(*xDSLoop))
    {
        do
        {
            xContainer = rTreeView.make_iterator(xDSLoop.get());
            if (rTreeView.iter_children(*xContainer))
            {
                // 1st child is queries
                DBTreeListUserData* pQueriesData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xContainer).toUInt64());
                if (pQueriesData && pQueriesData->xContainer == rxNameAccess)
                    break;

                if (rTreeView.iter_next_sibling(*xContainer))
                {
                    // 2nd child is tables
                    DBTreeListUserData* pTablesData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xContainer).toUInt64());
                    if (pTablesData && pTablesData->xContainer == rxNameAccess)
                        break;
                }
            }
            xContainer.reset();
        }
        while (rTreeView.iter_next_sibling(*xDSLoop));
    }

    return xContainer;
}

void SAL_CALL SbaTableQueryBrowser::elementInserted(const ContainerEvent& rEvent)
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(rEvent.Source, UNO_QUERY);
    // first search for a definition container where we can insert this element

    std::unique_ptr<weld::TreeIter> xEntry = getEntryFromContainer(xNames);
    if (xEntry)  // found one
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        rTreeView.make_unsorted();

        // insert the new entry into the tree
        DBTreeListUserData* pContainerData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xEntry).toUInt64());
        OSL_ENSURE(pContainerData, "elementInserted: There must be user data for this type!");

        DBTreeListUserData* pNewData = new DBTreeListUserData;
        bool bIsTable = etTableContainer == pContainerData->eType;
        if ( bIsTable )
        {
            rEvent.Element >>= pNewData->xObjectProperties;// remember the new element
            pNewData->eType = etTableOrView;
        }
        else
        {
            if (rTreeView.iter_n_children(*xEntry) < xNames->getElementNames().getLength() - 1)
            {
                // the item inserts its children on demand, but it has not been expanded yet. So ensure here and
                // now that it has all items
                populateTree(xNames, *xEntry, etQuery);
            }
            pNewData->eType = etQuery;
        }
        implAppendEntry(xEntry.get(), ::comphelper::getString(rEvent.Accessor), pNewData);

        rTreeView.make_sorted();
    }
    else
        SbaXDataBrowserController::elementInserted(rEvent);
}

bool SbaTableQueryBrowser::isCurrentlyDisplayedChanged(std::u16string_view rName, const weld::TreeIter& rContainer)
{
    if (!m_xCurrentlyDisplayed)
        return false;
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    if (getEntryType(*m_xCurrentlyDisplayed) != getChildType(rContainer))
        return false;
    if (rTreeView.get_text(*m_xCurrentlyDisplayed) != rName)
        return false;
    std::unique_ptr<weld::TreeIter> xParent = rTreeView.make_iterator(m_xCurrentlyDisplayed.get());
    return rTreeView.iter_parent(*xParent) && rTreeView.iter_compare(*xParent, rContainer) == 0;
}

void SAL_CALL SbaTableQueryBrowser::elementRemoved( const ContainerEvent& _rEvent )
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    // get the top-level representing the removed data source
    // and search for the queries and tables
    std::unique_ptr<weld::TreeIter> xContainer = getEntryFromContainer(xNames);
    if (xContainer)
    {
        // a query or table has been removed
        OUString aName = ::comphelper::getString(_rEvent.Accessor);

        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        if (isCurrentlyDisplayedChanged(aName, *xContainer))
        {
            // the element displayed currently has been replaced

            // we need to remember the old value
            std::unique_ptr<weld::TreeIter> xTemp = rTreeView.make_iterator(m_xCurrentlyDisplayed.get());

            // unload
            unloadAndCleanup( false ); // don't dispose the connection

            DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xTemp).toUInt64());
            rTreeView.set_id(*xTemp, OUString());
            delete pData; // the data could be null because we have a table which isn't correct
            rTreeView.remove(*xTemp);
        }
        else
        {
            // remove the entry from the model
            std::unique_ptr<weld::TreeIter> xChild(rTreeView.make_iterator(xContainer.get()));
            if (rTreeView.get_iter_first(*xChild))
            {
                do
                {
                    if (rTreeView.get_text(*xChild) == aName)
                    {
                        DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xChild).toUInt64());
                        rTreeView.set_id(*xChild, OUString());
                        delete pData;
                        rTreeView.remove(*xChild);
                        break;
                    }
                } while (rTreeView.iter_next_sibling(*xChild));
            }
        }

        // maybe the object which is part of the document data source has been removed
        checkDocumentDataSource();
    }
    else
        SbaXDataBrowserController::elementRemoved(_rEvent);
}

void SAL_CALL SbaTableQueryBrowser::elementReplaced( const ContainerEvent& _rEvent )
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    std::unique_ptr<weld::TreeIter> xContainer = getEntryFromContainer(xNames);
    if (xContainer)
    {
        // a table or query as been replaced
        OUString aName = ::comphelper::getString(_rEvent.Accessor);

        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        if (isCurrentlyDisplayedChanged(aName, *xContainer))
        {   // the element displayed currently has been replaced

            // we need to remember the old value
            std::unique_ptr<weld::TreeIter> xTemp = rTreeView.make_iterator(m_xCurrentlyDisplayed.get());
            unloadAndCleanup( false ); // don't dispose the connection

            DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xTemp).toUInt64());
            if (pData)
            {
                if ( etTableOrView == pData->eType )
                {
                    // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                    _rEvent.Element >>= pData->xObjectProperties;  // remember the new element
                }
                else
                {
                    rTreeView.set_id(*xTemp, OUString());
                    delete pData;
                }
            }
        }
        else
        {
            // find the entry for this name
            std::unique_ptr<weld::TreeIter> xChild(rTreeView.make_iterator(xContainer.get()));
            if (rTreeView.get_iter_first(*xChild))
            {
                do
                {
                    if (rTreeView.get_text(*xChild) == aName)
                    {
                        DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xChild).toUInt64());
                        if (pData)
                        {
                            if ( etTableOrView == pData->eType )
                            {
                                // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                                _rEvent.Element >>= pData->xObjectProperties;   // remember the new element
                            }
                            else
                            {
                                rTreeView.set_id(*xChild, OUString());
                                delete pData;
                            }
                        }
                        break;
                    }
                } while (rTreeView.iter_next_sibling(*xChild));
            }
        }

        // maybe the object which is part of the document data source has been removed
        checkDocumentDataSource();
    }
    else if (xNames.get() == m_xDatabaseContext.get())
    {   // a datasource has been replaced in the context
        SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::elementReplaced: no support for replaced data sources!");
            // very suspicious: the database context should not allow to replace data source, only to register
            // and revoke them
    }
    else
        SbaXDataBrowserController::elementReplaced(_rEvent);
}

void SbaTableQueryBrowser::impl_releaseConnection( SharedConnection& _rxConnection )
{
    // remove as event listener
    Reference< XComponent > xComponent( _rxConnection, UNO_QUERY );
    if ( xComponent.is() )
    {
        Reference< XEventListener > xListener( static_cast< ::cppu::OWeakObject* >( this ), UNO_QUERY );
        xComponent->removeEventListener( xListener );
    }

    try
    {
        // temporary (hopefully!) hack for #i55274#
        Reference< XFlushable > xFlush( _rxConnection, UNO_QUERY );
        if ( xFlush.is() )
            xFlush->flush();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    // clear
    _rxConnection.clear();
        // will implicitly dispose if we have the ownership, since xConnection is a SharedConnection
}

void SbaTableQueryBrowser::disposeConnection(const weld::TreeIter* pDSEntry)
{
    OSL_ENSURE( pDSEntry, "SbaTableQueryBrowser::disposeConnection: invalid entry (NULL)!" );
    OSL_ENSURE( impl_isDataSourceEntry( pDSEntry ), "SbaTableQueryBrowser::disposeConnection: invalid entry (not top-level)!" );

    if (pDSEntry)
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        DBTreeListUserData* pTreeListData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*pDSEntry).toUInt64());
        if (pTreeListData)
            impl_releaseConnection(pTreeListData->xConnection);
    }
}

void SbaTableQueryBrowser::closeConnection(const weld::TreeIter& rDSEntry, bool _bDisposeConnection)
{
    OSL_ENSURE(impl_isDataSourceEntry(&rDSEntry), "SbaTableQueryBrowser::closeConnection: invalid entry (not top-level)!");

    weld::TreeView& rTreeView = m_pTreeView->GetWidget();

    // if one of the entries of the given DS is displayed currently, unload the form
    if (m_xCurrentlyDisplayed)
    {
        std::unique_ptr<weld::TreeIter> xRoot = m_pTreeView->GetRootLevelParent(m_xCurrentlyDisplayed.get());
        if (rTreeView.iter_compare(*xRoot, rDSEntry) == 0)
            unloadAndCleanup(_bDisposeConnection);
    }

    // collapse the query/table container
    std::unique_ptr<weld::TreeIter> xContainers(rTreeView.make_iterator(&rDSEntry));
    if (rTreeView.iter_children(*xContainers))
    {
        do
        {
            std::unique_ptr<weld::TreeIter> xElements(rTreeView.make_iterator(xContainers.get()));
            if (rTreeView.iter_children(*xElements))
            {
                rTreeView.collapse_row(*xContainers);
                // and delete their children (they are connection-relative)
                bool bElements = true;
                while (bElements)
                {
                    std::unique_ptr<weld::TreeIter> xRemove(rTreeView.make_iterator(xElements.get()));
                    bElements = rTreeView.iter_next_sibling(*xElements);
                    DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xRemove).toUInt64());
                    rTreeView.set_id(*xRemove, OUString());
                    delete pData;
                    rTreeView.remove(*xRemove);
                }
            }
        }
        while (rTreeView.iter_next_sibling(*xContainers));
    }

    // collapse the entry itself
    rTreeView.collapse_row(rDSEntry);

    // dispose/reset the connection
    if ( _bDisposeConnection )
        disposeConnection(&rDSEntry);
}

void SbaTableQueryBrowser::unloadAndCleanup( bool _bDisposeConnection )
{
    if (!m_xCurrentlyDisplayed)
        // nothing to do
        return;

    std::unique_ptr<weld::TreeIter> xDSEntry = m_pTreeView->GetRootLevelParent(m_xCurrentlyDisplayed.get());

    // de-select the path for the currently displayed table/query
    selectPath(m_xCurrentlyDisplayed.get(), false);
    m_xCurrentlyDisplayed.reset();

    try
    {
        // get the active connection. We need to dispose it.

        // unload the form
        Reference< XLoadable > xLoadable = getLoadable();
        if (xLoadable->isLoaded())
            xLoadable->unload();

        // clear the grid control
        Reference< XNameContainer > xConta(getControlModel(),UNO_QUERY);
        clearGridColumns(xConta);

        // dispose the connection
        if(_bDisposeConnection)
            disposeConnection(xDSEntry.get());
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
            SAL_WARN("dbaccess.ui", "SbaTableQueryBrowser::unloadAndCleanup: something strange happened!");
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
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        return xDataSource;
    }
}

void SbaTableQueryBrowser::impl_initialize()
{
    SolarMutexGuard aGuard;
        // doin' a lot of VCL stuff here -> lock the SolarMutex

    // first initialize the parent
    SbaXDataBrowserController::impl_initialize();

    Reference<XConnection> xForeignConnection;
    Reference< XFrame > xFrame;

    OUString aTableName, aCatalogName, aSchemaName;

    bool bEscapeProcessing = true;
    sal_Int32 nInitialDisplayCommandType = CommandType::COMMAND;
    OUString sInitialDataSourceName;
    OUString sInitialCommand;

    const NamedValueCollection& rArguments( getInitParams() );

    rArguments.get_ensureType( PROPERTY_DATASOURCENAME, sInitialDataSourceName );
    rArguments.get_ensureType( PROPERTY_COMMAND_TYPE, nInitialDisplayCommandType );
    rArguments.get_ensureType( PROPERTY_COMMAND, sInitialCommand );
    rArguments.get_ensureType( PROPERTY_ACTIVE_CONNECTION, xForeignConnection );
    rArguments.get_ensureType( PROPERTY_UPDATE_CATALOGNAME, aCatalogName );
    rArguments.get_ensureType( PROPERTY_UPDATE_SCHEMANAME, aSchemaName );
    rArguments.get_ensureType( PROPERTY_UPDATE_TABLENAME, aTableName );
    rArguments.get_ensureType( PROPERTY_ESCAPE_PROCESSING, bEscapeProcessing );
    rArguments.get_ensureType( "Frame", xFrame );
    rArguments.get_ensureType( PROPERTY_SHOWMENU, m_bShowMenu );

    // disable the browser if either of ShowTreeViewButton (compatibility name) or EnableBrowser
    // is present and set to FALSE
    bool bDisableBrowser =  !rArguments.getOrDefault( "ShowTreeViewButton", true )   // compatibility name
                            ||  !rArguments.getOrDefault( PROPERTY_ENABLE_BROWSER, true );
    OSL_ENSURE( !rArguments.has( "ShowTreeViewButton" ),
        "SbaTableQueryBrowser::impl_initialize: ShowTreeViewButton is superseded by EnableBrowser!" );
    m_bEnableBrowser = !bDisableBrowser;

    // hide the tree view it is disabled in general, or if the settings tell to hide it initially
    bool bHideTreeView =    ( !m_bEnableBrowser )
                            ||  !rArguments.getOrDefault( "ShowTreeView", true )  // compatibility name
                            ||  !rArguments.getOrDefault( PROPERTY_SHOW_BROWSER, true );
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
            Sequence< OUString> aProperties
            {
                "AlwaysShowCursor", PROPERTY_BORDER, "HasNavigationBar", "HasRecordMarker", "Tabstop"
            };
            Sequence< Any> aValues
            {
                Any(false), Any(sal_Int16(0)), Any(false), Any(false), Any(false)
            };
            Reference< XMultiPropertySet >  xFormMultiSet(getFormComponent(), UNO_QUERY);
            if ( xFormMultiSet.is() )
                xFormMultiSet->setPropertyValues(aProperties, aValues);
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }

    // are we loaded into a (sub)frame of an embedded document (i.e. a form belonging to a database
    // document)?
    bool bSubFrameOfEmbeddedDocument = false;
    if ( xFrame.is() )
    {
        Reference<XFramesSupplier> xSup = xFrame->getCreator();
        Reference<XController> xCont = xSup.is() ? xSup->getController() : Reference<XController>();

        bSubFrameOfEmbeddedDocument = xCont.is() && ::dbtools::isEmbeddedInDatabase( xCont->getModel(), xForeignConnection );
    }

    // if we have a connection at this point, it was either passed from outside, our
    // determined from an outer DB document. In both cases, do not dispose it later on.
    SharedConnection xConnection( xForeignConnection, SharedConnection::NoTakeOwnership );

    // should we display all registered databases in the left hand side tree?
    // or only *one* special?
    bool bLimitedTreeEntries = false;
    // if we're part of a frame which is a secondary frame of a database document, then only
    // display the database for this document, not all registered ones
    bLimitedTreeEntries |= bSubFrameOfEmbeddedDocument;
    // if the tree view is not to be displayed at all, then only display the data source
    // which was given as initial selection
    bLimitedTreeEntries |= !m_bEnableBrowser;

    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    rTreeView.make_unsorted();

    if ( bLimitedTreeEntries )
    {
        if ( xConnection.is() )
        {
            startConnectionListening( xConnection );

            // if no initial name was given, try to obtain one from the data source
            if ( sInitialDataSourceName.isEmpty() )
            {
                Reference< XChild > xChild( xConnection, UNO_QUERY );
                Reference< XPropertySet > xDataSourceProperties;
                if ( xChild.is() )
                    xDataSourceProperties.set(xChild->getParent(), css::uno::UNO_QUERY);
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

        std::unique_ptr<weld::TreeIter> xFirst(rTreeView.make_iterator());
        if (rTreeView.get_iter_first(*xFirst))
            rTreeView.expand_row(*xFirst);
    }
    else
        initializeTreeModel();

    rTreeView.make_sorted();

    if ( m_bEnableBrowser )
    {
        m_aDocScriptSupport = ::std::optional< bool >( false );
    }
    else
    {
        // we are not used as "browser", but as mere view for a single table/query/command. In particular,
        // there is a specific database document which we belong to.
        Reference< XOfficeDatabaseDocument > xDocument( getDataSourceOrModel(
            lcl_getDataSource( m_xDatabaseContext, sInitialDataSourceName, xConnection ) ), UNO_QUERY );
        m_aDocScriptSupport = ::std::optional< bool >( Reference< XEmbeddedScripts >( xDocument, UNO_QUERY ).is() );
    }

    if ( implSelect( sInitialDataSourceName, sInitialCommand, nInitialDisplayCommandType, bEscapeProcessing, xConnection, true ) )
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

bool SbaTableQueryBrowser::haveExplorer() const
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

bool SbaTableQueryBrowser::ensureConnection(const weld::TreeIter* pAnyEntry, SharedConnection& rConnection)
{
    std::unique_ptr<weld::TreeIter> xDSEntry = m_pTreeView->GetRootLevelParent(pAnyEntry);
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    DBTreeListUserData* pDSData =
                xDSEntry
            ?   reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xDSEntry).toUInt64())
            :   nullptr;

    return ensureConnection(xDSEntry.get(), pDSData, rConnection);
}

std::unique_ptr< ImageProvider > SbaTableQueryBrowser::getImageProviderFor(const weld::TreeIter* pAnyEntry)
{
    std::unique_ptr<ImageProvider> xImageProvider(new ImageProvider);
    SharedConnection xConnection;
    if (getExistentConnectionFor(pAnyEntry, xConnection))
        xImageProvider.reset(new ImageProvider(xConnection));
    return xImageProvider;
}

bool SbaTableQueryBrowser::getExistentConnectionFor(const weld::TreeIter* pAnyEntry, SharedConnection& rConnection)
{
    std::unique_ptr<weld::TreeIter> xDSEntry = m_pTreeView->GetRootLevelParent(pAnyEntry);
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    DBTreeListUserData* pDSData =
                xDSEntry
            ?   reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xDSEntry).toUInt64())
            :   nullptr;
    if (pDSData)
        rConnection = pDSData->xConnection;
    return rConnection.is();
}

bool SbaTableQueryBrowser::impl_isDataSourceEntry(const weld::TreeIter* pEntry) const
{
    if (!pEntry)
        return false;
    std::unique_ptr<weld::TreeIter> xRoot(m_pTreeView->GetRootLevelParent(pEntry));
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    return rTreeView.iter_compare(*xRoot, *pEntry) == 0;
}

bool SbaTableQueryBrowser::ensureConnection(const weld::TreeIter* pDSEntry, void* pDSData, SharedConnection& rConnection)
{
    OSL_ENSURE( impl_isDataSourceEntry( pDSEntry ), "SbaTableQueryBrowser::ensureConnection: this entry does not denote a data source!" );
    if (pDSEntry)
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        OUString aDSName = rTreeView.get_text(*pDSEntry);

        DBTreeListUserData* pTreeListData = static_cast<DBTreeListUserData*>(pDSData);
        if ( pTreeListData )
            rConnection = pTreeListData->xConnection;

        if ( !rConnection.is() && pTreeListData )
        {
            // show the "connecting to ..." status
            OUString sConnecting(DBA_RES(STR_CONNECTING_DATASOURCE));
            sConnecting = sConnecting.replaceFirst("$name$", aDSName);
            BrowserViewStatusDisplay aShowStatus(static_cast<UnoDataBrowserView*>(getView()), sConnecting);

            // build a string showing context information in case of error
            OUString sConnectingContext(DBA_RES(STR_COULDNOTCONNECT_DATASOURCE));
            sConnectingContext = sConnectingContext.replaceFirst("$name$", aDSName);

            // connect
            rConnection.reset(
                connect(getDataSourceAccessor(*pDSEntry), sConnectingContext, nullptr),
                SharedConnection::TakeOwnership);

            // remember the connection
            pTreeListData->xConnection = rConnection;
        }
    }
    return rConnection.is();
}

int SbaTableQueryBrowser::OnTreeEntryCompare(const weld::TreeIter& rLHS, const weld::TreeIter& rRHS)
{
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();

    // we want the table entry and the end so we have to do a check
    if (isContainer(rRHS))
    {
        // don't use getEntryType (directly or indirectly) for the LHS:
        // LHS is currently being inserted, so it is not "completely valid" at the moment

        const EntryType eRight = getEntryType(rRHS);
        if (etTableContainer == eRight)
            // every other container should be placed _before_ the bookmark container
            return -1;

        const OUString sLeft = rTreeView.get_text(rLHS);

        EntryType eLeft = etTableContainer;
        if (DBA_RES(RID_STR_TABLES_CONTAINER) == sLeft)
            eLeft = etTableContainer;
        else if (DBA_RES(RID_STR_QUERIES_CONTAINER) == sLeft)
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

    OUString sLeftText = rTreeView.get_text(rLHS);
    OUString sRightText = rTreeView.get_text(rRHS);

    sal_Int32 nCompareResult = 0;   // equal by default

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
        // default behaviour if we do not have a collator -> do the simple string compare
        nCompareResult = sLeftText.compareTo(sRightText);

    return nCompareResult;
}

void SbaTableQueryBrowser::implAdministrate(const weld::TreeIter& rApplyTo)
{
    try
    {
        // get the desktop object
        Reference< XDesktop2 > xFrameLoader = Desktop::create( getORB() );

        // the initial selection
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        std::unique_ptr<weld::TreeIter> xTopLevelSelected(rTreeView.make_iterator(&rApplyTo));

        while (rTreeView.get_iter_depth(*xTopLevelSelected))
            rTreeView.iter_parent(*xTopLevelSelected);

        OUString sInitialSelection = getDataSourceAccessor(*xTopLevelSelected);

        Reference< XDataSource > xDataSource( getDataSourceByName( sInitialSelection, getFrameWeld(), getORB(), nullptr ) );
        Reference< XModel > xDocumentModel( getDataSourceOrModel( xDataSource ), UNO_QUERY );

        if ( xDocumentModel.is() )
        {
            Reference< XInteractionHandler2 > xInteractionHandler(
                InteractionHandler::createWithParent(getORB(), nullptr) );

            ::comphelper::NamedValueCollection aLoadArgs;
            aLoadArgs.put( "Model", xDocumentModel );
            aLoadArgs.put( "InteractionHandler", xInteractionHandler );
            aLoadArgs.put( "MacroExecutionMode", MacroExecMode::USE_CONFIG );

            Sequence< PropertyValue > aLoadArgPV;
            aLoadArgs >>= aLoadArgPV;

            xFrameLoader->loadComponentFromURL(
                xDocumentModel->getURL(),
                "_default",
                FrameSearchFlag::ALL | FrameSearchFlag::GLOBAL,
                aLoadArgPV
            );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

bool SbaTableQueryBrowser::requestQuickHelp(const void* pUserData, OUString& rText) const
{
    const DBTreeListUserData* pData = static_cast<const DBTreeListUserData*>(pUserData);
    if (pData->eType == etDatasource && !pData->sAccessor.isEmpty())
    {
        rText = ::svt::OFileNotation(pData->sAccessor).get( ::svt::OFileNotation::N_SYSTEM);
        return true;
    }
    return false;
}

OUString SbaTableQueryBrowser::getContextMenuResourceName() const
{
    return "explorer";
}

IController& SbaTableQueryBrowser::getCommandController()
{
    return *this;
}

::comphelper::OInterfaceContainerHelper2* SbaTableQueryBrowser::getContextMenuInterceptors()
{
    return &m_aContextMenuInterceptors;
}

Any SbaTableQueryBrowser::getCurrentSelection(weld::TreeView& rControl) const
{
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();

    OSL_PRECOND( &rTreeView == &rControl,
        "SbaTableQueryBrowser::getCurrentSelection: where does this come from?" );

    if (&rTreeView != &rControl)
        return Any();

    std::unique_ptr<weld::TreeIter> xSelected(rTreeView.make_iterator());
    if (!rTreeView.get_selected(xSelected.get()))
        return Any();

    NamedDatabaseObject aSelectedObject;
    DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xSelected).toUInt64());
    aSelectedObject.Type = static_cast< sal_Int32 >( pData->eType );

    switch ( aSelectedObject.Type )
    {
    case DatabaseObject::QUERY:
    case DatabaseObject::TABLE:
        aSelectedObject.Name = rTreeView.get_text(*xSelected);
        break;

    case DatabaseObjectContainer::DATA_SOURCE:
    case DatabaseObjectContainer::QUERIES:
    case DatabaseObjectContainer::TABLES:
        aSelectedObject.Name = getDataSourceAccessor(*xSelected);
        break;

    default:
        SAL_WARN("dbaccess.ui",  "SbaTableQueryBrowser::getCurrentSelection: invalid (unexpected) object type!" );
        break;
    }

    return makeAny( aSelectedObject );
}

vcl::Window* SbaTableQueryBrowser::getMenuParent() const
{
    return m_pTreeView;
}

void SbaTableQueryBrowser::adjustMenuPosition(const weld::TreeView&, ::Point&) const
{
}

bool SbaTableQueryBrowser::implGetQuerySignature( OUString& _rCommand, bool& _bEscapeProcessing )
{
    _rCommand.clear();
    _bEscapeProcessing = false;

    try
    {
        // contain the dss (data source signature) of the form
        OUString sDataSourceName;
        OUString sCommand;
        sal_Int32       nCommandType = CommandType::COMMAND;
        Reference< XPropertySet > xRowsetProps( getRowSet(), UNO_QUERY );
        ODataAccessDescriptor aDesc( xRowsetProps );
        sDataSourceName = aDesc.getDataSource();
        aDesc[ DataAccessDescriptorProperty::Command ]      >>= sCommand;
        aDesc[ DataAccessDescriptorProperty::CommandType ]  >>= nCommandType;

        // do we need to do anything?
        if ( CommandType::QUERY != nCommandType )
            return false;

        // get the query object
        Reference< XQueryDefinitionsSupplier > xSuppQueries;
        Reference< XNameAccess > xQueries;
        Reference< XPropertySet > xQuery;
        m_xDatabaseContext->getByName( sDataSourceName ) >>= xSuppQueries;
        if ( xSuppQueries.is() )
            xQueries = xSuppQueries->getQueryDefinitions();
        if ( xQueries.is() )
            xQueries->getByName( sCommand ) >>= xQuery;
        OSL_ENSURE( xQuery.is(), "SbaTableQueryBrowser::implGetQuerySignature: could not retrieve the query object!" );

        // get the two properties we need
        if ( xQuery.is() )
        {
            xQuery->getPropertyValue( PROPERTY_COMMAND ) >>= _rCommand;
            _bEscapeProcessing = ::cppu::any2bool( xQuery->getPropertyValue( PROPERTY_ESCAPE_PROCESSING ) );
            return true;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    return false;
}

void SbaTableQueryBrowser::frameAction(const css::frame::FrameActionEvent& aEvent)
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
    // first we have to clear the grid
    Reference< XInterface > xColumn;
    const Sequence<OUString> aColNames = _xColContainer->getElementNames();
    for (const OUString& rName : aColNames)
    {
        _xColContainer->getByName(rName) >>= xColumn;
        _xColContainer->removeByName(rName);
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
        Reference< css::frame::XLayoutManager > xLayoutManager = getLayoutManager(_xFrame);

        if ( xLayoutManager.is() )
        {
            xLayoutManager->lock();
            xLayoutManager->createElement( "private:resource/toolbar/toolbar" );
            xLayoutManager->unlock();
            xLayoutManager->doLayout();
        }
        onLoadedMenu( xLayoutManager );
    }
}

OUString SbaTableQueryBrowser::getPrivateTitle() const
{
    OUString sTitle;
    if (m_xCurrentlyDisplayed)
    {
        weld::TreeView& rTreeView = m_pTreeView->GetWidget();
        std::unique_ptr<weld::TreeIter> xContainer = rTreeView.make_iterator(m_xCurrentlyDisplayed.get());
        if (!rTreeView.iter_parent(*xContainer))
            return OUString();
        // get the entry for the datasource
        std::unique_ptr<weld::TreeIter> xConnection = implGetConnectionEntry(*xContainer);
        OUString sName = rTreeView.get_text(*m_xCurrentlyDisplayed);
        sTitle = GetEntryText(*xConnection);
        INetURLObject aURL(sTitle);
        if ( aURL.GetProtocol() != INetProtocol::NotValid )
            sTitle = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::WithCharset);
        if ( !sName.isEmpty() )
        {
            sName += " - " + sTitle;
            sTitle = sName;
        }
    }

    return sTitle;
}

bool SbaTableQueryBrowser::preReloadForm()
{
    bool bIni = false;
    if (!m_xCurrentlyDisplayed)
    {
        // switch the grid to design mode while loading
        getBrowserView()->getGridControl()->setDesignMode(true);
        // we had an invalid statement so we need to connect the column models
        Reference<XPropertySet> xRowSetProps(getRowSet(),UNO_QUERY);
        svx::ODataAccessDescriptor aDesc(xRowSetProps);
        // extract the props
        OUString sDataSource;
        OUString sCommand;
        sal_Int32 nCommandType = CommandType::COMMAND;
        bool bEscapeProcessing = true;
        extractDescriptorProps(aDesc, sDataSource, sCommand, nCommandType, bEscapeProcessing);
        if ( !sDataSource.isEmpty() && !sCommand.isEmpty() && (-1 != nCommandType) )
        {
            m_xCurrentlyDisplayed = getObjectEntry(sDataSource, sCommand, nCommandType, nullptr, nullptr);
            bIni = true;
        }
    }
    return bIni;
}

void SbaTableQueryBrowser::postReloadForm()
{
    InitializeGridModel(getFormComponent());
    LoadFinished(true);
}

Reference< XEmbeddedScripts > SAL_CALL SbaTableQueryBrowser::getScriptContainer()
{
    // update our database document
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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    Reference< XEmbeddedScripts > xScripts( xDocument, UNO_QUERY );
    OSL_ENSURE( xScripts.is() || !xDocument.is(),
        "SbaTableQueryBrowser::getScriptContainer: invalid database document!" );
    return xScripts;
}

void SAL_CALL SbaTableQueryBrowser::registerContextMenuInterceptor( const Reference< XContextMenuInterceptor >& Interceptor )
{
    if ( Interceptor.is() )
        m_aContextMenuInterceptors.addInterface( Interceptor );
}

void SAL_CALL SbaTableQueryBrowser::releaseContextMenuInterceptor( const Reference< XContextMenuInterceptor >& Interceptor )
{
    if ( Interceptor.is() )
        m_aContextMenuInterceptors.removeInterface( Interceptor );
}

void SAL_CALL SbaTableQueryBrowser::registeredDatabaseLocation( const DatabaseRegistrationEvent& Event )
{
    SolarMutexGuard aGuard;
    implAddDatasource( Event.Name, SharedConnection() );
}

void SbaTableQueryBrowser::impl_cleanupDataSourceEntry(std::u16string_view rDataSourceName)
{
    // get the top-level representing the removed data source
    weld::TreeView& rTreeView = m_pTreeView->GetWidget();
    std::unique_ptr<weld::TreeIter> xDataSourceEntry(rTreeView.make_iterator());
    bool bDataSourceEntry = rTreeView.get_iter_first(*xDataSourceEntry);
    while (bDataSourceEntry)
    {
        if (rTreeView.get_text(*xDataSourceEntry) == rDataSourceName)
            break;
        bDataSourceEntry = rTreeView.iter_next_sibling(*xDataSourceEntry);
    }

    OSL_ENSURE( bDataSourceEntry, "SbaTableQueryBrowser::impl_cleanupDataSourceEntry: do not know this data source!" );
    if (!bDataSourceEntry)
        return;

    if (isSelected(*xDataSourceEntry))
    {
        // a table or query belonging to the deleted data source is currently being displayed.
        unloadAndCleanup();
    }

    std::unique_ptr<weld::TreeIter> xChild(rTreeView.make_iterator(xDataSourceEntry.get()));
    if (rTreeView.iter_children(*xChild))
    {
        do
        {
            // delete any user data of the child entries of the to-be-removed entry
            const DBTreeListUserData* pData = reinterpret_cast<const DBTreeListUserData*>(rTreeView.get_id(*xChild).toUInt64());
            rTreeView.set_id(*xChild, OUString());
            delete pData;
        } while (rTreeView.iter_next_sibling(*xChild));
    }

    // remove the entry
    DBTreeListUserData* pData = reinterpret_cast<DBTreeListUserData*>(rTreeView.get_id(*xDataSourceEntry).toUInt64());
    rTreeView.set_id(*xDataSourceEntry, OUString());
    delete pData;
    rTreeView.remove(*xDataSourceEntry);
}

void SAL_CALL SbaTableQueryBrowser::revokedDatabaseLocation( const DatabaseRegistrationEvent& Event )
{
    SolarMutexGuard aGuard;

    impl_cleanupDataSourceEntry( Event.Name );

    // maybe the object which is part of the document data source has been removed
    checkDocumentDataSource();
}

void SAL_CALL SbaTableQueryBrowser::changedDatabaseLocation( const DatabaseRegistrationEvent& Event )
{
    SolarMutexGuard aGuard;

    // in case the data source was expanded, and connected, we need to clean it up
    // for simplicity, just do as if the data source were completely removed and re-added
    impl_cleanupDataSourceEntry( Event.Name );
    implAddDatasource( Event.Name, SharedConnection() );
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

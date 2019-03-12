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
#include <dbexchange.hxx>
#include <dbtreelistbox.hxx>
#include "dbtreemodel.hxx"
#include "dbtreeview.hxx"
#include <dbu_reghelper.hxx>
#include <stringconstants.hxx>
#include <strings.hrc>
#include <dlgsave.hxx>
#include <uiservices.hxx>
#include <HtmlReader.hxx>
#include <imageprovider.hxx>
#include <listviewitems.hxx>
#include <QEnumTypes.hxx>
#include <RtfReader.hxx>
#include <sbagrid.hrc>
#include <sbagrid.hxx>
#include <sqlmessage.hxx>
#include <TokenWriter.hxx>
#include <UITools.hxx>
#include <unodatbr.hxx>
#include <WColumnSelect.hxx>
#include <WCopyTable.hxx>
#include <WCPage.hxx>
#include <WExtendPages.hxx>
#include <WNameMatch.hxx>

#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
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
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/ui/XContextMenuInterceptor.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sot/storage.hxx>
#include <svl/filenotation.hxx>
#include <svl/intitem.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/treelistentry.hxx>
#include <svx/algitem.hxx>
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

// SbaTableQueryBrowser
extern "C" void createRegistryInfo_OBrowser()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::SbaTableQueryBrowser > aAutoRegistration;
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
    return getImplementationName_Static();
}

css::uno::Sequence<OUString> SAL_CALL SbaTableQueryBrowser::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

OUString SbaTableQueryBrowser::getImplementationName_Static()
{
    return OUString("org.openoffice.comp.dbu.ODatasourceBrowser");
}

css::uno::Sequence<OUString> SbaTableQueryBrowser::getSupportedServiceNames_Static()
{
    css::uno::Sequence<OUString> aSupported { "com.sun.star.sdb.DataSourceBrowser" };
    return aSupported;
}

Reference< XInterface > SbaTableQueryBrowser::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    SolarMutexGuard aGuard;
    return *(new SbaTableQueryBrowser(comphelper::getComponentContext(_rxFactory)));
}

SbaTableQueryBrowser::SbaTableQueryBrowser(const Reference< XComponentContext >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_aSelectionListeners( getMutex() )
    ,m_aContextMenuInterceptors( getMutex() )
    ,m_aTableCopyHelper(this)
    ,m_pTreeView(nullptr)
    ,m_pSplitter(nullptr)
    ,m_pCurrentlyDisplayed(nullptr)
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
        auto newEnd = std::remove_if( aTypes.begin(), aTypes.end(),
                                      [](const Type& type)
                                      { return type == cppu::UnoType<XScriptInvocationContext>::get(); } );
        aTypes.realloc( std::distance(aTypes.begin(), newEnd) );
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
        // Need to do some cleaup of the data pointed to the tree view entries before we remove the treeview
        clearTreeModel();
        m_pTreeView = nullptr;
        getBrowserView()->setTreeView(nullptr);
    }

    // remove ourself as status listener
    implRemoveStatusListeners();

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

    // check out from all the objects we are listening
    // the frame
    if (m_xCurrentFrameParent.is())
        m_xCurrentFrameParent->removeFrameActionListener(static_cast<css::frame::XFrameActionListener*>(this));
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
        const long  nFrameWidth = getBrowserView()->LogicToPixel(::Size(3, 0), MapMode(MapUnit::MapAppFont)).Width();

        m_pSplitter = VclPtr<Splitter>::Create(getBrowserView(),WB_HSCROLL);
        m_pSplitter->SetPosSizePixel( ::Point(0,0), ::Size(nFrameWidth,0) );
        m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetDialogColor() ) );

        m_pTreeView = VclPtr<DBTreeView>::Create(getBrowserView(), WB_TABSTOP | WB_BORDER);
        m_pTreeView->SetPreExpandHandler(LINK(this, SbaTableQueryBrowser, OnExpandEntry));

        m_pTreeView->setCopyHandler(LINK(this, SbaTableQueryBrowser, OnCopyEntry));

        m_pTreeView->getListBox().setContextMenuProvider( this );
        m_pTreeView->getListBox().setControlActionListener( this );
        m_pTreeView->SetHelpId(HID_CTL_TREEVIEW);

        // a default pos for the splitter, so that the listbox is about 80 (logical) pixels wide
        m_pSplitter->SetSplitPosPixel(getBrowserView()->LogicToPixel(::Size(80, 0), MapMode(MapUnit::MapAppFont)).Width());

        getBrowserView()->setSplitter(m_pSplitter);
        getBrowserView()->setTreeView(m_pTreeView);

        // fill view with data
        auto pTreeModel = m_pTreeView->GetTreeModel();
        pTreeModel->SetSortMode(SortAscending);
        pTreeModel->SetCompareHdl(LINK(this, SbaTableQueryBrowser, OnTreeEntryCompare));
        m_pTreeView->setSelChangeHdl( LINK( this, SbaTableQueryBrowser, OnSelectionChange ) );

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
        const Reference< XNameAccess > xTableNames( xSuppTables->getTables(), UNO_QUERY_THROW );

        // the columns participating in the statement
        const Reference< XColumnsSupplier > xSuppColumns( xComposer, UNO_QUERY_THROW );
        const Reference< XNameAccess > xColumnNames( xSuppColumns->getColumns(), UNO_QUERY_THROW );

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
        // ... or something like this ....
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

bool SbaTableQueryBrowser::InitializeForm( const Reference< XPropertySet > & i_formProperties )
{
    if(!m_pCurrentlyDisplayed)
        return true;

    // this method set all format settings from the original table or query
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
        for (const auto & aTransferPropertie : aTransferProperties)
        {
            if ( !xPSI->hasPropertyByName( aTransferPropertie ) )
                continue;
            aPropertyValues.put( aTransferPropertie, pData->xObjectProperties->getPropertyValue( aTransferPropertie ) );
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

        Reference< XChild > xGridAsChild(xGrid, UNO_QUERY);
        Reference< XLoadable > xFormAsLoadable;
        if (xGridAsChild.is())
            xFormAsLoadable.set(xGridAsChild->getParent(), css::uno::UNO_QUERY);
        if (xFormAsLoadable.is() && xFormAsLoadable->isLoaded())
        {
            // set the formats from the table
            if(m_pCurrentlyDisplayed)
            {
                Sequence< OUString> aProperties(6 + ( m_bPreview ? 5 : 0 ));
                Sequence< Any> aValues(7 + ( m_bPreview ? 5 : 0 ));

                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
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
            for (const OUString& rName : xColumns->getElementNames())
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

static Reference<XPropertySet> getColumnHelper(SvTreeListEntry const * _pCurrentlyDisplayed, const Reference<XPropertySet>& _rxSource)
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
        else if (evt.PropertyName == PROPERTY_WIDTH)
        {   // a column width has changed -> update the model
            // (the update of the view is done elsewhere)
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
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
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_HIDDEN,evt.NewValue);
        }

        // a columns alignment ?
        else if (evt.PropertyName == PROPERTY_ALIGN)
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
            Reference<XPropertySet> xProp = getColumnHelper(m_pCurrentlyDisplayed,xSource);
            if(xProp.is())
                xProp->setPropertyValue(PROPERTY_FORMATKEY,evt.NewValue);
        }

        // some table definition properties ?
        // the height of the rows in the grid ?
        else if (evt.PropertyName == PROPERTY_ROW_HEIGHT)
        {
            if(m_pCurrentlyDisplayed)
            {
                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(m_pCurrentlyDisplayed->GetUserData());
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
    SvTreeListEntry* pDataSourceEntry = nullptr;
    SvTreeListEntry* pContainerEntry = nullptr;
    SvTreeListEntry* pObjectEntry = getObjectEntry( m_aDocumentDataSource, &pDataSourceEntry, &pContainerEntry );
    bool bKnownDocDataSource = (nullptr != pObjectEntry);
    if (!bKnownDocDataSource)
    {
        if (nullptr != pDataSourceEntry)
        {   // at least the data source is known
            if (nullptr != pContainerEntry)
                bKnownDocDataSource = true; // assume we know it.
                // TODO: should we expand the object container? This may be too expensive just for checking ....
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

        virtual bool    includeEntry( SvTreeListEntry* _pEntry ) const override;
    };

    bool FilterByEntryDataId::includeEntry( SvTreeListEntry* _pEntry ) const
    {
        DBTreeListUserData* pData = static_cast< DBTreeListUserData* >( _pEntry->GetUserData() );
        return ( !pData || ( pData->sAccessor == sId ) );
    }
}

OUString SbaTableQueryBrowser::getDataSourceAccessor( SvTreeListEntry* _pDataSourceEntry ) const
{
    OSL_ENSURE( _pDataSourceEntry, "SbaTableQueryBrowser::getDataSourceAccessor: invalid entry!" );

    DBTreeListUserData* pData = static_cast< DBTreeListUserData* >( _pDataSourceEntry->GetUserData() );
    OSL_ENSURE( pData, "SbaTableQueryBrowser::getDataSourceAccessor: invalid entry data!" );
    OSL_ENSURE( pData->eType == etDatasource, "SbaTableQueryBrowser::getDataSourceAccessor: entry does not denote a data source!" );
    return !pData->sAccessor.isEmpty() ? pData->sAccessor : GetEntryText( _pDataSourceEntry );
}

SvTreeListEntry* SbaTableQueryBrowser::getObjectEntry(const OUString& _rDataSource, const OUString& _rCommand, sal_Int32 _nCommandType,
        SvTreeListEntry** _ppDataSourceEntry, SvTreeListEntry** _ppContainerEntry, bool _bExpandAncestors,
        const SharedConnection& _rxConnection )
{
    if (_ppDataSourceEntry)
        *_ppDataSourceEntry = nullptr;
    if (_ppContainerEntry)
        *_ppContainerEntry = nullptr;

    SvTreeListEntry* pObject = nullptr;
    if ( m_pTreeView )
    {
        // look for the data source entry
        OUString sDisplayName, sDataSourceId;
        bool bIsDataSourceURL = getDataSourceDisplayName_isURL( _rDataSource, sDisplayName, sDataSourceId );
            // the display name may differ from the URL for readability reasons
            // #i33699#

        FilterByEntryDataId aFilter( sDataSourceId );
        SvTreeListEntry* pDataSource = m_pTreeView->getListBox().GetEntryPosByName( sDisplayName, nullptr, &aFilter );
        if ( !pDataSource ) // check if the data source name is a file location
        {
            if ( bIsDataSourceURL )
            {
                // special case, the data source is a URL
                // add new entries to the list box model
                implAddDatasource( _rDataSource, _rxConnection );
                pDataSource = m_pTreeView->getListBox().GetEntryPosByName( sDisplayName, nullptr, &aFilter );
                OSL_ENSURE( pDataSource, "SbaTableQueryBrowser::getObjectEntry: hmm - did not find it again!" );
            }
        }
        if (_ppDataSourceEntry)
            // (caller wants to have it ...)
            *_ppDataSourceEntry = pDataSource;

        if (pDataSource)
        {
            // expand if required so
            if (_bExpandAncestors)
                m_pTreeView->getListBox().Expand(pDataSource);

            // look for the object container
            SvTreeListEntry* pCommandType = nullptr;
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
                // expand if required so
                if (_bExpandAncestors)
                {
                    m_pTreeView->getListBox().Expand(pCommandType);
                }

                // look for the object
                sal_Int32 nIndex = 0;
                do
                {
                    OUString sPath;
                    switch (_nCommandType)
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
                    pObject = m_pTreeView->getListBox().GetEntryPosByName(sPath, pCommandType);
                    pCommandType = pObject;
                    if ( nIndex >= 0 )
                    {
                        if (ensureEntryObject(pObject))
                        {
                            DBTreeListUserData* pParentData = static_cast< DBTreeListUserData* >( pObject->GetUserData() );
                            Reference< XNameAccess > xCollection( pParentData->xContainer, UNO_QUERY );
                            sal_Int32 nIndex2 = nIndex;
                            sPath = _rCommand.getToken( 0, '/', nIndex2 );
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

SvTreeListEntry* SbaTableQueryBrowser::getObjectEntry(const svx::ODataAccessDescriptor& _rDescriptor,
        SvTreeListEntry** _ppDataSourceEntry, SvTreeListEntry** _ppContainerEntry)
{
    // extract the props from the descriptor
    OUString sDataSource;
    OUString sCommand;
    sal_Int32 nCommandType = CommandType::COMMAND;
    bool bEscapeProcessing = true;
    extractDescriptorProps(_rDescriptor, sDataSource, sCommand, nCommandType, bEscapeProcessing);

    return getObjectEntry( sDataSource, sCommand, nCommandType, _ppDataSourceEntry, _ppContainerEntry, false/*_bExpandAncestors*/ );
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
        if ( bHaveDispatcher != pToolbox->IsItemVisible( _nId ) )
            bHaveDispatcher ? pToolbox->ShowItem( _nId ) : pToolbox->HideItem( _nId );
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
            {   // our connection is in dispose so we have to find the entry equal with this connection
                // and close it what means to collapse the entry
                // get the top-level representing the removed data source
                SvTreeListEntry* pDSLoop = m_pTreeView->getListBox().FirstChild(nullptr);
                while (pDSLoop)
                {
                    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pDSLoop->GetUserData());
                    if ( pData && pData->xConnection == xCon )
                    {
                        // we set the connection to null to avoid a second disposing of the connection
                        pData->xConnection.clear();
                        closeConnection(pDSLoop,false);
                        break;
                    }

                    pDSLoop = pDSLoop->NextSibling();
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

            // "toggle explorer" is always enabled (if we have a explorer)
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
            SvTreeListEntry* pCurrentEntry( m_pTreeView->getListBox().GetCurEntry() );
            EntryType eType = getEntryType( pCurrentEntry );
            if ( eType == etUnknown )
                return aReturn;

            SvTreeListEntry* pDataSourceEntry = m_pTreeView->getListBox().GetRootLevelParent( pCurrentEntry );
            DBTreeListUserData* pDSData
                =   pDataSourceEntry
                ?   static_cast< DBTreeListUserData* >( pDataSourceEntry->GetUserData() )
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
                aReturn.bEnabled = getORB().is() && ( pDataSourceEntry != nullptr ) && bHaveEditDatabase;
            }
            else if ( nId == ID_BROWSER_COPY )
            {
                aReturn.bEnabled = isEntryCopyAllowed( pCurrentEntry );
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
            implAdministrate( m_pTreeView->getListBox().GetCurEntry() );
            break;

        case ID_TREE_CLOSE_CONN:
            closeConnection( m_pTreeView->getListBox().GetRootLevelParent( m_pTreeView->getListBox().GetCurEntry() ) );
            break;

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

            SvTreeListEntry* pSelected = m_pCurrentlyDisplayed;
            // unload
            unloadAndCleanup( false );

            // reselect the entry
            if ( pSelected )
            {
                implSelect( pSelected );
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
                            long nIdx = pSelection->FirstSelected();
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
                        if ( aSelection.getLength() )
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
            // if it's not 0, such a async close is already pending
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
    // initialize the names/images if necessary
    if (_rQueryName.isEmpty())
        _rQueryName = DBA_RES(RID_STR_QUERIES_CONTAINER);
    if (_rTableName.isEmpty())
        _rTableName = DBA_RES(RID_STR_TABLES_CONTAINER);

    if (!_rQueryImage)
        _rQueryImage = ImageProvider::getFolderImage( DatabaseObject::QUERY );
    if (!_rTableImage)
        _rTableImage = ImageProvider::getFolderImage( DatabaseObject::TABLE );

    if (!_rDbImage)
        _rDbImage = ImageProvider::getDatabaseImage();

    // add the entry for the data source
    // special handling for data sources denoted by URLs - we do not want to display this ugly URL, do we?
    // #i33699#
    OUString sDSDisplayName, sDataSourceId;
    getDataSourceDisplayName_isURL( _rDbName, sDSDisplayName, sDataSourceId );

    SvTreeListEntry* pDatasourceEntry = m_pTreeView->getListBox().InsertEntry( sDSDisplayName, _rDbImage, _rDbImage );
    DBTreeListUserData* pDSData = new DBTreeListUserData;
    pDSData->eType = etDatasource;
    pDSData->sAccessor = sDataSourceId;
    pDSData->xConnection = _rxConnection;
    pDatasourceEntry->SetUserData(pDSData);

    // the child for the queries container
    {
        DBTreeListUserData* pQueriesData = new DBTreeListUserData;
        pQueriesData->eType = etQueryContainer;

        m_pTreeView->getListBox().InsertEntry(
            _rQueryName, _rQueryImage, _rQueryImage, pDatasourceEntry,
            true /*ChildrenOnDemand*/, TREELIST_APPEND, pQueriesData );
    }

    // the child for the tables container
    {
        DBTreeListUserData* pTablesData = new DBTreeListUserData;
        pTablesData->eType = etTableContainer;

        m_pTreeView->getListBox().InsertEntry(
            _rTableName, _rTableImage, _rTableImage, pDatasourceEntry,
            true /*ChildrenOnDemand*/, TREELIST_APPEND, pTablesData );
    }

}

void SbaTableQueryBrowser::initializeTreeModel()
{
    if (m_xDatabaseContext.is())
    {
        Image aDBImage, aQueriesImage, aTablesImage;
        OUString sQueriesName, sTablesName;

        // fill the model with the names of the registered datasources
        for (const OUString& rDatasource : m_xDatabaseContext->getElementNames())
            implAddDatasource( rDatasource, aDBImage, sQueriesName, aQueriesImage, sTablesName, aTablesImage, SharedConnection() );
    }
}

void SbaTableQueryBrowser::populateTree(const Reference<XNameAccess>& _xNameAccess,
                                            SvTreeListEntry* _pParent,
                                            EntryType _eEntryType)
{
    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(_pParent->GetUserData());
    if(pData) // don't ask if the nameaccess is already set see OnExpandEntry views and tables
        pData->xContainer = _xNameAccess;

    try
    {
        for (const OUString& rName : _xNameAccess->getElementNames())
        {
            if( !m_pTreeView->getListBox().GetEntryPosByName(rName,_pParent))
            {
                DBTreeListUserData* pEntryData = new DBTreeListUserData;
                pEntryData->eType = _eEntryType;
                if ( _eEntryType == etQuery )
                {
                    Reference<XNameAccess> xChild(_xNameAccess->getByName(rName),UNO_QUERY);
                    if ( xChild.is() )
                        pEntryData->eType = etQueryContainer;
                }
                implAppendEntry( _pParent, rName, pEntryData, pEntryData->eType );
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
    std::unique_ptr< ImageProvider > pImageProvider( getImageProviderFor( _pParent ) );

    Image aImage;
    pImageProvider->getImages( _rName, getDatabaseObjectType( _eEntryType ), aImage );

    SvTreeListEntry* pNewEntry = m_pTreeView->getListBox().InsertEntry( _rName, _pParent, _eEntryType == etQueryContainer , TREELIST_APPEND, _pUserData );

    m_pTreeView->getListBox().SetExpandedEntryBmp(  pNewEntry, aImage );
    m_pTreeView->getListBox().SetCollapsedEntryBmp( pNewEntry, aImage );

    return pNewEntry;
}

IMPL_LINK(SbaTableQueryBrowser, OnExpandEntry, SvTreeListEntry*, _pParent, bool)
{
    if (_pParent->HasChildren())
        // nothing to do...
        return true;

    SvTreeListEntry* pFirstParent = m_pTreeView->getListBox().GetRootLevelParent(_pParent);
    OSL_ENSURE(pFirstParent,"SbaTableQueryBrowser::OnExpandEntry: No rootlevelparent!");

    DBTreeListUserData* pData = static_cast< DBTreeListUserData* >(_pParent->GetUserData());
    assert(pData && "SbaTableQueryBrowser::OnExpandEntry: No user data!");
#if OSL_DEBUG_LEVEL > 0
    SvLBoxString* pString = static_cast<SvLBoxString*>(pFirstParent->GetFirstItem(SvLBoxItemType::String));
    OSL_ENSURE(pString,"SbaTableQueryBrowser::OnExpandEntry: No string item!");
#endif

    if (etTableContainer == pData->eType)
    {
        WaitObject aWaitCursor(getBrowserView());

        // it could be that we already have a connection
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

                // first insert the views because the tables can also include
                // views but that time the bitmap is the wrong one
                // the nameaccess will be overwritten in populateTree
                Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
                if(xViewSup.is())
                    populateTree( xViewSup->getViews(), _pParent, etTableOrView );

                Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
                if(xTabSup.is())
                {
                    populateTree( xTabSup->getTables(), _pParent, etTableOrView );
                    Reference<XContainer> xCont(xTabSup->getTables(),UNO_QUERY);
                    if(xCont.is())
                        // add as listener to know when elements are inserted or removed
                        xCont->addContainerListener(this);
                }

                if (xWarnings.is())
                {
                    SQLExceptionInfo aWarnings(xWarnings->getWarnings());
#if 0
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
    {   // we have to expand the queries or bookmarks
        if (ensureEntryObject(_pParent))
        {
            DBTreeListUserData* pParentData = static_cast< DBTreeListUserData* >( _pParent->GetUserData() );
            Reference< XNameAccess > xCollection( pParentData->xContainer, UNO_QUERY );
            populateTree( xCollection, _pParent, etQuery );
        }
    }
    return true;
}

bool SbaTableQueryBrowser::ensureEntryObject( SvTreeListEntry* _pEntry )
{
    OSL_ENSURE(_pEntry, "SbaTableQueryBrowser::ensureEntryObject: invalid argument!");
    if (!_pEntry)
        return false;

    EntryType eType = getEntryType( _pEntry );

    // the user data of the entry
    DBTreeListUserData* pEntryData = static_cast<DBTreeListUserData*>(_pEntry->GetUserData());
    OSL_ENSURE(pEntryData,"ensureEntryObject: user data should already be set!");

    SvTreeListEntry* pDataSourceEntry = m_pTreeView->getListBox().GetRootLevelParent(_pEntry);

    bool bSuccess = false;
    switch (eType)
    {
        case etQueryContainer:
            if ( pEntryData->xContainer.is() )
            {
                // nothing to do
                bSuccess = true;
                break;
            }

            {
                SvTreeListEntry* pParent = m_pTreeView->getListBox().GetParent(_pEntry);
                if ( pParent != pDataSourceEntry )
                {
                    SvLBoxString* pString = static_cast<SvLBoxString*>(_pEntry->GetFirstItem(SvLBoxItemType::String));
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
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }

                    bSuccess = pEntryData->xContainer.is();
                }
                else
                {
                    try
                    {
                        Reference< XQueryDefinitionsSupplier > xQuerySup;
                        m_xDatabaseContext->getByName( getDataSourceAccessor( pDataSourceEntry ) ) >>= xQuerySup;
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
    const sal_Int32 _nCommandType, const bool _bEscapeProcessing, const SharedConnection& _rxConnection)
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
        xProp->setPropertyValue(PROPERTY_COMMAND_TYPE, makeAny(_nCommandType));
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
            SAL_WARN("dbaccess", e);
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    InvalidateAll();
    return false;
}

bool SbaTableQueryBrowser::implSelect(const OUString& _rDataSourceName, const OUString& _rCommand,
                                      const sal_Int32 _nCommandType, const bool _bEscapeProcessing,
                                      const SharedConnection& _rxConnection,
                                      bool _bSelectDirect)
{
    if (_rDataSourceName.getLength() && _rCommand.getLength() && (-1 != _nCommandType))
    {
        SvTreeListEntry* pDataSource = nullptr;
        SvTreeListEntry* pCommandType = nullptr;
        SvTreeListEntry* pCommand = getObjectEntry( _rDataSourceName, _rCommand, _nCommandType, &pDataSource, &pCommandType, true, _rxConnection );

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
            {   // tell the old entry (if any) it has been deselected
                selectPath(m_pCurrentlyDisplayed, false);
                m_pCurrentlyDisplayed = nullptr;
            }

            // we have a command and need to display this in the rowset
            return implLoadAnything(_rDataSourceName, _rCommand, _nCommandType, _bEscapeProcessing, _rxConnection);
        }
    }
    return false;
}

IMPL_LINK_NOARG(SbaTableQueryBrowser, OnSelectionChange, LinkParamNone*, void)
{
    implSelect( m_pTreeView->getListBox().FirstSelected() );
}

SvTreeListEntry* SbaTableQueryBrowser::implGetConnectionEntry(SvTreeListEntry* _pEntry) const
{
    SvTreeListEntry* pCurrentEntry = _pEntry;
    DBTreeListUserData* pEntryData = static_cast< DBTreeListUserData* >( pCurrentEntry->GetUserData() );
    while(pEntryData->eType != etDatasource )
    {
        pCurrentEntry = m_pTreeView->GetTreeModel()->GetParent(pCurrentEntry);
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
            // nothing to do
            return false;
    }

    OSL_ENSURE(m_pTreeView->GetTreeModel()->HasParent(_pEntry), "SbaTableQueryBrowser::implSelect: invalid entry (1)!");
    OSL_ENSURE(m_pTreeView->GetTreeModel()->HasParent(m_pTreeView->GetTreeModel()->GetParent(_pEntry)), "SbaTableQueryBrowser::implSelect: invalid entry (2)!");

    // get the entry for the tables or queries
    SvTreeListEntry* pContainer = m_pTreeView->GetTreeModel()->GetParent(_pEntry);
    DBTreeListUserData* pContainerData = static_cast<DBTreeListUserData*>(pContainer->GetUserData());

    // get the entry for the datasource
    SvTreeListEntry* pConnection = implGetConnectionEntry(pContainer);
    DBTreeListUserData* pConData = static_cast<DBTreeListUserData*>(pConnection->GetUserData());

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
    SvLBoxString* pString = static_cast<SvLBoxString*>(_pEntry->GetFirstItem(SvLBoxItemType::String));
    OSL_ENSURE(pString,"There must be a string item!");
    const OUString sSimpleName = pString->GetText();
    OUStringBuffer sNameBuffer(sSimpleName);
    if ( etQueryContainer == pContainerData->eType )
    {
        SvTreeListEntry* pTemp = pContainer;
        while( m_pTreeView->GetTreeModel()->GetParent(pTemp) != pConnection )
        {
            sNameBuffer.insert(0,'/');
            pString = static_cast<SvLBoxString*>(pTemp->GetFirstItem(SvLBoxItemType::String));
            OSL_ENSURE(pString,"There must be a string item!");
            sNameBuffer.insert(0,pString->GetText());
            pTemp = m_pTreeView->GetTreeModel()->GetParent(pTemp);
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
            WaitObject aWaitCursor(getBrowserView());

            // tell the old entry it has been deselected
            selectPath(m_pCurrentlyDisplayed, false);
            m_pCurrentlyDisplayed = nullptr;

            // not really loaded
            m_pCurrentlyDisplayed = _pEntry;
            // tell the new entry it has been selected
            selectPath(m_pCurrentlyDisplayed);

            // get the name of the data source currently selected
            ensureConnection( m_pCurrentlyDisplayed, pConData->xConnection );

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
                DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(_pEntry->GetUserData());
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
                                                OUString sFilter = " WHERE ";
                                                sFilter = sFilter + xAnalyzer->getFilter();
                                                OUString sReplace(sSql);
                                                sReplace = sReplace.replaceFirst(sFilter, "");
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

            OUString sDataSourceName( getDataSourceAccessor( pConnection ) );
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

SvTreeListEntry* SbaTableQueryBrowser::getEntryFromContainer(const Reference<XNameAccess>& _rxNameAccess)
{
    DBTreeListBox& rListBox = m_pTreeView->getListBox();
    SvTreeListEntry* pContainer = nullptr;
    SvTreeListEntry* pDSLoop = rListBox.FirstChild(nullptr);
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

        pDSLoop     = pDSLoop->NextSibling();
        pContainer  = nullptr;
    }
    return pContainer;
}

void SAL_CALL SbaTableQueryBrowser::elementInserted( const ContainerEvent& _rEvent )
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    // first search for a definition container where we can insert this element

    SvTreeListEntry* pEntry = getEntryFromContainer(xNames);
    if(pEntry)  // found one
    {
        // insert the new entry into the tree
        DBTreeListUserData* pContainerData = static_cast<DBTreeListUserData*>(pEntry->GetUserData());
        OSL_ENSURE(pContainerData, "elementInserted: There must be user data for this type!");

        DBTreeListUserData* pNewData = new DBTreeListUserData;
        bool bIsTable = etTableContainer == pContainerData->eType;
        if ( bIsTable )
        {
            _rEvent.Element >>= pNewData->xObjectProperties;// remember the new element
            pNewData->eType = etTableOrView;
        }
        else
        {
            if (static_cast<sal_Int32>(m_pTreeView->getListBox().GetChildCount(pEntry)) < ( xNames->getElementNames().getLength() - 1 ) )
            {
                // the item inserts its children on demand, but it has not been expanded yet. So ensure here and
                // now that it has all items
                populateTree(xNames, pEntry, etQuery );
            }
            pNewData->eType = etQuery;
        }
        implAppendEntry( pEntry, ::comphelper::getString( _rEvent.Accessor ), pNewData, pNewData->eType );
    }
    else
        SbaXDataBrowserController::elementInserted(_rEvent);
}

bool SbaTableQueryBrowser::isCurrentlyDisplayedChanged(const OUString& _sName, SvTreeListEntry const * _pContainer)
{
    return m_pCurrentlyDisplayed
            &&  getEntryType(m_pCurrentlyDisplayed) == getChildType(_pContainer)
            &&  m_pTreeView->getListBox().GetParent(m_pCurrentlyDisplayed) == _pContainer
            &&  m_pTreeView->getListBox().GetEntryText(m_pCurrentlyDisplayed) == _sName;
}

void SAL_CALL SbaTableQueryBrowser::elementRemoved( const ContainerEvent& _rEvent )
{
    SolarMutexGuard aSolarGuard;

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    // get the top-level representing the removed data source
    // and search for the queries and tables
    SvTreeListEntry* pContainer = getEntryFromContainer(xNames);
    if ( pContainer )
    { // a query or table has been removed
        OUString aName = ::comphelper::getString(_rEvent.Accessor);

        if ( isCurrentlyDisplayedChanged( aName, pContainer) )
        {   // the element displayed currently has been replaced

            // we need to remember the old value
            SvTreeListEntry* pTemp = m_pCurrentlyDisplayed;

            // unload
            unloadAndCleanup( false ); // don't dispose the connection

            DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pTemp->GetUserData());
            pTemp->SetUserData(nullptr);
            delete pData;
                // the data could be null because we have a table which isn't correct
            m_pTreeView->GetTreeModel()->Remove(pTemp);
        }
        else
        {
            // remove the entry from the model
            SvTreeListEntry* pChild = m_pTreeView->GetTreeModel()->FirstChild(pContainer);
            while(pChild)
            {
                if (m_pTreeView->getListBox().GetEntryText(pChild) == aName)
                {
                    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pChild->GetUserData());
                    pChild->SetUserData(nullptr);
                    delete pData;
                    m_pTreeView->GetTreeModel()->Remove(pChild);
                    break;
                }
                pChild = pChild->NextSibling();
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
    SvTreeListEntry* pContainer = getEntryFromContainer(xNames);
    if ( pContainer )
    {    // a table or query as been replaced
        OUString aName = ::comphelper::getString(_rEvent.Accessor);

        if ( isCurrentlyDisplayedChanged( aName, pContainer) )
        {   // the element displayed currently has been replaced

            // we need to remember the old value
            SvTreeListEntry* pTemp = m_pCurrentlyDisplayed;
            unloadAndCleanup( false ); // don't dispose the connection

            DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pTemp->GetUserData());
            if (pData)
            {
                if ( etTableOrView == pData->eType )
                { // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                     _rEvent.Element >>= pData->xObjectProperties;  // remember the new element
                }
                else
                {
                    pTemp->SetUserData(nullptr);
                    delete pData;
                }
            }
        }
        else
        {
            // find the entry for this name
            SvTreeListEntry* pChild = m_pTreeView->GetTreeModel()->FirstChild(pContainer);
            while(pChild)
            {
                if (m_pTreeView->getListBox().GetEntryText(pChild) == aName)
                {
                    DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pChild->GetUserData());
                    if (pData)
                    {
                        if ( etTableOrView == pData->eType )
                        { // only insert userdata when we have a table because the query is only a commanddefinition object and not a query
                            _rEvent.Element >>= pData->xObjectProperties;   // remember the new element
                        }
                        else
                        {
                            pChild->SetUserData(nullptr);
                            delete pData;
                        }
                    }
                    break;
                }
                pChild = pChild->NextSibling();
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

void SbaTableQueryBrowser::closeConnection(SvTreeListEntry* _pDSEntry, bool _bDisposeConnection)
{
    OSL_ENSURE(_pDSEntry, "SbaTableQueryBrowser::closeConnection: invalid entry (NULL)!");
    OSL_ENSURE( impl_isDataSourceEntry( _pDSEntry ), "SbaTableQueryBrowser::closeConnection: invalid entry (not top-level)!");

    // if one of the entries of the given DS is displayed currently, unload the form
    if (m_pCurrentlyDisplayed && (m_pTreeView->getListBox().GetRootLevelParent(m_pCurrentlyDisplayed) == _pDSEntry))
        unloadAndCleanup(_bDisposeConnection);

    // collapse the query/table container
    for (SvTreeListEntry* pContainers = m_pTreeView->GetTreeModel()->FirstChild(_pDSEntry); pContainers; pContainers = pContainers->NextSibling())
    {
        SvTreeListEntry* pElements = m_pTreeView->GetTreeModel()->FirstChild(pContainers);
        if ( pElements )
            m_pTreeView->getListBox().Collapse(pContainers);
        m_pTreeView->getListBox().EnableExpandHandler(pContainers);
        // and delete their children (they are connection-relative)
        for (; pElements; )
        {
            SvTreeListEntry* pRemove = pElements;
            pElements = pElements->NextSibling();
            DBTreeListUserData* pData = static_cast<DBTreeListUserData*>(pRemove->GetUserData());
            pRemove->SetUserData(nullptr);
            delete pData;
            m_pTreeView->GetTreeModel()->Remove(pRemove);
        }
    }
    // collapse the entry itself
    m_pTreeView->getListBox().Collapse(_pDSEntry);

    // dispose/reset the connection
    if ( _bDisposeConnection )
        disposeConnection( _pDSEntry );
}

void SbaTableQueryBrowser::unloadAndCleanup( bool _bDisposeConnection )
{
    if (!m_pCurrentlyDisplayed)
        // nothing to do
        return;

    SvTreeListEntry* pDSEntry = m_pTreeView->getListBox().GetRootLevelParent(m_pCurrentlyDisplayed);

    // de-select the path for the currently displayed table/query
    if (m_pCurrentlyDisplayed)
    {
        selectPath(m_pCurrentlyDisplayed, false);
    }
    m_pCurrentlyDisplayed = nullptr;

    try
    {
        // get the active connection. We need to dispose it.
        Reference< XPropertySet > xRowSetProps(getRowSet(),UNO_QUERY);
#if OSL_DEBUG_LEVEL > 0
        {
            Reference< XComponent > xComp(
                xRowSetProps->getPropertyValue(PROPERTY_ACTIVE_CONNECTION),
                css::uno::UNO_QUERY);
        }
#endif

        // unload the form
        Reference< XLoadable > xLoadable = getLoadable();
        if (xLoadable->isLoaded())
            xLoadable->unload();

        // clear the grid control
        Reference< XNameContainer > xConta(getControlModel(),UNO_QUERY);
        clearGridColumns(xConta);

        // dispose the connection
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
        return xDataSource.get();
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
            Sequence< OUString> aProperties(5);
            Sequence< Any> aValues(5);

            OUString* pStringIter = aProperties.getArray();
            Any* pValueIter = aValues.getArray();
            *pStringIter++  = "AlwaysShowCursor";
            *pValueIter++   <<= false;
            *pStringIter++  = PROPERTY_BORDER;
            *pValueIter++   <<= sal_Int16(0);

            *pStringIter++  = "HasNavigationBar";
            *pValueIter++   <<= false;
            *pStringIter++  = "HasRecordMarker";
            *pValueIter++   <<= false;

            *pStringIter++  = "Tabstop";
            *pValueIter++   <<= false;

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
    // determined from a outer DB document. In both cases, do not dispose it later on.
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
        // we are not used as "browser", but as mere view for a single table/query/command. In particular,
        // there is a specific database document which we belong to.
        Reference< XOfficeDatabaseDocument > xDocument( getDataSourceOrModel(
            lcl_getDataSource( m_xDatabaseContext, sInitialDataSourceName, xConnection ) ), UNO_QUERY );
        m_aDocScriptSupport = ::boost::optional< bool >( Reference< XEmbeddedScripts >( xDocument, UNO_QUERY ).is() );
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

bool SbaTableQueryBrowser::ensureConnection(SvTreeListEntry* _pAnyEntry, SharedConnection& _rConnection)
{
    SvTreeListEntry* pDSEntry = m_pTreeView->getListBox().GetRootLevelParent(_pAnyEntry);
    DBTreeListUserData* pDSData =
                pDSEntry
            ?   static_cast<DBTreeListUserData*>(pDSEntry->GetUserData())
            :   nullptr;

    return ensureConnection( pDSEntry, pDSData, _rConnection );
}

std::unique_ptr< ImageProvider > SbaTableQueryBrowser::getImageProviderFor( SvTreeListEntry* _pAnyEntry )
{
    std::unique_ptr< ImageProvider > pImageProvider( new ImageProvider );
    SharedConnection xConnection;
    if ( getExistentConnectionFor( _pAnyEntry, xConnection ) )
        pImageProvider.reset( new ImageProvider( xConnection ) );
    return pImageProvider;
}

bool SbaTableQueryBrowser::getExistentConnectionFor( SvTreeListEntry* _pAnyEntry, SharedConnection& _rConnection )
{
    SvTreeListEntry* pDSEntry = m_pTreeView->getListBox().GetRootLevelParent( _pAnyEntry );
    DBTreeListUserData* pDSData =
                pDSEntry
            ?   static_cast< DBTreeListUserData* >( pDSEntry->GetUserData() )
            :   nullptr;
    if ( pDSData )
        _rConnection = pDSData->xConnection;
    return _rConnection.is();
}

bool SbaTableQueryBrowser::impl_isDataSourceEntry( SvTreeListEntry* _pEntry ) const
{
    return m_pTreeView->GetTreeModel()->GetRootLevelParent( _pEntry ) == _pEntry;
}

bool SbaTableQueryBrowser::ensureConnection( SvTreeListEntry* _pDSEntry, void* pDSData, SharedConnection& _rConnection )
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
            // show the "connecting to ..." status
            OUString sConnecting(DBA_RES(STR_CONNECTING_DATASOURCE));
            sConnecting = sConnecting.replaceFirst("$name$", aDSName);
            BrowserViewStatusDisplay aShowStatus(static_cast<UnoDataBrowserView*>(getView()), sConnecting);

            // build a string showing context information in case of error
            OUString sConnectingContext(DBA_RES(STR_COULDNOTCONNECT_DATASOURCE));
            sConnectingContext = sConnectingContext.replaceFirst("$name$", aDSName);

            // connect
            _rConnection.reset(
                connect( getDataSourceAccessor( _pDSEntry ), sConnectingContext, nullptr ),
                SharedConnection::TakeOwnership
            );

            // remember the connection
            pTreeListData->xConnection = _rConnection;
        }
    }

    return _rConnection.is();
}

IMPL_LINK( SbaTableQueryBrowser, OnTreeEntryCompare, const SvSortData&, _rSortData, sal_Int32 )
{
    const SvTreeListEntry* pLHS = _rSortData.pLeft;
    const SvTreeListEntry* pRHS = _rSortData.pRight;
    OSL_ENSURE(pLHS && pRHS, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid tree entries!");
    // we want the table entry and the end so we have to do a check

    if (isContainer(pRHS))
    {
        // don't use getEntryType (directly or indirecly) for the LHS:
        // LHS is currently being inserted, so it is not "completely valid" at the moment

        const EntryType eRight = getEntryType(pRHS);
        if (etTableContainer == eRight)
            // every other container should be placed _before_ the bookmark container
            return -1;

        const OUString sLeft = m_pTreeView->getListBox().GetEntryText(const_cast<SvTreeListEntry*>(pLHS));

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

    const SvLBoxString* pLeftTextItem = static_cast<const SvLBoxString*>(pLHS->GetFirstItem(SvLBoxItemType::String));
    const SvLBoxString* pRightTextItem = static_cast<const SvLBoxString*>(pRHS->GetFirstItem(SvLBoxItemType::String));
    OSL_ENSURE(pLeftTextItem && pRightTextItem, "SbaTableQueryBrowser::OnTreeEntryCompare: invalid text items!");

    OUString sLeftText = pLeftTextItem->GetText();
    OUString sRightText = pRightTextItem->GetText();

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

void SbaTableQueryBrowser::implAdministrate( SvTreeListEntry* _pApplyTo )
{
    OSL_PRECOND( _pApplyTo, "SbaTableQueryBrowser::implAdministrate: illegal entry!" );
    if ( !_pApplyTo )
        return;

    try
    {
        // get the desktop object
        Reference< XDesktop2 > xFrameLoader = Desktop::create( getORB() );

        // the initial selection
        SvTreeListEntry* pTopLevelSelected = _pApplyTo;
        while (pTopLevelSelected && m_pTreeView->getListBox().GetParent(pTopLevelSelected))
            pTopLevelSelected = m_pTreeView->getListBox().GetParent(pTopLevelSelected);
        OUString sInitialSelection;
        if (pTopLevelSelected)
            sInitialSelection = getDataSourceAccessor( pTopLevelSelected );

        Reference< XDataSource > xDataSource( getDataSourceByName( sInitialSelection, getView(), getORB(), nullptr ) );
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

bool SbaTableQueryBrowser::requestQuickHelp( const SvTreeListEntry* _pEntry, OUString& _rText ) const
{
    const DBTreeListUserData* pData = static_cast< const DBTreeListUserData* >( _pEntry->GetUserData() );
    if ( ( pData->eType == etDatasource ) && !pData->sAccessor.isEmpty() )
    {
        _rText = ::svt::OFileNotation( pData->sAccessor ).get( ::svt::OFileNotation::N_SYSTEM );
        return true;
    }
    return false;
}

OUString SbaTableQueryBrowser::getContextMenuResourceName( Control& _rControl ) const
{
    OSL_PRECOND( &m_pTreeView->getListBox() == &_rControl,
        "SbaTableQueryBrowser::getContextMenuResourceName: where does this come from?" );
    if ( &m_pTreeView->getListBox() != &_rControl )
        return OUString();

    return OUString("explorer");
}

IController& SbaTableQueryBrowser::getCommandController()
{
    return *this;
}

::comphelper::OInterfaceContainerHelper2* SbaTableQueryBrowser::getContextMenuInterceptors()
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

    OSL_ENSURE( m_pTreeView->getListBox().NextSelected( pSelected ) == nullptr,
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
        aSelectedObject.Name = getDataSourceAccessor( pSelected );
        break;

    default:
        SAL_WARN("dbaccess.ui",  "SbaTableQueryBrowser::getCurrentSelection: invalid (unexpected) object type!" );
        break;
    }

    return makeAny( aSelectedObject );
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
    for (const OUString& rName : _xColContainer->getElementNames())
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
    if ( m_pCurrentlyDisplayed )
    {
        SvTreeListEntry* pContainer = m_pTreeView->GetTreeModel()->GetParent(m_pCurrentlyDisplayed);
        // get the entry for the datasource
        SvTreeListEntry* pConnection = implGetConnectionEntry(pContainer);
        OUString sName = m_pTreeView->getListBox().GetEntryText(m_pCurrentlyDisplayed);
        sTitle = GetEntryText( pConnection );
        INetURLObject aURL(sTitle);
        if ( aURL.GetProtocol() != INetProtocol::NotValid )
            sTitle = aURL.getBase(INetURLObject::LAST_SEGMENT,true,INetURLObject::DecodeMechanism::WithCharset);
        if ( !sName.isEmpty() )
        {
            sName += " - ";
            sName += sTitle;
            sTitle = sName;
        }
    }

    return sTitle;
}

bool SbaTableQueryBrowser::preReloadForm()
{
    bool bIni = false;
    if ( !m_pCurrentlyDisplayed )
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
            SvTreeListEntry* pDataSource = nullptr;
            SvTreeListEntry* pCommandType = nullptr;
            m_pCurrentlyDisplayed = getObjectEntry( sDataSource, sCommand, nCommandType, &pDataSource, &pCommandType );
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

void SbaTableQueryBrowser::impl_cleanupDataSourceEntry( const OUString& _rDataSourceName )
{
    // get the top-level representing the removed data source
    SvTreeListEntry* pDataSourceEntry = m_pTreeView->getListBox().FirstChild( nullptr );
    while ( pDataSourceEntry )
    {
        if ( m_pTreeView->getListBox().GetEntryText( pDataSourceEntry ) == _rDataSourceName )
            break;

        pDataSourceEntry = pDataSourceEntry->NextSibling();
    }

    OSL_ENSURE( pDataSourceEntry, "SbaTableQueryBrowser::impl_cleanupDataSourceEntry: do not know this data source!" );
    if ( !pDataSourceEntry )
        return;

    if ( isSelected( pDataSourceEntry ) )
    {   // a table or query belonging to the deleted data source is currently being displayed.
        OSL_ENSURE( m_pTreeView->getListBox().GetRootLevelParent( m_pCurrentlyDisplayed ) == pDataSourceEntry,
            "SbaTableQueryBrowser::impl_cleanupDataSourceEntry: inconsistence (1)!" );
        unloadAndCleanup();
    }
    else
        OSL_ENSURE(
                ( nullptr == m_pCurrentlyDisplayed )
            ||  ( m_pTreeView->getListBox().GetRootLevelParent( m_pCurrentlyDisplayed ) != pDataSourceEntry ),
            "SbaTableQueryBrowser::impl_cleanupDataSourceEntry: inconsistence (2)!");

    // delete any user data of the child entries of the to-be-removed entry
    std::pair<SvTreeListEntries::const_iterator, SvTreeListEntries::const_iterator> aIters =
        m_pTreeView->GetTreeModel()->GetChildIterators(pDataSourceEntry);

    SvTreeListEntries::const_iterator it = aIters.first, itEnd = aIters.second;

    for (; it != itEnd; ++it)
    {
        SvTreeListEntry* pEntry = (*it).get();
        const DBTreeListUserData* pData = static_cast<const DBTreeListUserData*>(pEntry->GetUserData());
        pEntry->SetUserData(nullptr);
        delete pData;
    }

    // remove the entry
    DBTreeListUserData* pData = static_cast< DBTreeListUserData* >( pDataSourceEntry->GetUserData() );
    pDataSourceEntry->SetUserData( nullptr );
    delete pData;
    m_pTreeView->GetTreeModel()->Remove( pDataSourceEntry );
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

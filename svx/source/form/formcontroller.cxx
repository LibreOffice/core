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


#include <fmcontrolbordermanager.hxx>
#include <fmcontrollayout.hxx>
#include <formcontroller.hxx>
#include <formfeaturedispatcher.hxx>
#include <fmdocumentclassification.hxx>
#include <formcontrolling.hxx>
#include <fmprop.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <fmservs.hxx>
#include <svx/fmtools.hxx>
#include <fmurl.hxx>

#include <com/sun/star/awt/FocusChangeReason.hpp>
#include <com/sun/star/awt/XCheckBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/TabController.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XIdentifierReplace.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/form/XBoundControl.hpp>
#include <com/sun/star/form/XGridControl.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/form/control/FilterControl.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/sdb/ParametersRequest.hpp>
#include <com/sun/star/sdb/RowChangeAction.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/form/runtime/FormOperations.hpp>
#include <com/sun/star/form/runtime/FormFeature.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XColumn.hpp>

#include <comphelper/enumhelper.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/flagguard.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/IParseContext.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqlparse.hxx>
#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <iterator>

#include <o3tl/functional.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::dbtools;


css::uno::Reference< css::uno::XInterface >
    FormController_NewInstance_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxORB )
{
    return *( new ::svxform::FormController( comphelper::getComponentContext(_rxORB) ) );
}

namespace svxform
{

    using ::com::sun::star::sdb::XColumn;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::awt::TabController;
    using ::com::sun::star::awt::XToolkit;
    using ::com::sun::star::awt::XWindowPeer;
    using ::com::sun::star::form::XGrid;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::sdb::XInteractionSupplyParameters;
    using ::com::sun::star::awt::XTextComponent;
    using ::com::sun::star::awt::XTextListener;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::frame::XDispatch;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::sdbc::XRowSet;
    using ::com::sun::star::sdbc::XDatabaseMetaData;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::util::NumberFormatter;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::sdbcx::XColumnsSupplier;
    using ::com::sun::star::container::XNameAccess;
    using ::com::sun::star::lang::EventObject;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::container::XEnumeration;
    using ::com::sun::star::form::XFormComponent;
    using ::com::sun::star::form::runtime::XFormOperations;
    using ::com::sun::star::form::runtime::FilterEvent;
    using ::com::sun::star::form::runtime::XFilterControllerListener;
    using ::com::sun::star::awt::XControlContainer;
    using ::com::sun::star::container::XIdentifierReplace;
    using ::com::sun::star::lang::WrappedTargetException;
    using ::com::sun::star::form::XFormControllerListener;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::sdbc::XResultSet;
    using ::com::sun::star::awt::XControlModel;
    using ::com::sun::star::awt::XTabControllerModel;
    using ::com::sun::star::beans::PropertyChangeEvent;
    using ::com::sun::star::form::validation::XValidatableFormComponent;
    using ::com::sun::star::form::XLoadable;
    using ::com::sun::star::form::XBoundControl;
    using ::com::sun::star::beans::XPropertyChangeListener;
    using ::com::sun::star::awt::TextEvent;
    using ::com::sun::star::form::XBoundComponent;
    using ::com::sun::star::awt::XCheckBox;
    using ::com::sun::star::awt::XComboBox;
    using ::com::sun::star::awt::XListBox;
    using ::com::sun::star::awt::ItemEvent;
    using ::com::sun::star::util::XModifyListener;
    using ::com::sun::star::form::XReset;
    using ::com::sun::star::frame::XDispatchProviderInterception;
    using ::com::sun::star::form::XGridControl;
    using ::com::sun::star::awt::XVclWindowPeer;
    using ::com::sun::star::form::validation::XValidator;
    using ::com::sun::star::awt::FocusEvent;
    using ::com::sun::star::sdb::SQLContext;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::form::TabulatorCycle_RECORDS;
    using ::com::sun::star::container::ContainerEvent;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::lang::NoSupportException;
    using ::com::sun::star::sdb::RowChangeEvent;
    using ::com::sun::star::frame::XStatusListener;
    using ::com::sun::star::frame::XDispatchProviderInterceptor;
    using ::com::sun::star::sdb::SQLErrorEvent;
    using ::com::sun::star::form::DatabaseParameterEvent;
    using ::com::sun::star::sdb::ParametersRequest;
    using ::com::sun::star::task::XInteractionRequest;
    using ::com::sun::star::util::URL;
    using ::com::sun::star::frame::FeatureStateEvent;
    using ::com::sun::star::form::runtime::XFormControllerContext;
    using ::com::sun::star::task::InteractionHandler;
    using ::com::sun::star::task::XInteractionHandler;
    using ::com::sun::star::form::runtime::FormOperations;
    using ::com::sun::star::container::XContainer;
    using ::com::sun::star::sdbc::SQLWarning;

    namespace ColumnValue = ::com::sun::star::sdbc::ColumnValue;
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;
    namespace FocusChangeReason = ::com::sun::star::awt::FocusChangeReason;
    namespace RowChangeAction = ::com::sun::star::sdb::RowChangeAction;
    namespace FormFeature = ::com::sun::star::form::runtime::FormFeature;

struct ColumnInfo
{
    // information about the column itself
    Reference< XColumn >    xColumn;
    sal_Int32               nNullable;
    bool                bAutoIncrement;
    bool                bReadOnly;
    OUString         sName;

    // information about the control(s) bound to this column

    /// the first control which is bound to the given column, and which requires input
    Reference< XControl >   xFirstControlWithInputRequired;
    /** the first grid control which contains a column which is bound to the given database column, and requires
        input
    */
    Reference< XGrid >      xFirstGridWithInputRequiredColumn;
    /** if xFirstControlWithInputRequired is a grid control, then nRequiredGridColumn specifies the position
        of the grid column which is actually bound
    */
    sal_Int32               nRequiredGridColumn;

    ColumnInfo()
        :xColumn()
        ,nNullable( ColumnValue::NULLABLE_UNKNOWN )
        ,bAutoIncrement( false )
        ,bReadOnly( false )
        ,sName()
        ,xFirstControlWithInputRequired()
        ,xFirstGridWithInputRequiredColumn()
        ,nRequiredGridColumn( -1 )
    {
    }
};

class ColumnInfoCache
{
public:
    explicit ColumnInfoCache( const Reference< XColumnsSupplier >& _rxColSupplier );

    size_t        getColumnCount() const { return m_aColumns.size(); }
    const ColumnInfo&   getColumnInfo( size_t _pos );

    bool    controlsInitialized() const { return m_bControlsInitialized; }
    void    initializeControls( const Sequence< Reference< XControl > >& _rControls );
    void    deinitializeControls();

private:
    typedef ::std::vector< ColumnInfo > ColumnInfos;
    ColumnInfos                         m_aColumns;
    bool                                m_bControlsInitialized;
};


ColumnInfoCache::ColumnInfoCache( const Reference< XColumnsSupplier >& _rxColSupplier )
    :m_aColumns()
    ,m_bControlsInitialized( false )
{
    try
    {
        m_aColumns.clear();

        Reference< XIndexAccess > xColumns( _rxColSupplier->getColumns(), UNO_QUERY_THROW );
        sal_Int32 nColumnCount = xColumns->getCount();
        m_aColumns.reserve( nColumnCount );

        Reference< XPropertySet >   xColumnProps;
        for ( sal_Int32 i = 0; i < nColumnCount; ++i )
        {
            ColumnInfo aColInfo;
            aColInfo.xColumn.set( xColumns->getByIndex(i), UNO_QUERY_THROW );

            xColumnProps.set( aColInfo.xColumn, UNO_QUERY_THROW );
            OSL_VERIFY( xColumnProps->getPropertyValue( FM_PROP_ISNULLABLE ) >>= aColInfo.nNullable );
            OSL_VERIFY( xColumnProps->getPropertyValue( FM_PROP_AUTOINCREMENT ) >>= aColInfo.bAutoIncrement );
            OSL_VERIFY( xColumnProps->getPropertyValue( FM_PROP_NAME ) >>= aColInfo.sName );
            OSL_VERIFY( xColumnProps->getPropertyValue( FM_PROP_ISREADONLY ) >>= aColInfo.bReadOnly );

            m_aColumns.push_back( aColInfo );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


namespace
{
    bool lcl_isBoundTo( const Reference< XPropertySet >& _rxControlModel, const Reference< XInterface >& _rxNormDBField )
    {
        Reference< XInterface > xNormBoundField( _rxControlModel->getPropertyValue( FM_PROP_BOUNDFIELD ), UNO_QUERY );
        return ( xNormBoundField == _rxNormDBField );
    }

    bool lcl_isInputRequired( const Reference< XPropertySet >& _rxControlModel )
    {
        bool bInputRequired = false;
        OSL_VERIFY( _rxControlModel->getPropertyValue( FM_PROP_INPUT_REQUIRED ) >>= bInputRequired );
        return bInputRequired;
    }

    void lcl_resetColumnControlInfo( ColumnInfo& _rColInfo )
    {
        _rColInfo.xFirstControlWithInputRequired.clear();
        _rColInfo.xFirstGridWithInputRequiredColumn.clear();
        _rColInfo.nRequiredGridColumn = -1;
    }
}


void ColumnInfoCache::deinitializeControls()
{
    for (auto& rCol : m_aColumns)
    {
        lcl_resetColumnControlInfo( rCol );
    }
    m_bControlsInitialized = false;
}


void ColumnInfoCache::initializeControls( const Sequence< Reference< XControl > >& _rControls )
{
    try
    {
        // for every of our known columns, find the controls which are bound to this column
        for (auto& rCol : m_aColumns)
        {
            OSL_ENSURE( !rCol.xFirstControlWithInputRequired.is() && !rCol.xFirstGridWithInputRequiredColumn.is()
                && ( rCol.nRequiredGridColumn == -1 ), "ColumnInfoCache::initializeControls: called me twice?" );

            lcl_resetColumnControlInfo( rCol );

            Reference< XInterface > xNormColumn( rCol.xColumn, UNO_QUERY_THROW );

            const Reference< XControl >* pControl( _rControls.getConstArray() );
            const Reference< XControl >* pControlEnd( pControl + _rControls.getLength() );
            for ( ; pControl != pControlEnd; ++pControl )
            {
                if ( !pControl->is() )
                    continue;

                Reference< XPropertySet > xModel( (*pControl)->getModel(), UNO_QUERY_THROW );
                Reference< XPropertySetInfo > xModelPSI( xModel->getPropertySetInfo(), UNO_SET_THROW );

                // special handling for grid controls
                Reference< XGrid > xGrid( *pControl, UNO_QUERY );
                if ( xGrid.is() )
                {
                    Reference< XIndexAccess > xGridColAccess( xModel, UNO_QUERY_THROW );
                    sal_Int32 gridColCount = xGridColAccess->getCount();
                    sal_Int32 gridCol = 0;
                    for ( gridCol = 0; gridCol < gridColCount; ++gridCol )
                    {
                        Reference< XPropertySet > xGridColumnModel( xGridColAccess->getByIndex( gridCol ), UNO_QUERY_THROW );

                        if  (   !lcl_isBoundTo( xGridColumnModel, xNormColumn )
                            ||  !lcl_isInputRequired( xGridColumnModel )
                            )
                            continue;   // with next grid column

                        break;
                    }

                    if ( gridCol < gridColCount )
                    {
                        // found a grid column which is bound to the given
                        rCol.xFirstGridWithInputRequiredColumn = xGrid;
                        rCol.nRequiredGridColumn = gridCol;
                        break;
                    }

                    continue;   // with next control
                }

                if  (   !xModelPSI->hasPropertyByName( FM_PROP_BOUNDFIELD )
                    ||  !lcl_isBoundTo( xModel, xNormColumn )
                    ||  !lcl_isInputRequired( xModel )
                    )
                    continue;   // with next control

                break;
            }

            if ( pControl == pControlEnd )
                // did not find a control which is bound to this particular column, and for which the input is required
                continue;   // with next DB column

            rCol.xFirstControlWithInputRequired = *pControl;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    m_bControlsInitialized = true;
}


const ColumnInfo& ColumnInfoCache::getColumnInfo( size_t _pos )
{
    if ( _pos >= m_aColumns.size() )
        throw IndexOutOfBoundsException();

    return m_aColumns[ _pos ];
}

class OParameterContinuation : public OInteraction< XInteractionSupplyParameters >
{
    Sequence< PropertyValue >       m_aValues;

public:
    OParameterContinuation() { }

    const Sequence< PropertyValue >&   getValues() const { return m_aValues; }

// XInteractionSupplyParameters
    virtual void SAL_CALL setParameters( const Sequence< PropertyValue >& _rValues ) override;
};


void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues )
{
    m_aValues = _rValues;
}


// FmXAutoControl

struct FmFieldInfo
{
    OUString       aFieldName;
    Reference< XPropertySet >   xField;
    Reference< XTextComponent >  xText;

    FmFieldInfo(const Reference< XPropertySet >& _xField, const Reference< XTextComponent >& _xText)
        :xField(_xField)
        ,xText(_xText)
    {xField->getPropertyValue(FM_PROP_NAME) >>= aFieldName;}
};

class FmXAutoControl: public UnoControl

{
public:
    FmXAutoControl() :UnoControl()
    {
    }

    virtual OUString GetComponentServiceName() override {return OUString("Edit");}
    virtual void SAL_CALL createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) override;

protected:
    virtual void ImplSetPeerProperty( const OUString& rPropName, const Any& rVal ) override;
};


void FmXAutoControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer )
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    Reference< XTextComponent >  xText(getPeer() , UNO_QUERY);
    if (xText.is())
    {
        xText->setText(SvxResId(RID_STR_AUTOFIELD));
        xText->setEditable(false);
    }
}


void FmXAutoControl::ImplSetPeerProperty( const OUString& rPropName, const Any& rVal )
{
    // these properties are ignored
    if (rPropName == FM_PROP_TEXT)
        return;

    UnoControl::ImplSetPeerProperty( rPropName, rVal );
}


IMPL_LINK_NOARG( FormController, OnActivateTabOrder, Timer*, void )
{
    activateTabOrder();
}


struct UpdateAllListeners
{
    bool operator()( const Reference< XDispatch >& _rxDispatcher ) const
    {
        static_cast< svx::OSingleFeatureDispatcher* >( _rxDispatcher.get() )->updateAllListeners();
        // the return is a dummy only so we can use this struct in a lambda expression
        return true;
    }
};

IMPL_LINK_NOARG( FormController, OnInvalidateFeatures, Timer*, void )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    for (const auto& rFeature : m_aInvalidFeatures)
    {
        DispatcherContainer::const_iterator aDispatcherPos = m_aFeatureDispatchers.find( rFeature );
        if ( aDispatcherPos != m_aFeatureDispatchers.end() )
        {
            // TODO: for the real and actual listener notifications, we should release
            // our mutex
            UpdateAllListeners( )( aDispatcherPos->second );
        }
    }
}

FormController::FormController(const Reference< css::uno::XComponentContext > & _rxORB )
                  :FormController_BASE( m_aMutex )
                  ,OPropertySetHelper( FormController_BASE::rBHelper )
                  ,OSQLParserClient( _rxORB )
                  ,m_xComponentContext( _rxORB )
                  ,m_aActivateListeners(m_aMutex)
                  ,m_aModifyListeners(m_aMutex)
                  ,m_aErrorListeners(m_aMutex)
                  ,m_aDeleteListeners(m_aMutex)
                  ,m_aRowSetApproveListeners(m_aMutex)
                  ,m_aParameterListeners(m_aMutex)
                  ,m_aFilterListeners(m_aMutex)
                  ,m_xFormOperations()
                  ,m_aMode( OUString( "DataMode"  ) )
                  ,m_aLoadEvent( LINK( this, FormController, OnLoad ) )
                  ,m_aToggleEvent( LINK( this, FormController, OnToggleAutoFields ) )
                  ,m_aActivationEvent( LINK( this, FormController, OnActivated ) )
                  ,m_aDeactivationEvent( LINK( this, FormController, OnDeactivated ) )
                  ,m_nCurrentFilterPosition(-1)
                  ,m_bCurrentRecordModified(false)
                  ,m_bCurrentRecordNew(false)
                  ,m_bLocked(false)
                  ,m_bDBConnection(false)
                  ,m_bCycle(false)
                  ,m_bCanInsert(false)
                  ,m_bCanUpdate(false)
                  ,m_bCommitLock(false)
                  ,m_bModified(false)
                  ,m_bControlsSorted(false)
                  ,m_bFiltering(false)
                  ,m_bAttachEvents(true)
                  ,m_bDetachEvents(true)
                  ,m_bAttemptedHandlerCreation( false )
                  ,m_bSuspendFilterTextListening( false )
{

    osl_atomic_increment(&m_refCount);
    {
        m_xTabController = TabController::create( m_xComponentContext );
        m_xAggregate.set( m_xTabController, UNO_QUERY_THROW );
        m_xAggregate->setDelegator( *this );
    }
    osl_atomic_decrement(&m_refCount);

    m_aTabActivationIdle.SetPriority( TaskPriority::LOWEST );
    m_aTabActivationIdle.SetInvokeHandler( LINK( this, FormController, OnActivateTabOrder ) );

    m_aFeatureInvalidationTimer.SetTimeout( 200 );
    m_aFeatureInvalidationTimer.SetInvokeHandler( LINK( this, FormController, OnInvalidateFeatures ) );
}


FormController::~FormController()
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_aLoadEvent.CancelPendingCall();
        m_aToggleEvent.CancelPendingCall();
        m_aActivationEvent.CancelPendingCall();
        m_aDeactivationEvent.CancelPendingCall();

        if ( m_aTabActivationIdle.IsActive() )
            m_aTabActivationIdle.Stop();
    }

    if ( m_aFeatureInvalidationTimer.IsActive() )
        m_aFeatureInvalidationTimer.Stop();

    disposeAllFeaturesAndDispatchers();

    if ( m_xFormOperations.is() )
        m_xFormOperations->dispose();
    m_xFormOperations.clear();

    // release of aggregation
    if ( m_xAggregate.is() )
    {
        m_xAggregate->setDelegator( nullptr );
        m_xAggregate.clear();
    }
}


void SAL_CALL FormController::acquire() throw ()
{
    FormController_BASE::acquire();
}


void SAL_CALL FormController::release() throw ()
{
    FormController_BASE::release();
}


Any SAL_CALL FormController::queryInterface( const Type& _rType )
{
    Any aRet = FormController_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = m_xAggregate->queryAggregation( _rType );
    return aRet;
}


Sequence< sal_Int8 > SAL_CALL FormController::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

Sequence< Type > SAL_CALL FormController::getTypes(  )
{
    return comphelper::concatSequences(
        FormController_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

// XServiceInfo
sal_Bool SAL_CALL FormController::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

OUString SAL_CALL FormController::getImplementationName()
{
    return OUString("org.openoffice.comp.svx.FormController");
}

Sequence< OUString> SAL_CALL FormController::getSupportedServiceNames()
{
    // service names which are supported only, but cannot be used to created an
    // instance at a service factory
    Sequence<OUString> aNonCreatableServiceNames { "com.sun.star.form.FormControllerDispatcher" };

    // services which can be used to created an instance at a service factory
    Sequence< OUString > aCreatableServiceNames( getSupportedServiceNames_Static() );
    return ::comphelper::concatSequences( aCreatableServiceNames, aNonCreatableServiceNames );
}


sal_Bool SAL_CALL FormController::approveReset(const EventObject& /*rEvent*/)
{
    return true;
}


void SAL_CALL FormController::resetted(const EventObject& rEvent)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (getCurrentControl().is() &&  (getCurrentControl()->getModel() == rEvent.Source))
        m_bModified = false;
}


Sequence< OUString> const & FormController::getSupportedServiceNames_Static()
{
    static Sequence< OUString> const aServices
    {
        "com.sun.star.form.runtime.FormController",
        "com.sun.star.awt.control.TabController"
    };
    return aServices;
}


namespace
{
    struct ResetComponentText
    {
        void operator()( const Reference< XTextComponent >& _rxText )
        {
            _rxText->setText( OUString() );
        }
    };

    struct RemoveComponentTextListener
    {
        explicit RemoveComponentTextListener( const Reference< XTextListener >& _rxListener )
            :m_xListener( _rxListener )
        {
        }

        void operator()( const Reference< XTextComponent >& _rxText )
        {
            _rxText->removeTextListener( m_xListener );
        }

    private:
        Reference< XTextListener >  m_xListener;
    };
}


void FormController::impl_setTextOnAllFilter_throw()
{
    m_bSuspendFilterTextListening = true;
    ::comphelper::FlagGuard aResetFlag( m_bSuspendFilterTextListening );

    // reset the text for all controls
    ::std::for_each( m_aFilterComponents.begin(), m_aFilterComponents.end(), ResetComponentText() );

    if ( m_aFilterRows.empty() )
        // nothing to do anymore
        return;

    if ( m_nCurrentFilterPosition < 0 )
        return;

    // set the text for all filters
    OSL_ENSURE( m_aFilterRows.size() > static_cast<size_t>(m_nCurrentFilterPosition),
        "FormController::impl_setTextOnAllFilter_throw: m_nCurrentFilterPosition too big" );

    if ( static_cast<size_t>(m_nCurrentFilterPosition) < m_aFilterRows.size() )
    {
        FmFilterRow& rRow = m_aFilterRows[ m_nCurrentFilterPosition ];
        for (const auto& rEntry : rRow)
        {
            rEntry.first->setText( rEntry.second );
        }
    }
}
// OPropertySetHelper

sal_Bool FormController::convertFastPropertyValue( Any & /*rConvertedValue*/, Any & /*rOldValue*/,
                                            sal_Int32 /*nHandle*/, const Any& /*rValue*/ )
{
    return false;
}


void FormController::setFastPropertyValue_NoBroadcast( sal_Int32 /*nHandle*/, const Any& /*rValue*/ )
{
}


void FormController::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case FM_ATTR_FILTER:
        {
            OUStringBuffer aFilter;
            Reference<XConnection> xConnection(getConnection(Reference< XRowSet>(m_xModelAsIndex, UNO_QUERY)));
            if (xConnection.is())
            {
                Reference< XNumberFormatsSupplier> xFormatSupplier( getNumberFormats( xConnection, true ) );
                Reference< XNumberFormatter> xFormatter = NumberFormatter::create(m_xComponentContext);
                xFormatter->attachNumberFormatsSupplier(xFormatSupplier);

                // now add the filter rows
                try
                {
                    for (const FmFilterRow& rRow : m_aFilterRows)
                    {
                        if ( rRow.empty() )
                            continue;

                        OUStringBuffer aRowFilter;
                        for ( FmFilterRow::const_iterator condition = rRow.begin(); condition != rRow.end(); ++condition )
                        {
                            // get the field of the controls map
                            Reference< XControl > xControl( condition->first, UNO_QUERY_THROW );
                            Reference< XPropertySet > xModelProps( xControl->getModel(), UNO_QUERY_THROW );
                            Reference< XPropertySet > xField( xModelProps->getPropertyValue( FM_PROP_BOUNDFIELD ), UNO_QUERY_THROW );

                            OUString sFilterValue( condition->second );

                            OUString sErrorMsg, sCriteria;
                            const std::shared_ptr< OSQLParseNode > pParseNode =
                                predicateTree( sErrorMsg, sFilterValue, xFormatter, xField );
                            OSL_ENSURE( pParseNode != nullptr, "FormController::getFastPropertyValue: could not parse the field value predicate!" );
                            if ( pParseNode != nullptr )
                            {
                                // don't use a parse context here, we need it unlocalized
                                pParseNode->parseNodeToStr( sCriteria, xConnection );
                                if ( condition != rRow.begin() )
                                    aRowFilter.append( " AND " );
                                aRowFilter.append( sCriteria );
                            }
                        }
                        if ( !aRowFilter.isEmpty() )
                        {
                            if ( !aFilter.isEmpty() )
                                aFilter.append( " OR " );

                            aFilter.append( "( " );
                            aFilter.append( aRowFilter.makeStringAndClear() );
                            aFilter.append( " )" );
                        }
                    }
                }
                catch( const Exception& )
                {
                    DBG_UNHANDLED_EXCEPTION("svx");
                    aFilter.setLength(0);
                }
            }
            rValue <<= aFilter.makeStringAndClear();
        }
        break;

        case FM_ATTR_FORM_OPERATIONS:
            rValue <<= m_xFormOperations;
            break;
    }
}


Reference< XPropertySetInfo >  FormController::getPropertySetInfo()
{
    static Reference< XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


void FormController::fillProperties(
        Sequence< Property >& /* [out] */ _rProps,
        Sequence< Property >& /* [out] */ /*_rAggregateProps*/
        ) const
{
    _rProps.realloc(2);
    sal_Int32 nPos = 0;
    Property* pDesc = _rProps.getArray();

    pDesc[nPos++] = Property(FM_PROP_FILTER, FM_ATTR_FILTER,
                             cppu::UnoType<OUString>::get(),
                             PropertyAttribute::READONLY);
    pDesc[nPos++] = Property(FM_PROP_FORM_OPERATIONS, FM_ATTR_FORM_OPERATIONS,
                             cppu::UnoType<XFormOperations>::get(),
                             PropertyAttribute::READONLY);
}


::cppu::IPropertyArrayHelper& FormController::getInfoHelper()
{
    return *getArrayHelper();
}

// XFilterController

void SAL_CALL FormController::addFilterControllerListener( const Reference< XFilterControllerListener >& Listener )
{
    m_aFilterListeners.addInterface( Listener );
}


void SAL_CALL FormController::removeFilterControllerListener( const Reference< XFilterControllerListener >& Listener )
{
    m_aFilterListeners.removeInterface( Listener );
}


::sal_Int32 SAL_CALL FormController::getFilterComponents()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    return m_aFilterComponents.size();
}


::sal_Int32 SAL_CALL FormController::getDisjunctiveTerms()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    return m_aFilterRows.size();
}


void SAL_CALL FormController::setPredicateExpression( ::sal_Int32 Component, ::sal_Int32 Term, const OUString& PredicateExpression )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    if ( ( Component < 0 ) || ( Component >= getFilterComponents() ) || ( Term < 0 ) || ( Term >= getDisjunctiveTerms() ) )
        throw IndexOutOfBoundsException( OUString(), *this );

    Reference< XTextComponent > xText( m_aFilterComponents[ Component ] );
    xText->setText( PredicateExpression );

    FmFilterRow& rFilterRow = m_aFilterRows[ Term ];
    if ( !PredicateExpression.isEmpty() )
        rFilterRow[ xText ] = PredicateExpression;
    else
        rFilterRow.erase( xText );
}


Reference< XControl > FormController::getFilterComponent( ::sal_Int32 Component )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    if ( ( Component < 0 ) || ( Component >= getFilterComponents() ) )
        throw IndexOutOfBoundsException( OUString(), *this );

    return Reference< XControl >( m_aFilterComponents[ Component ], UNO_QUERY );
}


Sequence< Sequence< OUString > > FormController::getPredicateExpressions()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    Sequence< Sequence< OUString > > aExpressions( m_aFilterRows.size() );
    sal_Int32 termIndex = 0;
    for (const FmFilterRow& rRow : m_aFilterRows)
    {
        Sequence< OUString > aConjunction( m_aFilterComponents.size() );
        sal_Int32 componentIndex = 0;
        for (const auto& rComp : m_aFilterComponents)
        {
            FmFilterRow::const_iterator predicate = rRow.find( rComp );
            if ( predicate != rRow.end() )
                aConjunction[ componentIndex ] = predicate->second;
            ++componentIndex;
        }

        aExpressions[ termIndex ] = aConjunction;
        ++termIndex;
    }

    return aExpressions;
}


void SAL_CALL FormController::removeDisjunctiveTerm( ::sal_Int32 Term )
{
    // SYNCHRONIZED -->
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    if ( ( Term < 0 ) || ( Term >= getDisjunctiveTerms() ) )
        throw IndexOutOfBoundsException( OUString(), *this );

    // if the to-be-deleted row is our current row, we need to shift
    if ( Term == m_nCurrentFilterPosition )
    {
        if ( m_nCurrentFilterPosition < sal_Int32( m_aFilterRows.size() - 1 ) )
            ++m_nCurrentFilterPosition;
        else
            --m_nCurrentFilterPosition;
    }

    FmFilterRows::iterator pos = m_aFilterRows.begin() + Term;
    m_aFilterRows.erase( pos );

    // adjust m_nCurrentFilterPosition if the removed row preceded it
    if ( Term < m_nCurrentFilterPosition )
        --m_nCurrentFilterPosition;

    SAL_WARN_IF( !( ( m_nCurrentFilterPosition < 0 ) != ( m_aFilterRows.empty() ) ),
        "svx.form", "FormController::removeDisjunctiveTerm: inconsistency!" );

    // update the texts in the filter controls
    impl_setTextOnAllFilter_throw();

    FilterEvent aEvent;
    aEvent.Source = *this;
    aEvent.DisjunctiveTerm = Term;
    aGuard.clear();
    // <-- SYNCHRONIZED

    m_aFilterListeners.notifyEach( &XFilterControllerListener::disjunctiveTermRemoved, aEvent );
}


void SAL_CALL FormController::appendEmptyDisjunctiveTerm()
{
    // SYNCHRONIZED -->
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    impl_appendEmptyFilterRow( aGuard );
    // <-- SYNCHRONIZED
}


::sal_Int32 SAL_CALL FormController::getActiveTerm()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    return m_nCurrentFilterPosition;
}


void SAL_CALL FormController::setActiveTerm( ::sal_Int32 ActiveTerm )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    if ( ( ActiveTerm < 0 ) || ( ActiveTerm >= getDisjunctiveTerms() ) )
        throw IndexOutOfBoundsException( OUString(), *this );

    if ( ActiveTerm == getActiveTerm() )
        return;

    m_nCurrentFilterPosition = ActiveTerm;
    impl_setTextOnAllFilter_throw();
}

// XElementAccess

sal_Bool SAL_CALL FormController::hasElements()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return !m_aChildren.empty();
}


Type SAL_CALL  FormController::getElementType()
{
    return cppu::UnoType<XFormController>::get();

}

// XEnumerationAccess

Reference< XEnumeration > SAL_CALL  FormController::createEnumeration()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return new ::comphelper::OEnumerationByIndex(this);
}

// XIndexAccess

sal_Int32 SAL_CALL FormController::getCount()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_aChildren.size();
}


Any SAL_CALL FormController::getByIndex(sal_Int32 Index)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (Index < 0 ||
        Index >= static_cast<sal_Int32>(m_aChildren.size()))
        throw IndexOutOfBoundsException();

    return makeAny( m_aChildren[ Index ] );
}

//  EventListener

void SAL_CALL FormController::disposing(const EventObject& e)
{
    // has the container been disposed
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XControlContainer >  xContainer(e.Source, UNO_QUERY);
    if (xContainer.is())
    {
        setContainer(Reference< XControlContainer > ());
    }
    else
    {
        // has a control been disposed
        Reference< XControl >  xControl(e.Source, UNO_QUERY);
        if (xControl.is())
        {
            if (getContainer().is())
                removeControl(xControl);
        }
    }
}

// OComponentHelper

void FormController::disposeAllFeaturesAndDispatchers()
{
    for (auto& rDispatcher : m_aFeatureDispatchers)
    {
        try
        {
            ::comphelper::disposeComponent( rDispatcher.second );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }
    m_aFeatureDispatchers.clear();
}


void FormController::disposing()
{
    EventObject aEvt( *this );

    // if we're still active, simulate a "deactivated" event
    if ( m_xActiveControl.is() )
        m_aActivateListeners.notifyEach( &XFormControllerListener::formDeactivated, aEvt );

    // notify all our listeners
    m_aActivateListeners.disposeAndClear(aEvt);
    m_aModifyListeners.disposeAndClear(aEvt);
    m_aErrorListeners.disposeAndClear(aEvt);
    m_aDeleteListeners.disposeAndClear(aEvt);
    m_aRowSetApproveListeners.disposeAndClear(aEvt);
    m_aParameterListeners.disposeAndClear(aEvt);
    m_aFilterListeners.disposeAndClear(aEvt);

    removeBoundFieldListener();
    stopFiltering();

    m_aControlBorderManager.restoreAll();

    m_aFilterRows.clear();

    ::osl::MutexGuard aGuard( m_aMutex );
    m_xActiveControl = nullptr;
    implSetCurrentControl( nullptr );

    // clean up our children
    for (const auto& rpChild : m_aChildren)
    {
        // search the position of the model within the form
        Reference< XFormComponent >  xForm(rpChild->getModel(), UNO_QUERY);
        sal_uInt32 nPos = m_xModelAsIndex->getCount();
        Reference< XFormComponent > xTemp;
        for( ; nPos; )
        {

            m_xModelAsIndex->getByIndex( --nPos ) >>= xTemp;
            if ( xForm.get() == xTemp.get() )
            {
                Reference< XInterface > xIfc( rpChild, UNO_QUERY );
                m_xModelAsManager->detach( nPos, xIfc );
                break;
            }
        }

        Reference< XComponent > (rpChild, UNO_QUERY)->dispose();
    }
    m_aChildren.clear();

    disposeAllFeaturesAndDispatchers();

    if ( m_xFormOperations.is() )
        m_xFormOperations->dispose();
    m_xFormOperations.clear();

    if (m_bDBConnection)
        unload();

    setContainer( nullptr );
    setModel( nullptr );
    setParent( nullptr );

    ::comphelper::disposeComponent( m_xComposer );

    m_bDBConnection = false;
}


namespace
{
    bool lcl_shouldUseDynamicControlBorder( const Reference< XInterface >& _rxForm, const Any& _rDynamicColorProp )
    {
        bool bDoUse = false;
        if ( !( _rDynamicColorProp >>= bDoUse ) )
        {
            DocumentType eDocType = DocumentClassification::classifyHostDocument( _rxForm );
            return ControlLayouter::useDynamicBorderColor( eDocType );
        }
        return bDoUse;
    }
}


void SAL_CALL FormController::propertyChange(const PropertyChangeEvent& evt)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    if ( evt.PropertyName == FM_PROP_BOUNDFIELD )
    {
        Reference<XPropertySet> xOldBound;
        evt.OldValue >>= xOldBound;
        if ( !xOldBound.is() && evt.NewValue.hasValue() )
        {
            Reference< XControlModel > xControlModel(evt.Source,UNO_QUERY);
            Reference< XControl > xControl = findControl(m_aControls,xControlModel,false,false);
            if ( xControl.is() )
            {
                startControlModifyListening( xControl );
                Reference<XPropertySet> xProp(xControlModel,UNO_QUERY);
                if ( xProp.is() )
                    xProp->removePropertyChangeListener(FM_PROP_BOUNDFIELD, this);
            }
        }
    }
    else
    {
        bool bModifiedChanged = (evt.PropertyName == FM_PROP_ISMODIFIED);
        bool bNewChanged = (evt.PropertyName == FM_PROP_ISNEW);
        if (bModifiedChanged || bNewChanged)
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (bModifiedChanged)
                m_bCurrentRecordModified = ::comphelper::getBOOL(evt.NewValue);
            else
                m_bCurrentRecordNew = ::comphelper::getBOOL(evt.NewValue);

            // toggle the locking
            if (m_bLocked != determineLockState())
            {
                m_bLocked = !m_bLocked;
                setLocks();
                if (isListeningForChanges())
                    startListening();
                else
                    stopListening();
            }

            if ( bNewChanged )
                m_aToggleEvent.Call();

            if (!m_bCurrentRecordModified)
                m_bModified = false;
        }
        else if ( evt.PropertyName == FM_PROP_DYNAMIC_CONTROL_BORDER )
        {
            bool bEnable = lcl_shouldUseDynamicControlBorder( evt.Source, evt.NewValue );
            if ( bEnable )
            {
                m_aControlBorderManager.enableDynamicBorderColor();
                if ( m_xActiveControl.is() )
                    m_aControlBorderManager.focusGained( m_xActiveControl.get() );
            }
            else
            {
                m_aControlBorderManager.disableDynamicBorderColor();
            }
        }
    }
}


bool FormController::replaceControl( const Reference< XControl >& _rxExistentControl, const Reference< XControl >& _rxNewControl )
{
    bool bSuccess = false;
    try
    {
        Reference< XIdentifierReplace > xContainer( getContainer(), UNO_QUERY );
        DBG_ASSERT( xContainer.is(), "FormController::replaceControl: yes, it's not required by the service description, but XIdentifierReplace would be nice!" );
        if ( xContainer.is() )
        {
            // look up the ID of _rxExistentControl
            Sequence< sal_Int32 > aIdentifiers( xContainer->getIdentifiers() );
            const sal_Int32* pIdentifiers = aIdentifiers.getConstArray();
            const sal_Int32* pIdentifiersEnd = aIdentifiers.getConstArray() + aIdentifiers.getLength();
            for ( ; pIdentifiers != pIdentifiersEnd; ++pIdentifiers )
            {
                Reference< XControl > xCheck( xContainer->getByIdentifier( *pIdentifiers ), UNO_QUERY );
                if ( xCheck == _rxExistentControl )
                    break;
            }
            DBG_ASSERT( pIdentifiers != pIdentifiersEnd, "FormController::replaceControl: did not find the control in the container!" );
            if ( pIdentifiers != pIdentifiersEnd )
            {
                bool bReplacedWasActive = ( m_xActiveControl.get() == _rxExistentControl.get() );
                bool bReplacedWasCurrent = ( m_xCurrentControl.get() == _rxExistentControl.get() );

                if ( bReplacedWasActive )
                {
                    m_xActiveControl = nullptr;
                    implSetCurrentControl( nullptr );
                }
                else if ( bReplacedWasCurrent )
                {
                    implSetCurrentControl( _rxNewControl );
                }

                // carry over the model
                _rxNewControl->setModel( _rxExistentControl->getModel() );

                xContainer->replaceByIdentifer( *pIdentifiers, makeAny( _rxNewControl ) );
                bSuccess = true;

                if ( bReplacedWasActive )
                {
                    Reference< XWindow > xControlWindow( _rxNewControl, UNO_QUERY );
                    if ( xControlWindow.is() )
                        xControlWindow->setFocus();
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    Reference< XControl > xDisposeIt( bSuccess ? _rxExistentControl : _rxNewControl );
    ::comphelper::disposeComponent( xDisposeIt );
    return bSuccess;
}


void FormController::toggleAutoFields(bool bAutoFields)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );


    Sequence< Reference< XControl > > aControlsCopy( m_aControls );
    const Reference< XControl >* pControls = aControlsCopy.getConstArray();
    sal_Int32 nControls = aControlsCopy.getLength();

    if (bAutoFields)
    {
        // as we don't want new controls to be attached to the scripting environment
        // we change attach flags
        m_bAttachEvents = false;
        for (sal_Int32 i = nControls; i > 0;)
        {
            Reference< XControl > xControl = pControls[--i];
            if (xControl.is())
            {
                Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
                if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
                {
                    // does the model use a bound field ?
                    Reference< XPropertySet >  xField;
                    xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

                    // is it a autofield?
                    if  (   xField.is()
                        &&  ::comphelper::hasProperty( FM_PROP_AUTOINCREMENT, xField )
                        &&  ::comphelper::getBOOL( xField->getPropertyValue( FM_PROP_AUTOINCREMENT ) )
                        )
                    {
                        replaceControl( xControl, new FmXAutoControl() );
                    }
                }
            }
        }
        m_bAttachEvents = true;
    }
    else
    {
        m_bDetachEvents = false;
        for (sal_Int32 i = nControls; i > 0;)
        {
            Reference< XControl > xControl = pControls[--i];
            if (xControl.is())
            {
                Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
                if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
                {
                    // does the model use a bound field ?
                    Reference< XPropertySet >  xField;
                    xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

                    // is it a autofield?
                    if  (   xField.is()
                        &&  ::comphelper::hasProperty( FM_PROP_AUTOINCREMENT, xField )
                        &&  ::comphelper::getBOOL( xField->getPropertyValue(FM_PROP_AUTOINCREMENT ) )
                        )
                    {
                        OUString sServiceName;
                        OSL_VERIFY( xSet->getPropertyValue( FM_PROP_DEFAULTCONTROL ) >>= sServiceName );
                        Reference< XControl > xNewControl( m_xComponentContext->getServiceManager()->createInstanceWithContext( sServiceName, m_xComponentContext ), UNO_QUERY );
                        replaceControl( xControl, xNewControl );
                    }
                }
            }
        }
        m_bDetachEvents = true;
    }
}


IMPL_LINK_NOARG(FormController, OnToggleAutoFields, void*, void)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );

    toggleAutoFields(m_bCurrentRecordNew);
}

// XTextListener

void SAL_CALL FormController::textChanged(const TextEvent& e)
{
    // SYNCHRONIZED -->
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    if ( !m_bFiltering )
    {
        impl_onModify();
        return;
    }

    if ( m_bSuspendFilterTextListening )
        return;

    Reference< XTextComponent >  xText(e.Source,UNO_QUERY);
    OUString aText = xText->getText();

    if ( m_aFilterRows.empty() )
        appendEmptyDisjunctiveTerm();

    // find the current row
    if ( ( static_cast<size_t>(m_nCurrentFilterPosition) >= m_aFilterRows.size() ) || ( m_nCurrentFilterPosition < 0 ) )
    {
        OSL_ENSURE( false, "FormController::textChanged: m_nCurrentFilterPosition is wrong!" );
        return;
    }

    FmFilterRow& rRow = m_aFilterRows[ m_nCurrentFilterPosition ];

    // do we have a new filter
    if (!aText.isEmpty())
        rRow[xText] = aText;
    else
    {
        // do we have the control in the row
        FmFilterRow::iterator iter = rRow.find(xText);
        // erase the entry out of the row
        if (iter != rRow.end())
            rRow.erase(iter);
    }

    // multiplex the event to our FilterControllerListeners
    FilterEvent aEvent;
    aEvent.Source = *this;
    aEvent.FilterComponent = ::std::find( m_aFilterComponents.begin(), m_aFilterComponents.end(), xText ) - m_aFilterComponents.begin();
    aEvent.DisjunctiveTerm = getActiveTerm();
    aEvent.PredicateExpression = aText;

    aGuard.clear();
    // <-- SYNCHRONIZED

    // notify the changed filter expression
    m_aFilterListeners.notifyEach( &XFilterControllerListener::predicateExpressionChanged, aEvent );
}

// XItemListener

void SAL_CALL FormController::itemStateChanged(const ItemEvent& /*rEvent*/)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    impl_onModify();
}

// XModificationBroadcaster

void SAL_CALL FormController::addModifyListener(const Reference< XModifyListener > & l)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    m_aModifyListeners.addInterface( l );
}


void FormController::removeModifyListener(const Reference< XModifyListener > & l)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    m_aModifyListeners.removeInterface( l );
}

// XModificationListener

void FormController::modified( const EventObject& _rEvent )
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );

    try
    {
        if ( _rEvent.Source != m_xActiveControl )
        {   // let this control grab the focus
            // (this case may happen if somebody moves the scroll wheel of the mouse over a control
            // which does not have the focus)
            // 85511 - 29.05.2001 - frank.schoenheit@germany.sun.com

            // also, it happens when an image control gets a new image by double-clicking it
            // #i88458# / 2009-01-12 / frank.schoenheit@sun.com
            Reference< XWindow > xControlWindow( _rEvent.Source, UNO_QUERY_THROW );
            xControlWindow->setFocus();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    impl_onModify();
}


void FormController::impl_checkDisposed_throw() const
{
    if ( impl_isDisposed_nofail() )
        throw DisposedException( OUString(), *const_cast< FormController* >( this ) );
}


void FormController::impl_onModify()
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );

    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !m_bModified )
            m_bModified = true;
    }

    EventObject aEvt(static_cast<cppu::OWeakObject*>(this));
    m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvt );
}


void FormController::impl_addFilterRow( const FmFilterRow& _row )
{
    m_aFilterRows.push_back( _row );

    if ( m_aFilterRows.size() == 1 )
    {   // that's the first row ever
        OSL_ENSURE( m_nCurrentFilterPosition == -1, "FormController::impl_addFilterRow: inconsistency!" );
        m_nCurrentFilterPosition = 0;
    }
}


void FormController::impl_appendEmptyFilterRow( ::osl::ClearableMutexGuard& _rClearBeforeNotify )
{
    // SYNCHRONIZED -->
    impl_addFilterRow( FmFilterRow() );

    // notify the listeners
    FilterEvent aEvent;
    aEvent.Source = *this;
    aEvent.DisjunctiveTerm = static_cast<sal_Int32>(m_aFilterRows.size()) - 1;
    _rClearBeforeNotify.clear();
    // <-- SYNCHRONIZED
    m_aFilterListeners.notifyEach( &XFilterControllerListener::disjunctiveTermAdded, aEvent );
}


bool FormController::determineLockState() const
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    // a.) in filter mode we are always locked
    // b.) if we have no valid model or our model (a result set) is not alive -> we're locked
    // c.) if we are inserting everything is OK and we are not locked
    // d.) if are not updatable or on invalid position
    Reference< XResultSet >  xResultSet(m_xModelAsIndex, UNO_QUERY);
    if (m_bFiltering || !xResultSet.is() || !isRowSetAlive(xResultSet))
        return true;
    else
        return !(m_bCanInsert && m_bCurrentRecordNew)
            && (xResultSet->isBeforeFirst() || xResultSet->isAfterLast() || xResultSet->rowDeleted() || !m_bCanUpdate);
}

//  FocusListener

void FormController::focusGained(const FocusEvent& e)
{
    // SYNCHRONIZED -->
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aControlBorderManager.focusGained( e.Source );

    Reference< XControl >  xControl(e.Source, UNO_QUERY);
    if (m_bDBConnection)
    {
        // do we need to keep the locking of the commit
        // we hold the lock as long as the control differs from the current
        // otherwise we disabled the lock
        m_bCommitLock = m_bCommitLock && xControl.get() != m_xCurrentControl.get();
        if (m_bCommitLock)
            return;

        // when do we have to commit a value to form or a filter
        // a.) if the current value is modified
        // b.) there must be a current control
        // c.) and it must be different from the new focus owning control or
        // d.) the focus is moving around (so we have only one control)

        if  (   ( m_bModified || m_bFiltering )
            &&  m_xCurrentControl.is()
            &&  (   ( xControl.get() != m_xCurrentControl.get() )
                ||  (   ( e.FocusFlags & FocusChangeReason::AROUND )
                    &&  ( m_bCycle || m_bFiltering )
                    )
                )
            )
        {
            // check the old control if the content is ok
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
            Reference< XBoundControl >  xLockingTest(m_xCurrentControl, UNO_QUERY);
            bool bControlIsLocked = xLockingTest.is() && xLockingTest->getLock();
            assert(!bControlIsLocked && "FormController::Gained: I'm modified and the current control is locked ? How this ?");
            // normally, a locked control should not be modified, so probably my bModified must
            // have been set from a different context, which I would not understand ...
#endif
            DBG_ASSERT(m_xCurrentControl.is(), "no CurrentControl set");
            // first the control ask if it supports the IFace
            Reference< XBoundComponent >  xBound(m_xCurrentControl, UNO_QUERY);
            if (!xBound.is() && m_xCurrentControl.is())
                xBound.set(m_xCurrentControl->getModel(), UNO_QUERY);

            // lock if we lose the focus during commit
            m_bCommitLock = true;

            // commit unsuccessful, reset focus
            if (xBound.is() && !xBound->commit())
            {
                // the commit failed and we don't commit again until the current control
                // which couldn't be commit gains the focus again
                Reference< XWindow >  xWindow(m_xCurrentControl, UNO_QUERY);
                if (xWindow.is())
                    xWindow->setFocus();
                return;
            }
            else
            {
                m_bModified = false;
                m_bCommitLock = false;
            }
        }

        if (!m_bFiltering && m_bCycle && (e.FocusFlags & FocusChangeReason::AROUND) && m_xCurrentControl.is())
        {
            SQLErrorEvent aErrorEvent;
            OSL_ENSURE( m_xFormOperations.is(), "FormController::focusGained: hmm?" );
                // should have been created in setModel
            try
            {
                if ( e.FocusFlags & FocusChangeReason::FORWARD )
                {
                    if ( m_xFormOperations.is() && m_xFormOperations->isEnabled( FormFeature::MoveToNext ) )
                        m_xFormOperations->execute( FormFeature::MoveToNext );
                }
                else // backward
                {
                    if ( m_xFormOperations.is() && m_xFormOperations->isEnabled( FormFeature::MoveToPrevious ) )
                        m_xFormOperations->execute( FormFeature::MoveToPrevious );
                }
            }
            catch ( const Exception& )
            {
                // don't handle this any further. That's an ... admissible error.
                DBG_UNHANDLED_EXCEPTION("svx");
            }
        }
    }

    // still one and the same control
    if  (   ( m_xActiveControl == xControl )
        &&  ( xControl == m_xCurrentControl )
        )
    {
        DBG_ASSERT(m_xCurrentControl.is(), "No CurrentControl selected");
        return;
    }

    bool bActivated = !m_xActiveControl.is() && xControl.is();

    m_xActiveControl  = xControl;

    implSetCurrentControl( xControl );
    SAL_WARN_IF( !m_xCurrentControl.is(), "svx.form", "implSetCurrentControl did nonsense!" );

    if ( bActivated )
    {
        // (asynchronously) call activation handlers
        m_aActivationEvent.Call();

        // call modify listeners
        if ( m_bModified )
            m_aModifyListeners.notifyEach( &XModifyListener::modified, EventObject( *this ) );
    }

    // invalidate all features which depend on the currently focused control
    if ( m_bDBConnection && !m_bFiltering )
        implInvalidateCurrentControlDependentFeatures();

    if ( !m_xCurrentControl.is() )
        return;

    // control gets focus, then possibly in the visible range
    Reference< XFormControllerContext > xContext( m_xFormControllerContext );
    Reference< XControl > xCurrentControl( m_xCurrentControl );
    aGuard.clear();
    // <-- SYNCHRONIZED

    if ( xContext.is() )
        xContext->makeVisible( xCurrentControl );
}


IMPL_LINK_NOARG( FormController, OnActivated, void*, void )
{
    EventObject aEvent;
    aEvent.Source = *this;
    m_aActivateListeners.notifyEach( &XFormControllerListener::formActivated, aEvent );
}


IMPL_LINK_NOARG( FormController, OnDeactivated, void*, void )
{
    EventObject aEvent;
    aEvent.Source = *this;
    m_aActivateListeners.notifyEach( &XFormControllerListener::formDeactivated, aEvent );
}


void FormController::focusLost(const FocusEvent& e)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );

    m_aControlBorderManager.focusLost( e.Source );

    Reference< XWindowPeer >  xNext(e.NextFocus, UNO_QUERY);
    Reference< XControl >  xNextControl = isInList(xNext);
    if (!xNextControl.is())
    {
        m_xActiveControl = nullptr;
        m_aDeactivationEvent.Call();
    }
}


void SAL_CALL FormController::mousePressed( const awt::MouseEvent& /*_rEvent*/ )
{
    // not interested in
}


void SAL_CALL FormController::mouseReleased( const awt::MouseEvent& /*_rEvent*/ )
{
    // not interested in
}


void SAL_CALL FormController::mouseEntered( const awt::MouseEvent& _rEvent )
{
    m_aControlBorderManager.mouseEntered( _rEvent.Source );
}


void SAL_CALL FormController::mouseExited( const awt::MouseEvent& _rEvent )
{
    m_aControlBorderManager.mouseExited( _rEvent.Source );
}


void SAL_CALL FormController::componentValidityChanged( const EventObject& _rSource )
{
    Reference< XControl > xControl( findControl( m_aControls, Reference< XControlModel >( _rSource.Source, UNO_QUERY ), false, false ) );
    Reference< XValidatableFormComponent > xValidatable( _rSource.Source, UNO_QUERY );

    OSL_ENSURE( xControl.is() && xValidatable.is(), "FormController::componentValidityChanged: huh?" );

    if ( xControl.is() && xValidatable.is() )
        m_aControlBorderManager.validityChanged( xControl, xValidatable );
}


void FormController::setModel(const Reference< XTabControllerModel > & Model)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    DBG_ASSERT(m_xTabController.is(), "FormController::setModel : invalid aggregate !");

    try
    {
        // disconnect from the old model
        if (m_xModelAsIndex.is())
        {
            if (m_bDBConnection)
            {
                // we are currently working on the model
                EventObject aEvt(m_xModelAsIndex);
                unloaded(aEvt);
            }

            Reference< XLoadable >  xForm(m_xModelAsIndex, UNO_QUERY);
            if (xForm.is())
                xForm->removeLoadListener(this);

            Reference< XSQLErrorBroadcaster >  xBroadcaster(m_xModelAsIndex, UNO_QUERY);
            if (xBroadcaster.is())
                xBroadcaster->removeSQLErrorListener(this);

            Reference< XDatabaseParameterBroadcaster >  xParamBroadcaster(m_xModelAsIndex, UNO_QUERY);
            if (xParamBroadcaster.is())
                xParamBroadcaster->removeParameterListener(this);

        }

        disposeAllFeaturesAndDispatchers();

        if ( m_xFormOperations.is() )
            m_xFormOperations->dispose();
        m_xFormOperations.clear();

        // set the new model wait for the load event
        if (m_xTabController.is())
            m_xTabController->setModel(Model);
        m_xModelAsIndex.set(Model, UNO_QUERY);
        m_xModelAsManager.set(Model, UNO_QUERY);

        // only if both ifaces exit, the controller will work successful
        if (!m_xModelAsIndex.is() || !m_xModelAsManager.is())
        {
            m_xModelAsManager = nullptr;
            m_xModelAsIndex = nullptr;
        }

        if (m_xModelAsIndex.is())
        {
            // re-create m_xFormOperations
            m_xFormOperations = FormOperations::createWithFormController( m_xComponentContext, this );
            m_xFormOperations->setFeatureInvalidation( this );

            // adding load and ui interaction listeners
            Reference< XLoadable >  xForm(Model, UNO_QUERY);
            if (xForm.is())
                xForm->addLoadListener(this);

            Reference< XSQLErrorBroadcaster >  xBroadcaster(Model, UNO_QUERY);
            if (xBroadcaster.is())
                xBroadcaster->addSQLErrorListener(this);

            Reference< XDatabaseParameterBroadcaster >  xParamBroadcaster(Model, UNO_QUERY);
            if (xParamBroadcaster.is())
                xParamBroadcaster->addParameterListener(this);

            // well, is the database already loaded?
            // then we have to simulate a load event
            Reference< XLoadable >  xCursor(m_xModelAsIndex, UNO_QUERY);
            if (xCursor.is() && xCursor->isLoaded())
            {
                EventObject aEvt(xCursor);
                loaded(aEvt);
            }

            Reference< XPropertySet > xModelProps( m_xModelAsIndex, UNO_QUERY );
            Reference< XPropertySetInfo > xPropInfo( xModelProps->getPropertySetInfo() );
            if (  xPropInfo.is()
               && xPropInfo->hasPropertyByName( FM_PROP_DYNAMIC_CONTROL_BORDER )
               && xPropInfo->hasPropertyByName( FM_PROP_CONTROL_BORDER_COLOR_FOCUS )
               && xPropInfo->hasPropertyByName( FM_PROP_CONTROL_BORDER_COLOR_MOUSE )
               && xPropInfo->hasPropertyByName( FM_PROP_CONTROL_BORDER_COLOR_INVALID )
               )
            {
                bool bEnableDynamicControlBorder = lcl_shouldUseDynamicControlBorder(
                    xModelProps.get(), xModelProps->getPropertyValue( FM_PROP_DYNAMIC_CONTROL_BORDER ) );
                if ( bEnableDynamicControlBorder )
                    m_aControlBorderManager.enableDynamicBorderColor();
                else
                    m_aControlBorderManager.disableDynamicBorderColor();

                Color nColor;
                if ( xModelProps->getPropertyValue( FM_PROP_CONTROL_BORDER_COLOR_FOCUS ) >>= nColor )
                    m_aControlBorderManager.setStatusColor( ControlStatus::Focused, nColor );
                if ( xModelProps->getPropertyValue( FM_PROP_CONTROL_BORDER_COLOR_MOUSE ) >>= nColor )
                    m_aControlBorderManager.setStatusColor( ControlStatus::MouseHover, nColor );
                if ( xModelProps->getPropertyValue( FM_PROP_CONTROL_BORDER_COLOR_INVALID ) >>= nColor )
                    m_aControlBorderManager.setStatusColor( ControlStatus::Invalid, nColor );
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


Reference< XTabControllerModel >  FormController::getModel()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    DBG_ASSERT(m_xTabController.is(), "FormController::getModel : invalid aggregate !");
    if (!m_xTabController.is())
        return Reference< XTabControllerModel > ();
    return m_xTabController->getModel();
}


void FormController::addToEventAttacher(const Reference< XControl > & xControl)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    OSL_ENSURE( xControl.is(), "FormController::addToEventAttacher: invalid control - how did you reach this?" );
    if ( !xControl.is() )
        return; /* throw IllegalArgumentException(); */

    // register at the event attacher
    Reference< XFormComponent >  xComp(xControl->getModel(), UNO_QUERY);
    if (xComp.is() && m_xModelAsIndex.is())
    {
        // and look for the position of the ControlModel in it
        sal_uInt32 nPos = m_xModelAsIndex->getCount();
        Reference< XFormComponent > xTemp;
        for( ; nPos; )
        {
            m_xModelAsIndex->getByIndex(--nPos) >>= xTemp;
            if (xComp.get() == xTemp.get())
            {
                m_xModelAsManager->attach( nPos, Reference<XInterface>( xControl, UNO_QUERY ), makeAny(xControl) );
                break;
            }
        }
    }
}


void FormController::removeFromEventAttacher(const Reference< XControl > & xControl)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    OSL_ENSURE( xControl.is(), "FormController::removeFromEventAttacher: invalid control - how did you reach this?" );
    if ( !xControl.is() )
        return; /* throw IllegalArgumentException(); */

    // register at the event attacher
    Reference< XFormComponent >  xComp(xControl->getModel(), UNO_QUERY);
    if ( xComp.is() && m_xModelAsIndex.is() )
    {
        // and look for the position of the ControlModel in it
        sal_uInt32 nPos = m_xModelAsIndex->getCount();
        Reference< XFormComponent > xTemp;
        for( ; nPos; )
        {
            m_xModelAsIndex->getByIndex(--nPos) >>= xTemp;
            if (xComp.get() == xTemp.get())
            {
                m_xModelAsManager->detach( nPos, Reference<XInterface>( xControl, UNO_QUERY ) );
                break;
            }
        }
    }
}


void FormController::setContainer(const Reference< XControlContainer > & xContainer)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    Reference< XTabControllerModel >  xTabModel(getModel());
    DBG_ASSERT(xTabModel.is() || !xContainer.is(), "No Model defined");
        // if we have a new container we need a model
    DBG_ASSERT(m_xTabController.is(), "FormController::setContainer : invalid aggregate !");

    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XContainer >  xCurrentContainer;
    if (m_xTabController.is())
        xCurrentContainer.set(m_xTabController->getContainer(), UNO_QUERY);
    if (xCurrentContainer.is())
    {
        xCurrentContainer->removeContainerListener(this);

        if ( m_aTabActivationIdle.IsActive() )
            m_aTabActivationIdle.Stop();

        // clear the filter map
        ::std::for_each( m_aFilterComponents.begin(), m_aFilterComponents.end(), RemoveComponentTextListener( this ) );
        m_aFilterComponents.clear();

        // collecting the controls
        const Reference< XControl >* pControls = m_aControls.getConstArray();
        const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
        while ( pControls != pControlsEnd )
            implControlRemoved( *pControls++, true );

        // make database-specific things
        if (m_bDBConnection && isListeningForChanges())
            stopListening();

        m_aControls.realloc( 0 );
    }

    if (m_xTabController.is())
        m_xTabController->setContainer(xContainer);

    // What controls belong to the container?
    if (xContainer.is() && xTabModel.is())
    {
        Sequence< Reference< XControlModel > > aModels = xTabModel->getControlModels();
        const Reference< XControlModel > * pModels = aModels.getConstArray();
        Sequence< Reference< XControl > > aAllControls = xContainer->getControls();

        sal_Int32 nCount = aModels.getLength();
        m_aControls = Sequence< Reference< XControl > >( nCount );
        Reference< XControl > * pControls = m_aControls.getArray();

        // collecting the controls
        sal_Int32 i, j;
        for (i = 0, j = 0; i < nCount; ++i, ++pModels )
        {
            Reference< XControl > xControl = findControl( aAllControls, *pModels, false, true );
            if ( xControl.is() )
            {
                pControls[j++] = xControl;
                implControlInserted( xControl, true );
            }
        }

        // not every model had an associated control
        if (j != i)
            m_aControls.realloc(j);

        // listen at the container
        Reference< XContainer >  xNewContainer(xContainer, UNO_QUERY);
        if (xNewContainer.is())
            xNewContainer->addContainerListener(this);

        // make database-specific things
        if (m_bDBConnection)
        {
            m_bLocked = determineLockState();
            setLocks();
            if (!isLocked())
                startListening();
        }
    }
    // the controls are in the right order
    m_bControlsSorted = true;
}


Reference< XControlContainer >  FormController::getContainer()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    DBG_ASSERT(m_xTabController.is(), "FormController::getContainer : invalid aggregate !");
    if (!m_xTabController.is())
        return Reference< XControlContainer > ();
    return m_xTabController->getContainer();
}


Sequence< Reference< XControl > > FormController::getControls()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    if (!m_bControlsSorted)
    {
        Reference< XTabControllerModel >  xModel = getModel();
        if (!xModel.is())
            return m_aControls;

        Sequence< Reference< XControlModel > > aControlModels = xModel->getControlModels();
        const Reference< XControlModel > * pModels = aControlModels.getConstArray();
        sal_Int32 nModels = aControlModels.getLength();

        Sequence< Reference< XControl > > aNewControls(nModels);

        Reference< XControl > * pControls = aNewControls.getArray();
        Reference< XControl >  xControl;

        // rearrange the controls according to the tab order sequence
        sal_Int32 j = 0;
        for (sal_Int32 i = 0; i < nModels; ++i, ++pModels )
        {
            xControl = findControl( m_aControls, *pModels, true, true );
            if ( xControl.is() )
                pControls[j++] = xControl;
        }

        // not every model had an associated control
        if ( j != nModels )
            aNewControls.realloc( j );

        m_aControls = aNewControls;
        m_bControlsSorted = true;
    }
    return m_aControls;
}


void FormController::autoTabOrder()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    DBG_ASSERT(m_xTabController.is(), "FormController::autoTabOrder : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->autoTabOrder();
}


void FormController::activateTabOrder()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    DBG_ASSERT(m_xTabController.is(), "FormController::activateTabOrder : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->activateTabOrder();
}


void FormController::setControlLock(const Reference< XControl > & xControl)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    bool bLocked = isLocked();

    // It is locked
    // a. if the entire record is locked
    // b. if the associated field is locked
    Reference< XBoundControl >  xBound(xControl, UNO_QUERY);
    if (xBound.is() &&
        ( (bLocked && bLocked != bool(xBound->getLock())) ||
          !bLocked))    // always uncheck individual fields when unlocking
    {
        // there is a data source
        Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
        if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
        {
            // what about the ReadOnly and Enable properties
            bool bTouch = true;
            if (::comphelper::hasProperty(FM_PROP_ENABLED, xSet))
                bTouch = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ENABLED));
            if (::comphelper::hasProperty(FM_PROP_READONLY, xSet))
                bTouch = !::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_READONLY));

            if (bTouch)
            {
                Reference< XPropertySet >  xField;
                xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;
                if (xField.is())
                {
                    if (bLocked)
                        xBound->setLock(bLocked);
                    else
                    {
                        try
                        {
                            Any aVal = xField->getPropertyValue(FM_PROP_ISREADONLY);
                            if (aVal.hasValue() && ::comphelper::getBOOL(aVal))
                                xBound->setLock(true);
                            else
                                xBound->setLock(bLocked);
                        }
                        catch( const Exception& )
                        {
                            DBG_UNHANDLED_EXCEPTION("svx");
                        }

                    }
                }
            }
        }
    }
}


void FormController::setLocks()
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    // lock/unlock all controls connected to a data source
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
        setControlLock( *pControls++ );
}


namespace
{
    bool lcl_shouldListenForModifications( const Reference< XControl >& _rxControl, const Reference< XPropertyChangeListener >& _rxBoundFieldListener )
    {
        bool bShould = false;

        Reference< XBoundComponent > xBound( _rxControl, UNO_QUERY );
        if ( xBound.is() )
        {
            bShould = true;
        }
        else if ( _rxControl.is() )
        {
            Reference< XPropertySet > xModelProps( _rxControl->getModel(), UNO_QUERY );
            if ( xModelProps.is() && ::comphelper::hasProperty( FM_PROP_BOUNDFIELD, xModelProps ) )
            {
                Reference< XPropertySet > xField;
                xModelProps->getPropertyValue( FM_PROP_BOUNDFIELD ) >>= xField;
                bShould = xField.is();

                if ( !bShould && _rxBoundFieldListener.is() )
                    xModelProps->addPropertyChangeListener( FM_PROP_BOUNDFIELD, _rxBoundFieldListener );
            }
        }

        return bShould;
    }
}


void FormController::startControlModifyListening(const Reference< XControl > & xControl)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );

    bool bModifyListening = lcl_shouldListenForModifications( xControl, this );

    // artificial while
    while ( bModifyListening )
    {
        Reference< XModifyBroadcaster >  xMod(xControl, UNO_QUERY);
        if (xMod.is())
        {
            xMod->addModifyListener(this);
            break;
        }

        // all the text to prematurely recognize a modified
        Reference< XTextComponent >  xText(xControl, UNO_QUERY);
        if (xText.is())
        {
            xText->addTextListener(this);
            break;
        }

        Reference< XCheckBox >  xBox(xControl, UNO_QUERY);
        if (xBox.is())
        {
            xBox->addItemListener(this);
            break;
        }

        Reference< XComboBox >  xCbBox(xControl, UNO_QUERY);
        if (xCbBox.is())
        {
            xCbBox->addItemListener(this);
            break;
        }

        Reference< XListBox >  xListBox(xControl, UNO_QUERY);
        if (xListBox.is())
        {
            xListBox->addItemListener(this);
            break;
        }
        break;
    }
}


void FormController::stopControlModifyListening(const Reference< XControl > & xControl)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );

    bool bModifyListening = lcl_shouldListenForModifications( xControl, nullptr );

    // artificial while
    while (bModifyListening)
    {
        Reference< XModifyBroadcaster >  xMod(xControl, UNO_QUERY);
        if (xMod.is())
        {
            xMod->removeModifyListener(this);
            break;
        }
        // all the text to prematurely recognize a modified
        Reference< XTextComponent >  xText(xControl, UNO_QUERY);
        if (xText.is())
        {
            xText->removeTextListener(this);
            break;
        }

        Reference< XCheckBox >  xBox(xControl, UNO_QUERY);
        if (xBox.is())
        {
            xBox->removeItemListener(this);
            break;
        }

        Reference< XComboBox >  xCbBox(xControl, UNO_QUERY);
        if (xCbBox.is())
        {
            xCbBox->removeItemListener(this);
            break;
        }

        Reference< XListBox >  xListBox(xControl, UNO_QUERY);
        if (xListBox.is())
        {
            xListBox->removeItemListener(this);
            break;
        }
        break;
    }
}


void FormController::startListening()
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    m_bModified  = false;

    // now register at bound fields
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
        startControlModifyListening( *pControls++ );
}


void FormController::stopListening()
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    m_bModified  = false;

    // now register at bound fields
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
        stopControlModifyListening( *pControls++ );
}


Reference< XControl >  FormController::findControl(Sequence< Reference< XControl > >& _rControls, const Reference< XControlModel > & xCtrlModel ,bool _bRemove,bool _bOverWrite) const
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    DBG_ASSERT( xCtrlModel.is(), "findControl - which ?!" );

    Reference< XControl >* pControls = _rControls.getArray();
    Reference< XControlModel >  xModel;
    for ( sal_Int32 i = 0, nCount = _rControls.getLength(); i < nCount; ++i, ++pControls )
    {
        if ( pControls->is() )
        {
            xModel = (*pControls)->getModel();
            if ( xModel.get() == xCtrlModel.get() )
            {
                Reference< XControl > xControl( *pControls );
                if ( _bRemove )
                    ::comphelper::removeElementAt( _rControls, i );
                else if ( _bOverWrite )
                    pControls->clear();
                return xControl;
            }
        }
    }
    return Reference< XControl > ();
}


void FormController::implControlInserted( const Reference< XControl>& _rxControl, bool _bAddToEventAttacher )
{
    Reference< XWindow > xWindow( _rxControl, UNO_QUERY );
    if ( xWindow.is() )
    {
        xWindow->addFocusListener( this );
        xWindow->addMouseListener( this );

        if ( _bAddToEventAttacher )
            addToEventAttacher( _rxControl );
    }

    // add a dispatch interceptor to the control (if supported)
    Reference< XDispatchProviderInterception > xInterception( _rxControl, UNO_QUERY );
    if ( xInterception.is() )
        createInterceptor( xInterception );

    if ( _rxControl.is() )
    {
        Reference< XControlModel > xModel( _rxControl->getModel() );

        // we want to know about the reset of the model of our controls
        // (for correctly resetting m_bModified)
        Reference< XReset >  xReset( xModel, UNO_QUERY );
        if ( xReset.is() )
            xReset->addResetListener( this );

        // and we want to know about the validity, to visually indicate it
        Reference< XValidatableFormComponent > xValidatable( xModel, UNO_QUERY );
        if ( xValidatable.is() )
        {
            xValidatable->addFormComponentValidityListener( this );
            m_aControlBorderManager.validityChanged( _rxControl, xValidatable );
        }
    }

}


void FormController::implControlRemoved( const Reference< XControl>& _rxControl, bool _bRemoveFromEventAttacher )
{
    Reference< XWindow > xWindow( _rxControl, UNO_QUERY );
    if ( xWindow.is() )
    {
        xWindow->removeFocusListener( this );
        xWindow->removeMouseListener( this );

        if ( _bRemoveFromEventAttacher )
            removeFromEventAttacher( _rxControl );
    }

    Reference< XDispatchProviderInterception > xInterception( _rxControl, UNO_QUERY);
    if ( xInterception.is() )
        deleteInterceptor( xInterception );

    if ( _rxControl.is() )
    {
        Reference< XControlModel > xModel( _rxControl->getModel() );

        Reference< XReset >  xReset( xModel, UNO_QUERY );
        if ( xReset.is() )
            xReset->removeResetListener( this );

        Reference< XValidatableFormComponent > xValidatable( xModel, UNO_QUERY );
        if ( xValidatable.is() )
            xValidatable->removeFormComponentValidityListener( this );
    }
}


void FormController::implSetCurrentControl( const Reference< XControl >& _rxControl )
{
    if ( m_xCurrentControl.get() == _rxControl.get() )
        return;

    Reference< XGridControl > xGridControl( m_xCurrentControl, UNO_QUERY );
    if ( xGridControl.is() )
        xGridControl->removeGridControlListener( this );

    m_xCurrentControl = _rxControl;

    xGridControl.set( m_xCurrentControl, UNO_QUERY );
    if ( xGridControl.is() )
        xGridControl->addGridControlListener( this );
}


void FormController::insertControl(const Reference< XControl > & xControl)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    m_bControlsSorted = false;
    m_aControls.realloc(m_aControls.getLength() + 1);
    m_aControls.getArray()[m_aControls.getLength() - 1] = xControl;

    if (m_pColumnInfoCache)
        m_pColumnInfoCache->deinitializeControls();

    implControlInserted( xControl, m_bAttachEvents );

    if (m_bDBConnection && !m_bFiltering)
        setControlLock(xControl);

    if (isListeningForChanges() && m_bAttachEvents)
        startControlModifyListening( xControl );
}


void FormController::removeControl(const Reference< XControl > & xControl)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
    {
        if ( xControl.get() == (*pControls++).get() )
        {
            ::comphelper::removeElementAt( m_aControls, pControls - m_aControls.getConstArray() - 1 );
            break;
        }
    }

    FilterComponents::iterator componentPos = ::std::find( m_aFilterComponents.begin(), m_aFilterComponents.end(), xControl );
    if ( componentPos != m_aFilterComponents.end() )
        m_aFilterComponents.erase( componentPos );

    implControlRemoved( xControl, m_bDetachEvents );

    if ( isListeningForChanges() && m_bDetachEvents )
        stopControlModifyListening( xControl );
}

// XLoadListener

void FormController::loaded(const EventObject& rEvent)
{
    OSL_ENSURE( rEvent.Source == m_xModelAsIndex, "FormController::loaded: where did this come from?" );

    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XRowSet >  xForm(rEvent.Source, UNO_QUERY);
    // do we have a connected data source
    if (xForm.is() && getConnection(xForm).is())
    {
        Reference< XPropertySet >  xSet(xForm, UNO_QUERY);
        if (xSet.is())
        {
            Any aVal        = xSet->getPropertyValue(FM_PROP_CYCLE);
            sal_Int32 aVal2 = 0;
            ::cppu::enum2int(aVal2,aVal);
            m_bCycle        = !aVal.hasValue() || static_cast<form::TabulatorCycle>(aVal2) == TabulatorCycle_RECORDS;
            m_bCanUpdate    = canUpdate(xSet);
            m_bCanInsert    = canInsert(xSet);
            m_bCurrentRecordModified = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED));
            m_bCurrentRecordNew      = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));

            startFormListening( xSet, false );

            // set the locks for the current controls
            if (getContainer().is())
            {
                m_aLoadEvent.Call();
            }
        }
        else
        {
            m_bCanInsert = m_bCanUpdate = m_bCycle = false;
            m_bCurrentRecordModified = false;
            m_bCurrentRecordNew = false;
            m_bLocked = false;
        }
        m_bDBConnection = true;
    }
    else
    {
        m_bDBConnection = false;
        m_bCanInsert = m_bCanUpdate = m_bCycle = false;
        m_bCurrentRecordModified = false;
        m_bCurrentRecordNew = false;
        m_bLocked = false;
    }

    Reference< XColumnsSupplier > xFormColumns( xForm, UNO_QUERY );
    m_pColumnInfoCache.reset( xFormColumns.is() ? new ColumnInfoCache( xFormColumns ) : nullptr );

    updateAllDispatchers();
}


void FormController::updateAllDispatchers() const
{
    ::std::for_each(
        m_aFeatureDispatchers.begin(),
        m_aFeatureDispatchers.end(),
        [] (const DispatcherContainer::value_type& dispatcher) {
            UpdateAllListeners()(dispatcher.second);
        });
}


IMPL_LINK_NOARG(FormController, OnLoad, void*, void)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    m_bLocked = determineLockState();

    setLocks();

    if (!m_bLocked)
        startListening();

    // just one exception toggle the auto values
    if (m_bCurrentRecordNew)
        toggleAutoFields(true);
}


void FormController::unloaded(const EventObject& /*rEvent*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    updateAllDispatchers();
}


void FormController::reloading(const EventObject& /*aEvent*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    // do the same like in unloading
    // just one exception toggle the auto values
    m_aToggleEvent.CancelPendingCall();
    unload();
}


void FormController::reloaded(const EventObject& aEvent)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    loaded(aEvent);
}


void FormController::unloading(const EventObject& /*aEvent*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    unload();
}


void FormController::unload()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aLoadEvent.CancelPendingCall();

    // be sure not to have autofields
    if (m_bCurrentRecordNew)
        toggleAutoFields(false);

    // remove bound field listing again
    removeBoundFieldListener();

    if (m_bDBConnection && isListeningForChanges())
        stopListening();

    Reference< XPropertySet >  xSet( m_xModelAsIndex, UNO_QUERY );
    if ( m_bDBConnection && xSet.is() )
        stopFormListening( xSet, false );

    m_bDBConnection = false;
    m_bCanInsert = m_bCanUpdate = m_bCycle = false;
    m_bCurrentRecordModified = m_bCurrentRecordNew = m_bLocked = false;

    m_pColumnInfoCache.reset();
}


void FormController::removeBoundFieldListener()
{
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
    {
        Reference< XPropertySet > xProp( *pControls++, UNO_QUERY );
        if ( xProp.is() )
            xProp->removePropertyChangeListener( FM_PROP_BOUNDFIELD, this );
    }
}


void FormController::startFormListening( const Reference< XPropertySet >& _rxForm, bool _bPropertiesOnly )
{
    try
    {
        if ( m_bCanInsert || m_bCanUpdate )   // form can be modified
        {
            _rxForm->addPropertyChangeListener( FM_PROP_ISNEW, this );
            _rxForm->addPropertyChangeListener( FM_PROP_ISMODIFIED, this );

            if ( !_bPropertiesOnly )
            {
                // set the Listener for UI interaction
                Reference< XRowSetApproveBroadcaster > xApprove( _rxForm, UNO_QUERY );
                if ( xApprove.is() )
                    xApprove->addRowSetApproveListener( this );

                // listener for row set changes
                Reference< XRowSet > xRowSet( _rxForm, UNO_QUERY );
                if ( xRowSet.is() )
                    xRowSet->addRowSetListener( this );
            }
        }

        Reference< XPropertySetInfo > xInfo = _rxForm->getPropertySetInfo();
        if ( xInfo.is() && xInfo->hasPropertyByName( FM_PROP_DYNAMIC_CONTROL_BORDER ) )
            _rxForm->addPropertyChangeListener( FM_PROP_DYNAMIC_CONTROL_BORDER, this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


void FormController::stopFormListening( const Reference< XPropertySet >& _rxForm, bool _bPropertiesOnly )
{
    try
    {
        if ( m_bCanInsert || m_bCanUpdate )
        {
            _rxForm->removePropertyChangeListener( FM_PROP_ISNEW, this );
            _rxForm->removePropertyChangeListener( FM_PROP_ISMODIFIED, this );

            if ( !_bPropertiesOnly )
            {
                Reference< XRowSetApproveBroadcaster > xApprove( _rxForm, UNO_QUERY );
                if (xApprove.is())
                    xApprove->removeRowSetApproveListener(this);

                Reference< XRowSet > xRowSet( _rxForm, UNO_QUERY );
                if ( xRowSet.is() )
                    xRowSet->removeRowSetListener( this );
            }
        }

        Reference< XPropertySetInfo > xInfo = _rxForm->getPropertySetInfo();
        if ( xInfo.is() && xInfo->hasPropertyByName( FM_PROP_DYNAMIC_CONTROL_BORDER ) )
            _rxForm->removePropertyChangeListener( FM_PROP_DYNAMIC_CONTROL_BORDER, this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}

// css::sdbc::XRowSetListener

void FormController::cursorMoved(const EventObject& /*event*/)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    // toggle the locking ?
    if (m_bLocked != determineLockState())
    {
        m_bLocked = !m_bLocked;
        setLocks();
        if (isListeningForChanges())
            startListening();
        else
            stopListening();
    }

    // neither the current control nor the current record are modified anymore
    m_bCurrentRecordModified = m_bModified = false;
}


void FormController::rowChanged(const EventObject& /*event*/)
{
    // not interested in ...
}

void FormController::rowSetChanged(const EventObject& /*event*/)
{
    // not interested in ...
}


// XContainerListener

void SAL_CALL FormController::elementInserted(const ContainerEvent& evt)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    Reference< XControl > xControl( evt.Element, UNO_QUERY );
    if ( !xControl.is() )
        return;

    Reference< XFormComponent >  xModel(xControl->getModel(), UNO_QUERY);
    if (xModel.is() && m_xModelAsIndex == xModel->getParent())
    {
        insertControl(xControl);

        if ( m_aTabActivationIdle.IsActive() )
            m_aTabActivationIdle.Stop();

        m_aTabActivationIdle.Start();
    }
    // are we in filtermode and a XModeSelector has inserted an element
    else if (m_bFiltering && Reference< XModeSelector > (evt.Source, UNO_QUERY).is())
    {
        xModel.set(evt.Source, UNO_QUERY);
        if (xModel.is() && m_xModelAsIndex == xModel->getParent())
        {
            Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
            if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
            {
                // does the model use a bound field ?
                Reference< XPropertySet >  xField;
                xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

                Reference< XTextComponent >  xText(xControl, UNO_QUERY);
                // may we filter the field?
                if (xText.is() && xField.is() && ::comphelper::hasProperty(FM_PROP_SEARCHABLE, xField) &&
                    ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_SEARCHABLE)))
                {
                    m_aFilterComponents.push_back( xText );
                    xText->addTextListener( this );
                }
            }
        }
    }
}


void SAL_CALL FormController::elementReplaced(const ContainerEvent& evt)
{
    // simulate an elementRemoved
    ContainerEvent aRemoveEvent( evt );
    aRemoveEvent.Element = evt.ReplacedElement;
    aRemoveEvent.ReplacedElement = Any();
    elementRemoved( aRemoveEvent );

    // simulate an elementInserted
    ContainerEvent aInsertEvent( evt );
    aInsertEvent.ReplacedElement = Any();
    elementInserted( aInsertEvent );
}


void SAL_CALL FormController::elementRemoved(const ContainerEvent& evt)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    Reference< XControl >  xControl;
    evt.Element >>= xControl;
    if (!xControl.is())
        return;

    Reference< XFormComponent >  xModel(xControl->getModel(), UNO_QUERY);
    if (xModel.is() && m_xModelAsIndex == xModel->getParent())
    {
        removeControl(xControl);
        // Do not recalculate TabOrder, because it must already work internally!
    }
    // are we in filtermode and a XModeSelector has inserted an element
    else if (m_bFiltering && Reference< XModeSelector > (evt.Source, UNO_QUERY).is())
    {
        FilterComponents::iterator componentPos = ::std::find(
            m_aFilterComponents.begin(), m_aFilterComponents.end(), xControl );
        if ( componentPos != m_aFilterComponents.end() )
            m_aFilterComponents.erase( componentPos );
    }
}


Reference< XControl >  FormController::isInList(const Reference< XWindowPeer > & xPeer) const
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    const Reference< XControl >* pControls = m_aControls.getConstArray();

    sal_uInt32 nCtrls = m_aControls.getLength();
    for ( sal_uInt32 n = 0; n < nCtrls && xPeer.is(); ++n, ++pControls )
    {
        if ( pControls->is() )
        {
            Reference< XVclWindowPeer >  xCtrlPeer( (*pControls)->getPeer(), UNO_QUERY);
            if ( ( xCtrlPeer.get() == xPeer.get() ) || xCtrlPeer->isChild( xPeer ) )
                return *pControls;
        }
    }
    return Reference< XControl > ();
}


void FormController::activateFirst()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    DBG_ASSERT(m_xTabController.is(), "FormController::activateFirst : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->activateFirst();
}


void FormController::activateLast()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    DBG_ASSERT(m_xTabController.is(), "FormController::activateLast : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->activateLast();
}

// XFormController

Reference< XFormOperations > SAL_CALL FormController::getFormOperations()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    return m_xFormOperations;
}


Reference< XControl> SAL_CALL FormController::getCurrentControl()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    return m_xCurrentControl;
}


void SAL_CALL FormController::addActivateListener(const Reference< XFormControllerListener > & l)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    m_aActivateListeners.addInterface(l);
}

void SAL_CALL FormController::removeActivateListener(const Reference< XFormControllerListener > & l)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    m_aActivateListeners.removeInterface(l);
}


void SAL_CALL FormController::addChildController( const Reference< XFormController >& ChildController )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    if ( !ChildController.is() )
        throw IllegalArgumentException( OUString(), *this, 1 );
        // TODO: (localized) error message

    // the parent of our (to-be-)child must be our own model
    Reference< XFormComponent > xFormOfChild( ChildController->getModel(), UNO_QUERY );
    if ( !xFormOfChild.is() )
        throw IllegalArgumentException( OUString(), *this, 1 );
        // TODO: (localized) error message

    if ( xFormOfChild->getParent() != m_xModelAsIndex )
        throw IllegalArgumentException( OUString(), *this, 1 );
        // TODO: (localized) error message

    m_aChildren.push_back( ChildController );
    ChildController->setParent( *this );

    // search the position of the model within the form
    sal_uInt32 nPos = m_xModelAsIndex->getCount();
    Reference< XFormComponent > xTemp;
    for( ; nPos; )
    {
        m_xModelAsIndex->getByIndex(--nPos) >>= xTemp;
        if ( xFormOfChild == xTemp )
        {
            m_xModelAsManager->attach( nPos, Reference<XInterface>( ChildController, UNO_QUERY ), makeAny( ChildController) );
            break;
        }
    }
}


Reference< XFormControllerContext > SAL_CALL FormController::getContext()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    return m_xFormControllerContext;
}


void SAL_CALL FormController::setContext( const Reference< XFormControllerContext >& _context )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    m_xFormControllerContext = _context;
}


Reference< XInteractionHandler > SAL_CALL FormController::getInteractionHandler()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    return m_xInteractionHandler;
}


void SAL_CALL FormController::setInteractionHandler( const Reference< XInteractionHandler >& _interactionHandler )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();
    m_xInteractionHandler = _interactionHandler;
}


void FormController::setFilter(::std::vector<FmFieldInfo>& rFieldInfos)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    // create the composer
    Reference< XRowSet > xForm(m_xModelAsIndex, UNO_QUERY);
    Reference< XConnection > xConnection(getConnection(xForm));
    if (xForm.is())
    {
        try
        {
            Reference< XMultiServiceFactory > xFactory( xConnection, UNO_QUERY_THROW );
            m_xComposer.set(
                xFactory->createInstance("com.sun.star.sdb.SingleSelectQueryComposer"),
                UNO_QUERY_THROW );

            Reference< XPropertySet > xSet( xForm, UNO_QUERY );
            OUString sStatement  = ::comphelper::getString( xSet->getPropertyValue( FM_PROP_ACTIVECOMMAND ) );
            OUString sFilter     = ::comphelper::getString( xSet->getPropertyValue( FM_PROP_FILTER ) );
            m_xComposer->setElementaryQuery( sStatement );
            m_xComposer->setFilter( sFilter );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }

    if (m_xComposer.is())
    {
        Sequence< Sequence < PropertyValue > > aFilterRows = m_xComposer->getStructuredFilter();

        // ok, we receive the list of filters as sequence of fieldnames, value
        // now we have to transform the fieldname into UI names, that could be a label of the field or
        // a aliasname or the fieldname itself

        // first adjust the field names if necessary
        Reference< XNameAccess > xQueryColumns =
            Reference< XColumnsSupplier >( m_xComposer, UNO_QUERY_THROW )->getColumns();

        for (auto& rFieldInfo : rFieldInfos)
        {
            if ( xQueryColumns->hasByName(rFieldInfo.aFieldName) )
            {
                if ( (xQueryColumns->getByName(rFieldInfo.aFieldName) >>= rFieldInfo.xField) && rFieldInfo.xField.is() )
                    rFieldInfo.xField->getPropertyValue(FM_PROP_REALNAME) >>= rFieldInfo.aFieldName;
            }
        }

        Reference< XDatabaseMetaData> xMetaData(xConnection->getMetaData());
        // now transfer the filters into Value/TextComponent pairs
        ::comphelper::UStringMixEqual aCompare(xMetaData->storesMixedCaseQuotedIdentifiers());

        // need to parse criteria localized
        Reference< XNumberFormatsSupplier> xFormatSupplier( getNumberFormats(xConnection, true));
        Reference< XNumberFormatter> xFormatter = NumberFormatter::create(m_xComponentContext);
        xFormatter->attachNumberFormatsSupplier(xFormatSupplier);
        Locale aAppLocale = Application::GetSettings().GetUILanguageTag().getLocale();
        const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetUILocaleDataWrapper() );
        /* FIXME: casting this to sal_Char is plain wrong and of course only
         * works for ASCII separators, but
         * pParseNode->parseNodeToPredicateStr() expects a sal_Char. Fix it
         * there. */
        sal_Char cDecimalSeparator = static_cast<sal_Char>(rLocaleWrapper.getNumDecimalSep()[0]);
        SAL_WARN_IF( static_cast<sal_Unicode>(cDecimalSeparator) != rLocaleWrapper.getNumDecimalSep()[0],
                "svx.form", "FormController::setFilter: wrong cast of decimal separator to sal_Char!");

        // retrieving the filter
        const Sequence < PropertyValue >* pRow = aFilterRows.getConstArray();
        for (sal_Int32 i = 0, nLen = aFilterRows.getLength(); i < nLen; ++i)
        {
            FmFilterRow aRow;

            // search a field for the given name
            const PropertyValue* pRefValues = pRow[i].getConstArray();
            for (sal_Int32 j = 0, nLen1 = pRow[i].getLength(); j < nLen1; j++)
            {
                // look for the text component
                Reference< XPropertySet > xField;
                try
                {
                    Reference< XPropertySet > xSet;
                    OUString aRealName;

                    // first look with the given name
                    if (xQueryColumns->hasByName(pRefValues[j].Name))
                    {
                        xQueryColumns->getByName(pRefValues[j].Name) >>= xSet;

                        // get the RealName
                        xSet->getPropertyValue("RealName") >>= aRealName;

                        // compare the condition field name and the RealName
                        if (aCompare(aRealName, pRefValues[j].Name))
                            xField = xSet;
                    }
                    if (!xField.is())
                    {
                        // no we have to check every column to find the realname
                        Reference< XIndexAccess > xColumnsByIndex(xQueryColumns, UNO_QUERY);
                        for (sal_Int32 n = 0, nCount = xColumnsByIndex->getCount(); n < nCount; n++)
                        {
                            xColumnsByIndex->getByIndex(n) >>= xSet;
                            xSet->getPropertyValue("RealName") >>= aRealName;
                            if (aCompare(aRealName, pRefValues[j].Name))
                            {
                                // get the column by its alias
                                xField = xSet;
                                break;
                            }
                        }
                    }
                    if (!xField.is())
                        continue;
                }
                catch (const Exception&)
                {
                    continue;
                }

                // find the text component
                for (const auto& rFieldInfo : rFieldInfos)
                {
                    // we found the field so insert a new entry to the filter row
                    if (rFieldInfo.xField == xField)
                    {
                        // do we already have the control ?
                        if (aRow.find(rFieldInfo.xText) != aRow.end())
                        {
                            OUString aCompText = aRow[rFieldInfo.xText];
                            aCompText += " ";
                            OString aVal = m_pParser->getContext().getIntlKeywordAscii(IParseContext::InternationalKeyCode::And);
                            aCompText += OUString(aVal.getStr(),aVal.getLength(),RTL_TEXTENCODING_ASCII_US);
                            aCompText += " ";
                            aCompText += ::comphelper::getString(pRefValues[j].Value);
                            aRow[rFieldInfo.xText] = aCompText;
                        }
                        else
                        {
                            OUString sPredicate,sErrorMsg;
                            pRefValues[j].Value >>= sPredicate;
                            std::shared_ptr< OSQLParseNode > pParseNode = predicateTree(sErrorMsg, sPredicate, xFormatter, xField);
                            if ( pParseNode != nullptr )
                            {
                                OUString sCriteria;
                                switch (pRefValues[j].Handle)
                                {
                                    case css::sdb::SQLFilterOperator::EQUAL:
                                        sCriteria += "=";
                                        break;
                                    case css::sdb::SQLFilterOperator::NOT_EQUAL:
                                        sCriteria += "!=";
                                        break;
                                    case css::sdb::SQLFilterOperator::LESS:
                                        sCriteria += "<";
                                        break;
                                    case css::sdb::SQLFilterOperator::GREATER:
                                        sCriteria += ">";
                                        break;
                                    case css::sdb::SQLFilterOperator::LESS_EQUAL:
                                        sCriteria += "<=";
                                        break;
                                    case css::sdb::SQLFilterOperator::GREATER_EQUAL:
                                        sCriteria += ">=";
                                        break;
                                    case css::sdb::SQLFilterOperator::LIKE:
                                        sCriteria += "LIKE ";
                                        break;
                                    case css::sdb::SQLFilterOperator::NOT_LIKE:
                                        sCriteria += "NOT LIKE ";
                                        break;
                                    case css::sdb::SQLFilterOperator::SQLNULL:
                                        sCriteria += "IS NULL";
                                        break;
                                    case css::sdb::SQLFilterOperator::NOT_SQLNULL:
                                        sCriteria += "IS NOT NULL";
                                        break;
                                }
                                pParseNode->parseNodeToPredicateStr( sCriteria
                                                                    ,xConnection
                                                                    ,xFormatter
                                                                    ,xField
                                                                    ,OUString()
                                                                    ,aAppLocale
                                                                    ,cDecimalSeparator
                                                                    ,getParseContext());
                                aRow[rFieldInfo.xText] = sCriteria;
                            }
                        }
                    }
                }
            }

            if (aRow.empty())
                continue;

            impl_addFilterRow( aRow );
        }
    }

    // now set the filter controls
    for (const auto& rFieldInfo : rFieldInfos)
    {
        m_aFilterComponents.push_back( rFieldInfo.xText );
    }
}


void FormController::startFiltering()
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );

    Reference< XConnection >  xConnection( getConnection( Reference< XRowSet >( m_xModelAsIndex, UNO_QUERY ) ) );
    if ( !xConnection.is() )
        // nothing to do - can't filter a form which is not connected
        return;

    // stop listening for controls
    if (isListeningForChanges())
        stopListening();

    m_bFiltering = true;

    // as we don't want new controls to be attached to the scripting environment
    // we change attach flags
    m_bAttachEvents = false;

    // exchanging the controls for the current form
    Sequence< Reference< XControl > > aControlsCopy( m_aControls );
    const Reference< XControl >* pControls = aControlsCopy.getConstArray();
    sal_Int32 nControlCount = aControlsCopy.getLength();

    // the control we have to activate after replacement
    Reference< XNumberFormatsSupplier >  xFormatSupplier = getNumberFormats(xConnection, true);
    Reference< XNumberFormatter >  xFormatter = NumberFormatter::create(m_xComponentContext);
    xFormatter->attachNumberFormatsSupplier(xFormatSupplier);

    // structure for storing the field info
    ::std::vector<FmFieldInfo> aFieldInfos;

    for (sal_Int32 i = nControlCount; i > 0;)
    {
        Reference< XControl > xControl = pControls[--i];
        if (xControl.is())
        {
            // no events for the control anymore
            removeFromEventAttacher(xControl);

            // do we have a mode selector
            Reference< XModeSelector >  xSelector(xControl, UNO_QUERY);
            if (xSelector.is())
            {
                xSelector->setMode( "FilterMode" );

                // listening for new controls of the selector
                Reference< XContainer >  xContainer(xSelector, UNO_QUERY);
                if (xContainer.is())
                    xContainer->addContainerListener(this);

                Reference< XEnumerationAccess >  xElementAccess(xSelector, UNO_QUERY);
                if (xElementAccess.is())
                {
                    Reference< XEnumeration >  xEnumeration(xElementAccess->createEnumeration());
                    Reference< XControl >  xSubControl;
                    while (xEnumeration->hasMoreElements())
                    {
                        xEnumeration->nextElement() >>= xSubControl;
                        if (xSubControl.is())
                        {
                            Reference< XPropertySet >  xSet(xSubControl->getModel(), UNO_QUERY);
                            if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
                            {
                                // does the model use a bound field ?
                                Reference< XPropertySet >  xField;
                                xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

                                Reference< XTextComponent >  xText(xSubControl, UNO_QUERY);
                                // may we filter the field?
                                if (xText.is() && xField.is() && ::comphelper::hasProperty(FM_PROP_SEARCHABLE, xField) &&
                                    ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_SEARCHABLE)))
                                {
                                    aFieldInfos.emplace_back(xField, xText);
                                    xText->addTextListener(this);
                                }
                            }
                        }
                    }
                }
                continue;
            }

            Reference< XPropertySet >  xModel( xControl->getModel(), UNO_QUERY );
            if (xModel.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xModel))
            {
                // does the model use a bound field ?
                Any aVal = xModel->getPropertyValue(FM_PROP_BOUNDFIELD);
                Reference< XPropertySet >  xField;
                aVal >>= xField;

                // may we filter the field?

                if  (   xField.is()
                    &&  ::comphelper::hasProperty( FM_PROP_SEARCHABLE, xField )
                    && ::comphelper::getBOOL( xField->getPropertyValue( FM_PROP_SEARCHABLE ) )
                    )
                {
                    // create a filter control
                    Reference< XControl > xFilterControl = form::control::FilterControl::createWithFormat(
                        m_xComponentContext,
                        VCLUnoHelper::GetInterface( getDialogParentWindow() ),
                        xFormatter,
                        xModel);

                    if ( replaceControl( xControl, xFilterControl ) )
                    {
                        Reference< XTextComponent > xFilterText( xFilterControl, UNO_QUERY );
                        aFieldInfos.emplace_back( xField, xFilterText );
                        xFilterText->addTextListener(this);
                    }
                }
            }
            else
            {
                // unsubscribe from EventManager
            }
        }
    }

    // we have all filter controls now, so the next step is to read the filters from the form
    // resolve all aliases and set the current filter to the according structure
    setFilter(aFieldInfos);

    Reference< XPropertySet > xSet( m_xModelAsIndex, UNO_QUERY );
    if ( xSet.is() )
        stopFormListening( xSet, true );

    impl_setTextOnAllFilter_throw();

    // lock all controls which are not used for filtering
    m_bLocked = determineLockState();
    setLocks();
    m_bAttachEvents = true;
}


void FormController::stopFiltering()
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    if ( !m_bFiltering ) // #104693# OJ
    {   // nothing to do
        return;
    }

    m_bFiltering = false;
    m_bDetachEvents = false;

    ::comphelper::disposeComponent(m_xComposer);

    // exchanging the controls for the current form
    Sequence< Reference< XControl > > aControlsCopy( m_aControls );
    const Reference< XControl > * pControls = aControlsCopy.getConstArray();
    sal_Int32 nControlCount = aControlsCopy.getLength();

    // clear the filter control map
    ::std::for_each( m_aFilterComponents.begin(), m_aFilterComponents.end(), RemoveComponentTextListener( this ) );
    m_aFilterComponents.clear();

    for ( sal_Int32 i = nControlCount; i > 0; )
    {
        Reference< XControl > xControl = pControls[--i];
        if (xControl.is())
        {
            // now enable event handling again
            addToEventAttacher(xControl);

            Reference< XModeSelector >  xSelector(xControl, UNO_QUERY);
            if (xSelector.is())
            {
                xSelector->setMode( "DataMode" );

                // listening for new controls of the selector
                Reference< XContainer >  xContainer(xSelector, UNO_QUERY);
                if (xContainer.is())
                    xContainer->removeContainerListener(this);
                continue;
            }

            Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
            if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
            {
                // does the model use a bound field ?
                Reference< XPropertySet >  xField;
                xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

                // may we filter the field?
                if  (   xField.is()
                    &&  ::comphelper::hasProperty( FM_PROP_SEARCHABLE, xField )
                    &&  ::comphelper::getBOOL( xField->getPropertyValue( FM_PROP_SEARCHABLE ) )
                    )
                {
                    OUString sServiceName;
                    OSL_VERIFY( xSet->getPropertyValue( FM_PROP_DEFAULTCONTROL ) >>= sServiceName );
                    Reference< XControl > xNewControl( m_xComponentContext->getServiceManager()->createInstanceWithContext( sServiceName, m_xComponentContext ), UNO_QUERY );
                    replaceControl( xControl, xNewControl );
                }
            }
        }
    }

    Reference< XPropertySet >  xSet( m_xModelAsIndex, UNO_QUERY );
    if ( xSet.is() )
        startFormListening( xSet, true );

    m_bDetachEvents = true;

    m_aFilterRows.clear();
    m_nCurrentFilterPosition = -1;

    // release the locks if possible
    // lock all controls which are not used for filtering
    m_bLocked = determineLockState();
    setLocks();

    // restart listening for control modifications
    if (isListeningForChanges())
        startListening();
}

// XModeSelector

void FormController::setMode(const OUString& Mode)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    if (!supportsMode(Mode))
        throw NoSupportException();

    if (Mode == m_aMode)
        return;

    m_aMode = Mode;

    if ( Mode == "FilterMode" )
        startFiltering();
    else
        stopFiltering();

    for (const auto& rChild : m_aChildren)
    {
        Reference< XModeSelector > xMode(rChild, UNO_QUERY);
        if ( xMode.is() )
            xMode->setMode(Mode);
    }
}


OUString SAL_CALL FormController::getMode()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    return m_aMode;
}


Sequence< OUString > SAL_CALL FormController::getSupportedModes()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    static Sequence< OUString > const aModes
    {
        "DataMode",
        "FilterMode"
    };
    return aModes;
}


sal_Bool SAL_CALL FormController::supportsMode(const OUString& Mode)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    Sequence< OUString > aModes(getSupportedModes());
    const OUString* pModes = aModes.getConstArray();
    for (sal_Int32 i = aModes.getLength(); i > 0; )
    {
        if (pModes[--i] == Mode)
            return true;
    }
    return false;
}


vcl::Window* FormController::getDialogParentWindow()
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    vcl::Window* pParentWindow = nullptr;
    try
    {
        Reference< XControl > xContainerControl( getContainer(), UNO_QUERY_THROW );
        Reference< XWindowPeer > xContainerPeer( xContainerControl->getPeer(), UNO_QUERY_THROW );
        pParentWindow = VCLUnoHelper::GetWindow( xContainerPeer ).get();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    return pParentWindow;
}

bool FormController::checkFormComponentValidity( OUString& /* [out] */ _rFirstInvalidityExplanation, Reference< XControlModel >& /* [out] */ _rxFirstInvalidModel )
{
    try
    {
        Reference< XEnumerationAccess > xControlEnumAcc( getModel(), UNO_QUERY );
        Reference< XEnumeration > xControlEnumeration;
        if ( xControlEnumAcc.is() )
            xControlEnumeration = xControlEnumAcc->createEnumeration();
        OSL_ENSURE( xControlEnumeration.is(), "FormController::checkFormComponentValidity: cannot enumerate the controls!" );
        if ( !xControlEnumeration.is() )
            // assume all valid
            return true;

        Reference< XValidatableFormComponent > xValidatable;
        while ( xControlEnumeration->hasMoreElements() )
        {
            if ( !( xControlEnumeration->nextElement() >>= xValidatable ) )
                // control does not support validation
                continue;

            if ( xValidatable->isValid() )
                continue;

            Reference< XValidator > xValidator( xValidatable->getValidator() );
            OSL_ENSURE( xValidator.is(), "FormController::checkFormComponentValidity: invalid, but no validator?" );
            if ( !xValidator.is() )
                // this violates the interface definition of css.form.validation.XValidatableFormComponent ...
                continue;

            _rFirstInvalidityExplanation = xValidator->explainInvalid( xValidatable->getCurrentValue() );
            _rxFirstInvalidModel.set(xValidatable, css::uno::UNO_QUERY);
            return false;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    return true;
}


Reference< XControl > FormController::locateControl( const Reference< XControlModel >& _rxModel )
{
    try
    {
        Sequence< Reference< XControl > > aControls( getControls() );

        for ( auto const & control : aControls )
        {
            OSL_ENSURE( control.is(), "FormController::locateControl: NULL-control?" );
            if ( control.is() )
            {
                if ( control->getModel() == _rxModel )
                    return control;
            }
        }
        OSL_FAIL( "FormController::locateControl: did not find a control for this model!" );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    return nullptr;
}


namespace
{
    void displayErrorSetFocus( const OUString& _rMessage, const Reference< XControl >& _rxFocusControl, vcl::Window* _pDialogParent )
    {
        SQLContext aError;
        aError.Message = SvxResId(RID_STR_WRITEERROR);
        aError.Details = _rMessage;
        displayException( aError, _pDialogParent );

        if ( _rxFocusControl.is() )
        {
            Reference< XWindow > xControlWindow( _rxFocusControl, UNO_QUERY );
            OSL_ENSURE( xControlWindow.is(), "displayErrorSetFocus: invalid control!" );
            if ( xControlWindow.is() )
                xControlWindow->setFocus();
        }
    }

    bool lcl_shouldValidateRequiredFields_nothrow( const Reference< XInterface >& _rxForm )
    {
        try
        {
            static const char s_sFormsCheckRequiredFields[] = "FormsCheckRequiredFields";

            // first, check whether the form has a property telling us the answer
            // this allows people to use the XPropertyContainer interface of a form to control
            // the behaviour on a per-form basis.
            Reference< XPropertySet > xFormProps( _rxForm, UNO_QUERY_THROW );
            Reference< XPropertySetInfo > xPSI( xFormProps->getPropertySetInfo() );
            if ( xPSI->hasPropertyByName( s_sFormsCheckRequiredFields ) )
            {
                bool bShouldValidate = true;
                OSL_VERIFY( xFormProps->getPropertyValue( s_sFormsCheckRequiredFields ) >>= bShouldValidate );
                return bShouldValidate;
            }

            // next, check the data source which created the connection
            Reference< XChild > xConnectionAsChild( xFormProps->getPropertyValue( FM_PROP_ACTIVE_CONNECTION ), UNO_QUERY_THROW );
            Reference< XPropertySet > xDataSource( xConnectionAsChild->getParent(), UNO_QUERY );
            if ( !xDataSource.is() )
                // seldom (but possible): this is not a connection created by a data source
                return true;

            Reference< XPropertySet > xDataSourceSettings(
                xDataSource->getPropertyValue("Settings"),
                UNO_QUERY_THROW );

            bool bShouldValidate = true;
            OSL_VERIFY( xDataSourceSettings->getPropertyValue( s_sFormsCheckRequiredFields ) >>= bShouldValidate );
            return bShouldValidate;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }

        return true;
    }
}

// XRowSetApproveListener

sal_Bool SAL_CALL FormController::approveRowChange(const RowChangeEvent& _rEvent)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    ::comphelper::OInterfaceIteratorHelper2 aIter(m_aRowSetApproveListeners);
    bool bValid = true;
    if (aIter.hasMoreElements())
    {
        RowChangeEvent aEvt( _rEvent );
        aEvt.Source = *this;
        bValid = static_cast<XRowSetApproveListener*>(aIter.next())->approveRowChange(aEvt);
    }

    if ( !bValid )
        return bValid;

    if  (   ( _rEvent.Action != RowChangeAction::INSERT )
        &&  ( _rEvent.Action != RowChangeAction::UPDATE )
        )
        return bValid;

    // if some of the control models are bound to validators, check them
    OUString sInvalidityExplanation;
    Reference< XControlModel > xInvalidModel;
    if ( !checkFormComponentValidity( sInvalidityExplanation, xInvalidModel ) )
    {
        Reference< XControl > xControl( locateControl( xInvalidModel ) );
        aGuard.clear();
        displayErrorSetFocus( sInvalidityExplanation, xControl, getDialogParentWindow() );
        return false;
    }

    // check values on NULL and required flag
    if ( !lcl_shouldValidateRequiredFields_nothrow( _rEvent.Source ) )
        return true;

    OSL_ENSURE(m_pColumnInfoCache, "FormController::approveRowChange: no column infos!");
    if (!m_pColumnInfoCache)
        return true;

    try
    {
        if ( !m_pColumnInfoCache->controlsInitialized() )
            m_pColumnInfoCache->initializeControls( getControls() );

        size_t colCount = m_pColumnInfoCache->getColumnCount();
        for ( size_t col = 0; col < colCount; ++col )
        {
            const ColumnInfo& rColInfo = m_pColumnInfoCache->getColumnInfo( col );

            if ( rColInfo.bAutoIncrement )
                continue;

            if ( rColInfo.bReadOnly )
                continue;

            if ( !rColInfo.xFirstControlWithInputRequired.is() && !rColInfo.xFirstGridWithInputRequiredColumn.is() )
            {
                continue;
            }

            // TODO: in case of binary fields, this "getString" below is extremely expensive
            if ( !rColInfo.xColumn->wasNull() || !rColInfo.xColumn->getString().isEmpty() )
                continue;

            OUString sMessage( SvxResId( RID_ERR_FIELDREQUIRED ) );
            sMessage = sMessage.replaceFirst( "#", rColInfo.sName );

            // the control to focus
            Reference< XControl > xControl( rColInfo.xFirstControlWithInputRequired );
            if ( !xControl.is() )
                xControl.set( rColInfo.xFirstGridWithInputRequiredColumn, UNO_QUERY );

            aGuard.clear();
            displayErrorSetFocus( sMessage, rColInfo.xFirstControlWithInputRequired, getDialogParentWindow() );
            return false;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    return true;
}


sal_Bool SAL_CALL FormController::approveCursorMove(const EventObject& event)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    ::comphelper::OInterfaceIteratorHelper2 aIter(m_aRowSetApproveListeners);
    if (aIter.hasMoreElements())
    {
        EventObject aEvt(event);
        aEvt.Source = *this;
        return static_cast<XRowSetApproveListener*>(aIter.next())->approveCursorMove(aEvt);
    }

    return true;
}


sal_Bool SAL_CALL FormController::approveRowSetChange(const EventObject& event)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    ::comphelper::OInterfaceIteratorHelper2 aIter(m_aRowSetApproveListeners);
    if (aIter.hasMoreElements())
    {
        EventObject aEvt(event);
        aEvt.Source = *this;
        return static_cast<XRowSetApproveListener*>(aIter.next())->approveRowSetChange(aEvt);
    }

    return true;
}

// XRowSetApproveBroadcaster

void SAL_CALL FormController::addRowSetApproveListener(const Reference< XRowSetApproveListener > & _rxListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aRowSetApproveListeners.addInterface(_rxListener);
}


void SAL_CALL FormController::removeRowSetApproveListener(const Reference< XRowSetApproveListener > & _rxListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aRowSetApproveListeners.removeInterface(_rxListener);
}

// XErrorListener

void SAL_CALL FormController::errorOccured(const SQLErrorEvent& aEvent)
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    ::comphelper::OInterfaceIteratorHelper2 aIter(m_aErrorListeners);
    if (aIter.hasMoreElements())
    {
        SQLErrorEvent aEvt(aEvent);
        aEvt.Source = *this;
        static_cast<XSQLErrorListener*>(aIter.next())->errorOccured(aEvt);
    }
    else
    {
        aGuard.clear();
        displayException(aEvent, getDialogParentWindow());
    }
}

// XErrorBroadcaster

void SAL_CALL FormController::addSQLErrorListener(const Reference< XSQLErrorListener > & aListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aErrorListeners.addInterface(aListener);
}


void SAL_CALL FormController::removeSQLErrorListener(const Reference< XSQLErrorListener > & aListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aErrorListeners.removeInterface(aListener);
}

// XDatabaseParameterBroadcaster2

void SAL_CALL FormController::addDatabaseParameterListener(const Reference< XDatabaseParameterListener > & aListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aParameterListeners.addInterface(aListener);
}


void SAL_CALL FormController::removeDatabaseParameterListener(const Reference< XDatabaseParameterListener > & aListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aParameterListeners.removeInterface(aListener);
}

// XDatabaseParameterBroadcaster

void SAL_CALL FormController::addParameterListener(const Reference< XDatabaseParameterListener > & aListener)
{
    FormController::addDatabaseParameterListener( aListener );
}


void SAL_CALL FormController::removeParameterListener(const Reference< XDatabaseParameterListener > & aListener)
{
    FormController::removeDatabaseParameterListener( aListener );
}

// XDatabaseParameterListener

sal_Bool SAL_CALL FormController::approveParameter(const DatabaseParameterEvent& aEvent)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    ::comphelper::OInterfaceIteratorHelper2 aIter(m_aParameterListeners);
    if (aIter.hasMoreElements())
    {
        DatabaseParameterEvent aEvt(aEvent);
        aEvt.Source = *this;
        return static_cast<XDatabaseParameterListener*>(aIter.next())->approveParameter(aEvt);
    }
    else
    {
        // default handling: instantiate an interaction handler and let it handle the parameter request
        try
        {
            if ( !ensureInteractionHandler() )
                return false;

            // two continuations allowed: OK and Cancel
            OParameterContinuation* pParamValues = new OParameterContinuation;
            OInteractionAbort* pAbort = new OInteractionAbort;
            // the request
            ParametersRequest aRequest;
            aRequest.Parameters = aEvent.Parameters;
            aRequest.Connection = getConnection(Reference< XRowSet >(aEvent.Source, UNO_QUERY));
            OInteractionRequest* pParamRequest = new OInteractionRequest(makeAny(aRequest));
            Reference< XInteractionRequest > xParamRequest(pParamRequest);
            // some knittings
            pParamRequest->addContinuation(pParamValues);
            pParamRequest->addContinuation(pAbort);

            // handle the request
            m_xInteractionHandler->handle(xParamRequest);

            if (!pParamValues->wasSelected())
                // canceled
                return false;

            // transfer the values into the parameter supplier
            Sequence< PropertyValue > aFinalValues = pParamValues->getValues();
            if (aFinalValues.getLength() != aRequest.Parameters->getCount())
            {
                OSL_FAIL("FormController::approveParameter: the InteractionHandler returned nonsense!");
                return false;
            }
            const PropertyValue* pFinalValues = aFinalValues.getConstArray();
            for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
            {
                Reference< XPropertySet > xParam(
                    aRequest.Parameters->getByIndex(i), css::uno::UNO_QUERY);
                if (xParam.is())
                {
#ifdef DBG_UTIL
                    OUString sName;
                    xParam->getPropertyValue(FM_PROP_NAME) >>= sName;
                    DBG_ASSERT(sName == pFinalValues->Name, "FormController::approveParameter: suspicious value names!");
#endif
                    try { xParam->setPropertyValue(FM_PROP_VALUE, pFinalValues->Value); }
                    catch(Exception&)
                    {
                        OSL_FAIL("FormController::approveParameter: setting one of the properties failed!");
                    }
                }
            }
        }
        catch(Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("svx");
        }
    }
    return true;
}

// XConfirmDeleteBroadcaster

void SAL_CALL FormController::addConfirmDeleteListener(const Reference< XConfirmDeleteListener > & aListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aDeleteListeners.addInterface(aListener);
}


void SAL_CALL FormController::removeConfirmDeleteListener(const Reference< XConfirmDeleteListener > & aListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    m_aDeleteListeners.removeInterface(aListener);
}

// XConfirmDeleteListener

sal_Bool SAL_CALL FormController::confirmDelete(const RowChangeEvent& aEvent)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    impl_checkDisposed_throw();

    ::comphelper::OInterfaceIteratorHelper2 aIter(m_aDeleteListeners);
    if (aIter.hasMoreElements())
    {
        RowChangeEvent aEvt(aEvent);
        aEvt.Source = *this;
        return static_cast<XConfirmDeleteListener*>(aIter.next())->confirmDelete(aEvt);
    }
    // default handling: instantiate an interaction handler and let it handle the request

    OUString sTitle;
    sal_Int32 nLength = aEvent.Rows;
    if ( nLength > 1 )
    {
        sTitle = SvxResId( RID_STR_DELETECONFIRM_RECORDS );
        sTitle = sTitle.replaceFirst( "#", OUString::number(nLength) );
    }
    else
        sTitle = SvxResId( RID_STR_DELETECONFIRM_RECORD );

    try
    {
        if ( !ensureInteractionHandler() )
            return false;

        // two continuations allowed: Yes and No
        OInteractionApprove* pApprove = new OInteractionApprove;
        OInteractionDisapprove* pDisapprove = new OInteractionDisapprove;

        // the request
        SQLWarning aWarning;
        aWarning.Message = sTitle;
        SQLWarning aDetails;
        aDetails.Message = SvxResId(RID_STR_DELETECONFIRM);
        aWarning.NextException <<= aDetails;

        OInteractionRequest* pRequest = new OInteractionRequest( makeAny( aWarning ) );
        Reference< XInteractionRequest > xRequest( pRequest );

        // some knittings
        pRequest->addContinuation( pApprove );
        pRequest->addContinuation( pDisapprove );

        // handle the request
        m_xInteractionHandler->handle( xRequest );

        if ( pApprove->wasSelected() )
            return true;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    return false;
}


void SAL_CALL FormController::invalidateFeatures( const Sequence< ::sal_Int16 >& Features )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // for now, just copy the ids of the features, because ....
    ::std::copy( Features.begin(), Features.end(),
        ::std::insert_iterator< ::std::set< sal_Int16 > >( m_aInvalidFeatures, m_aInvalidFeatures.begin() )
    );

    // ... we will do the real invalidation asynchronously
    if ( !m_aFeatureInvalidationTimer.IsActive() )
        m_aFeatureInvalidationTimer.Start();
}


void SAL_CALL FormController::invalidateAllFeatures(  )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );

    Sequence< sal_Int16 > aInterceptedFeatures( comphelper::mapKeysToSequence(m_aFeatureDispatchers) );

    aGuard.clear();
    if ( aInterceptedFeatures.getLength() )
        invalidateFeatures( aInterceptedFeatures );
}


Reference< XDispatch >
FormController::interceptedQueryDispatch( const URL& aURL,
                                            const OUString& /*aTargetFrameName*/, sal_Int32 /*nSearchFlags*/)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    Reference< XDispatch >  xReturn;
    // dispatches handled by ourself
    if  (   ( aURL.Complete == FMURL_CONFIRM_DELETION )
        ||  (   ( aURL.Complete == "private:/InteractionHandler" )
            &&  ensureInteractionHandler()
            )
        )
        xReturn = static_cast< XDispatch* >( this );

    // dispatches of FormSlot-URLs we have to translate
    if ( !xReturn.is() && m_xFormOperations.is() )
    {
        // find the slot id which corresponds to the URL
        sal_Int32 nFeatureSlotId = svx::FeatureSlotTranslation::getControllerFeatureSlotIdForURL( aURL.Main );
        sal_Int16 nFormFeature = ( nFeatureSlotId != -1 ) ? svx::FeatureSlotTranslation::getFormFeatureForSlotId( nFeatureSlotId ) : -1;
        if ( nFormFeature > 0 )
        {
            // get the dispatcher for this feature, create if necessary
            DispatcherContainer::const_iterator aDispatcherPos = m_aFeatureDispatchers.find( nFormFeature );
            if ( aDispatcherPos == m_aFeatureDispatchers.end() )
            {
                aDispatcherPos = m_aFeatureDispatchers.emplace(
                    nFormFeature, new svx::OSingleFeatureDispatcher( aURL, nFormFeature, m_xFormOperations, m_aMutex )
                ).first;
            }

            OSL_ENSURE( aDispatcherPos->second.is(), "FormController::interceptedQueryDispatch: should have a dispatcher by now!" );
            return aDispatcherPos->second;
        }
    }

    // no more to offer
    return xReturn;
}


void SAL_CALL FormController::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArgs )
{
    if ( _rArgs.getLength() != 1 )
    {
        OSL_FAIL( "FormController::dispatch: no arguments -> no dispatch!" );
        return;
    }

    if ( _rURL.Complete == "private:/InteractionHandler" )
    {
        Reference< XInteractionRequest > xRequest;
        OSL_VERIFY( _rArgs[0].Value >>= xRequest );
        if ( xRequest.is() )
            handle( xRequest );
        return;
    }

    if  ( _rURL.Complete == FMURL_CONFIRM_DELETION )
    {
        OSL_FAIL( "FormController::dispatch: How do you expect me to return something via this call?" );
            // confirmDelete has a return value - dispatch hasn't
        return;
    }

    OSL_FAIL( "FormController::dispatch: unknown URL!" );
}


void SAL_CALL FormController::addStatusListener( const Reference< XStatusListener >& _rxListener, const URL& _rURL )
{
    if (_rURL.Complete == FMURL_CONFIRM_DELETION)
    {
        if (_rxListener.is())
        {   // send an initial statusChanged event
            FeatureStateEvent aEvent;
            aEvent.FeatureURL = _rURL;
            aEvent.IsEnabled = true;
            _rxListener->statusChanged(aEvent);
            // and don't add the listener at all (the status will never change)
        }
    }
    else
        OSL_FAIL("FormController::addStatusListener: invalid (unsupported) URL!");
}


Reference< XInterface > SAL_CALL FormController::getParent()
{
    return m_xParent;
}


void SAL_CALL FormController::setParent( const Reference< XInterface >& Parent)
{
    m_xParent = Parent;
}


void SAL_CALL FormController::removeStatusListener( const Reference< XStatusListener >& /*_rxListener*/, const URL& _rURL )
{
    OSL_ENSURE(_rURL.Complete == FMURL_CONFIRM_DELETION, "FormController::removeStatusListener: invalid (unsupported) URL!");
    // we never really added the listener, so we don't need to remove it
}


Reference< XDispatchProviderInterceptor >  FormController::createInterceptor(const Reference< XDispatchProviderInterception > & _xInterception)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
#ifdef DBG_UTIL
    // check if we already have a interceptor for the given object
    for ( const auto & it : m_aControlDispatchInterceptors )
    {
        if (it->getIntercepted() == _xInterception)
            OSL_FAIL("FormController::createInterceptor : we already do intercept this objects dispatches !");
    }
#endif

    rtl::Reference<DispatchInterceptionMultiplexer> pInterceptor(new DispatchInterceptionMultiplexer( _xInterception, this ));
    m_aControlDispatchInterceptors.push_back( pInterceptor );

    return pInterceptor.get();
}


bool FormController::ensureInteractionHandler()
{
    if ( m_xInteractionHandler.is() )
        return true;
    if ( m_bAttemptedHandlerCreation )
        return false;
    m_bAttemptedHandlerCreation = true;

    m_xInteractionHandler = InteractionHandler::createWithParent(m_xComponentContext,
                                                                 VCLUnoHelper::GetInterface(getDialogParentWindow()));
    return m_xInteractionHandler.is();
}


void SAL_CALL FormController::handle( const Reference< XInteractionRequest >& _rRequest )
{
    if ( !ensureInteractionHandler() )
        return;
    m_xInteractionHandler->handle( _rRequest );
}


void FormController::deleteInterceptor(const Reference< XDispatchProviderInterception > & _xInterception)
{
    OSL_ENSURE( !impl_isDisposed_nofail(), "FormController: already disposed!" );
    // search the interceptor responsible for the given object
    auto aIter = std::find_if(m_aControlDispatchInterceptors.begin(), m_aControlDispatchInterceptors.end(),
        [&_xInterception](const rtl::Reference<DispatchInterceptionMultiplexer>& rpInterceptor) {
            return rpInterceptor->getIntercepted() == _xInterception;
        });
    if (aIter != m_aControlDispatchInterceptors.end())
    {
        // log off the interception from its interception object
        (*aIter)->dispose();
        // remove the interceptor from our array
        m_aControlDispatchInterceptors.erase(aIter);
    }
}


void FormController::implInvalidateCurrentControlDependentFeatures()
{
    Sequence< sal_Int16 > aCurrentControlDependentFeatures(4);

    aCurrentControlDependentFeatures[0] = FormFeature::SortAscending;
    aCurrentControlDependentFeatures[1] = FormFeature::SortDescending;
    aCurrentControlDependentFeatures[2] = FormFeature::AutoFilter;
    aCurrentControlDependentFeatures[3] = FormFeature::RefreshCurrentControl;

    invalidateFeatures( aCurrentControlDependentFeatures );
}


void SAL_CALL FormController::columnChanged( const EventObject& /*_event*/ )
{
    implInvalidateCurrentControlDependentFeatures();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

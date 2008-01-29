/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmctrler.cxx,v $
 *
 *  $Revision: 1.68 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 17:09:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "fmctrler.hxx"
#include "fmtools.hxx"
#include "fmprop.hrc"
#include "svx/fmshell.hxx"
#include "confirmdelete.hxx"
#include "fmcontrolbordermanager.hxx"
#include "fmdocumentclassification.hxx"
#include "fmcontrollayout.hxx"
#include "fmurl.hxx"
#include "fmdispatch.hxx"
#include "trace.hxx"
#include "fmshimp.hxx"
#include "svx/fmview.hxx"
#include "svx/svdpagv.hxx"
#include "fmresids.hrc"
#include "svx/dialmgr.hxx"
#include "fmservs.hxx"
#include "svx/sdrpagewindow.hxx"

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ROWCHANGEACTION_HPP_
#include <com/sun/star/sdb/RowChangeAction.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCOMBOBOX_HPP_
#include <com/sun/star/awt/XComboBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCHECKBOX_HPP_
#include <com/sun/star/awt/XCheckBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FOCUSCHANGEREASON_HPP_
#include <com/sun/star/awt/FocusChangeReason.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_VALIDATION_XVALIDATABLEFORMCOMPONENT_HPP_
#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_TABULATORCYCLE_HPP_
#include <com/sun/star/form/TabulatorCycle.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCONTROL_HPP_
#include <com/sun/star/form/XBoundControl.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCOMPONENT_HPP_
#include <com/sun/star/form/XBoundComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONSUPPLYPARAMETERS_HPP_
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XIDENTIFIERREPLACE_HPP_
#include <com/sun/star/container/XIdentifierReplace.hpp>
#endif

#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/controls/unocontrol.hxx>

#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include <tools/shl.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/property.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propagg.hxx>
#include <comphelper/enumhelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/extract.hxx>

#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <algorithm>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::form::validation;
using namespace ::com::sun::star::form::runtime;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::container;
using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::svxform;
using namespace ::connectivity::simple;

//==================================================================
// FmXAutoControl
//==================================================================
class OParameterContinuation : public OInteraction< XInteractionSupplyParameters >
{
    Sequence< PropertyValue >       m_aValues;

public:
    OParameterContinuation() { }

    Sequence< PropertyValue >   getValues() const { return m_aValues; }

// XInteractionSupplyParameters
    virtual void SAL_CALL setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException);
};

//------------------------------------------------------------------
void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException)
{
    m_aValues = _rValues;
}

//==================================================================
// FmXAutoControl
//==================================================================
struct FmFieldInfo
{
    rtl::OUString       aFieldName;
    Reference< XPropertySet >   xField;
    Reference< XTextComponent >  xText;

    FmFieldInfo(const Reference< XPropertySet >& _xField, const Reference< XTextComponent >& _xText)
        :xField(_xField)
        ,xText(_xText)
    {xField->getPropertyValue(FM_PROP_NAME) >>= aFieldName;}
};

//==================================================================
// FmXAutoControl
//==================================================================
class FmXAutoControl: public UnoControl

{
    friend Reference< XInterface > SAL_CALL FmXAutoControl_NewInstance_Impl();

public:
    FmXAutoControl(){}

    virtual ::rtl::OUString GetComponentServiceName() {return ::rtl::OUString::createFromAscii("Edit");}
    virtual void SAL_CALL createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw( RuntimeException );

protected:
    virtual void ImplSetPeerProperty( const ::rtl::OUString& rPropName, const Any& rVal );
};

//------------------------------------------------------------------------------
void FmXAutoControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw( RuntimeException )
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    Reference< XTextComponent >  xText(getPeer() , UNO_QUERY);
    if (xText.is())
    {
        xText->setText(::rtl::OUString(String(SVX_RES(RID_STR_AUTOFIELD))));
        xText->setEditable(sal_False);
    }
}

//------------------------------------------------------------------------------
void FmXAutoControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const Any& rVal )
{
    // these properties are ignored
    if (rPropName == FM_PROP_TEXT)
        return;

    UnoControl::ImplSetPeerProperty( rPropName, rVal );
}

//------------------------------------------------------------------------------
IMPL_LINK( FmXFormController, OnActivateTabOrder, void*, /*EMPTYTAG*/ )
{
    activateTabOrder();
    return 1;
}

//------------------------------------------------------------------------------
struct UpdateAllListeners : public ::std::unary_function< Reference< XDispatch >, bool >
{
    bool operator()( const Reference< XDispatch >& _rxDispatcher ) const
    {
        static_cast< ::svx::OSingleFeatureDispatcher* >( _rxDispatcher.get() )->updateAllListeners();
        // the return is a dummy only so we can use this struct in a std::compose1 call
        return true;
    }
};
//..............................................................................
IMPL_LINK( FmXFormController, OnInvalidateFeatures, void*, /*_pNotInterestedInThisParam*/ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    for ( ::std::set< sal_Int32 >::const_iterator aLoop = m_aInvalidFeatures.begin();
          aLoop != m_aInvalidFeatures.end();
          ++aLoop
        )
    {
        DispatcherContainer::const_iterator aDispatcherPos = m_aFeatureDispatchers.find( *aLoop );
        if ( aDispatcherPos != m_aFeatureDispatchers.end() )
        {
            // TODO: for the real and actual listener notifications, we should release
            // our mutex
            UpdateAllListeners( )( aDispatcherPos->second );
        }
    }
    return 1;
}

/*************************************************************************/

//------------------------------------------------------------------
Reference< XInterface > SAL_CALL
    FmXFormController_NewInstance_Impl(const Reference< XMultiServiceFactory > & _rxORB)
{
    return *(new FmXFormController(_rxORB));
}

//------------------------------------------------------------------
namespace fmctrlr
{
    const ::rtl::OUString& getDataModeIdentifier()
    {
        static ::rtl::OUString s_sDataModeIdentifier = DATA_MODE;
        return s_sDataModeIdentifier;
    }
}
using namespace fmctrlr;

DBG_NAME( FmXFormController )
//------------------------------------------------------------------
FmXFormController::FmXFormController(const Reference< XMultiServiceFactory > & _rxORB,
                                     FmFormView* _pView, Window* _pWindow )
                  :FmXFormController_BASE1(m_aMutex)
                  ,OPropertySetHelper(FmXFormController_BASE1::rBHelper)
                  ,OSQLParserClient(_rxORB)
                  ,m_xORB(_rxORB)
                  ,m_aActivateListeners(m_aMutex)
                  ,m_aModifyListeners(m_aMutex)
                  ,m_aErrorListeners(m_aMutex)
                  ,m_aDeleteListeners(m_aMutex)
                  ,m_aRowSetApproveListeners(m_aMutex)
                  ,m_aParameterListeners(m_aMutex)
                  ,m_pView(_pView)
                  ,m_pWindow(_pWindow)
                  ,m_pControlBorderManager( new ::svxform::ControlBorderManager )
                  ,m_aControllerFeatures( _rxORB, this )
                  ,m_aMode(getDataModeIdentifier())
                  ,m_nLoadEvent( 0 )
                  ,m_nToggleEvent(0)
                  ,m_nCurrentFilterPosition(0)
                  ,m_bCurrentRecordModified(sal_False)
                  ,m_bCurrentRecordNew(sal_False)
                  ,m_bLocked(sal_False)
                  ,m_bDBConnection(sal_False)
                  ,m_bCycle(sal_False)
                  ,m_bCanInsert(sal_False)
                  ,m_bCanUpdate(sal_False)
                  ,m_bCommitLock(sal_False)
                  ,m_bModified(sal_False)
                  ,m_bControlsSorted(sal_False)
                  ,m_bFiltering(sal_False)
                  ,m_bAttachEvents(sal_True)
                  ,m_bDetachEvents(sal_True)
                  ,m_bAttemptedHandlerCreation( false )
{
    DBG_CTOR( FmXFormController, NULL );

    ::comphelper::increment(m_refCount);
    {
        {
            m_xAggregate = Reference< XAggregation >(
                m_xORB->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.awt.TabController" ) ),
                UNO_QUERY
            );
            DBG_ASSERT( m_xAggregate.is(), "FmXFormController::FmXFormController : could not create my aggregate !" );
            m_xTabController = Reference< XTabController >( m_xAggregate, UNO_QUERY );
        }

        if ( m_xAggregate.is() )
            m_xAggregate->setDelegator( *this );
    }
    ::comphelper::decrement(m_refCount);

    m_aTabActivationTimer.SetTimeout( 500 );
    m_aTabActivationTimer.SetTimeoutHdl( LINK( this, FmXFormController, OnActivateTabOrder ) );

    m_aFeatureInvalidationTimer.SetTimeout( 200 );
    m_aFeatureInvalidationTimer.SetTimeoutHdl( LINK( this, FmXFormController, OnInvalidateFeatures ) );
}

//------------------------------------------------------------------
FmXFormController::~FmXFormController()
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if( m_nLoadEvent )
            Application::RemoveUserEvent( m_nLoadEvent );

        if ( m_nToggleEvent )
            Application::RemoveUserEvent( m_nToggleEvent );

        if ( m_aTabActivationTimer.IsActive() )
            m_aTabActivationTimer.Stop();
    }

    if ( m_aFeatureInvalidationTimer.IsActive() )
        m_aFeatureInvalidationTimer.Stop();

    disposeAllFeaturesAndDispatchers();

    // Freigeben der Aggregation
    if ( m_xAggregate.is() )
    {
        m_xAggregate->setDelegator( NULL );
        m_xAggregate.clear();
    }

    DELETEZ( m_pControlBorderManager );

    DBG_DTOR( FmXFormController, NULL );
}

// -----------------------------------------------------------------------------
using namespace ::cppu;
using namespace ::osl;

void SAL_CALL FmXFormController::acquire() throw ()
{
    FmXFormController_BASE1::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL FmXFormController::release() throw ()
{
    FmXFormController_BASE1::release();
}
// -----------------------------------------------------------------------------
// XUnoTunnel
Sequence< sal_Int8 > FmXFormController::getUnoTunnelImplementationId()
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
//------------------------------------------------------------------------------
FmXFormController* FmXFormController::getImplementation( const Reference< XInterface >& _rxComponent )
{
    Reference< XUnoTunnel > xTunnel( _rxComponent, UNO_QUERY );
    if ( xTunnel.is() )
        return reinterpret_cast< FmXFormController* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );
    return NULL;
}
//------------------------------------------------------------------------------
sal_Int64 SAL_CALL FmXFormController::getSomething(Sequence<sal_Int8> const& rId)throw( RuntimeException )
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast< sal_Int64 >( this );

    return sal_Int64();
}

//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL FmXFormController::getImplementationId() throw( RuntimeException )
{
    static ::cppu::OImplementationId* pId = 0;
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

//------------------------------------------------------------------------------
Sequence< Type > SAL_CALL FmXFormController::getTypes(  ) throw(RuntimeException)
{
    return comphelper::concatSequences(FmXFormController_BASE1::getTypes(),FmXFormController_BASE2::getTypes(),FmXFormController_BASE3::getTypes());
}
//------------------------------------------------------------------
Any SAL_CALL FmXFormController::queryAggregation( const Type& _rType ) throw(RuntimeException)
{
    Any aRet = FmXFormController_BASE1::queryAggregation(_rType);
    if(!aRet.hasValue())
    {
        aRet = OPropertySetHelper::queryInterface(_rType);
        if(!aRet.hasValue())
        {
            aRet = FmXFormController_BASE2::queryInterface(_rType);
            if(!aRet.hasValue())
                aRet = FmXFormController_BASE3::queryInterface(_rType);
        }
    }
    if(aRet.hasValue())
        return aRet;
    return m_xAggregate->queryAggregation(_rType);
}

// XServiceInfo
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::supportsService(const ::rtl::OUString& ServiceName) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSNL(getSupportedServiceNames());
    const ::rtl::OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXFormController::getImplementationName() throw( RuntimeException )
{
    return ::rtl::OUString::createFromAscii("com.sun.star.form.FmXFormController");
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL FmXFormController::getSupportedServiceNames(void) throw( RuntimeException )
{
    // service names which are supported only, but cannot be used to created an
    // instance at a service factory
    Sequence< ::rtl::OUString > aNonCreatableServiceNames( 1 );
    aNonCreatableServiceNames[ 0 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.FormControllerDispatcher" ) );

    // services which can be used to created an instance at a service factory
    Sequence< ::rtl::OUString > aCreatableServiceNames( getSupportedServiceNames_Static() );
    return ::comphelper::concatSequences( aCreatableServiceNames, aNonCreatableServiceNames );
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::approveReset(const EventObject& /*rEvent*/) throw( RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::resetted(const EventObject& rEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (getCurrentControl().is() &&  (getCurrentControl()->getModel() == rEvent.Source))
        m_bModified = sal_False;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> FmXFormController::getSupportedServiceNames_Static(void)
{
    static Sequence< ::rtl::OUString> aServices;
    if (!aServices.getLength())
    {
        aServices.realloc(2);
        aServices.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormController");
        aServices.getArray()[1] = ::rtl::OUString::createFromAscii("com.sun.star.awt.control.TabController");
    }
    return aServices;
}

//------------------------------------------------------------------------------
void FmXFormController::setCurrentFilterPosition( sal_Int32 nPos )
{
    DBG_ASSERT(nPos < (sal_Int32)m_aFilters.size(), "Invalid Position");

    if (nPos != m_nCurrentFilterPosition)
    {
        m_nCurrentFilterPosition = nPos;

        // reset the text for all controls
        for (FmFilterControls::const_iterator iter = m_aFilterControls.begin();
             iter != m_aFilterControls.end(); iter++)
                 (*iter).first->setText(rtl::OUString());

        if ( nPos != -1 )
        {
            // set the text for all filters
            OSL_ENSURE( ( m_aFilters.size() > (size_t)m_nCurrentFilterPosition ) && ( m_nCurrentFilterPosition >= 0 ),
                "FmXFormController::setCurrentFilterPosition: m_nCurrentFilterPosition too big" );

            if ( ( m_nCurrentFilterPosition >= 0 ) && ( (size_t)m_nCurrentFilterPosition < m_aFilters.size() ) )
            {
                FmFilterRow& rRow = m_aFilters[m_nCurrentFilterPosition];
                for (FmFilterRow::const_iterator iter2 = rRow.begin();
                    iter2 != rRow.end(); iter2++)
                {
                    (*iter2).first->setText((*iter2).second);
                }
            }
        }
    }
}

// OPropertySetHelper
//------------------------------------------------------------------------------
sal_Bool FmXFormController::convertFastPropertyValue( Any & /*rConvertedValue*/, Any & /*rOldValue*/,
                                            sal_Int32 /*nHandle*/, const Any& /*rValue*/ )
                throw( IllegalArgumentException )
{
    return sal_False;
}

//------------------------------------------------------------------------------
void FmXFormController::setFastPropertyValue_NoBroadcast( sal_Int32 /*nHandle*/, const Any& /*rValue*/ )
                         throw( Exception )
{
}

//------------------------------------------------------------------------------
void FmXFormController::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case FM_ATTR_FILTER:
        {
            ::rtl::OUStringBuffer aFilter;
            OStaticDataAccessTools aStaticTools;
            Reference<XConnection> xConnection(aStaticTools.getRowSetConnection(Reference< XRowSet>(m_xModelAsIndex, UNO_QUERY)));
            if (xConnection.is())
            {
                Reference< XDatabaseMetaData> xMetaData(xConnection->getMetaData());
                Reference< XNumberFormatsSupplier> xFormatSupplier( aStaticTools.getNumberFormats(xConnection, sal_True));
                Reference< XNumberFormatter> xFormatter(m_xORB
                                ->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
                xFormatter->attachNumberFormatsSupplier(xFormatSupplier);

                Reference< XColumnsSupplier> xSupplyCols(m_xModelAsIndex, UNO_QUERY);
                Reference< XNameAccess> xFields(xSupplyCols->getColumns(), UNO_QUERY);

                ::rtl::OUString aQuote( xMetaData->getIdentifierQuoteString() );

                    // now add the filter rows
                for ( FmFilterRows::const_iterator row = m_aFilters.begin(); row != m_aFilters.end(); ++row )
                {
                    const FmFilterRow& rRow = *row;

                    if ( rRow.empty() )
                        continue;

                    if ( aFilter.getLength() )
                        aFilter.appendAscii( " OR " );

                    aFilter.appendAscii( "( " );
                    for ( FmFilterRow::const_iterator condition = rRow.begin(); condition != rRow.end(); ++condition )
                    {
                        // get the field of the controls map
                        Reference< XTextComponent > xText = condition->first;
                        Reference< XPropertySet > xField = m_aFilterControls.find( xText )->second;
                        DBG_ASSERT( xField.is(), "FmXFormController::getFastPropertyValue: no field found!" );
                        if ( condition != rRow.begin() )
                            aFilter.appendAscii( " AND " );

                        ::rtl::OUString sFilterValue( condition->second );

                        ::rtl::OUString sErrorMsg, sCriteria;
                        ::rtl::Reference< ISQLParseNode > xParseNode = predicateTree( sErrorMsg, sFilterValue, xFormatter, xField );
                        OSL_ENSURE( xParseNode.is(), "FmXFormController::getFastPropertyValue: could not parse the field value predicate!" );
                        if ( xParseNode.is() )
                        {
                            // don't use a parse context here, we need it unlocalized
                            xParseNode->parseNodeToStr( sCriteria, xConnection, NULL );
                            aFilter.append( sCriteria );
                        }
                    }
                    aFilter.appendAscii( " )" );
                }
            }
            rValue <<= aFilter.makeStringAndClear();
        }
        break;

        case FM_ATTR_FORM_OPERATIONS:
            rValue <<= m_aControllerFeatures->getFormOperations();
            break;
    }
}

//------------------------------------------------------------------------------
Reference< XPropertySetInfo >  FmXFormController::getPropertySetInfo() throw( RuntimeException )
{
    static Reference< XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
#define DECL_PROP_CORE(varname, type) \
pDesc[nPos++] = Property(FM_PROP_##varname, FM_ATTR_##varname, ::getCppuType((const type*)0),


#define DECL_PROP1(varname, type, attrib1)  \
    DECL_PROP_CORE(varname, type) PropertyAttribute::attrib1)

//------------------------------------------------------------------------------
void FmXFormController::fillProperties(
        Sequence< Property >& /* [out] */ _rProps,
        Sequence< Property >& /* [out] */ /*_rAggregateProps*/
        ) const
{
    _rProps.realloc(2);
    sal_Int32 nPos = 0;
    Property* pDesc = _rProps.getArray();
    DECL_PROP1(FILTER, rtl::OUString, READONLY);
    DECL_PROP1(FORM_OPERATIONS, Reference< XFormOperations >, READONLY);
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& FmXFormController::getInfoHelper()
{
    return *getArrayHelper();
}

// XElementAccess
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::hasElements(void) throw( RuntimeException )
{
::osl::MutexGuard aGuard( m_aMutex );
    return !m_aChilds.empty();
}

//------------------------------------------------------------------------------
Type SAL_CALL  FmXFormController::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((const Reference< XFormController>*)0);

}

// XEnumerationAccess
//------------------------------------------------------------------------------
Reference< XEnumeration > SAL_CALL  FmXFormController::createEnumeration(void) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return new ::comphelper::OEnumerationByIndex(this);
}

// XIndexAccess
//------------------------------------------------------------------------------
sal_Int32 SAL_CALL FmXFormController::getCount(void) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_aChilds.size();
}

//------------------------------------------------------------------------------
Any SAL_CALL FmXFormController::getByIndex(sal_Int32 Index) throw( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (Index < 0 ||
        Index >= (sal_Int32)m_aChilds.size())
        throw IndexOutOfBoundsException();

    return makeAny(m_aChilds[Index]);
    //  , ::getCppuType((const XFormController*)0));
}

//-----------------------------------------------------------------------------
void FmXFormController::addChild(FmXFormController* pChild)
{
    Reference< XFormController >  xController(pChild);
    m_aChilds.push_back(xController);
    pChild->setParent(static_cast< XFormController* >(this));

    Reference< XFormComponent >  xForm(pChild->getModel(), UNO_QUERY);

    // search the position of the model within the form
    sal_uInt32 nPos = m_xModelAsIndex->getCount();
    Reference< XFormComponent > xTemp;
    for( ; nPos; )
    {
        m_xModelAsIndex->getByIndex(--nPos) >>= xTemp;
        if ((XFormComponent*)xForm.get() == (XFormComponent*)xTemp.get())
        {
            Reference< XInterface >  xIfc(xController, UNO_QUERY);
            m_xModelAsManager->attach( nPos, xIfc, makeAny( xController) );
            break;
        }
    }
}

//  EventListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::disposing(const EventObject& e) throw( RuntimeException )
{
    // Ist der Container disposed worden
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XControlContainer >  xContainer(e.Source, UNO_QUERY);
    if (xContainer.is())
    {
        setContainer(Reference< XControlContainer > ());
    }
    else
    {
        // ist ein Control disposed worden
        Reference< XControl >  xControl(e.Source, UNO_QUERY);
        if (xControl.is())
        {
            if (getContainer().is())
                removeControl(xControl);
        }
    }
}

// OComponentHelper
//-----------------------------------------------------------------------------
void FmXFormController::disposeAllFeaturesAndDispatchers() SAL_THROW(())
{
    for ( DispatcherContainer::iterator aDispatcher = m_aFeatureDispatchers.begin();
          aDispatcher != m_aFeatureDispatchers.end();
          ++aDispatcher
        )
    {
        try
        {
            ::comphelper::disposeComponent( aDispatcher->second );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FmXFormController::disposeAllFeaturesAndDispatchers: caught an exception while disposing a dispatcher!" );
        }
    }
    m_aFeatureDispatchers.clear();
    m_aControllerFeatures.dispose();
}

//-----------------------------------------------------------------------------
void FmXFormController::disposing(void)
{
    EventObject aEvt(static_cast< XFormController* >(this));

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

    removeBoundFieldListener();
    stopFiltering();

    m_pControlBorderManager->restoreAll();

    m_aFilters.clear();

    ::osl::MutexGuard aGuard( m_aMutex );
    m_xActiveControl  = m_xCurrentControl = NULL;

    // clean up our children
    for (FmFormControllers::const_iterator i = m_aChilds.begin();
        i != m_aChilds.end(); i++)
    {
        // search the position of the model within the form
        Reference< XFormComponent >  xForm((*i)->getModel(), UNO_QUERY);
        sal_uInt32 nPos = m_xModelAsIndex->getCount();
        Reference< XFormComponent > xTemp;
        for( ; nPos; )
        {

            m_xModelAsIndex->getByIndex( --nPos ) >>= xTemp;
            if ( xForm.get() == xTemp.get() )
            {
                Reference< XInterface > xIfc( *i, UNO_QUERY );
                m_xModelAsManager->detach( nPos, xIfc );
                break;
            }
        }

        Reference< XComponent > (*i, UNO_QUERY)->dispose();
    }
    m_aChilds.clear();

    disposeAllFeaturesAndDispatchers();

    if (m_bDBConnection)
        unload();

    setContainer( NULL );
    setModel( NULL );
    setParent( NULL );

    ::comphelper::disposeComponent( m_xComposer );

    m_xORB              = NULL;
    m_bDBConnection = sal_False;
}

//------------------------------------------------------------------------------
namespace
{
    static bool lcl_shouldUseDynamicControlBorder( const Reference< XInterface >& _rxForm, const Any& _rDynamicColorProp )
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

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::propertyChange(const PropertyChangeEvent& evt) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if ( evt.PropertyName == FM_PROP_BOUNDFIELD )
    {
        Reference<XPropertySet> xOldBound;
        evt.OldValue >>= xOldBound;
        if ( !xOldBound.is() && evt.NewValue.hasValue() )
        {
            Reference< XControlModel > xControlModel(evt.Source,UNO_QUERY);
            Reference< XControl > xControl = findControl(m_aControls,xControlModel,sal_False,sal_False);
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
        sal_Bool bModifiedChanged = (evt.PropertyName == FM_PROP_ISMODIFIED);
        sal_Bool bNewChanged = (evt.PropertyName == FM_PROP_ISNEW);
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

            if (bNewChanged)
            {
                if (m_nToggleEvent)
                    Application::RemoveUserEvent( m_nToggleEvent );
                m_nToggleEvent = Application::PostUserEvent( LINK(this, FmXFormController,
                                                        OnToggleAutoFields) );
            }

            if (!m_bCurrentRecordModified)
                m_bModified = sal_False;
        }
        else if ( evt.PropertyName == FM_PROP_DYNAMIC_CONTROL_BORDER )
        {
            bool bEnable = lcl_shouldUseDynamicControlBorder( evt.Source, evt.NewValue );
            if ( bEnable )
            {
                m_pControlBorderManager->enableDynamicBorderColor();
                if ( m_xActiveControl.is() )
                    m_pControlBorderManager->focusGained( m_xActiveControl.get() );
            }
            else
            {
                m_pControlBorderManager->disableDynamicBorderColor();
            }
        }
    }
}

//------------------------------------------------------------------------------
bool FmXFormController::replaceControl( const Reference< XControl >& _rxExistentControl, const Reference< XControl >& _rxNewControl )
{
    bool bSuccess = false;
    try
    {
        Reference< XIdentifierReplace > xContainer( getContainer(), UNO_QUERY );
        DBG_ASSERT( xContainer.is(), "FmXFormController::replaceControl: yes, it's not required by the service description, but XItentifierReplaces would be nice!" );
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
            DBG_ASSERT( pIdentifiers != pIdentifiersEnd, "FmXFormController::replaceControl: did not find the control in the container!" );
            if ( pIdentifiers != pIdentifiersEnd )
            {
                bool bReplacedWasActive = ( m_xActiveControl.get() == _rxExistentControl.get() );
                bool bReplacedWasCurrent = ( m_xCurrentControl.get() == _rxExistentControl.get() );

                if ( bReplacedWasActive )
                {
                    m_xActiveControl = m_xCurrentControl = NULL;
                }
                else if ( bReplacedWasCurrent )
                    m_xCurrentControl = _rxNewControl;

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
      OSL_ENSURE( sal_False, "FmXFormController::replaceControl: caught an exception!" );
    }

    Reference< XControl > xDisposeIt( bSuccess ? _rxExistentControl : _rxNewControl );
    ::comphelper::disposeComponent( xDisposeIt );
    return bSuccess;
}

//------------------------------------------------------------------------------
void FmXFormController::toggleAutoFields(sal_Bool bAutoFields)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");


    Sequence< Reference< XControl > > aControlsCopy( m_aControls );
    const Reference< XControl >* pControls = aControlsCopy.getConstArray();
    sal_Int32 nControls = aControlsCopy.getLength();

    if (bAutoFields)
    {
        // as we don't want new controls to be attached to the scripting environment
        // we change attach flags
        m_bAttachEvents = sal_False;
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
                        replaceControl( xControl, new FmXAutoControl );
                    }
                }
            }
        }
        m_bAttachEvents = sal_True;
    }
    else
    {
        m_bDetachEvents = sal_False;
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
                        ::rtl::OUString sServiceName;
                        OSL_VERIFY( xSet->getPropertyValue( FM_PROP_DEFAULTCONTROL ) >>= sServiceName );
                        Reference< XControl > xNewControl( m_xORB->createInstance( sServiceName ), UNO_QUERY );
                        replaceControl( xControl, xNewControl );
                    }
                }
            }
        }
        m_bDetachEvents = sal_True;
    }
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormController, OnToggleAutoFields, void*, EMPTYARG)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
        m_nToggleEvent = 0;
    }
    toggleAutoFields(m_bCurrentRecordNew);
    return 1L;
}

// XTextListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::textChanged(const TextEvent& e) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if (m_bFiltering)
    {
        Reference< XTextComponent >  xText(e.Source,UNO_QUERY);
        ::rtl::OUString aText = xText->getText();

        // Suchen der aktuellen Row
        OSL_ENSURE( ( m_aFilters.size() > (size_t)m_nCurrentFilterPosition ) && ( m_nCurrentFilterPosition >= 0 ),
            "FmXFormController::textChanged: m_nCurrentFilterPosition too big" );

        if ( ( m_nCurrentFilterPosition >= 0 ) && ( (size_t)m_nCurrentFilterPosition < m_aFilters.size() ) )
        {
            FmFilterRow& rRow = m_aFilters[m_nCurrentFilterPosition];

            // do we have a new filter
            if (aText.getLength())
                rRow[xText] = aText;
            else
            {
                // do we have the control in the row
                FmFilterRow::iterator iter = rRow.find(xText);
                // erase the entry out of the row
                if (iter != rRow.end())
                    rRow.erase(iter);
            }
        }
    }
    else if (!m_bModified)
        onModify( e.Source );
}

// XItemListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::itemStateChanged(const ItemEvent& rEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if (!m_bModified)
        onModify( rEvent.Source );
}

// XModificationBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addModifyListener(const Reference< XModifyListener > & l) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aModifyListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void FmXFormController::removeModifyListener(const Reference< XModifyListener > & l) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aModifyListeners.removeInterface( l );
}

// XModificationListener
//------------------------------------------------------------------------------
void FmXFormController::modified(const EventObject& rEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if (!m_bModified)
        onModify( rEvent.Source );
}

//------------------------------------------------------------------------------
void FmXFormController::onModify( const Reference< XInterface >& _rxControl )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if (!m_bModified)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bModified = sal_True;

    }

    Reference< XControl > xSourceControl(_rxControl, UNO_QUERY);
    if  (xSourceControl.get() != m_xCurrentControl.get())
    {   // let this control grab the focus
        // (this case may happen if somebody moves the scroll wheel of the mouse over a control
        // which does not have the focus)
        // 85511 - 29.05.2001 - frank.schoenheit@germany.sun.com
        Reference< XWindow > xControlWindow(_rxControl, UNO_QUERY);
        if (xControlWindow.is())
            xControlWindow->setFocus();
    }

    EventObject aEvt(static_cast<cppu::OWeakObject*>(this));
    m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvt );
}

//------------------------------------------------------------------------------
sal_Bool FmXFormController::determineLockState() const
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    // a.) in filter mode we are always locked
    // b.) if we have no valid model or our model (a result set) is not alive -> we're locked
    // c.) if we are inserting everything is OK and we are not locked
    // d.) if are not updatable or on invalid position
    Reference< XResultSet >  xResultSet(m_xModelAsIndex, UNO_QUERY);
    if (m_bFiltering || !xResultSet.is() || !isRowSetAlive(xResultSet))
        return sal_True;
    else
        return (m_bCanInsert && m_bCurrentRecordNew) ? sal_False
        :  xResultSet->isBeforeFirst() || xResultSet->isAfterLast() || xResultSet->rowDeleted() || !m_bCanUpdate;
}

//  FocusListener
//------------------------------------------------------------------------------
void FmXFormController::focusGained(const FocusEvent& e) throw( RuntimeException )
{
    TRACE_RANGE( "FmXFormController::focusGained" );

    OSL_ENSURE( !FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController::focusGained: Object already disposed!" );
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XControl >  xControl(e.Source, UNO_QUERY);

    m_pControlBorderManager->focusGained( e.Source );

    if (m_bDBConnection)
    {
        // do we need to keep the locking of the commit
        // we hold the lock as long as the control differs from the current
        // otherwhise we disabled the lock
        m_bCommitLock = m_bCommitLock && (XControl*)xControl.get() != (XControl*)m_xCurrentControl.get();
        if (m_bCommitLock)
            return;

        // when do we have to commit a value to form or a filter
        // a.) if the current value is modified
        // b.) there must be a current control
        // c.) and it must be different from the new focus owning control or
        // d.) the focus is moving around (so we have only one control)

        if ((m_bModified || m_bFiltering) &&
            m_xCurrentControl.is() &&
            ((xControl.get() != m_xCurrentControl.get()) ||
             ((e.FocusFlags & FocusChangeReason::AROUND) && (m_bCycle || m_bFiltering)))
            )
        {
            // check the old control if the content is ok
#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
            Reference< XBoundControl >  xLockingTest(m_xCurrentControl, UNO_QUERY);
            sal_Bool bControlIsLocked = xLockingTest.is() && xLockingTest->getLock();
            OSL_ENSURE(!bControlIsLocked, "FmXFormController::Gained: I'm modified and the current control is locked ? How this ?");
            // normalerweise sollte ein gelocktes Control nicht modified sein, also muss wohl mein bModified aus einem anderen Kontext
            // gesetzt worden sein, was ich nicht verstehen wuerde ...
#endif
            DBG_ASSERT(m_xCurrentControl.is(), "kein CurrentControl gesetzt");
            // zunaechst das Control fragen ob es das IFace unterstuetzt
            Reference< XBoundComponent >  xBound(m_xCurrentControl, UNO_QUERY);
            if (!xBound.is() && m_xCurrentControl.is())
                xBound  = Reference< XBoundComponent > (m_xCurrentControl->getModel(), UNO_QUERY);

            // lock if we lose the focus during commit
            m_bCommitLock = sal_True;

            // Commit nicht erfolgreich, Focus zuruecksetzen
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
                m_bModified = sal_False;
                m_bCommitLock = sal_False;
            }
        }

        if (!m_bFiltering && m_bCycle && (e.FocusFlags & FocusChangeReason::AROUND) && m_xCurrentControl.is())
        {
            if ( e.FocusFlags & FocusChangeReason::FORWARD )
            {
                if ( m_aControllerFeatures->isEnabled( SID_FM_RECORD_NEXT ) )
                    m_aControllerFeatures->moveRight();
            }
            else // backward
            {
                if ( m_aControllerFeatures->isEnabled( SID_FM_RECORD_PREV ) )
                    m_aControllerFeatures->moveLeft();
            }
        }
    }

    // Immer noch ein und dasselbe Control
    if  (   (m_xActiveControl.get() == xControl.get())
        &&  (xControl.get() == m_xCurrentControl.get())
        )
    {
        DBG_ASSERT(m_xCurrentControl.is(), "Kein CurrentControl selektiert");
        return;
    }

    sal_Bool bActivated = !m_xActiveControl.is() && xControl.is();

    m_xActiveControl  = xControl;
    m_xCurrentControl = xControl;

    DBG_ASSERT(m_xCurrentControl.is(), "Kein CurrentControl selektiert");

    if (bActivated)
        onActivate();

    // invalidate all features which depend on the currently focused control
    if ( m_bDBConnection && !m_bFiltering && m_pView )
    {
        ::std::vector< sal_Int32 > aFocusDependentFeatures;
        aFocusDependentFeatures.push_back( SID_FM_SORTUP );
        aFocusDependentFeatures.push_back( SID_FM_SORTDOWN );
        aFocusDependentFeatures.push_back( SID_FM_AUTOFILTER );
        if ( m_pView && m_pView->GetFormShell() && m_pView->GetFormShell()->GetImpl() )
            m_pView->GetFormShell()->GetImpl()->invalidateFeatures( aFocusDependentFeatures );
        invalidateFeatures( aFocusDependentFeatures );
    }

    if (m_xCurrentControl.is())
    {
        // Control erhaelt Focus, dann eventuell in den sichtbaren Bereich
        Reference< XWindow >  xWindow(xControl, UNO_QUERY);
        if (xWindow.is() && m_pView && m_pWindow)
        {
            ::com::sun::star::awt::Rectangle aRect = xWindow->getPosSize();
            ::Rectangle aNewRect(aRect.X,aRect.Y,aRect.X+aRect.Width,aRect.Y+aRect.Height);
            aNewRect = m_pWindow->PixelToLogic(aNewRect);
            m_pView->MakeVisible( aNewRect, *const_cast< Window* >( m_pWindow ) );
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormController::onActivate()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    // benachrichtigen, dass form activiert
    EventObject aEvt;
    aEvt.Source = *this;
    m_aActivateListeners.notifyEach( &XFormControllerListener::formActivated, aEvt );

    // verschicken ob modified
    if (m_bModified)
        m_aModifyListeners.notifyEach( &XModifyListener::modified, aEvt );
}

//------------------------------------------------------------------------------
void FmXFormController::focusLost(const FocusEvent& e) throw( RuntimeException )
{
    OSL_ENSURE( !FmXFormController_BASE1::rBHelper.bDisposed, "FmXFormController::focusLost: Object already disposed!" );

    m_pControlBorderManager->focusLost( e.Source );

    Reference< XControl >  xControl(e.Source, UNO_QUERY);
    Reference< XWindowPeer >  xNext(e.NextFocus, UNO_QUERY);
    Reference< XControl >  xNextControl = isInList(xNext);
    if (!xNextControl.is())
    {
        m_xActiveControl = NULL;
        EventObject aEvt;
        aEvt.Source = *this;
        m_aActivateListeners.notifyEach( &XFormControllerListener::formDeactivated, aEvt );
    }
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::mousePressed( const awt::MouseEvent& /*_rEvent*/ ) throw (RuntimeException)
{
    // not interested in
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::mouseReleased( const awt::MouseEvent& /*_rEvent*/ ) throw (RuntimeException)
{
    // not interested in
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::mouseEntered( const awt::MouseEvent& _rEvent ) throw (RuntimeException)
{
    m_pControlBorderManager->mouseEntered( _rEvent.Source );
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::mouseExited( const awt::MouseEvent& _rEvent ) throw (RuntimeException)
{
    m_pControlBorderManager->mouseExited( _rEvent.Source );
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::componentValidityChanged( const EventObject& _rSource ) throw (RuntimeException)
{
    Reference< XControl > xControl( findControl( m_aControls, Reference< XControlModel >( _rSource.Source, UNO_QUERY ), sal_False, sal_False ) );
    Reference< XValidatableFormComponent > xValidatable( _rSource.Source, UNO_QUERY );

    OSL_ENSURE( xControl.is() && xValidatable.is(), "FmXFormController::componentValidityChanged: huh?" );

    if ( xControl.is() && xValidatable.is() )
        m_pControlBorderManager->validityChanged( xControl, xValidatable );
}

//--------------------------------------------------------------------
void FmXFormController::setModel(const Reference< XTabControllerModel > & Model) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::setModel : invalid aggregate !");

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

        // set the new model wait for the load event
        if (m_xTabController.is())
            m_xTabController->setModel(Model);
        m_xModelAsIndex = Reference< XIndexAccess > (Model, UNO_QUERY);
        m_xModelAsManager = Reference< XEventAttacherManager > (Model, UNO_QUERY);

        // only if both ifaces exit, the controller will work successful
        if (!m_xModelAsIndex.is() || !m_xModelAsManager.is())
        {
            m_xModelAsManager = NULL;
            m_xModelAsIndex = NULL;
        }

        if (m_xModelAsIndex.is())
        {
            m_aControllerFeatures.assign( this );

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
                    m_pControlBorderManager->enableDynamicBorderColor();
                else
                    m_pControlBorderManager->disableDynamicBorderColor();

                sal_Int32 nColor = 0;
                if ( xModelProps->getPropertyValue( FM_PROP_CONTROL_BORDER_COLOR_FOCUS ) >>= nColor )
                    m_pControlBorderManager->setStatusColor( CONTROL_STATUS_FOCUSED, nColor );
                if ( xModelProps->getPropertyValue( FM_PROP_CONTROL_BORDER_COLOR_MOUSE ) >>= nColor )
                    m_pControlBorderManager->setStatusColor( CONTROL_STATUS_MOUSE_HOVER, nColor );
                if ( xModelProps->getPropertyValue( FM_PROP_CONTROL_BORDER_COLOR_INVALID ) >>= nColor )
                    m_pControlBorderManager->setStatusColor( CONTROL_STATUS_INVALID, nColor );
            }
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "FmXFormController::setModel: caught an exception!" );
    }
}

//------------------------------------------------------------------------------
Reference< XTabControllerModel >  FmXFormController::getModel() throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::getModel : invalid aggregate !");
    if (!m_xTabController.is())
        return Reference< XTabControllerModel > ();
    return m_xTabController->getModel();
}

//------------------------------------------------------------------------------
void FmXFormController::addToEventAttacher(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    OSL_ENSURE( xControl.is(), "FmXFormController::addToEventAttacher: invalid control - how did you reach this?" );
    if ( !xControl.is() )
        return; /* throw IllegalArgumentException(); */

    // anmelden beim Eventattacher
    Reference< XFormComponent >  xComp(xControl->getModel(), UNO_QUERY);
    if (xComp.is() && m_xModelAsIndex.is())
    {
        // Und die Position des ControlModel darin suchen
        sal_uInt32 nPos = m_xModelAsIndex->getCount();
        Reference< XFormComponent > xTemp;
        for( ; nPos; )
        {
            m_xModelAsIndex->getByIndex(--nPos) >>= xTemp;
            if ((XFormComponent*)xComp.get() == (XFormComponent*)xTemp.get())
            {
                Reference< XInterface >  xIfc(xControl, UNO_QUERY);
                m_xModelAsManager->attach( nPos, xIfc, makeAny(xControl) );
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormController::removeFromEventAttacher(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    OSL_ENSURE( xControl.is(), "FmXFormController::removeFromEventAttacher: invalid control - how did you reach this?" );
    if ( !xControl.is() )
        return; /* throw IllegalArgumentException(); */

    // abmelden beim Eventattacher
    Reference< XFormComponent >  xComp(xControl->getModel(), UNO_QUERY);
    if ( xComp.is() && m_xModelAsIndex.is() )
    {
        // Und die Position des ControlModel darin suchen
        sal_uInt32 nPos = m_xModelAsIndex->getCount();
        Reference< XFormComponent > xTemp;
        for( ; nPos; )
        {
            m_xModelAsIndex->getByIndex(--nPos) >>= xTemp;
            if ((XFormComponent*)xComp.get() == (XFormComponent*)xTemp.get())
            {
                Reference< XInterface >  xIfc(xControl, UNO_QUERY);
                m_xModelAsManager->detach( nPos, xIfc );
                break;
            }
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormController::setContainer(const Reference< XControlContainer > & xContainer) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    Reference< XTabControllerModel >  xTabModel(getModel());
    DBG_ASSERT(xTabModel.is() || !xContainer.is(), "No Model defined");
        // if we have a new container we need a model
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::setContainer : invalid aggregate !");

    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XContainer >  xCurrentContainer;
    if (m_xTabController.is())
        xCurrentContainer = Reference< XContainer > (m_xTabController->getContainer(), UNO_QUERY);
    if (xCurrentContainer.is())
    {
        xCurrentContainer->removeContainerListener(this);

        if ( m_aTabActivationTimer.IsActive() )
            m_aTabActivationTimer.Stop();

        // clear the filter map
        for (FmFilterControls::const_iterator iter = m_aFilterControls.begin();
             iter != m_aFilterControls.end(); ++iter)
            (*iter).first->removeTextListener(this);

        m_aFilterControls.clear();

        // einsammeln der Controls
        const Reference< XControl >* pControls = m_aControls.getConstArray();
        const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
        while ( pControls != pControlsEnd )
            implControlRemoved( *pControls++, true );

        // Datenbank spezifische Dinge vornehmen
        if (m_bDBConnection && isListeningForChanges())
            stopListening();

        m_aControls.realloc( 0 );
    }

    if (m_xTabController.is())
        m_xTabController->setContainer(xContainer);

    // Welche Controls gehoeren zum Container ?
    if (xContainer.is() && xTabModel.is())
    {
        Sequence< Reference< XControlModel > > aModels = xTabModel->getControlModels();
        const Reference< XControlModel > * pModels = aModels.getConstArray();
        Sequence< Reference< XControl > > aAllControls = xContainer->getControls();

        sal_Int32 nCount = aModels.getLength();
        m_aControls = Sequence< Reference< XControl > >( nCount );
        Reference< XControl > * pControls = m_aControls.getArray();

        // einsammeln der Controls
        sal_Int32 i, j;
        for (i = 0, j = 0; i < nCount; ++i, ++pModels )
        {
            Reference< XControl > xControl = findControl( aAllControls, *pModels, sal_False, sal_True );
            if ( xControl.is() )
            {
                pControls[j++] = xControl;
                implControlInserted( xControl, true );
            }
        }

        // not every model had an associated control
        if (j != i)
            m_aControls.realloc(j);

        // am Container horchen
        Reference< XContainer >  xNewContainer(xContainer, UNO_QUERY);
        if (xNewContainer.is())
            xNewContainer->addContainerListener(this);

        // Datenbank spezifische Dinge vornehmen
        if (m_bDBConnection)
        {
            m_bLocked = determineLockState();
            setLocks();
            if (!isLocked())
                startListening();
        }
    }
    // befinden sich die Controls in der richtigen Reihenfolge
    m_bControlsSorted = sal_True;
}

//------------------------------------------------------------------------------
Reference< XControlContainer >  FmXFormController::getContainer() throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::getContainer : invalid aggregate !");
    if (!m_xTabController.is())
        return Reference< XControlContainer > ();
    return m_xTabController->getContainer();
}

//------------------------------------------------------------------------------
Sequence< Reference< XControl > > FmXFormController::getControls(void) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
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

        // Umsortieren der Controls entsprechend der TabReihenfolge
        sal_Int32 j = 0;
        for (sal_Int32 i = 0; i < nModels; ++i, ++pModels )
        {
            xControl = findControl( m_aControls, *pModels, sal_True, sal_True );
            if ( xControl.is() )
                pControls[j++] = xControl;
        }

        // not every model had an associated control
        if ( j != nModels )
            aNewControls.realloc( j );

        m_aControls = aNewControls;
        m_bControlsSorted = sal_True;
    }
    return m_aControls;
}

//------------------------------------------------------------------------------
void FmXFormController::autoTabOrder() throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::autoTabOrder : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->autoTabOrder();
}

//------------------------------------------------------------------------------
void FmXFormController::activateTabOrder() throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::activateTabOrder : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->activateTabOrder();
}

//------------------------------------------------------------------------------
void FmXFormController::setControlLock(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    sal_Bool bLocked = isLocked();

    // es wird gelockt
    // a.) wenn der ganze Datensatz gesperrt ist
    // b.) wenn das zugehoerige Feld gespeert ist
    Reference< XBoundControl >  xBound(xControl, UNO_QUERY);
    if (xBound.is() && ((bLocked && bLocked != xBound->getLock() ||
                         !bLocked)))    // beim entlocken immer einzelne Felder ueberprüfen
    {
        // gibt es eine Datenquelle
        Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
        if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
        {
            // wie sieht mit den Properties ReadOnly und Enable aus
            sal_Bool bTouch = sal_True;
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
                                xBound->setLock(sal_True);
                            else
                                xBound->setLock(bLocked);
                        }
                        catch(...)
                        {
                        }

                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void FmXFormController::setLocks()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    // alle Controls, die mit einer Datenquelle verbunden sind locken/unlocken
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
        setControlLock( *pControls++ );
}

//------------------------------------------------------------------------------
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
            xBound = Reference< XBoundComponent >( _rxControl->getModel(), UNO_QUERY );

            Reference< XPropertySet > xModelProps( xBound, UNO_QUERY );
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

//------------------------------------------------------------------------------
void FmXFormController::startControlModifyListening(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController::startControlModifyListening: Object already disposed!");

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

        // alle die Text um vorzeitig ein modified zu erkennen
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

//------------------------------------------------------------------------------
void FmXFormController::stopControlModifyListening(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");

    bool bModifyListening = lcl_shouldListenForModifications( xControl, NULL );

    // kuenstliches while
    while (bModifyListening)
    {
        Reference< XModifyBroadcaster >  xMod(xControl, UNO_QUERY);
        if (xMod.is())
        {
            xMod->removeModifyListener(this);
            break;
        }
        // alle die Text um vorzeitig ein modified zu erkennen
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

//------------------------------------------------------------------------------
void FmXFormController::startListening()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_bModified  = sal_False;

    // jetzt anmelden bei gebundenen feldern
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
        startControlModifyListening( *pControls++ );
}

//------------------------------------------------------------------------------
void FmXFormController::stopListening()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_bModified  = sal_False;

    // jetzt anmelden bei gebundenen feldern
    const Reference< XControl >* pControls = m_aControls.getConstArray();
    const Reference< XControl >* pControlsEnd = pControls + m_aControls.getLength();
    while ( pControls != pControlsEnd )
        stopControlModifyListening( *pControls++ );
}


//------------------------------------------------------------------------------
Reference< XControl >  FmXFormController::findControl(Sequence< Reference< XControl > >& _rControls, const Reference< XControlModel > & xCtrlModel ,sal_Bool _bRemove,sal_Bool _bOverWrite) const
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    DBG_ASSERT( xCtrlModel.is(), "findControl - welches ?!" );

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
                    *pControls = Reference< XControl >();
                return xControl;
            }
        }
    }
    return Reference< XControl > ();
}

//------------------------------------------------------------------------------
void FmXFormController::implControlInserted( const Reference< XControl>& _rxControl, bool _bAddToEventAttacher )
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

        // we want to know about the reset of the the model of our controls
        // (for correctly resetting m_bModified)
        Reference< XReset >  xReset( xModel, UNO_QUERY );
        if ( xReset.is() )
            xReset->addResetListener( this );

        // and we want to know about the validity, to visually indicate it
        Reference< XValidatableFormComponent > xValidatable( xModel, UNO_QUERY );
        if ( xValidatable.is() )
        {
            xValidatable->addFormComponentValidityListener( this );
            m_pControlBorderManager->validityChanged( _rxControl, xValidatable );
        }
    }

}

//------------------------------------------------------------------------------
void FmXFormController::implControlRemoved( const Reference< XControl>& _rxControl, bool _bRemoveFromEventAttacher )
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

//------------------------------------------------------------------------------
void FmXFormController::insertControl(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_bControlsSorted = sal_False;
    m_aControls.realloc(m_aControls.getLength() + 1);
    m_aControls.getArray()[m_aControls.getLength() - 1] = xControl;

    implControlInserted( xControl, m_bAttachEvents );

    if (m_bDBConnection && !m_bFiltering)
        setControlLock(xControl);

    if (isListeningForChanges() && m_bAttachEvents)
        startControlModifyListening( xControl );
}

//------------------------------------------------------------------------------
void FmXFormController::removeControl(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
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

    if (m_aFilterControls.size())
    {
        Reference< XTextComponent >  xText(xControl, UNO_QUERY);
        FmFilterControls::iterator iter = m_aFilterControls.find(xText);
        if (iter != m_aFilterControls.end())
            m_aFilterControls.erase(iter);
    }

    implControlRemoved( xControl, m_bDetachEvents );

    if ( isListeningForChanges() && m_bDetachEvents )
        stopControlModifyListening( xControl );
}

// XLoadListener
//------------------------------------------------------------------------------
void FmXFormController::loaded(const EventObject& rEvent) throw( RuntimeException )
{
    OSL_ENSURE( rEvent.Source == m_xModelAsIndex, "FmXFormController::loaded: where did this come from?" );

    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XRowSet >  xForm(rEvent.Source, UNO_QUERY);
    // do we have a connected data source
    OStaticDataAccessTools aStaticTools;
    if (xForm.is() && aStaticTools.getRowSetConnection(xForm).is())
    {
        Reference< XPropertySet >  xSet(xForm, UNO_QUERY);
        if (xSet.is())
        {
            Any aVal        = xSet->getPropertyValue(FM_PROP_CYCLE);
            sal_Int32 aVal2 = 0;
            ::cppu::enum2int(aVal2,aVal);
            m_bCycle        = !aVal.hasValue() || aVal2 == TabulatorCycle_RECORDS;
            m_bCanUpdate    = aStaticTools.canUpdate(xSet);
            m_bCanInsert    = aStaticTools.canInsert(xSet);
            m_bCurrentRecordModified = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISMODIFIED));
            m_bCurrentRecordNew      = ::comphelper::getBOOL(xSet->getPropertyValue(FM_PROP_ISNEW));

            startFormListening( xSet, sal_False );

            // set the locks for the current controls
            if (getContainer().is())
            {
                if (m_nLoadEvent)
                    Application::RemoveUserEvent( m_nLoadEvent );
                m_nLoadEvent = Application::PostUserEvent( LINK(this, FmXFormController,
                                            OnLoad) );
            }
        }
        else
        {
            m_bCanInsert = m_bCanUpdate = m_bCycle = sal_False;
            m_bCurrentRecordModified = sal_False;
            m_bCurrentRecordNew = sal_False;
            m_bLocked = sal_False;
        }
        m_bDBConnection = sal_True;
    }
    else
    {
        m_bDBConnection = sal_False;
        m_bCanInsert = m_bCanUpdate = m_bCycle = sal_False;
        m_bCurrentRecordModified = sal_False;
        m_bCurrentRecordNew = sal_False;
        m_bLocked = sal_False;
    }

    updateAllDispatchers();
}

//------------------------------------------------------------------------------
void FmXFormController::updateAllDispatchers() const
{
    ::std::for_each(
        m_aFeatureDispatchers.begin(),
        m_aFeatureDispatchers.end(),
        ::std::compose1(
            UpdateAllListeners(),
            ::std::select2nd< DispatcherContainer::value_type >()
        )
    );
}

//------------------------------------------------------------------------------
IMPL_LINK(FmXFormController, OnLoad, void*, EMPTYARG)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_nLoadEvent = 0;
    m_bLocked = determineLockState();

    setLocks();

    if (!m_bLocked)
        startListening();

    // just one exception toggle the auto values
    if (m_bCurrentRecordNew)
        toggleAutoFields(sal_True);

    return 1L;
}

//------------------------------------------------------------------------------
void FmXFormController::unloaded(const EventObject& /*rEvent*/) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    updateAllDispatchers();
}

//------------------------------------------------------------------------------
void FmXFormController::reloading(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    // do the same like in unloading
    // just one exception toggle the auto values
    if (m_nToggleEvent)
    {
        Application::RemoveUserEvent( m_nToggleEvent );
        m_nToggleEvent = 0;
    }
    unload();
}

//------------------------------------------------------------------------------
void FmXFormController::reloaded(const EventObject& aEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    loaded(aEvent);
}

//------------------------------------------------------------------------------
void FmXFormController::unloading(const EventObject& /*aEvent*/) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    unload();
}

//------------------------------------------------------------------------------
void FmXFormController::unload() throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    if (m_nLoadEvent)
    {
        Application::RemoveUserEvent( m_nLoadEvent );
        m_nLoadEvent = 0;
    }

    // be sure not to have autofields
    if (m_bCurrentRecordNew)
        toggleAutoFields(sal_False);

    // remove bound field listing again
    removeBoundFieldListener();

    if (m_bDBConnection && isListeningForChanges())
        stopListening();

    Reference< XPropertySet >  xSet( m_xModelAsIndex, UNO_QUERY );
    if ( m_bDBConnection && xSet.is() )
        stopFormListening( xSet, sal_False );

    m_bDBConnection = sal_False;
    m_bCanInsert = m_bCanUpdate = m_bCycle = sal_False;
    m_bCurrentRecordModified = m_bCurrentRecordNew = m_bLocked = sal_False;
}

// -----------------------------------------------------------------------------
void FmXFormController::removeBoundFieldListener()
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

//------------------------------------------------------------------------------
void FmXFormController::startFormListening( const Reference< XPropertySet >& _rxForm, sal_Bool _bPropertiesOnly )
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
        OSL_ENSURE( sal_False, "FmXFormController::startFormListening: caught an exception!" );
    }
}

//------------------------------------------------------------------------------
void FmXFormController::stopFormListening( const Reference< XPropertySet >& _rxForm, sal_Bool _bPropertiesOnly )
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
        OSL_ENSURE( sal_False, "FmXFormController::stopFormListening: caught an exception!" );
    }
}

// com::sun::star::sdbc::XRowSetListener
//------------------------------------------------------------------------------
void FmXFormController::cursorMoved(const EventObject& /*event*/) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    // toggle the locking ?
    if (m_bLocked != determineLockState())
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_bLocked = !m_bLocked;
        setLocks();
        if (isListeningForChanges())
            startListening();
        else
            stopListening();
    }

    // neither the current control nor the current record are modified anymore
    m_bCurrentRecordModified = m_bModified = sal_False;
}

//------------------------------------------------------------------------------
void FmXFormController::rowChanged(const EventObject& /*event*/) throw( RuntimeException )
{
    // not interested in ...
}
//------------------------------------------------------------------------------
void FmXFormController::rowSetChanged(const EventObject& /*event*/) throw( RuntimeException )
{
    // not interested in ...
}


// XContainerListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::elementInserted(const ContainerEvent& evt) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    Reference< XControl >  xControl;
    evt.Element >>= xControl;
    if (!xControl.is())
        return;

    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XFormComponent >  xModel(xControl->getModel(), UNO_QUERY);
    if (xModel.is() && m_xModelAsIndex == xModel->getParent())
    {
        insertControl(xControl);

        if ( m_aTabActivationTimer.IsActive() )
            m_aTabActivationTimer.Stop();

        m_aTabActivationTimer.Start();
    }
    // are we in filtermode and a XModeSelector has inserted an element
    else if (m_bFiltering && Reference< XModeSelector > (evt.Source, UNO_QUERY).is())
    {
        xModel = Reference< XFormComponent > (evt.Source, UNO_QUERY);
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
                    m_aFilterControls[xText] = xField;
                    xText->addTextListener(this);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::elementReplaced(const ContainerEvent& evt) throw( RuntimeException )
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

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::elementRemoved(const ContainerEvent& evt) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XControl >  xControl;
    evt.Element >>= xControl;
    if (!xControl.is())
        return;

    Reference< XFormComponent >  xModel(xControl->getModel(), UNO_QUERY);
    if (xModel.is() && m_xModelAsIndex == xModel->getParent())
    {
        removeControl(xControl);
        // TabOrder nicht neu berechnen, da das intern schon funktionieren muß!
    }
    // are we in filtermode and a XModeSelector has inserted an element
    else if (m_bFiltering && Reference< XModeSelector > (evt.Source, UNO_QUERY).is())
    {
        Reference< XTextComponent >  xText(xControl, UNO_QUERY);
        FmFilterControls::iterator iter = m_aFilterControls.find(xText);
        if (iter != m_aFilterControls.end())
            m_aFilterControls.erase(iter);
    }
}

//------------------------------------------------------------------------------
Reference< XControl >  FmXFormController::isInList(const Reference< XWindowPeer > & xPeer) const
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
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

//------------------------------------------------------------------------------
void FmXFormController::activateFirst() throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::activateFirst : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->activateFirst();
}

//------------------------------------------------------------------------------
void FmXFormController::activateLast() throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );
    DBG_ASSERT(m_xTabController.is(), "FmXFormController::activateLast : invalid aggregate !");
    if (m_xTabController.is())
        m_xTabController->activateLast();
}

// XFormController
//------------------------------------------------------------------------------
Reference< XControl> SAL_CALL FmXFormController::getCurrentControl(void) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_xCurrentControl;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addActivateListener(const Reference< XFormControllerListener > & l) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aActivateListeners.addInterface(l);
}
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::removeActivateListener(const Reference< XFormControllerListener > & l) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aActivateListeners.removeInterface(l);
}

//------------------------------------------------------------------------------
void FmXFormController::setFilter(::std::vector<FmFieldInfo>& rFieldInfos)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    // create the composer
    Reference< XRowSet > xForm(m_xModelAsIndex, UNO_QUERY);
    Reference< XConnection > xConnection(OStaticDataAccessTools().getRowSetConnection(xForm));
    if (xForm.is())
    {
        Reference< XSQLQueryComposerFactory >  xFactory(xConnection, UNO_QUERY);
        if (xFactory.is())
        {
            m_xComposer = xFactory->createQueryComposer();
            try
            {
                Reference< XPropertySet >  xSet(xForm, UNO_QUERY);
                ::rtl::OUString aStatement  = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_ACTIVECOMMAND));
                ::rtl::OUString aFilter     = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_FILTER));
                m_xComposer->setQuery(aStatement);
                m_xComposer->setFilter(aFilter);
            }
            catch(const Exception&)
            {
                DBG_ERROR("Exception occured!");
            }
        }
    }

    if (m_xComposer.is())
    {
        Sequence < PropertyValue> aLevel;
        Sequence< Sequence < PropertyValue > > aFilterRows = m_xComposer->getStructuredFilter();

        // ok, we recieve the list of filters as sequence of fieldnames, value
        // now we have to transform the fieldname into UI names, that could be a label of the field or
        // a aliasname or the fieldname itself

        // first adjust the field names if necessary
        Reference< XNameAccess> xQueryColumns = Reference< XColumnsSupplier >
                                            (m_xComposer, UNO_QUERY)->getColumns();

        for (::std::vector<FmFieldInfo>::iterator iter = rFieldInfos.begin();
            iter != rFieldInfos.end(); iter++)
        {
            if ( xQueryColumns->hasByName((*iter).aFieldName) )
            {
                if ( (xQueryColumns->getByName((*iter).aFieldName) >>= (*iter).xField) && (*iter).xField.is() )
                    (*iter).xField->getPropertyValue(FM_PROP_REALNAME) >>= (*iter).aFieldName;
            }
        }

        Reference< XDatabaseMetaData> xMetaData(xConnection->getMetaData());
        // now transfer the filters into Value/TextComponent pairs
        ::comphelper::UStringMixEqual aCompare(xMetaData->storesMixedCaseQuotedIdentifiers());

        // need to parse criteria localized
        OStaticDataAccessTools aStaticTools;
        Reference< XNumberFormatsSupplier> xFormatSupplier( aStaticTools.getNumberFormats(xConnection, sal_True));
        Reference< XNumberFormatter> xFormatter(m_xORB
                        ->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
        xFormatter->attachNumberFormatsSupplier(xFormatSupplier);
        Locale aAppLocale = Application::GetSettings().GetUILocale();
        LocaleDataWrapper aLocaleWrapper(m_xORB,aAppLocale);

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
                    ::rtl::OUString aRealName;

                    // first look with the given name
                    if (xQueryColumns->hasByName(pRefValues[j].Name))
                    {
                        xQueryColumns->getByName(pRefValues[j].Name) >>= xSet;

                        // get the RealName
                        xSet->getPropertyValue(::rtl::OUString::createFromAscii("RealName")) >>= aRealName;

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
                            xSet->getPropertyValue(::rtl::OUString::createFromAscii("RealName")) >>= aRealName;
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
                for (::std::vector<FmFieldInfo>::iterator iter = rFieldInfos.begin();
                    iter != rFieldInfos.end(); iter++)
                {
                    // we found the field so insert a new entry to the filter row
                    if ((*iter).xField == xField)
                    {
                        // do we already have the control ?
                        if (aRow.find((*iter).xText) != aRow.end())
                        {
                            ::rtl::OUString aCompText = aRow[(*iter).xText];
                            aCompText += ::rtl::OUString::createFromAscii(" ");
                            ::rtl::OString aVal = m_xParser->getContext().getIntlKeywordAscii(OParseContext::KEY_AND);
                            aCompText += ::rtl::OUString(aVal.getStr(),aVal.getLength(),RTL_TEXTENCODING_ASCII_US);
                            aCompText += ::rtl::OUString::createFromAscii(" ");
                            aCompText += ::comphelper::getString(pRefValues[j].Value);
                            aRow[(*iter).xText] = aCompText;
                        }
                        else
                        {
                            ::rtl::OUString sPredicate,sErrorMsg;
                            pRefValues[j].Value >>= sPredicate;
                            ::rtl::Reference< ISQLParseNode > xParseNode = predicateTree(sErrorMsg, sPredicate, xFormatter, xField);
                            if ( xParseNode.is() )
                            {
                                ::rtl::OUString sCriteria;
                                xParseNode->parseNodeToPredicateStr( sCriteria
                                                                    ,xConnection
                                                                    ,xFormatter
                                                                    ,xField
                                                                    ,aAppLocale
                                                                    ,(sal_Char)aLocaleWrapper.getNumDecimalSep().GetChar(0)
                                                                    ,getParseContext());
                                aRow[(*iter).xText] = sCriteria;
                            }
                        }
                        break;
                    }
                }
            }

            if (aRow.empty())
                continue;

            m_aFilters.push_back(aRow);
        }
    }

    // now set the filter controls
    for (::std::vector<FmFieldInfo>::iterator iter = rFieldInfos.begin();
         iter != rFieldInfos.end(); iter++)
    {
        m_aFilterControls[(*iter).xText] = (*iter).xField;
    }

    // add an empty row
    m_aFilters.push_back(FmFilterRow());
}

//------------------------------------------------------------------------------
void FmXFormController::startFiltering()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");

    OStaticDataAccessTools aStaticTools;
    Reference< XConnection >  xConnection( aStaticTools.getRowSetConnection( Reference< XRowSet >( m_xModelAsIndex, UNO_QUERY ) ) );
    if ( !xConnection.is() )
        // nothing to do - can't filter a form which is not connected
        // 98023 - 19.03.2002 - fs@openoffice.org
        return;

    // stop listening for controls
    if (isListeningForChanges())
        stopListening();

    m_bFiltering = sal_True;

    // as we don't want new controls to be attached to the scripting environment
    // we change attach flags
    m_bAttachEvents = sal_False;

    // Austauschen der Kontrols fuer das aktuelle Formular
    Sequence< Reference< XControl > > aControlsCopy( m_aControls );
    const Reference< XControl >* pControls = aControlsCopy.getConstArray();
    sal_Int32 nControlCount = aControlsCopy.getLength();

    // the control we have to activate after replacement
    Reference< XDatabaseMetaData >  xMetaData(xConnection->getMetaData());
    Reference< XNumberFormatsSupplier >  xFormatSupplier = aStaticTools.getNumberFormats(xConnection, sal_True);
    Reference< XNumberFormatter >  xFormatter(m_xORB
                        ->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
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
                xSelector->setMode(FILTER_MODE);

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
                                    aFieldInfos.push_back(FmFieldInfo(xField, xText));
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
                    Sequence< Any > aCreationArgs( 3 );
                    aCreationArgs[ 0 ] <<= NamedValue( ::rtl::OUString::createFromAscii( "MessageParent" ), makeAny( VCLUnoHelper::GetInterface( getDialogParentWindow() ) ) );
                    aCreationArgs[ 1 ] <<= NamedValue( ::rtl::OUString::createFromAscii( "NumberFormatter" ), makeAny( xFormatter ) );
                    aCreationArgs[ 2 ] <<= NamedValue( ::rtl::OUString::createFromAscii( "ControlModel" ), makeAny( xModel ) );
                    Reference< XControl > xFilterControl(
                        m_xORB->createInstanceWithArguments(
                            ::rtl::OUString::createFromAscii( "com.sun.star.form.control.FilterControl" ),
                            aCreationArgs
                        ),
                        UNO_QUERY
                    );
                    DBG_ASSERT( xFilterControl.is(), "FmXFormController::startFiltering: could not create a filter control!" );

                    if ( replaceControl( xControl, xFilterControl ) )
                    {
                        Reference< XTextComponent > xFilterText( xFilterControl, UNO_QUERY );
                        aFieldInfos.push_back( FmFieldInfo( xField, xFilterText ) );
                        xFilterText->addTextListener(this);
                    }
                }
            }
            else
            {
                // abmelden vom EventManager
            }
        }
    }

    // we have all filter controls now, so the next step is to read the filters from the form
    // resolve all aliases and set the current filter to the according structure
    setFilter(aFieldInfos);

    Reference< XPropertySet > xSet( m_xModelAsIndex, UNO_QUERY );
    if ( xSet.is() )
        stopFormListening( xSet, sal_True );

    // set the text for all filters
    OSL_ENSURE( ( m_aFilters.size() > (size_t)m_nCurrentFilterPosition ) && ( m_nCurrentFilterPosition >= 0 ),
        "FmXFormController::startFiltering: m_nCurrentFilterPosition too big" );
    if ( ( m_nCurrentFilterPosition >= 0 ) && ( (size_t)m_nCurrentFilterPosition < m_aFilters.size() ) )
    {
        FmFilterRow& rRow = m_aFilters[m_nCurrentFilterPosition];
        for (FmFilterRow::const_iterator iter2 = rRow.begin();
            iter2 != rRow.end(); iter2++)
        {
            (*iter2).first->setText((*iter2).second);
        }
    }

    // lock all controls which are not used for filtering
    m_bLocked = determineLockState();
    setLocks();
    m_bAttachEvents = sal_True;
}

//------------------------------------------------------------------------------
void FmXFormController::stopFiltering()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if ( !m_bFiltering ) // #104693# OJ
    {   // nothing to do
        return;
    }

    m_bFiltering = sal_False;
    m_bDetachEvents = sal_False;

    ::comphelper::disposeComponent(m_xComposer);

    // Austauschen der Kontrols fuer das aktuelle Formular
    Sequence< Reference< XControl > > aControlsCopy( m_aControls );
    const Reference< XControl > * pControls = aControlsCopy.getConstArray();
    sal_Int32 nControlCount = aControlsCopy.getLength();
    // SdrPageView* pCurPageView = m_pView->GetSdrPageView();

    // sal_uInt16 nPos = pCurPageView ? pCurPageView->GetWinList().Find((OutputDevice*)m_pView->GetActualOutDev()) : SDRPAGEVIEWWIN_NOTFOUND;
    // const SdrPageWindow* pWindow = pCurPageView ? pCurPageView->FindPageWindow(*((OutputDevice*)m_pView->GetActualOutDev())) : 0L;

    // clear the filter control map
    for (FmFilterControls::const_iterator iter = m_aFilterControls.begin();
         iter != m_aFilterControls.end(); iter++)
         (*iter).first->removeTextListener(this);

    m_aFilterControls.clear();

    for ( sal_Int32 i = nControlCount; i > 0; )
    {
        Reference< XControl > xControl = pControls[--i];
        if (xControl.is())
        {
            // now enable eventhandling again
            addToEventAttacher(xControl);

            Reference< XModeSelector >  xSelector(xControl, UNO_QUERY);
            if (xSelector.is())
            {
                xSelector->setMode(DATA_MODE);

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
                    ::rtl::OUString sServiceName;
                    OSL_VERIFY( xSet->getPropertyValue( FM_PROP_DEFAULTCONTROL ) >>= sServiceName );
                    Reference< XControl > xNewControl( m_xORB->createInstance( sServiceName ), UNO_QUERY );
                    replaceControl( xControl, xNewControl );
                }
            }
        }
    }

    Reference< XPropertySet >  xSet( m_xModelAsIndex, UNO_QUERY );
    if ( xSet.is() )
        startFormListening( xSet, sal_True );

    m_bDetachEvents = sal_True;

    m_aFilters.clear();
    m_nCurrentFilterPosition = 0;

    // release the locks if possible
    // lock all controls which are not used for filtering
    m_bLocked = determineLockState();
    setLocks();

    // restart listening for control modifications
    if (isListeningForChanges())
        startListening();
}

// XModeSelector
//------------------------------------------------------------------------------
void FmXFormController::setMode(const ::rtl::OUString& Mode) throw( NoSupportException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if (!supportsMode(Mode))
        throw NoSupportException();

    if (Mode == m_aMode)
        return;

    m_aMode = Mode;

    if (Mode == FILTER_MODE)
        startFiltering();
    else
        stopFiltering();

    for (FmFormControllers::const_iterator i = m_aChilds.begin();
        i != m_aChilds.end(); ++i)
    {
        Reference< XModeSelector > xMode(*i, UNO_QUERY);
        if ( xMode.is() )
            xMode->setMode(Mode);
    }
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL FmXFormController::getMode(void) throw( RuntimeException )
{
::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    return m_aMode;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL FmXFormController::getSupportedModes(void) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    static Sequence< ::rtl::OUString > aModes;
    if (!aModes.getLength())
    {
        aModes.realloc(2);
        ::rtl::OUString* pModes = aModes.getArray();
        pModes[0] = DATA_MODE;
        pModes[1] = FILTER_MODE;
    }
    return aModes;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::supportsMode(const ::rtl::OUString& Mode) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    Sequence< ::rtl::OUString > aModes(getSupportedModes());
    const ::rtl::OUString* pModes = aModes.getConstArray();
    for (sal_Int32 i = aModes.getLength(); i > 0; )
    {
        if (pModes[--i] == Mode)
            return sal_True;
    }
    return sal_False;
}

//------------------------------------------------------------------------------
Window* FmXFormController::getDialogParentWindow()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    Window* pParent = m_pWindow;
    if ( !pParent )
    {
        try
        {
            Reference< XControl > xContainerControl( getContainer(), UNO_QUERY_THROW );
            Reference< XWindowPeer > xContainerPeer( xContainerControl->getPeer(), UNO_QUERY_THROW );
            pParent = VCLUnoHelper::GetWindow( xContainerPeer );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "FmXFormController::getDialogParentWindow: caught an exception!" );
        }
    }
    return pParent;
}
//------------------------------------------------------------------------------
bool FmXFormController::checkFormComponentValidity( ::rtl::OUString& /* [out] */ _rFirstInvalidityExplanation, Reference< XControlModel >& /* [out] */ _rxFirstInvalidModel ) SAL_THROW(())
{
    try
    {
        Reference< XEnumerationAccess > xControlEnumAcc( getModel(), UNO_QUERY );
        Reference< XEnumeration > xControlEnumeration;
        if ( xControlEnumAcc.is() )
            xControlEnumeration = xControlEnumAcc->createEnumeration();
        OSL_ENSURE( xControlEnumeration.is(), "FmXFormController::checkFormComponentValidity: cannot enumerate the controls!" );
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
            OSL_ENSURE( xValidator.is(), "FmXFormController::checkFormComponentValidity: invalid, but no validator?" );
            if ( !xValidator.is() )
                // this violates the interface definition of css.form.validation.XValidatableFormComponent ...
                continue;

            _rFirstInvalidityExplanation = xValidator->explainInvalid( xValidatable->getCurrentValue() );
            _rxFirstInvalidModel = _rxFirstInvalidModel.query( xValidatable );
            return false;
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "FmXFormController::checkFormComponentValidity: caught an exception!" );
    }
    return true;
}

//------------------------------------------------------------------------------
Reference< XControl > FmXFormController::locateControl( const Reference< XControlModel >& _rxModel ) SAL_THROW(())
{
    try
    {
        Sequence< Reference< XControl > > aControls( getControls() );
        const Reference< XControl >* pControls = aControls.getConstArray();
        const Reference< XControl >* pControlsEnd = aControls.getConstArray() + aControls.getLength();

        for ( ; pControls != pControlsEnd; ++pControls )
        {
            OSL_ENSURE( pControls->is(), "FmXFormController::locateControl: NULL-control?" );
            if ( pControls->is() )
            {
                if ( ( *pControls)->getModel() == _rxModel )
                    return *pControls;
            }
        }
        OSL_ENSURE( sal_False, "FmXFormController::locateControl: did not find a control for this model!" );
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "FmXFormController::locateControl: caught an exception!" );
    }
    return NULL;
}

//------------------------------------------------------------------------------
namespace
{
    void displayErrorSetFocus( const String& _rMessage, const Reference< XControl >& _rxFocusControl, Window* _pDialogParent )
    {
        SQLContext aError;
        aError.Message = String( SVX_RES( RID_STR_WRITEERROR ) );
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

    sal_Bool lcl_shouldValidateRequiredFields_nothrow( const Reference< XInterface >& _rxForm )
    {
        try
        {
            static ::rtl::OUString s_sFormsCheckRequiredFields( RTL_CONSTASCII_USTRINGPARAM( "FormsCheckRequiredFields" ) );

            // first, check whether the form has a property telling us the answer
            // this allows people to use the XPropertyContainer interface of a form to control
            // the behaviour on a per-form basis.
            Reference< XPropertySet > xFormProps( _rxForm, UNO_QUERY_THROW );
            Reference< XPropertySetInfo > xPSI( xFormProps->getPropertySetInfo() );
            if ( xPSI->hasPropertyByName( s_sFormsCheckRequiredFields ) )
            {
                sal_Bool bShouldValidate = true;
                OSL_VERIFY( xFormProps->getPropertyValue( s_sFormsCheckRequiredFields ) >>= bShouldValidate );
                return bShouldValidate;
            }

            // next, check the data source which created the connection
            Reference< XChild > xConnectionAsChild( xFormProps->getPropertyValue( FM_PROP_ACTIVE_CONNECTION ), UNO_QUERY_THROW );
            Reference< XPropertySet > xDataSource( xConnectionAsChild->getParent(), UNO_QUERY );
            if ( !xDataSource.is() )
                // seldom (but possible): this is not a connection created by a data source
                return sal_True;

            Reference< XPropertySet > xDataSourceSettings(
                xDataSource->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Settings" ) ) ),
                UNO_QUERY_THROW );

            sal_Bool bShouldValidate = true;
            OSL_VERIFY( xDataSourceSettings->getPropertyValue( s_sFormsCheckRequiredFields ) >>= bShouldValidate );
            return bShouldValidate;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return sal_True;
    }
}

// XRowSetApproveListener
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::approveRowChange(const RowChangeEvent& _rEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
    sal_Bool bValid = sal_True;
    if (aIter.hasMoreElements())
    {
        RowChangeEvent aEvt( _rEvent );
        aEvt.Source = *this;
        bValid = ((XRowSetApproveListener*)aIter.next())->approveRowChange(aEvt);
    }

    if ( !bValid )
        return bValid;

    if  (   ( _rEvent.Action != RowChangeAction::INSERT )
        &&  ( _rEvent.Action != RowChangeAction::UPDATE )
        )
        return bValid;

    // if some of the control models are bound to validators, check them
    ::rtl::OUString sInvalidityExplanation;
    Reference< XControlModel > xInvalidModel;
    if ( !checkFormComponentValidity( sInvalidityExplanation, xInvalidModel ) )
    {
        displayErrorSetFocus( sInvalidityExplanation, locateControl( xInvalidModel ), getDialogParentWindow() );
        return false;
    }

    // check values on NULL and required flag
    if ( !lcl_shouldValidateRequiredFields_nothrow( _rEvent.Source ) )
        return sal_True;

    try
    {
        Reference< XColumnsSupplier > xSupplyCols( _rEvent.Source, UNO_QUERY_THROW );
        Reference< XEnumerationAccess > xEnumAccess( xSupplyCols->getColumns(), UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumeration( xEnumAccess->createEnumeration(), UNO_QUERY_THROW );

        Reference< XPropertySet >  xFieldSet;
        while ( xEnumeration->hasMoreElements() )
        {
            xEnumeration->nextElement() >>= xFieldSet;
            Reference< XColumn > xColumn( xFieldSet, UNO_QUERY);
            if ( !xColumn.is() )
            {
                DBG_ERROR( "FmXFormController::approveRowChange: invalid field !" );
                continue;
            }

            sal_Bool bRequired = ::comphelper::getINT32( xFieldSet->getPropertyValue( FM_PROP_ISNULLABLE ) ) == ColumnValue::NO_NULLS;
            if ( !bRequired )
                continue;

            sal_Bool bAutoIncrement = ::comphelper::getBOOL( xFieldSet->getPropertyValue( FM_PROP_AUTOINCREMENT ) );
            if ( bAutoIncrement )
                continue;

            ::rtl::OUString aFieldName( ::comphelper::getString( xFieldSet->getPropertyValue( FM_PROP_NAME ) ) );

            // TODO: in case of binary fields, this "getString" below is extremely expensive
            if ( !xColumn->getString().getLength() && xColumn->wasNull() )
            {
                Sequence< Reference< XControl > > aControls( getControls() );
                sal_Int32 nLength = aControls.getLength();
                sal_Int32 i(0);
                const Reference< XControl > * pControls = aControls.getConstArray();
                for (; i < nLength; i++)
                {
                    if (pControls[i].is())
                    {
                        Reference< XPropertySet >  xModel(pControls[i]->getModel(), UNO_QUERY);
                        if (xModel.is())
                        {
                            if (::comphelper::hasProperty(FM_PROP_CONTROLSOURCE, xModel))
                            {
                                ::rtl::OUString aName = ::comphelper::getString(xModel->getPropertyValue(FM_PROP_CONTROLSOURCE));
                                if (aName == aFieldName)    // Control gefunden
                                    break;
                            }
                        }
                    }
                }

                String sMessage( SVX_RES( RID_ERR_FIELDREQUIRED ) );
                sMessage.SearchAndReplace( '#', aFieldName.getStr() );
                displayErrorSetFocus( sMessage, ( i < nLength ) ? pControls[i] : Reference< XControl >(), getDialogParentWindow() );

                return sal_False;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return true;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::approveCursorMove(const EventObject& event) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
    if (aIter.hasMoreElements())
    {
        EventObject aEvt(event);
        aEvt.Source = *this;
        return ((XRowSetApproveListener*)aIter.next())->approveCursorMove(aEvt);
    }

    return sal_True;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::approveRowSetChange(const EventObject& event) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
    if (aIter.hasMoreElements())
    {
        EventObject aEvt(event);
        aEvt.Source = *this;
        return ((XRowSetApproveListener*)aIter.next())->approveRowSetChange(aEvt);
    }

    return sal_True;
}

// XRowSetApproveBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addRowSetApproveListener(const Reference< XRowSetApproveListener > & _rxListener) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aRowSetApproveListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::removeRowSetApproveListener(const Reference< XRowSetApproveListener > & _rxListener) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aRowSetApproveListeners.removeInterface(_rxListener);
}

// XErrorListener
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::errorOccured(const SQLErrorEvent& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");

    ::cppu::OInterfaceIteratorHelper aIter(m_aErrorListeners);
    if (aIter.hasMoreElements())
    {
        SQLErrorEvent aEvt(aEvent);
        aEvt.Source = *this;
        ((XSQLErrorListener*)aIter.next())->errorOccured(aEvt);
    }
    else
    {
        displayException(aEvent);
    }
}

// XErrorBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addSQLErrorListener(const Reference< XSQLErrorListener > & aListener) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aErrorListeners.addInterface(aListener);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::removeSQLErrorListener(const Reference< XSQLErrorListener > & aListener) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aErrorListeners.removeInterface(aListener);
}

// XDatabaseParameterBroadcaster2
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addDatabaseParameterListener(const Reference< XDatabaseParameterListener > & aListener) throw( RuntimeException )
{
    OSL_ENSURE( !FmXFormController_BASE1::rBHelper.bDisposed, "FmXFormController::addDatabaseParameterListener: Object already disposed!" );
    m_aParameterListeners.addInterface(aListener);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::removeDatabaseParameterListener(const Reference< XDatabaseParameterListener > & aListener) throw( RuntimeException )
{
    OSL_ENSURE( !FmXFormController_BASE1::rBHelper.bDisposed, "FmXFormController::removeDatabaseParameterListener: Object already disposed!" );
    m_aParameterListeners.removeInterface(aListener);
}

// XDatabaseParameterBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addParameterListener(const Reference< XDatabaseParameterListener > & aListener) throw( RuntimeException )
{
    FmXFormController::addDatabaseParameterListener( aListener );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::removeParameterListener(const Reference< XDatabaseParameterListener > & aListener) throw( RuntimeException )
{
    FmXFormController::removeDatabaseParameterListener( aListener );
}

// XDatabaseParameterListener
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::approveParameter(const DatabaseParameterEvent& aEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");

    ::cppu::OInterfaceIteratorHelper aIter(m_aParameterListeners);
    if (aIter.hasMoreElements())
    {
        DatabaseParameterEvent aEvt(aEvent);
        aEvt.Source = *this;
        return ((XDatabaseParameterListener*)aIter.next())->approveParameter(aEvt);
    }
    else
    {
        // default handling: instantiate an interaction handler and let it handle the parameter request
        try
        {
            if ( !ensureInteractionHandler() )
                return sal_False;

            // two continuations allowed: OK and Cancel
            OParameterContinuation* pParamValues = new OParameterContinuation;
            OInteractionAbort* pAbort = new OInteractionAbort;
            // the request
            ParametersRequest aRequest;
            aRequest.Parameters = aEvent.Parameters;
            aRequest.Connection = OStaticDataAccessTools().getRowSetConnection(Reference< XRowSet >(aEvent.Source, UNO_QUERY));
            OInteractionRequest* pParamRequest = new OInteractionRequest(makeAny(aRequest));
            Reference< XInteractionRequest > xParamRequest(pParamRequest);
            // some knittings
            pParamRequest->addContinuation(pParamValues);
            pParamRequest->addContinuation(pAbort);

            // handle the request
            {
                ::vos::OGuard aGuard(Application::GetSolarMutex());
                m_xInteractionHandler->handle(xParamRequest);
            }

            if (!pParamValues->wasSelected())
                // canceled
                return sal_False;

            // transfer the values into the parameter supplier
            Sequence< PropertyValue > aFinalValues = pParamValues->getValues();
            if (aFinalValues.getLength() != aRequest.Parameters->getCount())
            {
                DBG_ERROR("FmXFormController::approveParameter: the InteractionHandler returned nonsense!");
                return sal_False;
            }
            const PropertyValue* pFinalValues = aFinalValues.getConstArray();
            for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
            {
                Reference< XPropertySet > xParam;
                ::cppu::extractInterface(xParam, aRequest.Parameters->getByIndex(i));
                if (xParam.is())
                {
#ifdef DBG_UTIL
                    ::rtl::OUString sName;
                    xParam->getPropertyValue(FM_PROP_NAME) >>= sName;
                    DBG_ASSERT(sName.equals(pFinalValues->Name), "FmXFormController::approveParameter: suspicious value names!");
#endif
                    try { xParam->setPropertyValue(FM_PROP_VALUE, pFinalValues->Value); }
                    catch(Exception&)
                    {
                        DBG_ERROR("FmXFormController::approveParameter: setting one of the properties failed!");
                    }
                }
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("FmXFormController::approveParameter: caught an Exception (tried to let the InteractionHandler handle it)!");
        }
    }
    return sal_True;
}

// XConfirmDeleteBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addConfirmDeleteListener(const Reference< XConfirmDeleteListener > & aListener) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aDeleteListeners.addInterface(aListener);
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::removeConfirmDeleteListener(const Reference< XConfirmDeleteListener > & aListener) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_aDeleteListeners.removeInterface(aListener);
}

// XConfirmDeleteListener
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::confirmDelete(const RowChangeEvent& aEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");

    ::cppu::OInterfaceIteratorHelper aIter(m_aDeleteListeners);
    if (aIter.hasMoreElements())
    {
        RowChangeEvent aEvt(aEvent);
        aEvt.Source = *this;
        return ((XConfirmDeleteListener*)aIter.next())->confirmDelete(aEvt);
    }
    else
    {
        // default handling
        UniString aTitle;
        sal_Int32 nLength = aEvent.Rows;
        if (nLength > 1)
        {
            aTitle = SVX_RES(RID_STR_DELETECONFIRM_RECORDS);
            aTitle.SearchAndReplace('#', String::CreateFromInt32(nLength));
        }
        else
            aTitle = SVX_RES(RID_STR_DELETECONFIRM_RECORD);

        ConfirmDeleteDialog aDlg(getDialogParentWindow(), aTitle);
        return RET_YES == aDlg.Execute();
    }
}

//------------------------------------------------------------------------------
void FmXFormController::invalidateFeatures( const ::std::vector< sal_Int32 >& _rFeatures )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // for now, just copy the ids of the features, because ....
    ::std::copy( _rFeatures.begin(), _rFeatures.end(),
        ::std::insert_iterator< ::std::set< sal_Int32 > >( m_aInvalidFeatures, m_aInvalidFeatures.begin() )
    );

    // ... we will do the real invalidation asynchronously
    if ( !m_aFeatureInvalidationTimer.IsActive() )
        m_aFeatureInvalidationTimer.Start();
}

//------------------------------------------------------------------------------
Reference< XDispatch >
FmXFormController::interceptedQueryDispatch(sal_uInt16 /*_nId*/, const URL& aURL,
                                            const ::rtl::OUString& /*aTargetFrameName*/, sal_Int32 /*nSearchFlags*/)
                                            throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    Reference< XDispatch >  xReturn;
    // dispatches handled by ourself
    if  (   ( aURL.Complete == FMURL_CONFIRM_DELETION )
        ||  (   ( aURL.Complete.equalsAscii( "private:/InteractionHandler" ) )
            &&  ensureInteractionHandler()
            )
        )
        xReturn = static_cast< XDispatch* >( this );

    // dispatches of FormSlot-URLs we have to translate
    if ( !xReturn.is() && m_aControllerFeatures.isAssigned() )
    {
        // find the slot id which corresponds to the URL
        sal_Int32 nFeatureId = ::svx::FeatureSlotTranslation::getControllerFeatureSlotIdForURL( aURL.Main );
        if ( nFeatureId > 0 )
        {
            // get the dispatcher for this feature, create if necessary
            DispatcherContainer::const_iterator aDispatcherPos = m_aFeatureDispatchers.find( nFeatureId );
            if ( aDispatcherPos == m_aFeatureDispatchers.end() )
            {
                aDispatcherPos = m_aFeatureDispatchers.insert(
                    DispatcherContainer::value_type( nFeatureId, new ::svx::OSingleFeatureDispatcher( aURL, nFeatureId, *m_aControllerFeatures, m_aMutex ) )
                ).first;
            }

            OSL_ENSURE( aDispatcherPos->second.is(), "FmXFormController::interceptedQueryDispatch: should have a dispatcher by now!" );
            return aDispatcherPos->second;
        }
    }

    // no more to offer
    return xReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::dispatch( const URL& _rURL, const Sequence< PropertyValue >& _rArgs ) throw (RuntimeException)
{
    if ( _rArgs.getLength() != 1 )
    {
        DBG_ERROR( "FmXFormController::dispatch: no arguments -> no dispatch!" );
        return;
    }

    if ( _rURL.Complete.equalsAscii( "private:/InteractionHandler" ) )
    {
        Reference< XInteractionRequest > xRequest;
        OSL_VERIFY( _rArgs[0].Value >>= xRequest );
        if ( xRequest.is() )
            handle( xRequest );
        return;
    }

    if  ( _rURL.Complete == FMURL_CONFIRM_DELETION )
    {
        DBG_ERROR( "FmXFormController::dispatch: How do you expect me to return something via this call?" );
            // confirmDelete has a return value - dispatch hasn't
        return;
    }

    DBG_ERROR( "FmXFormController::dispatch: unknown URL!" );
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::addStatusListener( const Reference< XStatusListener >& _rxListener, const URL& _rURL ) throw (RuntimeException)
{
    if (_rURL.Complete == FMURL_CONFIRM_DELETION)
    {
        if (_rxListener.is())
        {   // send an initial statusChanged event
            FeatureStateEvent aEvent;
            aEvent.FeatureURL = _rURL;
            aEvent.IsEnabled = sal_True;
            _rxListener->statusChanged(aEvent);
            // and don't add the listener at all (the status will never change)
        }
    }
    else
        OSL_ENSURE(sal_False, "FmXFormController::addStatusListener: invalid (unsupported) URL!");
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::removeStatusListener( const Reference< XStatusListener >& /*_rxListener*/, const URL& _rURL ) throw (RuntimeException)
{
    (void)_rURL;
    OSL_ENSURE(_rURL.Complete == FMURL_CONFIRM_DELETION, "FmXFormController::removeStatusListener: invalid (unsupported) URL!");
    // we never really added the listener, so we don't need to remove it
}

//------------------------------------------------------------------------------
Reference< XDispatchProviderInterceptor >  FmXFormController::createInterceptor(const Reference< XDispatchProviderInterception > & _xInterception)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
#ifdef DBG_UTIL
    // check if we already have a interceptor for the given object
    for (   ConstInterceptorsIterator aIter = m_aControlDispatchInterceptors.begin();
            aIter != m_aControlDispatchInterceptors.end();
            ++aIter
        )
    {
        if ((*aIter)->getIntercepted() == _xInterception)
            DBG_ERROR("FmXFormController::createInterceptor : we already do intercept this objects dispatches !");
    }
#endif

    ::rtl::OUString sInterceptorScheme(RTL_CONSTASCII_USTRINGPARAM("*"));
    FmXDispatchInterceptorImpl* pInterceptor = new FmXDispatchInterceptorImpl(_xInterception, this, 0, Sequence< ::rtl::OUString >(&sInterceptorScheme, 1));
    pInterceptor->acquire();
    m_aControlDispatchInterceptors.insert(m_aControlDispatchInterceptors.end(), pInterceptor);

    return (XDispatchProviderInterceptor*)pInterceptor;
}

//------------------------------------------------------------------------------
bool FmXFormController::ensureInteractionHandler()
{
    if ( m_xInteractionHandler.is() )
        return true;
    if ( m_bAttemptedHandlerCreation )
        return false;
    m_bAttemptedHandlerCreation = true;
    if ( !m_xORB.is() )
        return false;

    m_xInteractionHandler.set( m_xORB->createInstance( SRV_SDB_INTERACTION_HANDLER ), UNO_QUERY );
    OSL_ENSURE( m_xInteractionHandler.is(), "FmXFormController::ensureInteractionHandler: could not create an interaction handler!" );
    return m_xInteractionHandler.is();
}

//------------------------------------------------------------------------------
void SAL_CALL FmXFormController::handle( const Reference< XInteractionRequest >& _rRequest ) throw (RuntimeException)
{
    if ( !ensureInteractionHandler() )
        return;
    m_xInteractionHandler->handle( _rRequest );
}

//------------------------------------------------------------------------------
void FmXFormController::deleteInterceptor(const Reference< XDispatchProviderInterception > & _xInterception)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    // search the interceptor responsible for the given object
    InterceptorsIterator aIter;
    for (   aIter = m_aControlDispatchInterceptors.begin();
            aIter != m_aControlDispatchInterceptors.end();
            ++aIter
        )
    {
        if ((*aIter)->getIntercepted() == _xInterception)
            break;
    }
    if (aIter == m_aControlDispatchInterceptors.end())
    {
        return;
    }

    // log off the interception from it's interception object
    FmXDispatchInterceptorImpl* pInterceptorImpl = *aIter;
    pInterceptorImpl->dispose();
    pInterceptorImpl->release();

    // remove the interceptor from our array
    m_aControlDispatchInterceptors.erase(aIter);
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
{
    DBG_ASSERT( !m_xInteractionHandler.is(), "FmXFormController::initialize: already initialized!" );
        // currently, we only initialize our interaction handler here, so it's sufficient to assert this

    NamedValue aNamed;
    PropertyValue aProperty;

    const Any* pArg = aArguments.getConstArray();
    const Any* pArgEnd = aArguments.getConstArray() + aArguments.getLength();
    while ( pArg != pArgEnd )
    {
        if ( ( *pArg >>= aNamed ) && aNamed.Name.equalsAscii( "InteractionHandler" ) )
        {
            OSL_VERIFY( aNamed.Value >>= m_xInteractionHandler );
            break;
        }

        if ( ( *pArg >>= aProperty ) && aProperty.Name.equalsAscii( "InteractionHandler" ) )
        {
            OSL_VERIFY( aProperty.Value >>= m_xInteractionHandler );
            break;
        }

        if ( *pArg >>= m_xInteractionHandler )
        {
            break;
        }

        ++pArg;
    }
}

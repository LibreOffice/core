/*************************************************************************
 *
 *  $RCSfile: fmctrler.cxx,v $
 *
 *  $Revision: 1.45 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:50:20 $
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
#pragma hdrstop

#define USE_PROPERTY_HELPER

#ifndef _SVX_FMCTRLER_HXX
#include "fmctrler.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef SVX_FORM_CONFIRMDELETE_HXX
#include "confirmdelete.hxx"
#endif
#ifndef SVX_SOURCE_INC_FMCONTROLBORDERMANAGER_HXX
#include "fmcontrolbordermanager.hxx"
#endif
#ifndef SVX_SOURCE_INC_FMDOCUMENTCLASSIFICATION_HXX
#include "fmdocumentclassification.hxx"
#endif
#ifndef _SVX_FMURL_HXX
#include "fmurl.hxx"
#endif
#ifndef SVX_FMDISPATCH_HXX
#include "fmdispatch.hxx"
#endif

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
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _SVDPAGV_HXX //autogen
#include <svdpagv.hxx>
#endif

#ifndef _SVX_FMSHIMP_HXX
#include <fmshimp.hxx>
#endif

#ifndef _SVX_FMVIEW_HXX
#include <fmview.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen wg. RET_YES
#include <vcl/msgbox.hxx>
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

#ifndef _SVX_FMPROP_HXX
#include "fmprop.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_AGGREGATION_HXX_
#include <comphelper/propagg.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _TOOLKIT_CONTROLS_UNOCONTROL_HXX_
#include <toolkit/controls/unocontrol.hxx>
#endif

#include <algorithm>

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
        xText->setText(::rtl::OUString(SVX_RES(RID_STR_AUTOFIELD)));
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
IMPL_LINK( FmXFormController, OnActivateTabOrder, void*, EMPTYTAG )
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
IMPL_LINK( FmXFormController, OnInvalidateFeatures, void*, _pNotInterestedInThisParam )
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
                  ,m_aControllerFeatures( _rxORB, this )
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
                  ,m_bDBConnection(sal_False)
                  ,m_bCycle(sal_False)
                  ,m_bCanUpdate(sal_False)
                  ,m_bCanInsert(sal_False)
                  ,m_bModified(sal_False)
                  ,m_nLoadEvent( 0 )
                  ,m_bCommitLock(sal_False)
                  ,m_bCurrentRecordModified(sal_False)
                  ,m_bCurrentRecordNew(sal_False)
                  ,m_bLocked(sal_False)
                  ,m_aMode(getDataModeIdentifier())
                  ,m_bFiltering(sal_False)
                  ,m_bAttachEvents(sal_True)
                  ,m_bDetachEvents(sal_True)
                  ,m_nCurrentFilterPosition(0)
                  ,m_nToggleEvent(0)
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

    m_aControllerFeatures.dispose( );

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
sal_Int64 SAL_CALL FmXFormController::getSomething(Sequence<sal_Int8> const& rId)throw( RuntimeException )
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

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
sal_Bool SAL_CALL FmXFormController::approveReset(const EventObject& rEvent) throw( RuntimeException )
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
sal_Bool FmXFormController::convertFastPropertyValue( Any & rConvertedValue, Any & rOldValue,
                                            sal_Int32 nHandle, const Any& rValue )
                throw( IllegalArgumentException )
{
    return sal_False;
}

//------------------------------------------------------------------------------
void FmXFormController::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
                         throw( Exception )
{
}

//------------------------------------------------------------------------------
void FmXFormController::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case FM_ATTR_FILTER_CRITERIA:
        {
            ::rtl::OUString aFilter;
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
                for (FmFilterRows::const_iterator i = m_aFilters.begin(); i != m_aFilters.end(); ++i)
                {
                    ::rtl::OUString aTest, aErrorMsg, aCriteria;
                    const FmFilterRow& rRow = *i;

                    if (!rRow.empty())
                    {
                        if (aFilter.getLength())
                            aFilter += ::rtl::OUString::createFromAscii(" OR ");

                        aFilter += ::rtl::OUString::createFromAscii("(");
                        for (FmFilterRow::const_iterator j = rRow.begin(); j != rRow.end(); j++)
                        {
                            // get the field of the controls map
                            Reference< XTextComponent > xText = (*j).first;
                            Reference< XPropertySet> xField = (*m_aFilterControls.find(xText)).second;
                            DBG_ASSERT(xField.is(), "NoField found");
                            if (j != rRow.begin())
                                aFilter += ::rtl::OUString::createFromAscii(" AND ");

                            aTest = (*j).second;
                            aErrorMsg = ::rtl::OUString();
                            ::rtl::Reference< ISQLParseNode > xParseNode = predicateTree(aErrorMsg, aTest, xFormatter, xField);
                            if (xParseNode.is())
                            {
                                aCriteria = ::rtl::OUString();
                                // don't use a parse context here, we need it unlocalized
                                xParseNode->parseNodeToStr(aCriteria, xMetaData,NULL);
                                aFilter += aCriteria;
                            }
                        }
                        aFilter += ::rtl::OUString::createFromAscii(")");
                    }
                }
            }
            rValue <<= ::rtl::OUString(aFilter);
        }   break;
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
        Sequence< Property >& /* [out] */ _rAggregateProps
        ) const
{
    _rProps.realloc(1);
    sal_Int32 nPos = 0;
    Property* pDesc = _rProps.getArray();
    DECL_PROP1(FILTER_CRITERIA, rtl::OUString, READONLY);
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
void FmXFormController::disposing(void)
{
    EventObject aEvt(static_cast< XFormController* >(this));

    // if we're still active, simulate a "deactivated" event
    if ( m_xActiveControl.is() )
    {
        NOTIFY_LISTENERS(m_aActivateListeners, XFormControllerListener, formDeactivated, aEvt);
    }

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

    m_aControllerFeatures.dispose();

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
            bDoUse = ( eDocType == eElectronicForm ) || ( eDocType == eDatabaseForm );
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
                startControlListening(xControl);
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

            if (bNewChanged && m_pView)
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
void FmXFormController::toggleAutoFields(sal_Bool bAutoFields)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    if (!m_pView)
    {
        DBG_ERROR("FmXFormController::toggleAutoFields : you can't call toggleAutoFields when no view exists !");
        return;
    }


    // Austauschen der Kontrols fuer das aktuelle Formular
    Sequence < Reference< XControl >  > aControls( m_aControls );
    const Reference< XControl > * pControls = m_aControls.getConstArray();
    SdrPageView* pCurPageView = m_pView->GetPageViewPvNum(0);

    const SdrPageViewWindow* pWindow = pCurPageView ? pCurPageView->FindWindow(*((OutputDevice*)m_pView->GetActualOutDev())) : 0L;
    if(!pWindow)
    {
        return;
    }

    //sal_uInt16 nPos = pCurPageView ? pCurPageView->GetWinList().Find((OutputDevice*)m_pView->GetActualOutDev()) : SDRPAGEVIEWWIN_NOTFOUND;
    //if (nPos == SDRPAGEVIEWWIN_NOTFOUND)
    //    return;

    // the control we have to activate after replacement
    Reference< XControl > xNewActiveControl;
    if (bAutoFields)
    {
        // as we don't want new controls to be attached to the scripting environment
        // we change attach flags
        m_bAttachEvents = sal_False;
        for (sal_Int32 i = aControls.getLength(); i > 0;)
        {
            const Reference< XControl > & xControl = pControls[--i];
            if (xControl.is())
            {
                Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
                if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
                {
                    // does the model use a bound field ?
                    Reference< XPropertySet >  xField;
                    xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

                    // is it a autofield?
                    if (xField.is() && ::comphelper::hasProperty(FM_PROP_AUTOINCREMENT, xField) &&
                        ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_AUTOINCREMENT)))
                    {
                        //const SdrPageViewWinRec& rWR = pCurPageView->GetWinList()[nPos];
                        const SdrUnoControlList& rControlList = pWindow->GetControlList();
                        sal_uInt16 nCtrlNum = rControlList.Find(xControl);

                        if (nCtrlNum != SDRUNOCONTROL_NOTFOUND)
                        {
                            // ok create an autocontrol
                            SdrUnoControlRec& rControlRec = (SdrUnoControlRec&)rControlList[nCtrlNum];
                            FmXAutoControl* pAutoControl = new FmXAutoControl();
                            Reference< XControl >  xNewControl(pAutoControl);

                            // setting the focus if the current control
                            // is the active one
                            if ((XControl*)m_xActiveControl.get() == (XControl*)xControl.get())
                            {
                                xNewActiveControl = xNewControl;
                                m_xActiveControl = m_xCurrentControl = NULL;
                            }
                            else if ((XControl*)m_xCurrentControl.get() == (XControl*)xControl.get())
                                m_xCurrentControl = xNewControl;

                            rControlRec.ReplaceControl(xNewControl);
                        }
                    }
                }
            }
        }
        m_bAttachEvents = sal_True;
    }
    else
    {
        m_bDetachEvents = sal_False;
        for (sal_Int32 i = aControls.getLength(); i > 0;)
        {
            const Reference< XControl > & xControl = pControls[--i];
            if (xControl.is())
            {
                Reference< XPropertySet >  xSet(xControl->getModel(), UNO_QUERY);
                if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
                {
                    // does the model use a bound field ?
                    Reference< XPropertySet >  xField;
                    xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;

                    // is it a autofield?
                    if (xField.is() && ::comphelper::hasProperty(FM_PROP_AUTOINCREMENT, xField) &&
                        ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_AUTOINCREMENT)))
                    {
                        // const SdrPageViewWinRec& rWR = pCurPageView->GetWinList()[nPos];
                        // const SdrPageViewWindow& rWindow = pCurPageView->GetPageViewWindows().Get(nPos);
                        const SdrUnoControlList& rControlList = pWindow->GetControlList();
                        sal_uInt16 nCtrlNum = rControlList.Find(xControl);
                        if (nCtrlNum != SDRUNOCONTROL_NOTFOUND)
                        {
                            // Zuruecksetzen des Controls auf das Defaultcontrol
                            SdrUnoControlRec& rControlRec = (SdrUnoControlRec&)rControlList[nCtrlNum];
                            ::rtl::OUString aServiceName = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_DEFAULTCONTROL));
                            Reference< XControl >  xNewControl(m_xORB->createInstance(aServiceName), UNO_QUERY);

                            // setting the focus if the current control
                            // is the active one
                            if ((XControl*)m_xActiveControl.get() == (XControl*)xControl.get())
                            {
                                xNewActiveControl = xNewControl;
                                m_xActiveControl = m_xCurrentControl = NULL;
                            }
                            else if ((XControl*)m_xCurrentControl.get() == (XControl*)xControl.get())
                                m_xCurrentControl = xNewControl;

                            rControlRec.ReplaceControl(xNewControl);
                        }
                    }
                }
            }
        }
        m_bDetachEvents = sal_True;
    }

    // set the focus async if possible
    // setting the focus to the replacing control
    Reference< XWindow >  xWindow(xNewActiveControl, UNO_QUERY);
    if (xWindow.is())
        xWindow->setFocus();
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
    NOTIFY_LISTENERS(m_aModifyListeners, XModifyListener, modified, aEvt);
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
#if (OSL_DEBUG_LEVEL > 1) || DBG_UTIL
            Reference< XBoundControl >  xLockingTest(m_xCurrentControl, UNO_QUERY);
            sal_Bool bControlIsLocked = xLockingTest.is() && xLockingTest->getLock();
            DBG_ASSERT(!bControlIsLocked, "FmXFormController::Gained: I'm modified and the current control is locked ? How this ?");
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
                if ( m_aControllerFeatures->canMoveRight() )
                    m_aControllerFeatures->moveRight();
            }
            else // backward
            {
                if ( m_aControllerFeatures->canMoveLeft() )
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
            m_pView->MakeVisible(aNewRect, *m_pWindow);
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
    NOTIFY_LISTENERS(m_aActivateListeners, XFormControllerListener, formActivated, aEvt);

    // verschicken ob modified
    if (m_bModified)
    {
        NOTIFY_LISTENERS(m_aModifyListeners, XModifyListener, modified, aEvt);
    }
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
        NOTIFY_LISTENERS(m_aActivateListeners, XFormControllerListener, formDeactivated, aEvt);
    }
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::mousePressed( const awt::MouseEvent& _rEvent ) throw (RuntimeException)
{
    // not interested in
}

//--------------------------------------------------------------------
void SAL_CALL FmXFormController::mouseReleased( const awt::MouseEvent& _rEvent ) throw (RuntimeException)
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

//------------------------------------------------------------------------------
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

        m_aControllerFeatures.dispose();

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
               && xModelProps->getPropertySetInfo()->hasPropertyByName( FM_PROP_DYNAMIC_CONTROL_BORDER )
               && xModelProps->getPropertySetInfo()->hasPropertyByName( FM_PROP_CONTROL_BORDER_COLOR_FOCUS )
               && xModelProps->getPropertySetInfo()->hasPropertyByName( FM_PROP_CONTROL_BORDER_COLOR_MOUSE )
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
                {
                    m_pControlBorderManager->setStatusColor( CONTROL_STATUS_FOCUSED, nColor );
                    m_pControlBorderManager->setStatusColor( CONTROL_STATUS_BOTH, nColor );
                }
                if ( xModelProps->getPropertyValue( FM_PROP_CONTROL_BORDER_COLOR_MOUSE ) >>= nColor )
                    m_pControlBorderManager->setStatusColor( CONTROL_STATUS_MOUSE_HOVER, nColor );
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

        // bei den Controls abmelden
        sal_Int32 nCount = m_aControls.getLength();
        const Reference< XControl > * pControls = m_aControls.getConstArray();

        // clear the filter map
        for (FmFilterControls::const_iterator iter = m_aFilterControls.begin();
             iter != m_aFilterControls.end(); ++iter)
            (*iter).first->removeTextListener(this);

        m_aFilterControls.clear();

        // einsammeln der Controls
        for (sal_Int32 i = 0; i < nCount; ++i, ++pControls)
        {
            // bei jedem Control als FocusListener anmelden
            Reference< XWindow >  xWindow(*pControls, UNO_QUERY);
            if (xWindow.is())
            {
                xWindow->removeFocusListener( this );
                xWindow->removeMouseListener( this );

                // abmelden beim Eventattacher
                removeFromEventAttacher(*pControls);
            }

            Reference< XDispatchProviderInterception >  xInterception(*pControls, UNO_QUERY);
            if (xInterception.is())
                deleteInterceptor(xInterception);

            if (pControls->is())
            {
                Reference< XReset >  xControlReset((*pControls)->getModel(), UNO_QUERY);
                if (xControlReset.is())
                    xControlReset->removeResetListener((XResetListener*)this);
            }
        }

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
        Sequence< Reference< XControl > > xCtrls = xContainer->getControls();

        sal_Int32 nCount = aModels.getLength();
        m_aControls = Sequence< Reference< XControl > >( nCount );
        Reference< XControl > * pControls = m_aControls.getArray();

        // einsammeln der Controls
        sal_Int32 i, j;
        for (i = 0, j = 0; i < nCount; ++i )
        {
            Reference< XControlModel >  xCtrlModel = pModels[i];
            // Zum Model passendes Control suchen
            Reference< XControl >  xCtrl = findControl( xCtrls, xCtrlModel,sal_False );
            if (xCtrl.is())
            {
                pControls[j++] = xCtrl;

                // bei jedem Control als FocusListener anmelden
                Reference< XWindow >  xWindow(xCtrl, UNO_QUERY);
                if (xWindow.is())
                {
                    xWindow->addFocusListener( this );
                    xWindow->addMouseListener( this );
                    // anmelden beim Eventattacher
                    addToEventAttacher(xCtrl);
                }

                // add a dispatch interceptor to the control (if supported)
                Reference< XDispatchProviderInterception >  xInterception(xCtrl, UNO_QUERY);
                if (xInterception.is())
                    createInterceptor(xInterception);

                // we want to know about the reset of the the model of our controls
                // (for correctly resetting m_bModified)
                Reference< XReset >  xControlReset(xCtrl->getModel(), UNO_QUERY);
                if (xControlReset.is())
                    xControlReset->addResetListener((XResetListener*)this);
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
        Reference< XControlModel >  xCtrlModel;
        Reference< XControl >  xCtrl;

        // Umsortieren der Controls entsprechend der TabReihenfolge
        sal_Int32 j = 0;
        for (sal_Int32 i = 0; i < nModels; i++)
        {
            xCtrlModel = pModels[i];

            // Zum Model passendes Control suchen
            xCtrl = findControl(m_aControls, xCtrlModel);
            if (xCtrl.is())
                pControls[j++] = xCtrl;
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
    sal_Int32 nLength = m_aControls.getLength();
    const Reference< XControl > * pControls = m_aControls.getConstArray();
    for (sal_Int32 i = 0; i < nLength; i++)
        setControlLock(pControls[i]);
}

//------------------------------------------------------------------------------
void FmXFormController::startControlListening(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    // jetzt anmelden bei gebundenen feldern
    sal_Bool bModifyListening = sal_False;
    Reference< XBoundComponent >  xBound(xControl, UNO_QUERY);
    if (xBound.is()) // gebundene Controls,
        bModifyListening = sal_True;
    else
    {
        xBound = Reference< XBoundComponent > (xControl->getModel(), UNO_QUERY);

        // gibt es eine Datenquelle
        Reference< XPropertySet >  xSet(xBound, UNO_QUERY);
        if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
        {
            Reference< XPropertySet >  xField;
            xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;
            bModifyListening = (xField.is());
            if ( !bModifyListening )
                xSet->addPropertyChangeListener( FM_PROP_BOUNDFIELD, this );
        }
    }

    // kuenstliches while
    while (bModifyListening)
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
void FmXFormController::stopControlListening(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
//  Reference< XDatabaseCursor >  xCursor(getModel(), UNO_QUERY);
    sal_Bool bModifyListening = sal_False;

    Reference< XBoundComponent >  xBound(xControl, UNO_QUERY);
    if (xBound.is()) // gebundene Controls,
        bModifyListening = sal_True;
    else
    {
        xBound = Reference< XBoundComponent > (xControl->getModel(), UNO_QUERY);
        // gibt es eine Datenquelle
        Reference< XPropertySet >  xSet(xBound, UNO_QUERY);
        if (xSet.is() && ::comphelper::hasProperty(FM_PROP_BOUNDFIELD, xSet))
        {
            Reference< XPropertySet >  xField;
            xSet->getPropertyValue(FM_PROP_BOUNDFIELD) >>= xField;
            bModifyListening = (xField.is());
        }
    }

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
    const Reference< XControl > * pControls = m_aControls.getConstArray();
    for (sal_Int32 i = 0; i < m_aControls.getLength(); i++ )
        startControlListening(pControls[i]);
}

//------------------------------------------------------------------------------
void FmXFormController::stopListening()
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_bModified  = sal_False;

    // jetzt anmelden bei gebundenen feldern
    const Reference< XControl > * pControls = m_aControls.getConstArray();
    for (sal_Int32 i = 0; i < m_aControls.getLength(); i++ )
        stopControlListening(pControls[i]);
}


//------------------------------------------------------------------------------
Reference< XControl >  FmXFormController::findControl(Sequence< Reference< XControl > >& rCtrls, const Reference< XControlModel > & xCtrlModel ,sal_Bool _bRemove,sal_Bool _bOverWrite) const
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    DBG_ASSERT( xCtrlModel.is(), "findControl - welches ?!" );

    Reference< XControl > * pCtrls = rCtrls.getArray();
    Reference< XControlModel >  xModel;
    for ( sal_Int32 i = 0, nCount = rCtrls.getLength(); i < nCount; ++i )
    {
        // #66449# Speicherueberschreiber durch folgende Zeile
        // Reference< XControlModel >  xModel(pCtrls[i].is() ? pCtrls[i]->getModel() : Reference< XControlModel > ());
        if (pCtrls[i].is())
        {
            xModel = pCtrls[i]->getModel();
            if ((XControlModel*)xModel.get() == (XControlModel*)xCtrlModel.get())
            {
                Reference< XControl >  xCtrl( pCtrls[i] );
                if ( _bRemove )
                    ::comphelper::removeElementAt(rCtrls, i);
                else if ( _bOverWrite )
                    pCtrls[i] = Reference< XControl >();
                return xCtrl;
            }
        }
    }
    return Reference< XControl > ();
}

//------------------------------------------------------------------------------
void FmXFormController::insertControl(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    m_bControlsSorted = sal_False;
    m_aControls.realloc(m_aControls.getLength() + 1);
    m_aControls.getArray()[m_aControls.getLength() - 1] = xControl;

    Reference< XWindow >  xWindow(xControl, UNO_QUERY);
    if (xWindow.is())
    {
        xWindow->addFocusListener( this );
        xWindow->addMouseListener( this );

        // register at the Eventattacher but not in filter mode
        if (m_bAttachEvents)
            addToEventAttacher(xControl);
    }

    Reference< XDispatchProviderInterception >  xInterception(xControl, UNO_QUERY);
    if (xInterception.is())
        createInterceptor(xInterception);

    if (xControl.is())
    {
        // we want to know about the reset of the the model of our controls
        // (for correctly resetting m_bModified)
        Reference< XReset >  xControlReset(xControl->getModel(), UNO_QUERY);
        if (xControlReset.is())
            xControlReset->addResetListener((XResetListener*)this);
    }

    if (m_bDBConnection && !m_bFiltering)
        setControlLock(xControl);

    if (isListeningForChanges() && m_bAttachEvents)
        startControlListening(xControl);
}

//------------------------------------------------------------------------------
void FmXFormController::removeControl(const Reference< XControl > & xControl)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    const Reference< XControl > * pCtrls = m_aControls.getArray();
    sal_Int32 nCount = m_aControls.getLength();
    for ( sal_uInt32 i = 0; i < (sal_uInt32)nCount; i++ )
    {
        if ((XControl*)xControl.get() == (XControl*)pCtrls[i].get())
        {
            ::comphelper::removeElementAt(m_aControls, i);
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

    Reference< XWindow >  xWindow(xControl, UNO_QUERY);
    if (xWindow.is())
    {
        xWindow->removeFocusListener( this );
        xWindow->removeMouseListener( this );
        if (m_bDetachEvents)
            removeFromEventAttacher(xControl);
    }

    Reference< XDispatchProviderInterception >  xInterception(xControl, UNO_QUERY);
    if (xInterception.is())
        deleteInterceptor(xInterception);

    if (xControl.is())
    {
        Reference< XReset >  xControlReset(xControl->getModel(), UNO_QUERY);
        if (xControlReset.is())
            xControlReset->removeResetListener((XResetListener*)this);
    }

    if (isListeningForChanges() && m_bDetachEvents)
        stopControlListening(xControl);
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
            sal_Int32 aVal2;
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
    if (m_bCurrentRecordNew && m_pView)
        toggleAutoFields(sal_True);

    return 1L;
}

//------------------------------------------------------------------------------
void FmXFormController::unloaded(const EventObject& rEvent) throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    updateAllDispatchers();
}

//------------------------------------------------------------------------------
void FmXFormController::reloading(const EventObject& aEvent) throw( RuntimeException )
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
void FmXFormController::unloading(const EventObject& aEvent) throw( RuntimeException )
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
    if (m_bCurrentRecordNew && m_pView)
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
    const Reference< XControl > * pControls = m_aControls.getConstArray();
    for (sal_Int32 i = 0; i < m_aControls.getLength(); ++i,++pControls )
    {
        Reference<XPropertySet> xProp(*pControls,UNO_QUERY);
        if ( xProp.is() )
            xProp->removePropertyChangeListener(FM_PROP_BOUNDFIELD, this);
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

        if ( _rxForm->getPropertySetInfo().is() && _rxForm->getPropertySetInfo()->hasPropertyByName( FM_PROP_DYNAMIC_CONTROL_BORDER ) )
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

        if ( _rxForm->getPropertySetInfo().is() && _rxForm->getPropertySetInfo()->hasPropertyByName( FM_PROP_DYNAMIC_CONTROL_BORDER ) )
            _rxForm->removePropertyChangeListener( FM_PROP_DYNAMIC_CONTROL_BORDER, this );
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "FmXFormController::stopFormListening: caught an exception!" );
    }
}

// com::sun::star::sdbc::XRowSetListener
//------------------------------------------------------------------------------
void FmXFormController::cursorMoved(const EventObject& event) throw( RuntimeException )
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
void FmXFormController::rowChanged(const EventObject& event) throw( RuntimeException )
{
    // not interested in ...
}
//------------------------------------------------------------------------------
void FmXFormController::rowSetChanged(const EventObject& event) throw( RuntimeException )
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
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::osl::MutexGuard aGuard( m_aMutex );

    // Remove the control
    Reference< XControl >  xReplControl;
    evt.ReplacedElement >>= xReplControl;
    Reference< XFormComponent >  xModel(xReplControl->getModel(), UNO_QUERY);
    if (xModel.is() && m_xModelAsIndex == xModel->getParent())
        removeControl(xReplControl);
    else if (m_aFilterControls.size())
    {
        Reference< XTextComponent >  xText(xReplControl, UNO_QUERY);
        FmFilterControls::iterator iter = m_aFilterControls.find(xText);
        if (iter != m_aFilterControls.end())
            m_aFilterControls.erase(iter);
    }

    // Add the new one
    elementInserted(evt);
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
    else if (m_aFilterControls.size())
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
    const Reference< XControl > * pCtrls = m_aControls.getConstArray();

    sal_uInt32 nCtrls = m_aControls.getLength();
    for ( sal_uInt32 n = 0; n < nCtrls && xPeer.is(); n++ )
    {
        if (pCtrls[n].is())
        {
            Reference< XVclWindowPeer >  xCtrlPeer(pCtrls[n]->getPeer(), UNO_QUERY);
            if ((XWindowPeer*)xCtrlPeer.get() == (XWindowPeer*)xPeer.get() || xCtrlPeer->isChild(xPeer))
                return pCtrls[n];
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
void FmXFormController::setFilter(vector<FmFieldInfo>& rFieldInfos)
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
                ::rtl::OUString aFilter     = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_FILTER_CRITERIA));
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

        for (vector<FmFieldInfo>::iterator iter = rFieldInfos.begin();
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
                for (vector<FmFieldInfo>::iterator iter = rFieldInfos.begin();
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
                                                                    ,xMetaData
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
    for (vector<FmFieldInfo>::iterator iter = rFieldInfos.begin();
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
    if (!m_pView)
    {
        DBG_ERROR("FmXFormController::startFiltering : you can't filter if you created me as service !");
        return;
    }

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
    Sequence < Reference< XControl >  > aControls(m_aControls);
    const Reference< XControl > * pControls = m_aControls.getConstArray();
    SdrPageView* pCurPageView = m_pView->GetPageViewPvNum(0);

    // the control we have to activate after replacement
    Reference< XControl >  xNewActiveControl;
    Reference< XDatabaseMetaData >  xMetaData(xConnection->getMetaData());
    Reference< XNumberFormatsSupplier >  xFormatSupplier = aStaticTools.getNumberFormats(xConnection, sal_True);
    Reference< XNumberFormatter >  xFormatter(m_xORB
                        ->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
    xFormatter->attachNumberFormatsSupplier(xFormatSupplier);

    // structure for storing the field info
    vector<FmFieldInfo> aFieldInfos;

    // sal_uInt16 nPos = pCurPageView ? pCurPageView->GetWinList().Find((OutputDevice*)m_pView->GetActualOutDev()) : SDRPAGEVIEWWIN_NOTFOUND;
    const SdrPageViewWindow* pWindow = pCurPageView ? pCurPageView->FindWindow(*((OutputDevice*)m_pView->GetActualOutDev())) : 0L;

    if(pWindow)
    {
        for (sal_Int32 i = aControls.getLength(); i > 0;)
        {
            const Reference< XControl > & xControl = pControls[--i];
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
                        Reference< XControl >  xControl;
                        while (xEnumeration->hasMoreElements())
                        {
                            xEnumeration->nextElement() >>= xControl;
                            if (xControl.is())
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

                    if (xField.is() && ::comphelper::hasProperty(FM_PROP_SEARCHABLE, xField) &&
                        ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_SEARCHABLE)))
                    {
                        // const SdrPageViewWinRec& rWR = pCurPageView->GetWinList()[nPos];
                        // const SdrPageViewWindow& rWindow = pCurPageView->GetPageViewWindows().Get(nPos);
                        const SdrUnoControlList& rControlList = pWindow->GetControlList();
                        sal_uInt16 nCtrlNum = rControlList.Find(xControl);
                        if (nCtrlNum != SDRUNOCONTROL_NOTFOUND)
                        {
                            // Setzen des FilterControls
                            SdrUnoControlRec& rControlRec = (SdrUnoControlRec&)rControlList[nCtrlNum];

                            // create a filter control
                            Sequence< Any > aCreationArgs( 3 );
                            aCreationArgs[ 0 ] <<= NamedValue( ::rtl::OUString::createFromAscii( "MessageParent" ), makeAny( VCLUnoHelper::GetInterface( m_pWindow ) ) );
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

                            Reference< XTextComponent >  xText( xFilterControl, UNO_QUERY );

                            // merken in der Map
                            aFieldInfos.push_back(FmFieldInfo(xField, xText));
                            xText->addTextListener(this);

                            // setting the focus if the current control
                            // is the active one
                            if (m_xActiveControl.get() == xControl.get())
                            {
                                xNewActiveControl = xFilterControl;
                                m_xActiveControl = m_xCurrentControl = NULL;
                            }
                            else if (m_xCurrentControl.get() == xControl.get())
                                m_xCurrentControl = xFilterControl;

                            rControlRec.ReplaceControl( xFilterControl );
                        }
                    }
                }
                else
                {
                    // abmelden vom EventManager
                }
            }
        }
    }

    // we have all filter controls now, so the next step is to read the filters from the form
    // resolve all aliases and set the current filter to the according structure
    setFilter(aFieldInfos);

    // setting the focus to the replacing control
    Reference< XWindow >  xWindow(xNewActiveControl, UNO_QUERY);
    if (xWindow.is())
        xWindow->setFocus();

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

    if (!m_pView)
    {
        DBG_ERROR("FmXFormController::startFiltering : you can't filter if you created me as service !");
        return;
    }

    m_bFiltering = sal_False;
    m_bDetachEvents = sal_False;

    ::comphelper::disposeComponent(m_xComposer);

    // Austauschen der Kontrols fuer das aktuelle Formular
    Sequence < Reference< ::com::sun::star::awt::XControl >  > aControls(m_aControls);
    const Reference< ::com::sun::star::awt::XControl > * pControls = m_aControls.getConstArray();
    SdrPageView* pCurPageView = m_pView->GetPageViewPvNum(0);

    // sal_uInt16 nPos = pCurPageView ? pCurPageView->GetWinList().Find((OutputDevice*)m_pView->GetActualOutDev()) : SDRPAGEVIEWWIN_NOTFOUND;
    const SdrPageViewWindow* pWindow = pCurPageView ? pCurPageView->FindWindow(*((OutputDevice*)m_pView->GetActualOutDev())) : 0L;

    // the control we have to activate after replacement
    Reference< ::com::sun::star::awt::XControl >  xNewActiveControl;

    // clear the filter control map
    for (FmFilterControls::const_iterator iter = m_aFilterControls.begin();
         iter != m_aFilterControls.end(); iter++)
         (*iter).first->removeTextListener(this);

    m_aFilterControls.clear();

    if (pWindow)
    {
        for (sal_Int32 i = aControls.getLength(); i > 0;)
        {
            const Reference< XControl > & xControl = pControls[--i];
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
                    if (xField.is() && ::comphelper::hasProperty(FM_PROP_SEARCHABLE, xField) &&
                        ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_SEARCHABLE)))
                    {
                        // const SdrPageViewWinRec& rWR = pCurPageView->GetWinList()[nPos];
                        // const SdrPageViewWindow& rWindow = pCurPageView->GetPageViewWindows().Get(nPos);
                        const SdrUnoControlList& rControlList = pWindow->GetControlList();
                        sal_uInt16 nCtrlNum = rControlList.Find(xControl);
                        if (nCtrlNum != SDRUNOCONTROL_NOTFOUND)
                        {
                            // Zuruecksetzen des Controls auf das Defaultcontrol
                            SdrUnoControlRec& rControlRec = (SdrUnoControlRec&)rControlList[nCtrlNum];
                            ::rtl::OUString aServiceName = ::comphelper::getString(xSet->getPropertyValue(FM_PROP_DEFAULTCONTROL));
                            Reference< XControl >  xNewControl(m_xORB->createInstance(aServiceName), UNO_QUERY);

                            // setting the focus if the current control
                            // is the active one
                            if ((XControl*)m_xActiveControl.get() == (XControl*)xControl.get())
                            {
                                xNewActiveControl = xNewControl;
                                m_xActiveControl = m_xCurrentControl = NULL;
                            }
                            else if ((XControl*)m_xCurrentControl.get() == (XControl*)xControl.get())
                                m_xCurrentControl = xNewControl;

                            rControlRec.ReplaceControl(xNewControl);
                        }
                    }
                }
            }
        }
    }

    // setting the focus to the replacing control
    Reference< XWindow >  xWindow(xNewActiveControl, UNO_QUERY);
    if (xWindow.is())
        xWindow->setFocus();

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
    if (!pParent)
    {
        Reference< XControlContainer >  xContainer = getContainer();
        Reference< XControl >  xCtrl(xContainer, UNO_QUERY);
        if (xCtrl.is())
        {
            Reference< XWindowPeer >  xPeer(xCtrl->getPeer(), UNO_QUERY);
            if (xPeer.is())
                pParent = VCLUnoHelper::GetWindow(xPeer);
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
}

// XRowSetApproveListener
//------------------------------------------------------------------------------
sal_Bool SAL_CALL FmXFormController::approveRowChange(const RowChangeEvent& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    ::cppu::OInterfaceIteratorHelper aIter(m_aRowSetApproveListeners);
    sal_Bool bValid = sal_True;
    if (aIter.hasMoreElements())
    {
        RowChangeEvent aEvt(aEvent);
        aEvt.Source = *this;
        bValid = ((XRowSetApproveListener*)aIter.next())->approveRowChange(aEvt);
    }

    if (bValid && (
        aEvent.Action == RowChangeAction::INSERT ||
        aEvent.Action == RowChangeAction::UPDATE))
    {
        if (m_pView)
        {   // we're working for a FormView
            if (!(m_pWindow || (m_pView->GetActualOutDev() == (const OutputDevice*)m_pWindow ||
                !m_pView->GetActualOutDev() && m_pWindow->IsActive())))
                // we're not active
                return sal_True;
        }

        // if some of the control modes are bound to validators, check them
        ::rtl::OUString sInvalidityExplanation;
        Reference< XControlModel > xInvalidModel;
        if ( !checkFormComponentValidity( sInvalidityExplanation, xInvalidModel ) )
        {
            displayErrorSetFocus( sInvalidityExplanation, locateControl( xInvalidModel ), getDialogParentWindow() );
            return false;
        }

        // check Values on NULL and required flag
        Reference< XColumnsSupplier >  xSupplyCols(aEvent.Source, UNO_QUERY);
        Reference< XEnumerationAccess >  xEnumAccess;
        if (xSupplyCols.is())
            xEnumAccess = Reference< XEnumerationAccess > (xSupplyCols->getColumns(),UNO_QUERY);

        if (!xSupplyCols.is() || !xEnumAccess.is())
        {
            DBG_ERROR("FmXFormController::approveRowChange : source is no columns supplier or columns aren't enumerable !");
            // can't do anything here ...
            return sal_True;
        }

        Reference< XEnumeration >  xEnumeration = xEnumAccess->createEnumeration();
        Reference< XPropertySet >  xFieldSet;
        while (xEnumeration->hasMoreElements())
        {
            xEnumeration->nextElement() >>= xFieldSet;
            Reference< XColumn >  xColumn(xFieldSet, UNO_QUERY);
            if (!xFieldSet.is() || !xColumn.is())
            {
                DBG_ERROR("FmXFormController::approveRowChange : invalid field !");
                continue;
            }

            sal_Bool bRequired = ::comphelper::getINT32(xFieldSet->getPropertyValue(FM_PROP_ISNULLABLE)) == ColumnValue::NO_NULLS;
            if (!bRequired)
                continue;

            sal_Bool bAutoIncrement = ::comphelper::getBOOL(xFieldSet->getPropertyValue(FM_PROP_AUTOINCREMENT));
            if (bAutoIncrement)
                continue;

            ::rtl::OUString aFieldName(::comphelper::getString(xFieldSet->getPropertyValue(FM_PROP_NAME)));

            if (!xColumn->getString().getLength() && xColumn->wasNull())
            {
                Sequence< Reference< XControl > > aControls(getControls());
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
                sMessage.SearchAndReplace('#', aFieldName.getStr());
                displayErrorSetFocus( sMessage, ( i < nLength ) ? pControls[i] : Reference< XControl >(), getDialogParentWindow() );

                return sal_False;
            }
        }
    }
    return bValid;
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
    if (m_pView)
    {   // we're working for a FormView
        if (!(m_pWindow || (m_pView->GetActualOutDev() == (const OutputDevice*)m_pWindow ||
            !m_pView->GetActualOutDev() && m_pWindow->IsActive())))
            // we're not active
            return;
    }

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
    if (m_pView)
    {   // we're working for a FormView
        if (!(m_pWindow || (m_pView->GetActualOutDev() == (const OutputDevice*)m_pWindow ||
            !m_pView->GetActualOutDev() && m_pWindow->IsActive())))
            // we're not active
            return sal_True;
    }

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

            // create the handler, let it handle the request
            Reference< XInteractionHandler > xHandler(m_xORB->createInstance(SRV_SDB_INTERACTION_HANDLER), UNO_QUERY);
            if (xHandler.is())
            {
                ::vos::OGuard aGuard(Application::GetSolarMutex());
                xHandler->handle(xParamRequest);
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
    if (m_pView)
    {   // we're working for a FormView
        if (!(m_pWindow || (m_pView->GetActualOutDev() == (const OutputDevice*)m_pWindow ||
            !m_pView->GetActualOutDev() && m_pWindow->IsActive())))
            // we're not active
            return sal_True;
    }

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
FmXFormController::interceptedQueryDispatch(sal_uInt16 _nId, const URL& aURL,
                                            const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags)
                                            throw( RuntimeException )
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
    Reference< XDispatch >  xReturn;
    // dispatches handled by ourself
    if (aURL.Complete == FMURL_CONFIRM_DELETION)
        xReturn = static_cast<XDispatch*>(this);

    // dispatches of FormSlot-URLs we have to translate
    if ( !xReturn.is() && m_aControllerFeatures.isAssigned() )
            {
        // find the slot id which corresponds to the URL
        sal_Int32 nFeatureId = ::svx::FeatureSlotTranslation::getControllerFeatureIdForURL( aURL.Main );
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
    OSL_ENSURE(sal_False, "FmXFormController::dispatch: never to be called!");
    // we use the dispatch mechanism only for exposing the XConfirmDeleteListener interface
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
void SAL_CALL FmXFormController::removeStatusListener( const Reference< XStatusListener >& _rxListener, const URL& _rURL ) throw (RuntimeException)
{
    OSL_ENSURE(_rURL.Complete == FMURL_CONFIRM_DELETION, "FmXFormController::removeStatusListener: invalid (unsupported) URL!");
    // we never really added the listener, so we don't need to remove it
}

//------------------------------------------------------------------------------
Reference< XDispatchProviderInterceptor >  FmXFormController::createInterceptor(const Reference< XDispatchProviderInterception > & _xInterception)
{
    OSL_ENSURE(!FmXFormController_BASE1::rBHelper.bDisposed,"FmXFormController: Object already disposed!");
#if DBG_UTIL
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





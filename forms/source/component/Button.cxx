/*************************************************************************
 *
 *  $RCSfile: Button.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:33:05 $
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

#ifndef _FRM_BUTTON_HXX_
#include "Button.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//==================================================================
//= OButtonModel
//==================================================================
DBG_NAME(OButtonModel)
//------------------------------------------------------------------
InterfaceRef SAL_CALL OButtonModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OButtonModel(_rxFactory));
}

//------------------------------------------------------------------
OButtonModel::OButtonModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OImageModel( _rxFactory, VCL_CONTROLMODEL_COMMANDBUTTON, FRM_SUN_CONTROL_COMMANDBUTTON )
                                    // use the old control name for compatibility reasons
{
    DBG_CTOR( OButtonModel, NULL );
    m_nClassId = FormComponentType::COMMANDBUTTON;
}

//------------------------------------------------------------------
OButtonModel::OButtonModel( const OButtonModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OImageModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OButtonModel, NULL );
    implInitializeImageURL();
}

//------------------------------------------------------------------------------
OButtonModel::~OButtonModel()
{
    DBG_DTOR(OButtonModel, NULL);
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OButtonModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OButtonModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 5, OImageModel )
        DECL_PROP1(BUTTONTYPE,      FormButtonType,             BOUND);
        DECL_PROP1(DISPATCHURLINTERNAL, sal_Bool,               BOUND);
        DECL_PROP1(TARGET_URL,      ::rtl::OUString,            BOUND);
        DECL_PROP1(TARGET_FRAME,    ::rtl::OUString,            BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,                  BOUND);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OButtonModel::getInfoHelper()
{
    return *const_cast<OButtonModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OButtonModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OButtonModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_COMMANDBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
::rtl::OUString OButtonModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_COMMANDBUTTON; // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OImageModel::write(_rxOutStream);

    _rxOutStream->writeShort(0x0003);   // Version

    {
        OStreamSection aSection( _rxOutStream.get() );
            // this will allow readers to skip unknown bytes in their dtor

        _rxOutStream->writeShort( (sal_uInt16)m_eButtonType );

        ::rtl::OUString sTmp = INetURLObject::decode(INetURLObject::AbsToRel( m_sTargetURL ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
        _rxOutStream << sTmp;
        _rxOutStream << m_sTargetFrame;
        writeHelpTextCompatibly(_rxOutStream);
        _rxOutStream << isDispatchUrlInternal();
    }
}

//------------------------------------------------------------------------------
void OButtonModel::read(const Reference<XObjectInputStream>& _rxInStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OImageModel::read(_rxInStream);

    sal_uInt16 nVersion = _rxInStream->readShort();     // Version
    switch (nVersion)
    {
        case 0x0001:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = INetURLObject::RelToAbs( sTmp );
            _rxInStream >> m_sTargetFrame;
        }
        break;

        case 0x0002:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = INetURLObject::RelToAbs( sTmp );
            _rxInStream >> m_sTargetFrame;
            readHelpTextCompatibly(_rxInStream);
        }
        break;

        case 0x0003:
        {
            OStreamSection aSection( _rxInStream.get() );
                // this will skip any unknown bytes in it's dtor

            // button type
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            // URL
            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = INetURLObject::RelToAbs( sTmp );

            // target frame
            _rxInStream >> m_sTargetFrame;

            // help text
            readHelpTextCompatibly(_rxInStream);

            // DispatchInternal
            sal_Bool bDispath;
            _rxInStream >> bDispath;
            setDispatchUrlInternal(bDispath);
        }
        break;

        default:
            DBG_ERROR("OButtonModel::read : unknown version !");
            m_eButtonType = FormButtonType_PUSH;
            m_sTargetURL = ::rtl::OUString();
            m_sTargetFrame = ::rtl::OUString();
            break;
    }
}

//==================================================================
// OButtonControl
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OButtonControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OButtonControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OButtonControl::_getTypes()
{
    return ::comphelper::concatSequences(
        OButtonControl_BASE::getTypes(),
        OImageControl::_getTypes(),
        OFormNavigationHelper::getTypes()
    );
}

//------------------------------------------------------------------------------
StringSequence  OButtonControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_COMMANDBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
OButtonControl::OButtonControl(const Reference<XMultiServiceFactory>& _rxFactory)
                 :OImageControl(_rxFactory, VCL_CONTROL_COMMANDBUTTON)
                 ,OFormNavigationHelper( _rxFactory )
                 ,m_nClickEvent( 0 )
                 ,m_nTargetUrlFeatureId( -1 )
                 ,m_bEnabledByPropertyValue( sal_False )
{
    increment(m_refCount);
    {
        // als ActionListener anmelden
        Reference<XButton>  xButton;
        query_aggregation( m_xAggregate, xButton);
        if (xButton.is())
            xButton->addActionListener(this);
    }
    // Refcount bei 1 fuer Listener
    sal_Int32 n = decrement(m_refCount);
}

//------------------------------------------------------------------------------
OButtonControl::~OButtonControl()
{
    if (m_nClickEvent)
        Application::RemoveUserEvent(m_nClickEvent);
}

// UNO Anbindung
//------------------------------------------------------------------------------
Any SAL_CALL OButtonControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    // if asked for the XTypeProvider, don't let OButtonControl_BASE do this
    Any aReturn;
    if ( !_rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >( NULL ) ) ) )
        aReturn = OButtonControl_BASE::queryInterface( _rType );

    if ( !aReturn.hasValue() )
        aReturn = OImageControl::queryAggregation( _rType );

    if ( !aReturn.hasValue() )
        aReturn = OFormNavigationHelper::queryInterface( _rType );

    return aReturn;
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::disposing()
{
    startOrStopModelPropertyListening( false );

    OImageControl::disposing();
    OFormNavigationHelper::dispose();
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::disposing( const EventObject& _rSource ) throw( RuntimeException )
{
    OControl::disposing( _rSource );
    OFormNavigationHelper::disposing( _rSource );
}

// ActionListener
//------------------------------------------------------------------------------
void OButtonControl::actionPerformed(const ActionEvent& rEvent) throw ( ::com::sun::star::uno::RuntimeException)
{
    // Asynchron fuer starutil::URL-Button
    sal_uInt32 n = Application::PostUserEvent( LINK(this, OButtonControl,OnClick) );
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_nClickEvent = n;
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( OButtonControl, OnClick, void*, EMPTYARG )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    m_nClickEvent = 0;

    if (m_aApproveActionListeners.getLength())
    {
        // Wenn es ApproveAction-Lisener gibt, muss ein eigener Thread
        // aufgemacht werden.
        if( !m_pThread )
        {
            m_pThread = new OImageControlThread_Impl( this );
            m_pThread->acquire();
            m_pThread->create();
        }
        m_pThread->addEvent();
    }
    else
    {
        // Sonst nicht. Dann darf man aber auf keinen Fal die Listener
        // benachrichtigen, auch dann nicht, wenn er spaeter hinzukommt.
        aGuard.clear();

        // recognize the button type
        Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
        if (!xSet.is())
            return 0L;

        if (FormButtonType_PUSH == *(FormButtonType*)xSet->getPropertyValue(PROPERTY_BUTTONTYPE).getValue())
        {
            // notify the action listeners for a push button
            ::cppu::OInterfaceIteratorHelper aIter(m_aActionListeners);
            ActionEvent aEvt(static_cast<XWeak*>(this), m_aActionCommand);
            while(aIter.hasMoreElements() )
            {
                // catch exceptions
                // and catch them on a per-listener basis - if one listener fails, the others still need
                // to get notified
                // 97676 - 21.02.2002 - fs@openoffice.org
                try
                {
                    static_cast< XActionListener* >( aIter.next() )->actionPerformed(aEvt);
                }
#ifdef DBG_UTIL
                catch( const RuntimeException& )
                {
                    // silent this
                }
#endif
                catch( const Exception& )
                {
                    DBG_ERROR( "OButtonControl::OnClick: caught a exception other than RuntimeException!" );
                }
            }
        }
        else
            actionPerformed_Impl( sal_False, ::com::sun::star::awt::MouseEvent() );
    }
    return 0L;
}

//------------------------------------------------------------------------------
void OButtonControl::actionPerformed_Impl( sal_Bool _bNotifyListener, const ::com::sun::star::awt::MouseEvent& _rEvt )
{
    {
        sal_Int32 nFeatureId = -1;
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            nFeatureId = m_nTargetUrlFeatureId;
        }

        if ( nFeatureId != -1 )
        {
            if ( !approveAction() )
                return;

            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            dispatch( nFeatureId );
            return;
        }
    }

    OImageControl::actionPerformed_Impl( _bNotifyListener, _rEvt );
}

// XButton
//------------------------------------------------------------------------------
void OButtonControl::setLabel(const ::rtl::OUString& Label) throw( RuntimeException )
{
    Reference<XButton>  xButton;
    query_aggregation( m_xAggregate, xButton );
    if (xButton.is())
        xButton->setLabel(Label);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::setActionCommand(const ::rtl::OUString& _rCommand) throw( RuntimeException )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aActionCommand = _rCommand;
    }

    Reference<XButton>  xButton;
    query_aggregation( m_xAggregate, xButton);
    if (xButton.is())
        xButton->setActionCommand(_rCommand);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::addActionListener(const Reference<XActionListener>& _rxListener) throw( RuntimeException )
{
    m_aActionListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::removeActionListener(const Reference<XActionListener>& _rxListener) throw( RuntimeException )
{
    m_aActionListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
class DoPropertyListening
{
private:
    Reference< XPropertySet >               m_xProps;
    Reference< XPropertyChangeListener >    m_xListener;
    bool                                    m_bStartListening;

public:
    DoPropertyListening(
        const Reference< XInterface >& _rxComponent,
        const Reference< XPropertyChangeListener >& _rxListener,
        bool _bStart
    );

    void    handleListening( const ::rtl::OUString& _rPropertyName );
};

//..............................................................................
DoPropertyListening::DoPropertyListening(
        const Reference< XInterface >& _rxComponent, const Reference< XPropertyChangeListener >& _rxListener,
        bool _bStart )
    :m_xProps( _rxComponent, UNO_QUERY )
    ,m_xListener( _rxListener )
    ,m_bStartListening( _bStart )
{
    DBG_ASSERT( m_xProps.is() || !_rxComponent.is(), "DoPropertyListening::DoPropertyListening: valid component, but no property set!" );
    DBG_ASSERT( m_xListener.is(), "DoPropertyListening::DoPropertyListening: invalid listener!" );
}

//..............................................................................
void DoPropertyListening::handleListening( const ::rtl::OUString& _rPropertyName )
{
    if ( m_xProps.is() )
        if ( m_bStartListening )
            m_xProps->addPropertyChangeListener( _rPropertyName, m_xListener );
        else
            m_xProps->removePropertyChangeListener( _rPropertyName, m_xListener );
}

//------------------------------------------------------------------------------
void OButtonControl::startOrStopModelPropertyListening( bool _bStart )
{
    DoPropertyListening aListeningHandler( getModel(), this, _bStart );
    aListeningHandler.handleListening( PROPERTY_TARGET_URL );
    aListeningHandler.handleListening( PROPERTY_BUTTONTYPE );
    aListeningHandler.handleListening( PROPERTY_ENABLED );
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OButtonControl::setModel( const Reference< XControlModel >& _rxModel ) throw ( RuntimeException )
{
    startOrStopModelPropertyListening( false );
    sal_Bool bResult = OImageControl::setModel( _rxModel );
    startOrStopModelPropertyListening( true );

    m_bEnabledByPropertyValue = sal_True;
    Reference< XPropertySet > xModelProps( _rxModel, UNO_QUERY );
    if ( xModelProps.is() )
        xModelProps->getPropertyValue( PROPERTY_ENABLED ) >>= m_bEnabledByPropertyValue;

    modelFeatureUrlPotentiallyChanged( );

    return bResult;
}

//------------------------------------------------------------------------------
void OButtonControl::modelFeatureUrlPotentiallyChanged( )
{
    sal_Int32 nOldUrlFeatureId = m_nTargetUrlFeatureId;

    // doe we have another TargetURL now? If so, we need to update our dispatches
    m_nTargetUrlFeatureId = getModelUrlFeatureId( );
    if ( nOldUrlFeatureId != m_nTargetUrlFeatureId )
    {
        invalidateSupportedFeaturesSet();
        if ( !isDesignMode() )
            updateDispatches( );
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::propertyChange( const PropertyChangeEvent& _rEvent ) throw ( RuntimeException )
{
    if  (   _rEvent.PropertyName.equals( PROPERTY_TARGET_URL )
        ||  _rEvent.PropertyName.equals( PROPERTY_BUTTONTYPE )
        )
    {
        modelFeatureUrlPotentiallyChanged( );
    }
    else if ( _rEvent.PropertyName.equals( PROPERTY_ENABLED ) )
    {
        _rEvent.NewValue >>= m_bEnabledByPropertyValue;
    }

    OImageControl::propertyChange( _rEvent );
}

//------------------------------------------------------------------------------
namespace
{
    bool isFormControllerURL( const ::rtl::OUString& _rURL )
    {
        const sal_Int32 nPrefixLen = URL_CONTROLLER_PREFIX.length;
        return  ( _rURL.getLength() > nPrefixLen )
            &&  ( _rURL.compareToAscii( URL_CONTROLLER_PREFIX, nPrefixLen ) == 0 );
    }
}

//------------------------------------------------------------------------------
sal_Int32 OButtonControl::getModelUrlFeatureId( ) const
{
    sal_Int32 nFeatureId = -1;

    // some URL related properties of the model
    ::rtl::OUString sUrl;
    FormButtonType eButtonType = FormButtonType_PUSH;

    Reference< XPropertySet > xModelProps( const_cast< OButtonControl* >( this )->getModel(), UNO_QUERY );
    if ( xModelProps.is() )
    {
        xModelProps->getPropertyValue( PROPERTY_TARGET_URL ) >>= sUrl;
        xModelProps->getPropertyValue( PROPERTY_BUTTONTYPE ) >>= eButtonType;
    }

    // are we an URL button?
    if ( eButtonType == FormButtonType_URL )
    {
        // is it a feature URL?
        if ( isFormControllerURL( sUrl ) )
        {
            OFormNavigationMapper aMapper( getORB() );
            nFeatureId = aMapper.getFeatureId( sUrl );
        }
    }

    return nFeatureId;
}

//------------------------------------------------------------------
void SAL_CALL OButtonControl::setDesignMode( sal_Bool _bOn ) throw( RuntimeException )
{
    OImageControl::setDesignMode( _bOn  );

    if ( _bOn )
        disconnectDispatchers();
    else
        connectDispatchers();
        // this will connect if not already connected and just update else
}

//------------------------------------------------------------------------------
void OButtonControl::getSupportedFeatures( ::std::vector< sal_Int32 >& /* [out] */ _rFeatureIds )
{
    if ( -1 != m_nTargetUrlFeatureId )
        _rFeatureIds.push_back( m_nTargetUrlFeatureId );
}

//------------------------------------------------------------------
void OButtonControl::featureStateChanged( sal_Int32 _nFeatureId, sal_Bool _bEnabled )
{
    if ( _nFeatureId == m_nTargetUrlFeatureId )
    {
        // enable or disable our peer, according to the new state
        Reference< XVclWindowPeer > xPeer( getPeer(), UNO_QUERY );
        if ( xPeer.is() )
            xPeer->setProperty( PROPERTY_ENABLED, makeAny( m_bEnabledByPropertyValue ? _bEnabled : sal_False ) );
            // if we're disabled according to our model's property, then
            // we don't care for the feature state, but *are* disabled.
            // If the model's property states that we're enabled, then we *do*
            // care for the feature state
    }

    // base class
    OFormNavigationHelper::featureStateChanged( _nFeatureId, _bEnabled );
}

//------------------------------------------------------------------
void OButtonControl::allFeatureStatesChanged( )
{
    if ( -1 != m_nTargetUrlFeatureId )
        // we have only one supported feature, so simulate it has changed ...
        featureStateChanged( m_nTargetUrlFeatureId, isEnabled( m_nTargetUrlFeatureId ) );

    // base class
    OFormNavigationHelper::allFeatureStatesChanged( );
}

//------------------------------------------------------------------
bool OButtonControl::isEnabled( sal_Int32 _nFeatureId ) const
{
    if ( const_cast< OButtonControl* >( this )->isDesignMode() )
        // TODO: the model property?
       return true;

    return OFormNavigationHelper::isEnabled( _nFeatureId );
}

//.........................................................................
}   // namespace frm
//.........................................................................


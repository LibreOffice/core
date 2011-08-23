/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

#ifndef _FRM_BUTTON_HXX_
#include "Button.hxx"
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::sdbcx;
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
               :OImageModel(_rxFactory, VCL_CONTROLMODEL_COMMANDBUTTON, FRM_CONTROL_COMMANDBUTTON)
                                    // use the old control name for compytibility reasons
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
    FRM_BEGIN_PROP_HELPER(8)
        DECL_PROP2(CLASSID,			sal_Int16,					READONLY, TRANSIENT);
        DECL_PROP1(BUTTONTYPE,		FormButtonType,				BOUND);
        DECL_PROP1(DISPATCHURLINTERNAL,	sal_Bool,				BOUND);
        DECL_PROP1(TARGET_URL,		::rtl::OUString,			BOUND);
        DECL_PROP1(TARGET_FRAME,	::rtl::OUString,			BOUND);
        DECL_PROP1(NAME,			::rtl::OUString,			BOUND);
        DECL_PROP1(TAG,				::rtl::OUString,			BOUND);
        DECL_PROP1(TABINDEX,		sal_Int16,					BOUND);
    FRM_END_PROP_HELPER();
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
StringSequence	OButtonModel::getSupportedServiceNames() throw()
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
    return FRM_COMPONENT_COMMANDBUTTON;	// old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OImageModel::write(_rxOutStream);

    _rxOutStream->writeShort(0x0003); 	// Version

    {
        OStreamSection aSection( _rxOutStream.get() );
            // this will allow readers to skip unknown bytes in their dtor

        _rxOutStream->writeShort( (sal_uInt16)m_eButtonType );

        ::rtl::OUString sTmp = INetURLObject::decode(::binfilter::StaticBaseUrl::AbsToRel( m_sTargetURL ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
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

    sal_uInt16 nVersion = _rxInStream->readShort(); 	// Version
    switch (nVersion)
    {
        case 0x0001:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = ::binfilter::StaticBaseUrl::RelToAbs( sTmp );
            _rxInStream >> m_sTargetFrame;
        }
        break;

        case 0x0002:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();

            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = ::binfilter::StaticBaseUrl::RelToAbs( sTmp );
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
            m_sTargetURL = ::binfilter::StaticBaseUrl::RelToAbs( sTmp );

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
        OButtonControl_BASE::getTypes(), OImageControl::_getTypes()
    );
}

//------------------------------------------------------------------------------
StringSequence	OButtonControl::getSupportedServiceNames() throw()
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
                 ,nClickEvent(0)
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
    if (nClickEvent)
        Application::RemoveUserEvent(nClickEvent);
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

    return aReturn;
}

// ActionListener
//------------------------------------------------------------------------------
void OButtonControl::actionPerformed(const ActionEvent& rEvent) throw ( ::com::sun::star::uno::RuntimeException)
{
    // Asynchron fuer starutil::URL-Button
    sal_uInt32 n = Application::PostUserEvent( LINK(this, OButtonControl,OnClick) );
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        nClickEvent = n;
    }
}

//------------------------------------------------------------------------------
IMPL_LINK( OButtonControl, OnClick, void*, EMPTYARG )
{
    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    nClickEvent = 0;

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
            actionPerformed_Impl( sal_False );
    }
    return 0L;
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

//.........................................................................
}	// namespace frm
//.........................................................................

}

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

#include "ImageButton.hxx"
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/MouseButton.hpp>

namespace frm
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

// OImageButtonModel
InterfaceRef SAL_CALL OImageButtonModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
     return *(new OImageButtonModel( comphelper::getComponentContext(_rxFactory) ));
}

OImageButtonModel::OImageButtonModel(const Reference<XComponentContext>& _rxFactory)
                    :OClickableImageBaseModel( _rxFactory, VCL_CONTROLMODEL_IMAGEBUTTON, FRM_SUN_CONTROL_IMAGEBUTTON )
                                    // use the old control name for compytibility reasons
{
    m_nClassId = FormComponentType::IMAGEBUTTON;
}

OImageButtonModel::OImageButtonModel( const OImageButtonModel* _pOriginal, const Reference<XComponentContext>& _rxFactory)
    :OClickableImageBaseModel( _pOriginal, _rxFactory )
{
    implInitializeImageURL();
}

IMPLEMENT_DEFAULT_CLONING( OImageButtonModel )

OImageButtonModel::~OImageButtonModel()
{
}

// XServiceInfo
StringSequence  OImageButtonModel::getSupportedServiceNames() throw(std::exception)
{
    StringSequence aSupported = OClickableImageBaseModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_IMAGEBUTTON;
    return aSupported;
}

void OImageButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 5, OClickableImageBaseModel )
        DECL_PROP1(BUTTONTYPE,          FormButtonType,     BOUND);
        DECL_PROP1(DISPATCHURLINTERNAL, sal_Bool,           BOUND);
        DECL_PROP1(TARGET_URL,          OUString,    BOUND);
        DECL_PROP1(TARGET_FRAME,        OUString,    BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,          BOUND);
    END_DESCRIBE_PROPERTIES();
}

OUString OImageButtonModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString(FRM_COMPONENT_IMAGEBUTTON);   // old (non-sun) name for compatibility !
}

void OImageButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);
    _rxOutStream->writeShort((sal_uInt16)m_eButtonType);

    OUString sTmp(INetURLObject::decode( m_sTargetURL, '%', INetURLObject::DECODE_UNAMBIGUOUS));
    _rxOutStream << sTmp;
    _rxOutStream << m_sTargetFrame;
    writeHelpTextCompatibly(_rxOutStream);
}

void OImageButtonModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    OControlModel::read(_rxInStream);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();

    switch (nVersion)
    {
        case 0x0001:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();
        }
        break;
        case 0x0002:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();
            _rxInStream >> m_sTargetURL;
            _rxInStream >> m_sTargetFrame;
        }
        break;
        case 0x0003:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();
            _rxInStream >> m_sTargetURL;
            _rxInStream >> m_sTargetFrame;
            readHelpTextCompatibly(_rxInStream);
        }
        break;

        default :
            OSL_FAIL("OImageButtonModel::read : unknown version !");
            m_eButtonType = FormButtonType_PUSH;
            m_sTargetURL = OUString();
            m_sTargetFrame = OUString();
            break;
    }
}

// OImageButtonControl
InterfaceRef SAL_CALL OImageButtonControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OImageButtonControl( comphelper::getComponentContext(_rxFactory) ));
}

Sequence<Type> OImageButtonControl::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
        aTypes = concatSequences(OClickableImageBaseControl::_getTypes(), OImageButtonControl_BASE::getTypes());
    return aTypes;
}

StringSequence  OImageButtonControl::getSupportedServiceNames() throw(std::exception)
{
    StringSequence aSupported = OClickableImageBaseControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_IMAGEBUTTON;
    return aSupported;
}

OImageButtonControl::OImageButtonControl(const Reference<XComponentContext>& _rxFactory)
            :OClickableImageBaseControl(_rxFactory, VCL_CONTROL_IMAGEBUTTON)
{
    increment(m_refCount);
    {
        // als MouseListener anmelden
        Reference< awt::XWindow >  xComp;
        query_aggregation( m_xAggregate, xComp);
        if (xComp.is())
            xComp->addMouseListener( static_cast< awt::XMouseListener* >( this ) );
    }
    decrement(m_refCount);
}

// UNO Anbindung
Any SAL_CALL OImageButtonControl::queryAggregation(const Type& _rType) throw (RuntimeException, std::exception)
{
    Any aReturn = OClickableImageBaseControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OImageButtonControl_BASE::queryInterface(_rType);

    return aReturn;
}

void OImageButtonControl::mousePressed(const awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;

    if (e.Buttons != awt::MouseButton::LEFT)
        return;

    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    if( m_aApproveActionListeners.getLength() )
    {
        // if there are listeners, start the action in an own thread, to not allow
        // them to block us here (we're in the application's main thread)
        getImageProducerThread()->OComponentEventThread::addEvent( &e );
    }
    else
    {
        // Sonst nicht. Dann darf man aber auf keinen Fal die Listener
        // benachrichtigen, auch dann nicht, wenn er spaeter hinzukommt.
        aGuard.clear();
        actionPerformed_Impl( sal_False, e );
    }
}

void SAL_CALL OImageButtonControl::mouseReleased(const awt::MouseEvent& /*e*/) throw ( RuntimeException, std::exception)
{
}

void SAL_CALL OImageButtonControl::mouseEntered(const awt::MouseEvent& /*e*/) throw ( RuntimeException, std::exception)
{
}

void SAL_CALL OImageButtonControl::mouseExited(const awt::MouseEvent& /*e*/) throw ( RuntimeException, std::exception)
{
}

}   // namespace frm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

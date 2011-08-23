/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <comphelper/proparrhlp.hxx>

#include "ImageButton.hxx"

#include <tools/urlobj.hxx>

#include <com/sun/star/awt/MouseButton.hpp>

#include <com/sun/star/form/FormComponentType.hpp>

#include <com/sun/star/beans/PropertyAttribute.hpp>


#include "services.hxx"

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
//= OImageButtonModel
//==================================================================
DBG_NAME(OImageButtonModel)
//------------------------------------------------------------------
InterfaceRef SAL_CALL OImageButtonModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
     return *(new OImageButtonModel(_rxFactory));
}

//------------------------------------------------------------------
OImageButtonModel::OImageButtonModel(const Reference<XMultiServiceFactory>& _rxFactory)
                    :OImageModel(_rxFactory, VCL_CONTROLMODEL_IMAGEBUTTON, FRM_CONTROL_IMAGEBUTTON)
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR(OImageButtonModel, NULL);
    m_nClassId = FormComponentType::IMAGEBUTTON;
}

//------------------------------------------------------------------
OImageButtonModel::OImageButtonModel( const OImageButtonModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory)
    :OImageModel( _pOriginal, _rxFactory )
{
    DBG_CTOR(OImageButtonModel, NULL);
    implInitializeImageURL();
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OImageButtonModel )

//------------------------------------------------------------------------------
OImageButtonModel::~OImageButtonModel()
{
    DBG_DTOR(OImageButtonModel, NULL);
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OImageButtonModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence	OImageButtonModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_IMAGEBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
void OImageButtonModel::fillProperties(
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
::cppu::IPropertyArrayHelper& OImageButtonModel::getInfoHelper()
{
    return *const_cast<OImageButtonModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString OImageButtonModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_IMAGEBUTTON;	// old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OImageButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);
    _rxOutStream->writeShort((sal_uInt16)m_eButtonType);

    ::rtl::OUString sTmp(INetURLObject::decode(::binfilter::StaticBaseUrl::AbsToRel(m_sTargetURL), '%', INetURLObject::DECODE_UNAMBIGUOUS));
    _rxOutStream << sTmp;
    _rxOutStream << m_sTargetFrame;
    writeHelpTextCompatibly(_rxOutStream);
}

//------------------------------------------------------------------------------
void OImageButtonModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
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
            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = ::binfilter::StaticBaseUrl::RelToAbs( sTmp );
            _rxInStream >> m_sTargetFrame;
        }
        break;
        case 0x0003:
        {
            m_eButtonType = (FormButtonType)_rxInStream->readShort();
            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = ::binfilter::StaticBaseUrl::RelToAbs( sTmp );
            _rxInStream >> m_sTargetFrame;
            readHelpTextCompatibly(_rxInStream);
        }
        break;

        default :
            DBG_ERROR("OImageButtonModel::read : unknown version !");
            m_eButtonType = FormButtonType_PUSH;
            m_sTargetURL = ::rtl::OUString();
            m_sTargetFrame = ::rtl::OUString();
            break;
    }
}

//==================================================================
// OImageButtonControl
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OImageButtonControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OImageButtonControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OImageButtonControl::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
        aTypes = concatSequences(OImageControl::_getTypes(), OImageButtonControl_BASE::getTypes());
    return aTypes;
}

//------------------------------------------------------------------------------
StringSequence	OImageButtonControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_IMAGEBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
OImageButtonControl::OImageButtonControl(const Reference<XMultiServiceFactory>& _rxFactory)
            :OImageControl(_rxFactory, VCL_CONTROL_IMAGEBUTTON)
{
    increment(m_refCount);
    {
        // als MouseListener anmelden
        Reference<XWindow>  xComp;
        query_aggregation( m_xAggregate, xComp);
        if (xComp.is())
            xComp->addMouseListener((XMouseListener*)this);
    }
    // Refcount bei 1 fuer angemeldeten Listener
    sal_Int32 n = decrement(m_refCount);
}

// UNO Anbindung
//------------------------------------------------------------------------------
Any SAL_CALL OImageButtonControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OImageControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OImageButtonControl_BASE::queryInterface(_rType);

    return aReturn;
}

//------------------------------------------------------------------------------
void OImageButtonControl::mousePressed(const MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
    //////////////////////////////////////////////////////////////////////
    // Nur linke Maustaste
    if (e.Buttons != MouseButton::LEFT)
        return;

    ::osl::ClearableMutexGuard aGuard( m_aMutex );
    if( m_aApproveActionListeners.getLength() )
    {
        // Wenn es ApproveAction-Lisener gibt, muss ein eigener Thread
        // aufgemacht werden.
        if( !m_pThread )
        {
            m_pThread = new OImageControlThread_Impl( this );
            m_pThread->acquire();
            m_pThread->create();
        }
        m_pThread->OComponentEventThread::addEvent( &e );
    }
    else
    {
        // Sonst nicht. Dann darf man aber auf keinen Fal die Listener
        // benachrichtigen, auch dann nicht, wenn er spaeter hinzukommt.
        aGuard.clear();
        actionPerformed_Impl( sal_False, e );
    }
}

//.........................................................................
}	// namespace frm
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

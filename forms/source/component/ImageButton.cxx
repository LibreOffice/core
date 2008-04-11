/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImageButton.cxx,v $
 * $Revision: 1.18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "ImageButton.hxx"
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <comphelper/basicio.hxx>
#include <com/sun/star/awt/MouseButton.hpp>

//.........................................................................
namespace frm
{
//.........................................................................
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
                    :OClickableImageBaseModel( _rxFactory, VCL_CONTROLMODEL_IMAGEBUTTON, FRM_SUN_CONTROL_IMAGEBUTTON )
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR(OImageButtonModel, NULL);
    m_nClassId = FormComponentType::IMAGEBUTTON;
}

//------------------------------------------------------------------
OImageButtonModel::OImageButtonModel( const OImageButtonModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory)
    :OClickableImageBaseModel( _pOriginal, _rxFactory )
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

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OImageButtonModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OClickableImageBaseModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_IMAGEBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
void OImageButtonModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 5, OClickableImageBaseModel )
        DECL_PROP1(BUTTONTYPE,          FormButtonType,     BOUND);
        DECL_PROP1(DISPATCHURLINTERNAL, sal_Bool,           BOUND);
        DECL_PROP1(TARGET_URL,          ::rtl::OUString,    BOUND);
        DECL_PROP1(TARGET_FRAME,        ::rtl::OUString,    BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,          BOUND);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
::rtl::OUString OImageButtonModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_IMAGEBUTTON;   // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OImageButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);
    _rxOutStream->writeShort((sal_uInt16)m_eButtonType);

    ::rtl::OUString sTmp(INetURLObject::decode( m_sTargetURL, '%', INetURLObject::DECODE_UNAMBIGUOUS));
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
        aTypes = concatSequences(OClickableImageBaseControl::_getTypes(), OImageButtonControl_BASE::getTypes());
    return aTypes;
}

//------------------------------------------------------------------------------
StringSequence  OImageButtonControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OClickableImageBaseControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_IMAGEBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
OImageButtonControl::OImageButtonControl(const Reference<XMultiServiceFactory>& _rxFactory)
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
//------------------------------------------------------------------------------
Any SAL_CALL OImageButtonControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OClickableImageBaseControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OImageButtonControl_BASE::queryInterface(_rType);

    return aReturn;
}

//------------------------------------------------------------------------------
void OImageButtonControl::mousePressed(const awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );

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

//------------------------------------------------------------------------------
void SAL_CALL OImageButtonControl::mouseReleased(const awt::MouseEvent& /*e*/) throw ( RuntimeException)
{
}

//------------------------------------------------------------------------------
void SAL_CALL OImageButtonControl::mouseEntered(const awt::MouseEvent& /*e*/) throw ( RuntimeException)
{
}

//------------------------------------------------------------------------------
void SAL_CALL OImageButtonControl::mouseExited(const awt::MouseEvent& /*e*/) throw ( RuntimeException)
{
}


//.........................................................................
}   // namespace frm
//.........................................................................


/*************************************************************************
 *
 *  $RCSfile: Button.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-02 10:28:06 $
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
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
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
               :OImageModel(_rxFactory, VCL_CONTROLMODEL_COMMANDBUTTON, FRM_CONTROL_COMMANDBUTTON)
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR(OButtonModel, NULL);
    m_nClassId = FormComponentType::COMMANDBUTTON;
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
    FRM_BEGIN_PROP_HELPER(7)
        DECL_PROP2(CLASSID,         sal_Int16,                  READONLY, TRANSIENT);
        DECL_PROP1(BUTTONTYPE,      FormButtonType, BOUND);
        DECL_PROP1(TARGET_URL,      ::rtl::OUString,            BOUND);
        DECL_PROP1(TARGET_FRAME,    ::rtl::OUString,            BOUND);
        DECL_PROP1(NAME,            ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,             ::rtl::OUString,            BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,                  BOUND);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OButtonModel::getInfoHelper()
{
    return *const_cast<OButtonModel*>(this)->getArrayHelper();
}

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
::rtl::OUString OButtonModel::getServiceName()
{
    return FRM_COMPONENT_COMMANDBUTTON; // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    OImageModel::write(_rxOutStream);

    _rxOutStream->writeShort(0x0002);   // Version
    _rxOutStream->writeShort( (sal_uInt16)m_eButtonType );

    ::rtl::OUString sTmp = INetURLObject::decode(INetURLObject::AbsToRel( m_sTargetURL ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
    _rxOutStream << sTmp;
    _rxOutStream << m_sTargetFrame;
    writeHelpTextCompatibly(_rxOutStream);
}

//------------------------------------------------------------------------------
void OButtonModel::read(const Reference<XObjectInputStream>& _rxInStream)
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
        default :
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
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class
        Sequence<Type> aBaseClassTypes = OImageControl::_getTypes();

        Sequence<Type> aOwnTypes(2);
        Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((Reference<XButton>*)NULL);
        pOwnTypes[1] = getCppuType((Reference<XActionListener>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
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
    Any aReturn = OImageControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OButtonControl_BASE::queryInterface(_rType);

    return aReturn;
}

// ActionListener
//------------------------------------------------------------------------------
void OButtonControl::actionPerformed(const ActionEvent& rEvent)
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
                ((XActionListener*)aIter.next())->actionPerformed(aEvt);
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
    query_aggregation( m_xAggregate, xButton);
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
}   // namespace frm
//.........................................................................


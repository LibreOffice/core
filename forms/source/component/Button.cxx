/*************************************************************************
 *
 *  $RCSfile: Button.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:04 $
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

//==================================================================
//= OButtonModel
//==================================================================
DBG_NAME(OButtonModel)
//------------------------------------------------------------------
InterfaceRef SAL_CALL OButtonModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OButtonModel(_rxFactory));
}

//------------------------------------------------------------------
OButtonModel::OButtonModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
               :OImageModel(_rxFactory, VCL_CONTROLMODEL_COMMANDBUTTON, FRM_CONTROL_COMMANDBUTTON)
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR(OButtonModel, NULL);
    m_nClassId = starform::FormComponentType::COMMANDBUTTON;
}

//------------------------------------------------------------------------------
OButtonModel::~OButtonModel()
{
    DBG_DTOR(OButtonModel, NULL);
}

//------------------------------------------------------------------------------
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL OButtonModel::getPropertySetInfo() throw( staruno::RuntimeException )
{
    staruno::Reference<starbeans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OButtonModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(8)
        DECL_PROP2(CLASSID,         sal_Int16,                  READONLY, TRANSIENT);
        DECL_PROP1(BUTTONTYPE,      starform::FormButtonType,   BOUND);
        DECL_PROP1(TARGET_URL,      ::rtl::OUString,            BOUND);
        DECL_PROP1(TARGET_FRAME,    ::rtl::OUString,            BOUND);
        DECL_PROP1(NAME,            ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,             ::rtl::OUString,            BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,                  BOUND);
        DECL_PROP1(HELPTEXT,        ::rtl::OUString,            BOUND);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OButtonModel::getInfoHelper()
{
    return *const_cast<OButtonModel*>(this)->getArrayHelper();
}

// starlang::XServiceInfo
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
void OButtonModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
{
    OImageModel::write(_rxOutStream);

    _rxOutStream->writeShort(0x0002);   // Version
    _rxOutStream->writeShort( (sal_uInt16)m_eButtonType );

    ::rtl::OUString sTmp = INetURLObject::decode(INetURLObject::AbsToRel( m_sTargetURL ), '%', INetURLObject::DECODE_UNAMBIGUOUS);
    _rxOutStream << sTmp;
    _rxOutStream << m_sTargetFrame;
    _rxOutStream << m_aHelpText;
}

//------------------------------------------------------------------------------
void OButtonModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream)
{
    OImageModel::read(_rxInStream);

    sal_uInt16 nVersion = _rxInStream->readShort();     // Version
    switch (nVersion)
    {
        case 0x0001:
        {
            m_eButtonType = (starform::FormButtonType)_rxInStream->readShort();

            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = INetURLObject::RelToAbs( sTmp );
            _rxInStream >> m_sTargetFrame;
        }
        break;
        case 0x0002:
        {
            m_eButtonType = (starform::FormButtonType)_rxInStream->readShort();

            ::rtl::OUString sTmp;
            _rxInStream >> sTmp;
            m_sTargetURL = INetURLObject::RelToAbs( sTmp );
            _rxInStream >> m_sTargetFrame;
            _rxInStream >> m_aHelpText;
        }
        break;
        default :
            DBG_ERROR("OButtonModel::read : unknown version !");
            m_eButtonType = starform::FormButtonType_PUSH;
            m_sTargetURL = ::rtl::OUString();
            m_sTargetFrame = ::rtl::OUString();
            break;
    }
}

//==================================================================
// OButtonControl
//==================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OButtonControl_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OButtonControl(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OButtonControl::_getTypes()
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class
        staruno::Sequence<staruno::Type> aBaseClassTypes = OImageControl::_getTypes();

        staruno::Sequence<staruno::Type> aOwnTypes(2);
        staruno::Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((staruno::Reference<starawt::XButton>*)NULL);
        pOwnTypes[1] = getCppuType((staruno::Reference<starawt::XActionListener>*)NULL);

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
OButtonControl::OButtonControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                 :OImageControl(_rxFactory, VCL_CONTROL_COMMANDBUTTON)
                 ,nClickEvent(0)
{
    increment(m_refCount);
    {
        // als ActionListener anmelden
        staruno::Reference<starawt::XButton>  xButton;
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
staruno::Any SAL_CALL OButtonControl::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OImageControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<starawt::XActionListener*>(this)
            ,static_cast<starawt::XButton*>(this)
        );

    return aReturn;
}

// ActionListener
//------------------------------------------------------------------------------
void OButtonControl::actionPerformed(const starawt::ActionEvent& rEvent)
{
    // Asynchron fuer starutil::URL-Button
    sal_uInt32 n = Application::PostUserEvent( LINK(this, OButtonControl,
                                          OnClick) );
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
        staruno::Reference<starbeans::XPropertySet>  xSet(getModel(), staruno::UNO_QUERY);
        if (!xSet.is())
            return 0L;

        if (starform::FormButtonType_PUSH == *(starform::FormButtonType*)xSet->getPropertyValue(PROPERTY_BUTTONTYPE).getValue())
        {
            // notify the action listeners for a push button
            ::cppu::OInterfaceIteratorHelper aIter(m_aActionListeners);
            starawt::ActionEvent aEvt(static_cast<staruno::XWeak*>(this), m_aActionCommand);
            while(aIter.hasMoreElements() )
            {
                ((starawt::XActionListener*)aIter.next())->actionPerformed(aEvt);
            }
        }
        else
            actionPerformed_Impl( sal_False );
    }
    return 0L;
}

// starawt::XButton
//------------------------------------------------------------------------------
void OButtonControl::setLabel(const ::rtl::OUString& Label) throw( staruno::RuntimeException )
{
    staruno::Reference<starawt::XButton>  xButton;
    query_aggregation( m_xAggregate, xButton);
    if (xButton.is())
        xButton->setLabel(Label);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::setActionCommand(const ::rtl::OUString& _rCommand) throw( staruno::RuntimeException )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_aActionCommand = _rCommand;
    }

    staruno::Reference<starawt::XButton>  xButton;
    query_aggregation( m_xAggregate, xButton);
    if (xButton.is())
        xButton->setActionCommand(_rCommand);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::addActionListener(const staruno::Reference<starawt::XActionListener>& _rxListener) throw( staruno::RuntimeException )
{
    m_aActionListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OButtonControl::removeActionListener(const staruno::Reference<starawt::XActionListener>& _rxListener) throw( staruno::RuntimeException )
{
    m_aActionListeners.removeInterface(_rxListener);
}

//.........................................................................
}   // namespace frm
//.........................................................................


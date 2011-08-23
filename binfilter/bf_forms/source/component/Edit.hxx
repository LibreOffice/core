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

#ifndef _FORMS_EDIT_HXX_
#define _FORMS_EDIT_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace frm
{

//==================================================================
//= OEditModel
//==================================================================
class OEditModel
                :public OEditBaseModel
                ,public ::comphelper::OAggregationArrayUsageHelper< OEditModel >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter> 		m_xFormatter;
    ::rtl::OUString					m_aSaveValue;
    sal_Int32					m_nFormatKey;
    ::com::sun::star::util::Date				m_aNullDate;
    sal_Int32					m_nFieldType;
    sal_Int16					m_nKeyType;
    sal_Bool					m_bMaxTextLenModified : 1;	// set to <TRUE/> when we change the MaxTextLen of the aggregate

    sal_Bool					m_bWritingFormattedFake : 1;
        // are we writing something which should be interpreted as formatted upon reading?
    sal_Bool					m_bNumericField : 1;
        // are we bound to some kind of numeric field?

    static sal_Int32			nTextHandle;

protected:
    virtual void _onValueChanged();
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

    DECLARE_DEFAULT_LEAF_XTOR( OEditModel );

    void enableFormattedWriteFake() { m_bWritingFormattedFake = sal_True; }
    void disableFormattedWriteFake() { m_bWritingFormattedFake = sal_False; }
    sal_Bool lastReadWasFormattedFake() const { return (getLastReadVersion() & PF_FAKE_FORMATTED_FIELD) != 0; }

    friend InterfaceRef SAL_CALL OEditModel_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    friend class OFormattedFieldWrapper;
    friend class OFormattedModel;	// temporary

public:
    virtual void SAL_CALL disposing();

// ::com::sun::star::form::XBoundComponent
    virtual sal_Bool _commit();

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
// ::com::sun::star::io::XPersistObject
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OEditModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::form::XLoadListener
    virtual void _loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void _unloaded();

// ::com::sun::star::form::XReset
    virtual void _reset();

// OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    virtual sal_Int16 getPersistenceFlags() const;

    DECLARE_XCLONEABLE();
};

//==================================================================
//= OEditControl
//==================================================================
typedef ::cppu::ImplHelper3<	::com::sun::star::awt::XFocusListener,
                                ::com::sun::star::awt::XKeyListener,
                                ::com::sun::star::form::XChangeBroadcaster > OEditControl_BASE;

class OEditControl : public OBoundControl
                      ,public OEditControl_BASE
{
    ::cppu::OInterfaceContainerHelper
                        m_aChangeListeners;

    ::rtl::OUString			m_aHtmlChangeValue;
    sal_uInt32 				m_nKeyEvent;

public:
    OEditControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OEditControl();

    DECLARE_UNO3_AGG_DEFAULTS(OEditControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

// OComponentHelper
    virtual void SAL_CALL disposing();

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OEditControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::form::XChangeBroadcaster
    virtual void SAL_CALL addChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::awt::XFocusListener
    virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw ( ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::awt::XKeyListener
    virtual void SAL_CALL keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL keyReleased(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException);

private:
    DECL_LINK( OnKeyPressed, void* );
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_EDIT_HXX_


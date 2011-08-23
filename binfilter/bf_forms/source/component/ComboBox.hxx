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

#ifndef _FORMS_COMBOBOX_HXX_
#define _FORMS_COMBOBOX_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_LISTSOURCETYPE_HPP_
#include <com/sun/star/form/ListSourceType.hpp>
#endif
#ifndef FORMS_ERRORBROADCASTER_HXX
#include "errorbroadcaster.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace frm
{

//==================================================================
// OComboBoxModel
//==================================================================
class OComboBoxModel
            :public OBoundControlModel
            ,public OErrorBroadcaster
            ,public ::comphelper::OAggregationArrayUsageHelper< OComboBoxModel >
{
    ::com::sun::star::uno::Any				m_aBoundColumn; 		// obsolet
    ::rtl::OUString 						m_aListSource;			//
    ::rtl::OUString 						m_aDefaultText; 		// DefaultText
    ::rtl::OUString 						m_aSaveValue;

    StringSequence							m_aDesignModeStringItems;
        // upon loading, in some cases we reset fill our string item list ourself. We don't want
        // to lose the user's items then, so we remember them here.

    
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter> m_xFormatter;

    ::com::sun::star::form::ListSourceType	m_eListSourceType;		// type der list source
    ::com::sun::star::util::Date			m_aNullDate;
    sal_Int32								m_nFormatKey;
    sal_Int16								m_nFieldType;
    sal_Int16								m_nKeyType;
    sal_Bool								m_bEmptyIsNull; 		// LeerString wird als NULL interpretiert

    

    static sal_Int32		nTextHandle;

protected:
    virtual void _onValueChanged();
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type>	_getTypes();

public:
    DECLARE_DEFAULT_LEAF_XTOR( OComboBoxModel );

    virtual void SAL_CALL disposing();

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException);

    // XLoadListener
    virtual void		 _loaded(const ::com::sun::star::lang::EventObject& rEvent);
    virtual void		 _unloaded();

    virtual void SAL_CALL reloaded( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    IMPLEMENTATION_NAME(OComboBoxModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OComboBoxModel, OBoundControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);

    // XBoundComponent
    virtual sal_Bool _commit();

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL	getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XReset
    virtual void _reset();

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    void loadData();

    DECLARE_XCLONEABLE();
};

//==================================================================
// OComboBoxControl
//==================================================================

class OComboBoxControl : public OBoundControl
{
public:
    OComboBoxControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    // XServiceInfo
    IMPLEMENTATION_NAME(OComboBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_COMBOBOX_HXX_


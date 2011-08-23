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

#ifndef _FRM_FORMATTED_FIELD_WRAPPER_HXX_
#define _FRM_FORMATTED_FIELD_WRAPPER_HXX_

#include "FormComponent.hxx"
namespace binfilter {

//.........................................................................
namespace frm
{

class OEditModel;
//==================================================================
//= OFormattedFieldWrapper
//==================================================================
typedef ::cppu::WeakAggImplHelper3	<	::com::sun::star::io::XPersistObject
                                    ,	::com::sun::star::lang::XServiceInfo
                                    ,	::com::sun::star::util::XCloneable
                                    >	OFormattedFieldWrapper_Base;

class OFormattedFieldWrapper : public OFormattedFieldWrapper_Base
{
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>	m_xServiceFactory;

protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation> 		m_xAggregate;

    OEditModel*		m_pEditPart;
        // if we act as formatted this is used to write the EditModel part
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XPersistObject> 	m_xFormattedPart;
        // if we act as formatted, this is the PersistObject interface of our aggregate, used
        // to read and write the FormattedModel part

    OFormattedFieldWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, sal_Bool _bActAsFormatted);
        // if _bActAsFormatted is sal_False, the state is undetermined until somebody calls ::read or does
        // anything which requires a living aggregate
    OFormattedFieldWrapper( const OFormattedFieldWrapper* _pCloneSource );
    virtual ~OFormattedFieldWrapper();

    friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    friend InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

public:
    // UNO
    DECLARE_UNO3_AGG_DEFAULTS(OFormattedFieldWrapper, OWeakAggObject);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XCloneable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone(  ) throw (::com::sun::star::uno::RuntimeException);

protected:
    /// ensure we're in a defined state, which means a FormattedModel _OR_ an EditModel
    void ensureAggregate();
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FRM_FORMATTED_FIELD_WRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: InternalData.hxx,v $
 * $Revision: 1.3 $
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
#ifndef CHART2_INTERNALDATA_HXX
#define CHART2_INTERNALDATA_HXX

#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/util/XCloseListener.hpp>
#include <cppuhelper/implbase2.hxx>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
    namespace embed {
        class XStorage;
        class XEmbeddedObject;
    }
    namespace chart2 {
        namespace data {
            class XDataProvider;
        }
    }
}}}

namespace chart
{

class InternalData :
        public ::cppu::WeakImplHelper2<
        ::com::sun::star::embed::XEmbeddedClient,
        ::com::sun::star::util::XCloseListener >
{
public:
    explicit InternalData(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XStorage > & xParentStorage );
    virtual ~InternalData();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataProvider > createEmbeddedObject() throw();

    void removeEmbeddedObject() throw();

    ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XEmbeddedObject > getEmbeddedObject() const throw();

    // ____ XEmbeddedClient ____
    virtual void SAL_CALL saveObject()
        throw (::com::sun::star::embed::ObjectSaveVetoException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL visibilityChanged( sal_Bool bVisible )
        throw (::com::sun::star::embed::WrongStateException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XComponentSupplier ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloseListener ____
    virtual void SAL_CALL queryClosing(
        const ::com::sun::star::lang::EventObject& Source,
        ::sal_Bool GetsOwnership )
        throw (::com::sun::star::util::CloseVetoException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyClosing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

private:
    const ::rtl::OUString m_aDataStorageName;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
        m_xContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject >
        m_xInternalData;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
        m_xParentStorage;
};

} //  namespace chart

// CHART2_INTERNALDATA_HXX
#endif

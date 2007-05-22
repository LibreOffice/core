/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: InternalData.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:39:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_INTERNALDATA_HXX
#define CHART2_INTERNALDATA_HXX

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDCLIENT_HPP_
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

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

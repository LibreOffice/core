/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventcfg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:25:11 $
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
#ifndef _EVENTCFG_HXX
#define _EVENTCFG_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase2.hxx>
#include <hash_map>
#include <vector>

typedef ::std::hash_map< ::rtl::OUString, ::rtl::OUString, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > EventBindingHash;
typedef ::std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > > FrameVector;
typedef ::std::vector< ::rtl::OUString > SupportedEventsVector;

class GlobalEventConfig_Impl : public utl::ConfigItem
{
    EventBindingHash m_eventBindingHash;
    FrameVector m_lFrames;
    SupportedEventsVector m_supportedEvents;

    void initBindingInfo();

public:
    GlobalEventConfig_Impl( );
    ~GlobalEventConfig_Impl( );

    void EstablishFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);
    void            Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames);
    void            Commit();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
    ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

};

class SVL_DLLPUBLIC GlobalEventConfig:
        public ::cppu::WeakImplHelper2 < ::com::sun::star::document::XEventsSupplier, ::com::sun::star::container::XNameReplace >
{
    public:
        GlobalEventConfig( );
        ~GlobalEventConfig( );
        static ::osl::Mutex& GetOwnStaticMutex();

        void EstablishFrameCallback(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents(  ) throw (::com::sun::star::uno::RuntimeException);
        void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException);
        ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException);


    private:
        static GlobalEventConfig_Impl* m_pImpl;
        static sal_Int32 m_nRefCount;
};

#endif // _EVENTCFG_HXX

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dummyobject.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 16:05:27 $
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

#ifndef _INC_DUMMYOBJECT_HXX_
#define _INC_DUMMYOBJECT_HXX_

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace embed {
        class XStorage;
    }
    namespace frame {
        class XModel;
        class XFrame;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace util {
        class XCloseListener;
    }
    namespace beans {
        struct PropertyValue;
        struct NamedValue;
    }
}}}

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

class ODummyEmbeddedObject : public ::cppu::WeakImplHelper2
                        < ::com::sun::star::embed::XEmbeddedObject
                        , ::com::sun::star::embed::XEmbedPersist >
{
    ::osl::Mutex    m_aMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;
    sal_Bool m_bDisposed;

    ::rtl::OUString m_aEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xParentStorage;
    sal_Int32 m_nObjectState;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient > m_xClientSite;

    sal_Int64 m_nCachedAspect;
    ::com::sun::star::awt::Size m_aCachedSize;
    sal_Bool m_bHasCachedSize;

    // following information will be used between SaveAs and SaveCompleted
    sal_Bool m_bWaitSaveCompleted;
    ::rtl::OUString m_aNewEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xNewParentStorage;

protected:
    void CheckInit();
    void PostEvent_Impl( const ::rtl::OUString& aEventName,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xSource );

public:

    ODummyEmbeddedObject()
    : m_pInterfaceContainer( NULL )
    , m_bDisposed( sal_False )
    , m_nObjectState( -1 )
    , m_nCachedAspect( 0 )
    , m_bHasCachedSize( sal_False )
    , m_bWaitSaveCompleted( sal_False )
    {}

    ~ODummyEmbeddedObject();

// XEmbeddedObject

    virtual void SAL_CALL changeState( sal_Int32 nNewState )
        throw ( ::com::sun::star::embed::UnreachableStateException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< sal_Int32 > SAL_CALL getReachableStates()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getCurrentState()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL doVerb( sal_Int32 nVerbID )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::embed::UnreachableStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor > SAL_CALL getSupportedVerbs()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setClientSite(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient >& xClient )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient > SAL_CALL getClientSite()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL update()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setUpdateMode( sal_Int32 nMode )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int64 SAL_CALL getStatus( sal_Int64 nAspect )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setContainerName( const ::rtl::OUString& sName )
        throw ( ::com::sun::star::uno::RuntimeException );


// XVisualObject

    virtual void SAL_CALL setVisualAreaSize( sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::awt::Size SAL_CALL getVisualAreaSize( sal_Int64 nAspect )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::embed::VisualRepresentation SAL_CALL getPreferredVisualRepresentation( ::sal_Int64 nAspect )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getMapUnit( sal_Int64 nAspect )
        throw ( ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException);

// XEmbedPersist

    virtual void SAL_CALL setPersistentEntry(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                    const ::rtl::OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL storeToEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::rtl::OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL storeAsEntry(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                const ::rtl::OUString& sEntName,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL saveCompleted( sal_Bool bUseNew )
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL hasEntry()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getEntryName()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );


// XCommonEmbedPersist

    virtual void SAL_CALL storeOwn()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isReadonly()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL reload(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lObjArgs )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );


// XClassifiedObject

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getClassID()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getClassName()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setClassInfo(
                const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID, const ::rtl::OUString& aClassName )
        throw ( ::com::sun::star::lang::NoSupportException,
                ::com::sun::star::uno::RuntimeException );


// XComponentSupplier

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseable > SAL_CALL getComponent()
        throw ( ::com::sun::star::uno::RuntimeException );

// XStateChangeBroadcaster
    virtual void SAL_CALL addStateChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeStateChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStateChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

// XCloseable

    virtual void SAL_CALL close( sal_Bool DeliverOwnership )
        throw ( ::com::sun::star::util::CloseVetoException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addCloseListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeCloseListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

// XEventBroadcaster
    virtual void SAL_CALL addEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

};

#endif


/*************************************************************************
 *
 *  $RCSfile: commonembobj.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-14 15:24:26 $
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

#ifndef _INC_COMMONEMBOBJ_HXX_
#define _INC_COMMONEMBOBJ_HXX_

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
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XLINKAGESUPPORT_HPP_
#include <com/sun/star/embed/XLinkageSupport.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XCLASSIFIEDOBJECT_HPP_
#include <com/sun/star/embed/XClassifiedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XCOMPONENTSUPPLIER_HPP_
#include <com/sun/star/embed/XComponentSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
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
}}}

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

#define NUM_SUPPORTED_STATES 3
#define NUM_SUPPORTED_VERBS 3

#include "docholder.hxx"

class Interceptor;

class OCommonEmbeddedObject : public ::com::sun::star::embed::XEmbeddedObject
                            , public ::com::sun::star::embed::XVisualObject
                            , public ::com::sun::star::embed::XEmbedPersist
                            , public ::com::sun::star::embed::XLinkageSupport
                            , public ::com::sun::star::embed::XClassifiedObject
                            , public ::com::sun::star::embed::XComponentSupplier
                            , public ::com::sun::star::document::XEventBroadcaster
                            , public ::cppu::OWeakObject
{
    ::osl::Mutex    m_aMutex;

    DocumentHolder* m_pDocHolder;

    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    sal_Bool m_bReadOnly;

    sal_Int32 m_bDisposed;
    sal_Int32 m_nObjectState;
    sal_Int32 m_nUpdateMode;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    ::com::sun::star::uno::Sequence< sal_Int8 > m_aClassID;
    ::rtl::OUString m_aClassName;

    ::rtl::OUString m_aDocServiceName;

    ::com::sun::star::uno::Sequence< sal_Int32 > m_aAcceptedStates;
    ::com::sun::star::uno::Sequence< sal_Int32 > m_pIntermediateStatesSeqs[NUM_SUPPORTED_STATES][NUM_SUPPORTED_STATES];
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< sal_Int32 > > m_aVerbTable;

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedClient > m_xClientSite;

    ::rtl::OUString m_aContainerName;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener > m_xClosePreventer;

    Interceptor* m_pInterceptor;

    // following information will be used between SaveAs and SaveCompleted
    sal_Bool m_bWaitSaveCompleted;
    ::rtl::OUString m_aNewEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xNewParentStorage;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xNewObjectStorage;

    sal_Bool m_bIsLink;

    // embedded object related stuff
    ::rtl::OUString m_aEntryName;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xParentStorage;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xObjectStorage;

    // link related stuff
    ::rtl::OUString m_aLinkURL;
    ::rtl::OUString m_aLinkFilterName;

private:
    void CommonInit_Impl();

    ::rtl::OUString GetDocumentServiceName() { return m_aDocServiceName; }

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > StoreDocumentToTempStream_Impl();

    sal_Int32 ConvertVerbToState_Impl( sal_Int32 nVerb );

    void SwitchStateTo_Impl( sal_Int32 nNextState );

    ::com::sun::star::uno::Sequence< sal_Int32 > GetIntermediateStatesSequence_Impl( sal_Int32 nNewState );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > LoadDocumentFromStorage_Impl(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > LoadLink_Impl();

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > InitNewDocument_Impl();

    void StoreDocToStorage_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > CreateDocFromMediaDescr_Impl(
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMedDescr );

    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > CreateTempDocFromLink_Impl();

public:
    OCommonEmbeddedObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID,
        const ::rtl::OUString& aClassName,
        const ::rtl::OUString& aDocServiceName );

    // no persistance for linked objects, so the url is provided in constructor
    OCommonEmbeddedObject(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory,
        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID,
        const ::rtl::OUString& aClassName,
        const ::rtl::OUString& aDocServiceName,
        const ::rtl::OUString& aLinkFilterName,
        const ::rtl::OUString& aLinkURL );

    virtual ~OCommonEmbeddedObject();

    void SaveObject_Impl();

    // not a real listener and should not be
    void PostEvent_Impl( const ::rtl::OUString& aEventName );

// XInterface

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& rType )
        throw( ::com::sun::star::uno::RuntimeException ) ;

    virtual void SAL_CALL acquire()
        throw();

    virtual void SAL_CALL release()
        throw();

// XTypeProvider

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException ) ;

    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw( ::com::sun::star::uno::RuntimeException ) ;

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

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescr > SAL_CALL getSupportedVerbs()
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


// XVisualObject

    virtual void SAL_CALL setContainerName( const ::rtl::OUString& sName )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setVisAreaSize( sal_Int64 nAspect, const ::com::sun::star::awt::Size& aSize )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::awt::Size SAL_CALL getVisAreaSize( sal_Int64 nAspect )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Any SAL_CALL getVisualCache( sal_Int64 nAspect )
        throw ( ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );


// XEmbedPersist

    virtual void SAL_CALL setPersistentEntry(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                    const ::rtl::OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL storeOwn()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL storeToEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage, const ::rtl::OUString& sEntName, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL storeAsEntry(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                const ::rtl::OUString& sEntName,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments )
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

    virtual sal_Bool SAL_CALL isReadonly()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL reload(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

// XLinkageSupport

    virtual void SAL_CALL breakLink( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                     const ::rtl::OUString& sEntName )
        throw ( ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::io::IOException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isLink()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getLinkURL()
        throw ( ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException);

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

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > SAL_CALL getComponent()
        throw ( ::com::sun::star::uno::RuntimeException );


// XEventBroadcaster
    virtual void SAL_CALL addEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventListener >& Listener )
        throw ( ::com::sun::star::uno::RuntimeException );

// XComponent
    virtual void SAL_CALL dispose()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL addEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeEventListener(
                const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

};

#endif


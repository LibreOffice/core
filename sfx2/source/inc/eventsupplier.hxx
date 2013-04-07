/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SFX_EVENTSUPPLIER_HXX_
#define _SFX_EVENTSUPPLIER_HXX_

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/frame/XGlobalEventBroadcaster.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/sequenceasvector.hxx>
#include <sfx2/sfxuno.hxx>

#include <cppuhelper/interfacecontainer.hxx>
#include <svl/lstner.hxx>
#include <unotools/eventcfg.hxx>

namespace comphelper
{
    class NamedValueCollection;
}

class SfxObjectShell;
class SvxMacro;

//--------------------------------------------------------------------------------------------------------

class SfxEvents_Impl : public ::cppu::WeakImplHelper2< css::container::XNameReplace, css::document::XEventListener  >
{
    css::uno::Sequence< OUString >     maEventNames;
    css::uno::Sequence< css::uno::Any >                 maEventData;
    css::uno::Reference< css::document::XEventBroadcaster >  mxBroadcaster;
    ::osl::Mutex                    maMutex;
    SfxObjectShell                 *mpObjShell;

    sal_Bool                    Warn_Impl( const String& );

public:
                                SfxEvents_Impl( SfxObjectShell* pShell,
                                                css::uno::Reference< css::document::XEventBroadcaster > xBroadcaster );
                               ~SfxEvents_Impl();

    //  --- XNameReplace ---
    virtual void SAL_CALL       replaceByName( const OUString & aName, const css::uno::Any & aElement )
                                    throw( css::lang::IllegalArgumentException, css::container::NoSuchElementException,
                                           css::lang::WrappedTargetException, css::uno::RuntimeException );

    //  --- XNameAccess ( parent of XNameReplace ) ---
    virtual css::uno::Any SAL_CALL        getByName( const OUString& aName )
                                    throw( css::container::NoSuchElementException, css::lang::WrappedTargetException,
                                           css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL   hasByName( const OUString& aName ) throw ( css::uno::RuntimeException );

    //  --- XElementAccess ( parent of XNameAccess ) ---
    virtual css::uno::Type SAL_CALL    getElementType() throw ( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL   hasElements() throw ( css::uno::RuntimeException );

    // --- ::document::XEventListener ---
    virtual void SAL_CALL       notifyEvent( const css::document::EventObject& aEvent )
                                    throw( css::uno::RuntimeException );

    // --- ::lang::XEventListener ---
    virtual void SAL_CALL       disposing( const css::lang::EventObject& Source )
                                    throw( css::uno::RuntimeException );

    static SvxMacro*            ConvertToMacro( const css::uno::Any& rElement, SfxObjectShell* pDoc, sal_Bool bNormalizeMacro );
    static void                 NormalizeMacro( const css::uno::Any& rIn, css::uno::Any& rOut, SfxObjectShell* pDoc );
    static void                 NormalizeMacro(
                                    const ::comphelper::NamedValueCollection& i_eventDescriptor,
                                    ::comphelper::NamedValueCollection& o_normalizedDescriptor,
                                    SfxObjectShell* i_document );
};

//=============================================================================
struct ModelCollectionMutexBase
{
    public:
        ::osl::Mutex m_aLock;
};

//=============================================================================
typedef ::std::vector< css::uno::Reference< css::frame::XModel > > TModelList;

//=============================================================================
class ModelCollectionEnumeration : public ModelCollectionMutexBase
                                 , public ::cppu::WeakImplHelper1< css::container::XEnumeration >
{

    //-------------------------------------------------------------------------
    // member
    //-------------------------------------------------------------------------
    private:
        TModelList m_lModels;
        TModelList::iterator m_pEnumerationIt;

    //-------------------------------------------------------------------------
    // native interface
    //-------------------------------------------------------------------------
    public:
        ModelCollectionEnumeration();
        virtual ~ModelCollectionEnumeration();
        void setModelList(const TModelList& rList);

    //-------------------------------------------------------------------------
    // uno interface
    //-------------------------------------------------------------------------
    public:

        // css.container.XEnumeration
        virtual sal_Bool SAL_CALL hasMoreElements()
            throw(css::uno::RuntimeException);

        virtual css::uno::Any SAL_CALL nextElement()
            throw(css::container::NoSuchElementException,
                  css::lang::WrappedTargetException     ,
                  css::uno::RuntimeException            );
};

//=============================================================================
//TODO: remove support of obsolete document::XEventBroadcaster/Listener
class SfxGlobalEvents_Impl : public ModelCollectionMutexBase
                           , public ::cppu::WeakImplHelper3< css::lang::XServiceInfo
                                                           , css::frame::XGlobalEventBroadcaster
                                                           , css::document::XEventListener
                                                            >
{
    css::uno::Reference< css::container::XNameReplace > m_xEvents;
    css::uno::Reference< css::document::XEventListener > m_xJobExecutorListener;
    ::cppu::OInterfaceContainerHelper m_aLegacyListeners;
    ::cppu::OInterfaceContainerHelper m_aDocumentListeners;
    TModelList m_lModels;
    GlobalEventConfig* pImp;

public:
    SfxGlobalEvents_Impl(const css::uno::Reference < css::uno::XComponentContext >& rxContext);
    virtual ~SfxGlobalEvents_Impl();

    SFX_DECL_XSERVICEINFO

    // css.document.XEventBroadcaster
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents()
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::document::XEventListener >& xListener)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::document::XEventListener >& xListener)
        throw(css::uno::RuntimeException);

    // css.document.XDocumentEventBroadcaster
    virtual void SAL_CALL addDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeDocumentEventListener( const css::uno::Reference< css::document::XDocumentEventListener >& _Listener ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL notifyDocumentEvent( const OUString& _EventName, const css::uno::Reference< css::frame::XController2 >& _ViewController, const css::uno::Any& _Supplement ) throw (css::lang::IllegalArgumentException, css::lang::NoSupportException, css::uno::RuntimeException);

    // css.document.XEventListener
    virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent)
        throw(css::uno::RuntimeException);

    // css.document.XDocumentEventListener
    virtual void SAL_CALL documentEventOccured( const css::document::DocumentEvent& Event ) throw (css::uno::RuntimeException);

    // css.container.XSet
    virtual sal_Bool SAL_CALL has(const css::uno::Any& aElement)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL insert(const css::uno::Any& aElement)
        throw(css::lang::IllegalArgumentException  ,
              css::container::ElementExistException,
              css::uno::RuntimeException           );

    virtual void SAL_CALL remove(const css::uno::Any& aElement)
        throw(css::lang::IllegalArgumentException   ,
              css::container::NoSuchElementException,
              css::uno::RuntimeException            );

    // css.container.XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration()
        throw(css::uno::RuntimeException);

    // css.container.XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw(css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasElements()
        throw(css::uno::RuntimeException);

    // css.lang.XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
        throw(css::uno::RuntimeException);

private:

    // threadsafe
    void implts_notifyJobExecution(const css::document::EventObject& aEvent);
    void implts_checkAndExecuteEventBindings(const css::document::DocumentEvent& aEvent);
    void implts_notifyListener(const css::document::DocumentEvent& aEvent);

    // not threadsafe
    TModelList::iterator impl_searchDoc(const css::uno::Reference< css::frame::XModel >& xModel);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

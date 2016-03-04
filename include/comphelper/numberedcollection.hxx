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

#ifndef INCLUDED_COMPHELPER_NUMBEREDCOLLECTION_HXX
#define INCLUDED_COMPHELPER_NUMBEREDCOLLECTION_HXX

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>

#include <functional>
#include <unordered_map>
#include <vector>


namespace comphelper{

/** @short  defines a collection of UNO components, where every component will get its own unique number.

    @descr  Such number will be unique at runtime only... but it supports fragmentation.
            Note: This collection uses weak references only to know her components.
            So lifetime of these components must be controlled outside.

    @threadsafe
 */
class COMPHELPER_DLLPUBLIC NumberedCollection : private ::cppu::BaseMutex
                                              , public  ::cppu::WeakImplHelper< css::frame::XUntitledNumbers >
{

    // types, const
    private:

        struct TNumberedItem
        {
            css::uno::WeakReference< css::uno::XInterface > xItem;
            ::sal_Int32 nNumber;
        };

        typedef std::unordered_map<
                    long,
                    TNumberedItem  > TNumberedItemHash;

        typedef ::std::vector< long > TDeadItemList;


    // interface
    public:


        /** @short  lightweight constructor.
         */
        NumberedCollection();


        /** @short  free all internally used resources.
         */
        virtual ~NumberedCollection();


        /** set an outside component which uses this container and must be set
            as source of all broadcasted messages, exceptions.

            It's holded weak only so we do not need any complex dispose sessions.

            Note: Passing NULL as parameter will be allowed. It will reset the internal
            member reference only.

            @param  xOwner
                    the new owner of this collection.
         */
        void setOwner (const css::uno::Reference< css::uno::XInterface >& xOwner);


        /** set the localized prefix to be used for untitled components.

            Localization has to be done outside. This container will return
            those value then. There are no further checks. Its up to you to define
            a suitable string here :-)

            @param  sPrefix
                    the new prefix for untitled components.
         */
        void setUntitledPrefix(const OUString& sPrefix);


        /** @see css.frame.XUntitledNumbers */
        virtual ::sal_Int32 SAL_CALL leaseNumber(const css::uno::Reference< css::uno::XInterface >& xComponent)
            throw (css::lang::IllegalArgumentException,
                   css::uno::RuntimeException, std::exception         ) override;


        /** @see css.frame.XUntitledNumbers */
        virtual void SAL_CALL releaseNumber(::sal_Int32 nNumber)
            throw (css::lang::IllegalArgumentException,
                   css::uno::RuntimeException, std::exception         ) override;


        /** @see css.frame.XUntitledNumbers */
        virtual void SAL_CALL releaseNumberForComponent(const css::uno::Reference< css::uno::XInterface >& xComponent)
            throw (css::lang::IllegalArgumentException,
                   css::uno::RuntimeException, std::exception         ) override;


        /** @see css.frame.XUntitledNumbers */
        virtual OUString SAL_CALL getUntitledPrefix()
            throw (css::uno::RuntimeException, std::exception) override;


    // internal
    private:


        /** @short  trys to find an unique number not already used within this collection.

            @descr  It reuses the smallest number which isn't used by any component
                    of this collection. (fragmentation!) If collection is full (means there
                    is no free number) the special value INVALID_NUMBER will be returned.

            @note   Those method can't be called within a multithreaded environment ..
                    Because such number wont be "reserved" for the calli of these method
                    it can happen that two calls returns the same number (reasoned by the fact that first calli
                    doesn't used the returned number already.

                    So the outside code has to make sure that retrieving and using of those number
                    will be an atomic operation.

            @return an unique number or special value INVALID_NUMBER if collection is full.
         */
        ::sal_Int32 impl_searchFreeNumber ();

        static void impl_cleanUpDeadItems (      TNumberedItemHash& lItems    ,
                                           const TDeadItemList&     lDeadItems);


    // member
    private:

        /// localized string to be used for untitled components
        OUString m_sUntitledPrefix;

        /// cache of all "leased numbers" and its bound components
        TNumberedItemHash m_lComponents;

        /// used as source of broadcasted messages or exceptions (can be null !)
        css::uno::WeakReference< css::uno::XInterface > m_xOwner;
};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_NUMBEREDCOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

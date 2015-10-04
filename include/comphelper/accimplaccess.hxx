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

#ifndef INCLUDED_COMPHELPER_ACCIMPLACCESS_HXX
#define INCLUDED_COMPHELPER_ACCIMPLACCESS_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/comphelperdllapi.h>
#include <memory>

namespace com { namespace sun { namespace star { namespace accessibility {
    class XAccessible;
    class XAccessibleContext;
}}}}

namespace comphelper
{
    //= OAccessibleImplementationAccess

    typedef ::cppu::ImplHelper  <   css::lang::XUnoTunnel
                                >   OAccImpl_Base;
    struct OAccImpl_Impl;

    /** This is a helper class which allows accessing several aspects of the implementation
        of an AccessibleContext.

        <p>For instance, when you want to implement a context which can be re-parented, you:
            <ul><li>derive your class from OAccessibleImplementationAccess</li>
                <li>use <code>setAccessibleParent( <em>component</em>, <em>new_parent</em> )</code>
            </ul>
        </p>

        <p>Another aspect which can be controlled from the outside are states. If you have a class which
        has only partial control over it's states, you may consider deriving from OAccessibleImplementationAccess.<br/>
        For instance, say you have an implementation (say component A) which is <em>unable</em> to know or to
        determine if the represented object is selected, but another component (say B) which uses A (and integrates
        it into a tree of accessibility components) is.<br/>
        In this case, if A is derived from OAccessibleImplementationAccess, B can manipulate this
        foreign-controlled state flag "SELECTED" by using the static helper methods on this class.</p>

        <p>Please note that the support for foreign controlled states is rather restrictive: You can't have states
        which <em>may be</em> controlled by a foreign instances. This is implied by the fact that a derived
        class can ask for states which are <em>set</em> only, not for the ones which are <em>reset</em> currently.
        </p>
    */
    class COMPHELPER_DLLPUBLIC OAccessibleImplementationAccess : public OAccImpl_Base
    {
    private:
        std::unique_ptr<OAccImpl_Impl>  m_pImpl;

    protected:
        /// retrieves the parent previously set via <method>setAccessibleParent</method>
        const css::uno::Reference< css::accessibility::XAccessible >&
                implGetForeignControlledParent( ) const;

        /** retrieves the set of currently set states which are controlled by a foreign instance
        @return
            a bit mask, where a set bit 2^n means that the AccessibleStateType n has been set
        */
        sal_Int64   implGetForeignControlledStates( ) const;

    protected:
        OAccessibleImplementationAccess( );
        virtual ~OAccessibleImplementationAccess( );

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& _rIdentifier ) throw (css::uno::RuntimeException, std::exception) override;

    public:

    private:
        COMPHELPER_DLLPRIVATE static const css::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
    };

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_ACCIMPLACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

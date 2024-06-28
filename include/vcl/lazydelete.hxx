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

#pragma once

#include <vcl/dllapi.h>

#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ref.hxx>

#include <optional>

namespace vcl
{
    /*
    You may not access vcl objects after DeInitVCL has been called this includes their destruction
    therefore disallowing the existence of static vcl object like e.g. a static BitmapEx
    To work around this use DeleteOnDeinit<BitmapEx> which will allow you to have a static object container,
    that will have its contents destroyed on DeinitVCL. The single drawback is that you need to check on the
    container object whether it still contains content before actually accessing it.

    caveat: when constructing a vcl object, you certainly want to ensure that InitVCL has run already.
    However this is not necessarily the case when using a class static member or a file level static variable.
    In these cases make judicious use of the set() method of DeleteOnDeinit, but beware of the changing
    ownership.

    example use case: use a lazy initialized on call BitmapEx in a paint method. Of course a paint method
    would not normally be called after DeInitVCL anyway, so the check might not be necessary in a
    Window::Paint implementation, but always checking is a good idea.

    SomeWindow::Paint()
    {
        static vcl::DeleteOnDeinit< BitmapEx > aBmp( ... );

        if( aBmp.get() ) // check whether DeInitVCL has been called already
            DrawBitmapEx( Point( 10, 10 ), *aBmp );
    }
    */

    class VCL_DLLPUBLIC DeleteOnDeinitBase
    {
    public:
        static void SAL_DLLPRIVATE ImplDeleteOnDeInit();
        virtual ~DeleteOnDeinitBase();
    protected:
        static void addDeinitContainer( DeleteOnDeinitBase* i_pContainer );

        virtual void doCleanup() = 0;
    };

    enum class DeleteOnDeinitFlag { Empty };

    template < typename T >
    class DeleteOnDeinit final : public DeleteOnDeinitBase
    {
        std::optional<T> m_pT;
        virtual void doCleanup() override { m_pT.reset(); }
    public:
        template <class... Args >
        DeleteOnDeinit(Args&&... args )
        {
            m_pT.emplace(args...);
            addDeinitContainer( this );
        }
        DeleteOnDeinit(DeleteOnDeinitFlag)
        {
            addDeinitContainer( this );
        }

        // get contents
        T* get() { return m_pT ? &*m_pT : nullptr; }

        // set contents, returning old contents
        // ownership is transferred !
        template <class... Args >
        std::optional<T> set(Args&&... args)
        {
            auto pOld = std::move(m_pT);
            m_pT.emplace(args...);
            return pOld;
        }
    };

    /** Similar to DeleteOnDeinit, the DeleteUnoReferenceOnDeinit
        template class makes sure that a static UNO object is disposed
        and released at the right time.

        Use like
            static DeleteUnoReferenceOnDeinit<lang::XMultiServiceFactory>
                xStaticFactory (\<create factory object>);
            Reference<lang::XMultiServiceFactory> xFactory (xStaticFactory.get());
            if (xFactory.is())
                \<do something with xFactory>
    */
    template <typename I>
    class DeleteUnoReferenceOnDeinit final : public vcl::DeleteOnDeinitBase
    {
        css::uno::Reference<I> m_xI;
        virtual void doCleanup() override { set(nullptr); }
    public:
        DeleteUnoReferenceOnDeinit(css::uno::Reference<I> _xI ) : m_xI(std::move( _xI )) {
            addDeinitContainer( this ); }

        css::uno::Reference<I> get() { return m_xI; }

        void set (const css::uno::Reference<I>& r_xNew )
        {
            css::uno::Reference< css::lang::XComponent> xComponent (m_xI, css::uno::UNO_QUERY);
            m_xI = r_xNew;
            if (xComponent.is()) try
            {
                xComponent->dispose();
            }
            catch( css::uno::Exception& )
            {
            }
        }
    };

    /** Similar to DeleteOnDeinit, the DeleteRtlReferenceOnDeinit
        template class makes sure that a static rtl::Reference managed object is disposed
        and released at the right time.

        Use like
            static DeleteUnoReferenceOnDeinit<Foo> xStaticFactory (new Foo);
            rtl::Reference<Foo> xFactory (xStaticFactory.get());
            if (xFactory.is())
                \<do something with xFactory>
    */
    template <typename I>
    class DeleteRtlReferenceOnDeinit final : public vcl::DeleteOnDeinitBase
    {
        rtl::Reference<I> m_xI;
        virtual void doCleanup() override { set(nullptr); }
    public:
        DeleteRtlReferenceOnDeinit(rtl::Reference<I> _xI ) : m_xI(std::move( _xI )) {
            addDeinitContainer( this ); }

        rtl::Reference<I> get() { return m_xI; }

        void set (const rtl::Reference<I>& r_xNew )
        {
            m_xI = r_xNew;
        }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

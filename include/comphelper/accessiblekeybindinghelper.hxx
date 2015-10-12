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

#ifndef INCLUDED_COMPHELPER_ACCESSIBLEKEYBINDINGHELPER_HXX
#define INCLUDED_COMPHELPER_ACCESSIBLEKEYBINDINGHELPER_HXX

#include <com/sun/star/accessibility/XAccessibleKeyBinding.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>

#include <vector>

#include <comphelper/comphelperdllapi.h>


namespace comphelper
{



    // OAccessibleKeyBindingHelper


    typedef ::cppu::WeakImplHelper <   ::com::sun::star::accessibility::XAccessibleKeyBinding
                                   >   OAccessibleKeyBindingHelper_Base;

    /** a helper class for implementing an accessible keybinding
     */
    class COMPHELPER_DLLPUBLIC OAccessibleKeyBindingHelper : public OAccessibleKeyBindingHelper_Base
    {
    private:
        typedef ::std::vector< ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke > > KeyBindings;

        KeyBindings     m_aKeyBindings;

    protected:
        ::osl::Mutex    m_aMutex;

        virtual ~OAccessibleKeyBindingHelper();

    public:
        OAccessibleKeyBindingHelper();
        OAccessibleKeyBindingHelper( const OAccessibleKeyBindingHelper& rHelper );

        void AddKeyBinding( const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke >& rKeyBinding ) throw (::com::sun::star::uno::RuntimeException);
        void AddKeyBinding( const ::com::sun::star::awt::KeyStroke& rKeyStroke ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleKeyBinding
        virtual sal_Int32 SAL_CALL getAccessibleKeyBindingCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::awt::KeyStroke > SAL_CALL getAccessibleKeyBinding( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_ACCESSIBLEKEYBINDINGHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

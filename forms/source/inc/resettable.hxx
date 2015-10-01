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

#ifndef INCLUDED_FORMS_SOURCE_INC_RESETTABLE_HXX
#define INCLUDED_FORMS_SOURCE_INC_RESETTABLE_HXX

#include <com/sun/star/form/XResetListener.hpp>

#include <cppuhelper/interfacecontainer.hxx>

namespace cppu
{
    class OWeakObject;
}


namespace frm
{

    class ResetHelper
    {
    public:
        ResetHelper( ::cppu::OWeakObject& _parent, ::osl::Mutex& _mutex )
            :m_rParent( _parent )
            ,m_aResetListeners( _mutex )
        {
        }

        // XReset equivalents
        void addResetListener( const css::uno::Reference< css::form::XResetListener >& _listener );
        void removeResetListener( const css::uno::Reference< css::form::XResetListener >& _listener );

        // calling listeners
        bool approveReset();
        void notifyResetted();
        void disposing();

    private:
        ::cppu::OWeakObject&                m_rParent;
        ::cppu::OInterfaceContainerHelper   m_aResetListeners;
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_INC_RESETTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <comphelper/servicedecl.hxx>

namespace comphelper::service_decl {
template <typename ImplT_, typename WithArgsT = with_args<false> >
struct vba_service_class_ : public serviceimpl_base< detail::OwnServiceImpl<ImplT_>, WithArgsT >
{
    typedef serviceimpl_base< detail::OwnServiceImpl<ImplT_>, WithArgsT > baseT;
    /** Default ctor.  Implementation class without args, expecting
        component context as single argument.
    */
    vba_service_class_() : baseT() {}
    template <typename PostProcessFuncT>
    /** Ctor to pass a post processing function/functor.

        @tpl PostProcessDefaultT let your compiler deduce this
        @param postProcessFunc function/functor that gets the yet unacquired
                               ImplT_ pointer returning a
                               uno::Reference<uno::XInterface>
    */
    explicit vba_service_class_( PostProcessFuncT const& postProcessFunc ) : baseT( postProcessFunc ) {}
};

} // namespace service_decl
} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: helperdecl.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef OOVBAAPI_SERV_DECL
#define OOVBAAPI_SERV_DECL
#include <comphelper/servicedecl.hxx>
#include <ooo/vba/XHelperInterface.hpp>

namespace comphelper {
namespace service_decl {
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
#endif

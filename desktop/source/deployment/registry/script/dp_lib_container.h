/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#if ! defined INCLUDED_DP_LIB_CONTAINER_H
#define INCLUDED_DP_LIB_CONTAINER_H

#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star {
    namespace uno {
        class XComponentContext;
    }
    namespace ucb {
        class XCommandEnvironment;
    }
}}}

namespace rtl {
    class OUString;
}

namespace css = ::com::sun::star;

namespace dp_registry {
namespace backend {
namespace script {

//==============================================================================
class LibraryContainer
{
public:
    static ::rtl::OUString get_libname(
        ::rtl::OUString const & url,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        css::uno::Reference<css::uno::XComponentContext> const & xContext );
};

}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

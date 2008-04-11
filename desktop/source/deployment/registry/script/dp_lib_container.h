/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_lib_container.h,v $
 * $Revision: 1.8 $
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

#include "rtl/ustring.hxx"
#include "osl/mutex.hxx"
#include "xmlscript/xmllib_imexp.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include <hash_map>
#include <list>


namespace css = ::com::sun::star;

namespace dp_registry {
namespace backend {
namespace script {

typedef ::std::list< ::xmlscript::LibDescriptor > t_descr_list;

//==============================================================================
class LibraryContainer
{
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    bool m_immediateFlush;

    typedef ::std::hash_map<
        ::rtl::OUString,
            ::xmlscript::LibDescriptor, ::rtl::OUStringHash > t_libs_map;

    ::rtl::OUString m_container_url;
    ::osl::Mutex & m_mutex;
    mutable t_libs_map m_map;
    mutable bool m_inited;
    mutable bool m_modified;

public:
    inline ~LibraryContainer()
        { OSL_ENSURE( !m_inited || !m_modified, "### no flush!" ); }
    inline LibraryContainer(
        ::rtl::OUString const & container_url,
        ::osl::Mutex & mutex,
        css::uno::Reference<css::uno::XComponentContext> const & xContext,
        bool immediateFlush = true )
        : m_xContext( xContext ),
          m_immediateFlush( immediateFlush ),
          m_container_url( container_url ),
          m_mutex( mutex ),
          m_inited( false ),
          m_modified( false )
        {}

    static ::rtl::OUString get_libname(
        ::rtl::OUString const & url,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        css::uno::Reference<css::uno::XComponentContext> const & xContext );

    void init(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        const;
    void flush(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        const;
    bool insert(
        ::rtl::OUString const & libname,
        ::rtl::OUString const & url,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv );
    bool remove(
        ::rtl::OUString const & libname,
        ::rtl::OUString const & url,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
        bool exact = true );
    bool has(
        ::rtl::OUString const & libname,
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        const;
    t_descr_list getLibs(
        css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv )
        const;
};

}
}
}

#endif

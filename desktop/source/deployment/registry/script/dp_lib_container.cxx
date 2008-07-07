/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_lib_container.cxx,v $
 * $Revision: 1.12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "dp_script.hrc"
#include "dp_resource.h"
#include "dp_xml.h"
#include "dp_lib_container.h"
#include "ucbhelper/content.hxx"


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using ::rtl::OUString;

namespace dp_registry {
namespace backend {
namespace script {

namespace {
struct StrCannotDetermineLibName : public StaticResourceString<
    StrCannotDetermineLibName, RID_STR_CANNOT_DETERMINE_LIBNAME> {};
}

//______________________________________________________________________________
OUString LibraryContainer::get_libname(
    OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    Reference<XComponentContext> const & xContext )
{
    ::xmlscript::LibDescriptor import;
    ::ucbhelper::Content ucb_content( url, xCmdEnv );
    xml_parse( ::xmlscript::importLibrary( import ), ucb_content, xContext );

    if (import.aName.getLength() == 0) {
        throw Exception( StrCannotDetermineLibName::get(),
                         Reference<XInterface>() );
    }
    return import.aName;
}

}
}
}


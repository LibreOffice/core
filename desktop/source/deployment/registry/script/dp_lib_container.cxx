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


#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <strings.hrc>
#include <dp_shared.hxx>
#include <dp_xml.h>
#include "dp_lib_container.h"

#include <rtl/ustring.hxx>
#include <ucbhelper/content.hxx>
#include <xmlscript/xmllib_imexp.hxx>


using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;

namespace dp_registry::backend::script {

namespace {
    OUString StrCannotDetermineLibName() { return DpResId(RID_STR_CANNOT_DETERMINE_LIBNAME); }
}

OUString LibraryContainer::get_libname(
    OUString const & url,
    Reference<XCommandEnvironment> const & xCmdEnv,
    Reference<XComponentContext> const & xContext )
{
    ::xmlscript::LibDescriptor import;
    ::ucbhelper::Content ucb_content( url, xCmdEnv, xContext );
    xml_parse( ::xmlscript::importLibrary( import ), ucb_content, xContext );

    if (import.aName.isEmpty()) {
        throw Exception( StrCannotDetermineLibName(),
                         Reference<XInterface>() );
    }
    return import.aName;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


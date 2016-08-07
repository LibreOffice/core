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


#ifndef _XMLSCRIPT_XMLMOD_IMEXP_HXX_
#define _XMLSCRIPT_XMLMOD_IMEXP_HXX_

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <xmlscript/xmlns.h>
#include "xmlscript/xcrdllapi.h"

namespace xmlscript
{

//==============================================================================
// Script module import/export
// HACK C++ struct to transport info. Later the container
// itself should do the export/import and use exportet XML
// functionality from xmlscript
struct XCR_DLLPUBLIC ModuleDescriptor
{
    ::rtl::OUString aName;
    ::rtl::OUString aLanguage;
    ::rtl::OUString aCode;
    ::rtl::OUString aModuleType; // VBA
};

XCR_DLLPUBLIC void
SAL_CALL exportScriptModule(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const ModuleDescriptor& rMod )
        SAL_THROW( (::com::sun::star::uno::Exception) );

XCR_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importScriptModule( ModuleDescriptor& rMod )
    SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif

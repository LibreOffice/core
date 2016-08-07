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


#ifndef _XMLSCRIPT_XMLLIB_IMEXP_HXX_
#define _XMLSCRIPT_XMLLIB_IMEXP_HXX_

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include "xmlscript/xmlns.h"
#include "xmlscript/xcrdllapi.h"

namespace xmlscript
{

//==============================================================================
// Library container export
// HACK C++ struct to transport info. Later the container
// itself should do the export/import and use exportet XML
// functionality from xmlscript
struct XCR_DLLPUBLIC LibDescriptor
{
    ::rtl::OUString aName;
    ::rtl::OUString aStorageURL;
    sal_Bool bLink;
    sal_Bool bReadOnly;
    sal_Bool bPasswordProtected;
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aElementNames;
    sal_Bool bPreload;
};

struct XCR_DLLPUBLIC LibDescriptorArray
{
    LibDescriptor* mpLibs;
    sal_Int32 mnLibCount;

    LibDescriptorArray( void ) { mpLibs = NULL; mnLibCount = 0; }
    LibDescriptorArray( sal_Int32 nLibCount );

    ~LibDescriptorArray();

};

XCR_DLLPUBLIC void
SAL_CALL exportLibraryContainer(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptorArray* pLibArray )
        SAL_THROW( (::com::sun::star::uno::Exception) );

XCR_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer( LibDescriptorArray* pLibArray )
        SAL_THROW( (::com::sun::star::uno::Exception) );


XCR_DLLPUBLIC void
SAL_CALL exportLibrary(
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) );

XCR_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL importLibrary( LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) );

}

#endif

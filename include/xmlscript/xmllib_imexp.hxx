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
#ifndef INCLUDED_XMLSCRIPT_XMLLIB_IMEXP_HXX
#define INCLUDED_XMLSCRIPT_XMLLIB_IMEXP_HXX

#include <com/sun/star/xml/sax/XWriter.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <xmlscript/xmlns.h>
#include <xmlscript/xmlscriptdllapi.h>

namespace xmlscript
{


// Library container export
// HACK C++ struct to transport info. Later the container
// itself should do the export/import and use exportet XML
// functionality from xmlscript
struct XMLSCRIPT_DLLPUBLIC LibDescriptor
{
    OUString aName;
    OUString aStorageURL;
    bool bLink;
    bool bReadOnly;
    bool bPasswordProtected;
    css::uno::Sequence< OUString > aElementNames;
    bool bPreload;
};

struct XMLSCRIPT_DLLPUBLIC LibDescriptorArray
{
    LibDescriptor* mpLibs;
    sal_Int32 mnLibCount;

    LibDescriptorArray() { mpLibs = NULL; mnLibCount = 0; }
    LibDescriptorArray( sal_Int32 nLibCount );

    ~LibDescriptorArray();

};

XMLSCRIPT_DLLPUBLIC void
SAL_CALL exportLibraryContainer(
    css::uno::Reference< css::xml::sax::XWriter > const & xOut,
    const LibDescriptorArray* pLibArray );

XMLSCRIPT_DLLPUBLIC css::uno::Reference<
    css::xml::sax::XDocumentHandler >
SAL_CALL importLibraryContainer( LibDescriptorArray* pLibArray );


XMLSCRIPT_DLLPUBLIC void
SAL_CALL exportLibrary(
    css::uno::Reference< css::xml::sax::XWriter > const & xOut,
    const LibDescriptor& rLib );

XMLSCRIPT_DLLPUBLIC css::uno::Reference<
    css::xml::sax::XDocumentHandler >
SAL_CALL importLibrary( LibDescriptor& rLib );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

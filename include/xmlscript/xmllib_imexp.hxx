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

#include <com/sun/star/uno/Sequence.hxx>

#include <xmlscript/xmlscriptdllapi.h>

#include <memory>

namespace com::sun::star::xml::sax { class XDocumentHandler; }
namespace com::sun::star::xml::sax { class XWriter; }

namespace xmlscript
{


// Library container export
// HACK C++ struct to transport info. Later the container
// itself should do the export/import and use exported XML
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
    std::unique_ptr<LibDescriptor[]> mpLibs;
    sal_Int32 mnLibCount;

    LibDescriptorArray() { mpLibs = nullptr; mnLibCount = 0; }
    LibDescriptorArray( sal_Int32 nLibCount );

    ~LibDescriptorArray();

};

XMLSCRIPT_DLLPUBLIC void
exportLibraryContainer(
    css::uno::Reference< css::xml::sax::XWriter > const & xOut,
    const LibDescriptorArray* pLibArray );

XMLSCRIPT_DLLPUBLIC css::uno::Reference<
    css::xml::sax::XDocumentHandler >
importLibraryContainer( LibDescriptorArray* pLibArray );


XMLSCRIPT_DLLPUBLIC void
exportLibrary(
    css::uno::Reference< css::xml::sax::XWriter > const & xOut,
    const LibDescriptor& rLib );

XMLSCRIPT_DLLPUBLIC css::uno::Reference<
    css::xml::sax::XDocumentHandler >
importLibrary( LibDescriptor& rLib );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

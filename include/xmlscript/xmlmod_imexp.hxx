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
#ifndef INCLUDED_XMLSCRIPT_XMLMOD_IMEXP_HXX
#define INCLUDED_XMLSCRIPT_XMLMOD_IMEXP_HXX

#include <com/sun/star/xml/sax/XWriter.hpp>
#include <xmlscript/xmlns.h>
#include <xmlscript/xmlscriptdllapi.h>

namespace xmlscript
{


// Script module import/export
// HACK C++ struct to transport info. Later the container
// itself should do the export/import and use exported XML
// functionality from xmlscript
struct ModuleDescriptor
{
    OUString aName;
    OUString aLanguage;
    OUString aCode;
    OUString aModuleType; // VBA
};

XMLSCRIPT_DLLPUBLIC void
SAL_CALL exportScriptModule(
    css::uno::Reference< css::xml::sax::XWriter > const & xOut,
    const ModuleDescriptor& rMod );

XMLSCRIPT_DLLPUBLIC css::uno::Reference< css::xml::sax::XDocumentHandler >
SAL_CALL importScriptModule( ModuleDescriptor& rMod );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

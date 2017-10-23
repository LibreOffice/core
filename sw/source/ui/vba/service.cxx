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
#include <cppuhelper/implementationentry.hxx>
#include <comphelper/servicedecl.hxx>

#include "service.hxx"

// component exports

namespace sdecl = comphelper::service_decl;

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL vbaswobj_component_getFactory(
    const sal_Char * pImplName, void *, void *)
{
    void* pRet = sdecl::component_getFactoryHelper(pImplName,
            {&globals::serviceDecl, &::document::serviceDecl,
             &wrapformat::serviceDecl, &vbaeventshelper::serviceDecl} );
    SAL_INFO("sw", "Ret is " << pRet);
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

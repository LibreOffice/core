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

#ifndef SC_SCDLL_HXX
#define SC_SCDLL_HXX

class SfxMedium;
class SfxFilter;

#include <sfx2/sfxdefs.hxx>
#include <sfx2/module.hxx>
#include <sfx2/docfac.hxx>

#include "scdllapi.h"

/**
 * This class is a wrapper for a Load-On-Demand-DLL. One instance per
 * SfxApplication will be created for the runtime of
 * SfxApplication-subclass::Main().
 *
 * Remember: Do export this class! It is used by the application.
 */
class ScDLL
{
public:
                    /// Ctor/Dtor must be linked to the application
                    ScDLL();
                    ~ScDLL();

                    /// static-init/exit-code must be linked to the application
    static void     LibInit();  ///< called from SfxApplication-subclass::Init()
    static void     LibExit();  ///< called from SfxApplication-subclass::Exit()
    static void     PreExit();  // muss vor LibExit gerufen werden

                    /// DLL-init/exit-code must be linked to the DLL only
    static SC_DLLPUBLIC void     Init();     ///< called directly after loading the DLL

    static sal_uLong    DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter,
                                    SfxFilterFlags nMust, SfxFilterFlags nDont );
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

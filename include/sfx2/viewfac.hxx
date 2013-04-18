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
#ifndef _VIEWFAC_HXX
#define _VIEWFAC_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <tools/string.hxx>
#include <tools/resid.hxx>

class SfxViewFrame;
class SfxViewShell;
class Window;

typedef SfxViewShell* (*SfxViewCtor)(SfxViewFrame*, SfxViewShell*);

// CLASS -----------------------------------------------------------------
class SFX2_DLLPUBLIC SfxViewFactory
{
public:
    SfxViewFactory( SfxViewCtor fnC,
                    sal_uInt16 nOrdinal, const sal_Char* asciiViewName );

    SfxViewShell *CreateInstance(SfxViewFrame *pViewFrame, SfxViewShell *pOldSh);
    sal_uInt16        GetOrdinal() const { return nOrd; }

    /// returns a legacy view name. This is "view" with an appended ordinal/ID.
    String        GetLegacyViewName() const;

    /** returns a API-compatible view name.

        For details on which view names are specified, see the XModel2.getAvailableViewControllerNames
        documentation.
    */
    String        GetAPIViewName() const;

private:
    SfxViewCtor fnCreate;
    sal_uInt16      nOrd;
    const String    m_sViewName;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

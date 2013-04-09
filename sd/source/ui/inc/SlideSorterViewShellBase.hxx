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

#ifndef SD_SLIDE_SORTER_VIEW_SHELL_BASE_HXX
#define SD_SLIDE_SORTER_VIEW_SHELL_BASE_HXX

#include "ImpressViewShellBase.hxx"

namespace sd {

/** This class exists to be able to register a factory that
    creates a slide sorter view shell as default.
*/
class SlideSorterViewShellBase
    : public ImpressViewShellBase
{
public:
    TYPEINFO();
    SFX_DECL_VIEWFACTORY(SlideSorterViewShellBase);

    /** This constructor is used by the view factory of the SFX
        macros.
    */
    SlideSorterViewShellBase (SfxViewFrame *_pFrame, SfxViewShell *_pOldShell)
        : ImpressViewShellBase (_pFrame, _pOldShell) {}
    virtual ~SlideSorterViewShellBase (void) {}
};

/** This class exists to be able to register a factory that
    creates a slide jockey view shell as default.
*/
class SlideJockeyViewShellBase
    : public ImpressViewShellBase
{
public:
    TYPEINFO();
    SFX_DECL_VIEWFACTORY(SlideJockeyViewShellBase);

    /** This constructor is used by the view factory of the SFX
        macros.
    */
    SlideJockeyViewShellBase (SfxViewFrame *_pFrame, SfxViewShell *_pOldShell)
        : ImpressViewShellBase (_pFrame, _pOldShell) {}
    virtual ~SlideJockeyViewShellBase (void) {}
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

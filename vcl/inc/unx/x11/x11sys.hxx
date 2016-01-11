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

#ifndef INCLUDED_VCL_INC_UNX_X11_X11SYS_HXX
#define INCLUDED_VCL_INC_UNX_X11_X11SYS_HXX

#include <unx/gensys.h>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <vclpluginapi.h>

class VCLPLUG_GEN_PUBLIC X11SalSystem : public SalGenericSystem
{
public:
    X11SalSystem() {}
    virtual ~X11SalSystem();

    // override pure virtual methods
    virtual unsigned int GetDisplayScreenCount() override;
    virtual bool         IsUnifiedDisplay() override;
    virtual unsigned int GetDisplayBuiltInScreen() override;
    virtual Rectangle    GetDisplayScreenPosSizePixel( unsigned int nScreen ) override;
    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::list< OUString >& rButtons,
                                  int nDefButton ) override;
};

#endif // INCLUDED_VCL_INC_UNX_X11_X11SYS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_X11SYS_H
#define _SV_X11SYS_H

#include <tools/solar.h>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>
#include <generic/gensys.h>
#include <vclpluginapi.h>

class VCLPLUG_GEN_PUBLIC X11SalSystem : public SalGenericSystem
{
public:
    X11SalSystem() {}
    virtual ~X11SalSystem();

    // overload pure virtual methods
    virtual unsigned int GetDisplayScreenCount();
    virtual bool         IsUnifiedDisplay();
    virtual unsigned int GetDisplayBuiltInScreen();
    virtual Rectangle    GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle    GetDisplayScreenWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetDisplayScreenName( unsigned int nScreen );
    virtual int ShowNativeDialog( const rtl::OUString& rTitle,
                                  const rtl::OUString& rMessage,
                                  const std::list< rtl::OUString >& rButtons,
                                  int nDefButton );
};

#endif // _SV_X11SYS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

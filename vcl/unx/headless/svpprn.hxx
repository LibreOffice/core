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

#ifndef _SVP_SVPPRN_HXX
#define _SVP_SVPPRN_HXX

#include "vcl/jobdata.hxx"
#include "vcl/printergfx.hxx"
#include "vcl/printerjob.hxx"
#include "vcl/salprn.hxx"
#include "salprn.h"

class PspGraphics;

class SvpSalInfoPrinter : public PspSalInfoPrinter
{
public:
    virtual sal_Bool                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData );
};

class SvpSalPrinter : public PspSalPrinter
{
public:
    SvpSalPrinter( SalInfoPrinter* pInfoPrinter ) : PspSalPrinter(pInfoPrinter) {}
};

#endif // _SVP_SVPPRN_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

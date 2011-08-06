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

#ifndef SC_FLTTYPES_HXX
#define SC_FLTTYPES_HXX

enum BiffTyp
{
    BiffX = 0x0000,
    Biff2 = 0x2000, Biff2M = 0x2002, Biff2C = 0x2004,
    Biff3 = 0x3000, Biff3W = 0x3001, Biff3M = 0x3002, Biff3C = 0x3004,
    Biff4 = 0x4000, Biff4W = 0x4001, Biff4M = 0x4002, Biff4C = 0x4004,
    Biff5 = 0x5000, Biff5W = 0x5001, Biff5V = 0x5002, Biff5C = 0x5004, Biff5M4 = 0x5008,
    Biff8 = 0x8000, Biff8W = 0x8001, Biff8V = 0x8002, Biff8C = 0x8004, Biff8M4 = 0x8008
};

enum Lotus123Typ
{
    Lotus_X,
    Lotus_WK1,
    Lotus_WK3,
    Lotus_WK4,
    Lotus_FM3
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

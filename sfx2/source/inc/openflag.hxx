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
#ifndef _SFX_OPENFLAG_HXX
#define _SFX_OPENFLAG_HXX

// Open file for editing, then only the third option (reading a copy) works
#define SFX_STREAM_READWRITE  (STREAM_READWRITE |  STREAM_SHARE_DENYWRITE)
// I work on the original, not a copy
// -> file then can not be opened for editing
#define SFX_STREAM_READONLY   (STREAM_READ | STREAM_SHARE_DENYWRITE) // + !bDirect
// Someone else is editing the file, a copy it created
// -> the file can then be opened for editing
#define SFX_STREAM_READONLY_MAKECOPY   (STREAM_READ | STREAM_SHARE_DENYNONE)


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

/***********************************************************************
*
*   Die vordefinierte Arrays werden ueber POSITIV-Defines aktiviert:
*   (die defines setzen sich aus "_SVSTDARR_" und dem Namen des Array
*    ohne "Sv" zusammen)
*
*   SortArr:    SvStringsSort, SvStringsSortDtor,
*               SvStringsISort, SvStringsISortDtor,
***********************************************************************/

#include "svl/svldllapi.h"
#include <svl/svarray.hxx>
#include <deque>

#include <tools/string.hxx>

typedef String* StringPtr;

#ifndef _SVSTDARR_STRINGSISORTDTOR_DECL
SV_DECL_PTRARR_SORT_DEL_VISIBILITY( SvStringsISortDtor, StringPtr, 1, SVL_DLLPUBLIC )
#define _SVSTDARR_STRINGSISORTDTOR_DECL
#endif

typedef std::deque< xub_StrLen > SvXub_StrLens;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

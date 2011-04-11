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

#ifndef INCLUDED_VBADLLAPI_H
#define INCLUDED_VBADLLAPI_H

#include "sal/types.h"

#if defined(VBAHELPER_DLLIMPLEMENTATION)
#define VBAHELPER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define VBAHELPER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define VBAHELPER_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_SVLDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

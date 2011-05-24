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

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#define INCLUDED_WRITERFILTERDLLAPI_H

#include "sal/types.h"

#if defined(WRITERFILTER_DOCTOK_DLLIMPLEMENTATION)
#define WRITERFILTER_DOCTOK_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define WRITERFILTER_DOCTOK_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#if defined(WRITERFILTER_OOXML_DLLIMPLEMENTATION)
#define WRITERFILTER_OOXML_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define WRITERFILTER_OOXML_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#if defined(WRITERFILTER_RTFTOK_DLLIMPLEMENTATION)
#define WRITERFILTER_RTFTOK_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define WRITERFILTER_RTFTOK_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#if defined(WRITERFILTER_RESOURCEMODEL_DLLIMPLEMENTATION)
#define WRITERFILTER_RESOURCEMODEL_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define WRITERFILTER_RESOURCEMODEL_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#if defined(WRITERFILTER_WRITERFILTER_DLLIMPLEMENTATION)
#define WRITERFILTER_WRITERFILTER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define WRITERFILTER_WRITERFILTER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#if defined(WRITERFILTER_WRITERFILTER_UNO_DLLIMPLEMENTATION)
#define WRITERFILTER_WRITERFILTER_UNO_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define WRITERFILTER_WRITERFILTER_UNO_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

// This one can be shared
#define WRITERFILTER_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_WRITERFILTERDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

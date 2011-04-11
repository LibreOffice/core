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

#ifndef INCLUDED_SLIDESHOW_DEBUG_HXX
#define INCLUDED_SLIDESHOW_DEBUG_HXX

#include <osl/diagnose.h>

#if OSL_DEBUG_LEVEL > 1

#include "animationnode.hxx"


namespace slideshow { namespace internal {

void Debug_ShowNodeTree (const AnimationNodeSharedPtr& rpNode);

// Change this to a valid filename.  The file is created anew with every
// office start (and execution of at least one TRACE... command.)
#define TRACE_LOG_FILE_NAME "d:\\tmp\\log.txt"

class DebugTraceScope
{
public:
    DebugTraceScope (const sal_Char* sFormat, ...);
    ~DebugTraceScope (void);
private:
    static const sal_Int32 mnBufferSize = 512;
    sal_Char* msMessage;
};

void SAL_CALL DebugTraceBegin (const sal_Char* sFormat, ...);
void SAL_CALL DebugTraceEnd (const sal_Char* sFormat, ...);
void SAL_CALL DebugTraceMessage (const sal_Char* sFormat, ...);

} } // end of namespace ::slideshow::internal


#define TRACE_BEGIN DebugTraceBegin
#define TRACE_END   DebugTraceEnd
#define TRACE       DebugTraceMessage
#define TRACE_SCOPE DebugTraceScope aTraceScope

#else // OSL_DEBUG_LEVEL > 1

#define TRACE_BEGIN 1 ? ((void)0) : DebugTraceBegin
#define TRACE_END   1 ? ((void)0) : DebugTraceEnd
#define TRACE       1 ? ((void)0) : DebugTraceMessage
#define TRACE_SCOPE

#endif // OSL_DEBUG_LEVEL > 1

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

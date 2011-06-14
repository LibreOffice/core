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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlsecurity.hxx"

/*
 * Implementation of the I/O interfaces based on stream and URI binding
 */
#include "errorcallback.hxx"

#include <sal/types.h>
//For reasons that escape me, this is what xmlsec does when size_t is not 4
#if SAL_TYPES_SIZEOFPOINTER != 4
#    define XMLSEC_NO_SIZE_T
#endif
#include "xmlsec/xmlsec.h"
#include "xmlsec/errors.h"

using namespace ::com::sun::star::xml::crypto;


extern "C"
void errorCallback(const char * /*file*/,
                   int /*line*/,
                   const char * /*func*/,
                   const char * /*errorObject*/,
                   const char * /*errorSubject*/,
                   int /*reason*/,
                   const char  * /*msg*/)
{
#if OSL_DEBUG_LEVEL > 1
//     const char * afunc = func ? func : "";
//     const char * errObj = errorObject ? errorObject : "";
//     const char * errSub = errorSubject ? errorSubject : "";
//     const char * amsg = msg ? msg : "";
//  fprintf(stdout, "xmlsec error: %s, %s,  %s, %i %s  \n", afunc, errObj, errSub, reason, amsg);
#endif
}

void setErrorRecorder()
{
    xmlSecErrorsSetCallback(errorCallback);
}

void clearErrorRecorder()
{
    xmlSecErrorsSetCallback(NULL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

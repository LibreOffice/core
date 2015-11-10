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


/*
 * Implementation of the I/O interfaces based on stream and URI binding
 */
#include "errorcallback.hxx"

#include "xmlsecurity/xmlsec-wrapper.h"

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
    xmlSecErrorsSetCallback(nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

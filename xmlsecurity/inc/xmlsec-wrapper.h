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

#ifndef INCLUDED_XMLSECURITY_INC_XMLSEC_WRAPPER_H
#define INCLUDED_XMLSECURITY_INC_XMLSEC_WRAPPER_H

#include <sal/config.h>

#include <sal/types.h>

// Cf. xmlsec's configure.in (but which isn't used for MSVC):
#if !defined _MSC_VER && SAL_TYPES_SIZEOFPOINTER != 4 && !defined SYSTEM_XMLSEC
#define XMLSEC_NO_SIZE_T
#endif

#include "xmlsec/base64.h"
#include "xmlsec/bn.h"
#include "xmlsec/crypto.h"
#include "xmlsec/errors.h"
#include "xmlsec/io.h"
#include "xmlsec/keysmngr.h"
#include "xmlsec/strings.h"
#include "xmlsec/xmldsig.h"
#include "xmlsec/xmlenc.h"
#include "xmlsec/xmlsec.h"
#include "xmlsec/xmltree.h"
#ifdef XMLSEC_CRYPTO_NSS
#include "xmlsec/nss/pkikeys.h"
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

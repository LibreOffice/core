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
#ifndef SW_UNOFLDMID_H
#define SW_UNOFLDMID_H

#include <tools/solar.h>

class String;
namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

//Abbildung der Properties auf den Descriptor
#define FIELD_PROP_PAR1             10
#define FIELD_PROP_PAR2             11
#define FIELD_PROP_PAR3             12
#define FIELD_PROP_FORMAT           13
#define FIELD_PROP_SUBTYPE          14
#define FIELD_PROP_BOOL1            15
#define FIELD_PROP_BOOL2            16
#define FIELD_PROP_DATE             17
#define FIELD_PROP_USHORT1          18
#define FIELD_PROP_USHORT2          19
#define FIELD_PROP_BYTE1            20
#define FIELD_PROP_DOUBLE           21
#define FIELD_PROP_BOOL3            22
#define FIELD_PROP_PAR4             23
#define FIELD_PROP_SHORT1           24
#define FIELD_PROP_DATE_TIME        25
#define FIELD_PROP_PROP_SEQ         26
#define FIELD_PROP_LOCALE           27
#define FIELD_PROP_BOOL4            28
#define FIELD_PROP_STRINGS          29
#define FIELD_PROP_PAR5             30
#define FIELD_PROP_PAR6             31

#define FIELD_PROP_IS_FIELD_USED        32
#define FIELD_PROP_IS_FIELD_DISPLAYED   33

#define FIELD_PROP_TEXT             34

String& GetString( const com::sun::star::uno::Any&, String& rStr );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

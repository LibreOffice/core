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

#pragma once
#if 1

struct TypeInfoDef {
    const char *typeName;
    sal_Int32 dataType;
    sal_Int32 precision;
    const char *literalPrefix;
    const char *literalSuffix;
    const char *createParams;
    sal_Int16 nullable;
    sal_Bool caseSensitive;
    sal_Int16 searchable;
    sal_Bool isUnsigned;
    sal_Bool fixedPrecScale;
    sal_Bool autoIncrement;
    const char *localTypeName;
    sal_Int32 minScale;
    sal_Int32 maxScale;
    sal_Int32 sqlDataType;
    sal_Int32 sqlDateTimeSub;
    sal_Int32 numPrecRadix;
};

extern TypeInfoDef mysqlc_types[];

#endif /* MYSQLC_TYPES_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

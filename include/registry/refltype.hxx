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

#ifndef INCLUDED_REGISTRY_REFLTYPE_HXX
#define INCLUDED_REGISTRY_REFLTYPE_HXX

#include <registry/types.hxx>
#include <sal/types.h>

/** specifies the type source of a binary type blob.

    Currently only RT_UNO_IDL type is used.
 */
enum RTTypeSource
{
    RT_UNO_IDL,
    RT_CORBA_IDL,
    RT_JAVA
};

/** specifies a helper class for const values.

    This class is used for easy handling of constants or enum values
    as fields in binary type blob.
 */
class RTConstValue
{
public:
    /// stores the type of the constant value.
    RTValueType m_type;
    /// stores the value of the constant.
    RTConstValueUnion m_value;

    /// Default constructor.
    RTConstValue()
        : m_type(RT_TYPE_NONE)
    {
        m_value.aDouble = 0.0;
    }
};

/** deprecated.

    An earlier version of UNO used a unique identifier for interfaces. In the
    current version of UNO this uik was eliminated and this type is no longer used.
 */
struct RTUik
{
    sal_uInt32 m_Data1;
    sal_uInt16 m_Data2;
    sal_uInt16 m_Data3;
    sal_uInt32 m_Data4;
    sal_uInt32 m_Data5;
};

/// specifies the calling convention for type reader/writer api
#define TYPEREG_CALLTYPE SAL_CALL

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

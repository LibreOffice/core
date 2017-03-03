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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_TABLEFIELDINFO_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_TABLEFIELDINFO_HXX

#include "QEnumTypes.hxx"
#include <sal/types.h>

namespace dbaui
{
    class OTableFieldInfo
    {
    private:
        ETableFieldType m_eFieldType;
        sal_Int32       m_eDataType;

    public:
        OTableFieldInfo();
        ~OTableFieldInfo();

        ETableFieldType  GetKeyType() const { return m_eFieldType; }
        void             SetKey(ETableFieldType bKey) { m_eFieldType = bKey; }
        sal_Int32        GetDataType() const { return m_eDataType; }
        void             SetDataType(sal_Int32 eTyp) { m_eDataType = eTyp; }
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_QUERYDESIGN_TABLEFIELDINFO_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

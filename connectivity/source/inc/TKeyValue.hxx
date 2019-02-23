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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_TKEYVALUE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_TKEYVALUE_HXX

#include <connectivity/FValue.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <osl/diagnose.h>

namespace connectivity
{
    class OOO_DLLPUBLIC_DBTOOLS OKeyValue final
    {
        std::vector<ORowSetValueDecoratorRef> m_aKeys;
        sal_Int32 m_nValue;

        OKeyValue(sal_Int32 nVal);
    public:

        ~OKeyValue();

        static std::unique_ptr<OKeyValue> createKeyValue(sal_Int32 nVal);

        void pushKey(const ORowSetValueDecoratorRef& _aValueRef)
        {
            m_aKeys.push_back(_aValueRef);
        }

        OUString getKeyString(std::vector<ORowSetValueDecoratorRef>::size_type i) const
        {
            OSL_ENSURE(m_aKeys.size() > i,"Wrong index for KEyValue");
            return m_aKeys[i]->getValue();
        }
        double          getKeyDouble(std::vector<ORowSetValueDecoratorRef>::size_type i) const
        {
            OSL_ENSURE(m_aKeys.size() > i,"Wrong index for KEyValue");
            return m_aKeys[i]->getValue();
        }

        sal_Int32 getValue() const { return m_nValue; }
    };
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_TKEYVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

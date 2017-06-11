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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MERRORRESOURCE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MERRORRESOURCE_HXX

namespace connectivity
{
    namespace mork
    {
        class ErrorDescriptor
        {
        private:
            const char* m_pErrorResourceId;
            sal_Int32       m_nErrorCondition;
            OUString m_sParameter;

        public:
            ErrorDescriptor()
                :m_pErrorResourceId(nullptr)
                ,m_nErrorCondition(0)
                ,m_sParameter()
            {
            }

            void setResId(const char* pErrorResourceId)
            {
                m_pErrorResourceId = pErrorResourceId;
            }
            void reset()
            {
                m_pErrorResourceId = nullptr;
                m_nErrorCondition = 0;
            }

            const char* getResId() const                  { return m_pErrorResourceId; }
            sal_Int32  getErrorCondition() const         { return m_nErrorCondition; }
            const OUString& getParameter() const  { return m_sParameter; }

            bool is() const { return ( m_pErrorResourceId != nullptr ) || ( m_nErrorCondition != 0 ); }
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MERRORRESOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

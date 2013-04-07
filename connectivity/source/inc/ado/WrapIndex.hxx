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
#ifndef CONNECTIVITY_ADO_WRAP_INDEX_HXX
#define CONNECTIVITY_ADO_WRAP_INDEX_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOIndex : public WpOLEBase<_ADOIndex>
        {
        public:
            WpADOIndex(_ADOIndex* pInt=NULL)    :   WpOLEBase<_ADOIndex>(pInt){}
            WpADOIndex(const WpADOIndex& rhs) : WpOLEBase<_ADOIndex>(rhs) {}

            inline WpADOIndex& operator=(const WpADOIndex& rhs)
                {WpOLEBase<_ADOIndex>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void            put_Name(const OUString& _rName);
            sal_Bool        get_Clustered() const;
            void            put_Clustered(sal_Bool _b);
            sal_Bool        get_Unique() const;
            void            put_Unique(sal_Bool _b);
            sal_Bool        get_PrimaryKey() const;
            void            put_PrimaryKey(sal_Bool _b);
            WpADOColumns    get_Columns() const;
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_INDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

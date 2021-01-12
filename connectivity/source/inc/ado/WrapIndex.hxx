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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_WRAPINDEX_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_WRAPINDEX_HXX

#include <sal/config.h>

#include <string_view>

#include <ado/WrapTypeDefs.hxx>

namespace connectivity::ado
{
        class WpADOIndex : public WpOLEBase<_ADOIndex>
        {
        public:
            WpADOIndex(_ADOIndex* pInt=nullptr)    :   WpOLEBase<_ADOIndex>(pInt){}
            WpADOIndex(const WpADOIndex& rhs) : WpOLEBase<_ADOIndex>(rhs) {}

            WpADOIndex& operator=(const WpADOIndex& rhs)
                {WpOLEBase<_ADOIndex>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void            put_Name(std::u16string_view _rName);
            bool            get_Clustered() const;
            void            put_Clustered(bool _b);
            bool            get_Unique() const;
            void            put_Unique(bool _b);
            bool            get_PrimaryKey() const;
            void            put_PrimaryKey(bool _b);
            WpADOColumns    get_Columns() const;
        };
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_WRAPINDEX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

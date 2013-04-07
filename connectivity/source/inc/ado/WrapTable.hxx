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
#ifndef CONNECTIVITY_ADO_WRAP_TABLE_HXX
#define CONNECTIVITY_ADO_WRAP_TABLE_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOCatalog;

        class WpADOTable : public WpOLEBase<_ADOTable>
        {
        public:
            WpADOTable(_ADOTable* pInt=NULL)    :   WpOLEBase<_ADOTable>(pInt){}
            WpADOTable(const WpADOTable& rhs) : WpOLEBase<_ADOTable>(rhs) {}

            inline WpADOTable& operator=(const WpADOTable& rhs)
                {WpOLEBase<_ADOTable>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void            put_Name(const OUString& _rName);
            OUString get_Type() const;
            WpADOColumns    get_Columns() const;
            WpADOIndexes    get_Indexes() const;
            WpADOKeys       get_Keys() const;
            WpADOCatalog    get_ParentCatalog() const;
            WpADOProperties get_Properties() const;
            void            putref_ParentCatalog(/* [in] */ _ADOCatalog __RPC_FAR *ppvObject);
        };


        typedef WpOLEAppendCollection<ADOTables,    _ADOTable,  WpADOTable>     WpADOTables;
    }
}

#endif //CONNECTIVITY_ADO_WRAP_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

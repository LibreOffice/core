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
#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_WRAPCOLUMN_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_WRAPCOLUMN_HXX

#include <sal/config.h>

#include <string_view>

#include <ado/Aolewrap.hxx>

#include <adoint.h>

namespace connectivity::ado
{
        class WpADOColumn : public WpOLEBase<_ADOColumn>
        {
        public:
            WpADOColumn(_ADOColumn* pInt=nullptr)  :   WpOLEBase<_ADOColumn>(pInt){}
            WpADOColumn(const WpADOColumn& rhs) : WpOLEBase<_ADOColumn>(rhs) {}

            void Create();

            WpADOColumn& operator=(const WpADOColumn& rhs)
                {WpOLEBase<_ADOColumn>::operator=(rhs); return *this;}

            OUString get_Name() const;
            OUString get_RelatedColumn() const;
            void put_Name(std::u16string_view _rName);
            void put_RelatedColumn(std::u16string_view _rName);
            DataTypeEnum get_Type() const;
            void put_Type(const DataTypeEnum& _eNum) ;
            sal_Int32 get_Precision() const;
            void put_Precision(sal_Int32 _nPre) ;
            sal_uInt8 get_NumericScale() const;
            void put_NumericScale(sal_Int8 _nScale);
            SortOrderEnum get_SortOrder() const;
            void put_SortOrder(SortOrderEnum _nScale);
            sal_Int32 get_DefinedSize() const;
            ColumnAttributesEnum get_Attributes() const;
            bool put_Attributes(const ColumnAttributesEnum& _eNum);
            WpADOProperties get_Properties() const;
            void put_ParentCatalog(/* [in] */ _ADOCatalog __RPC_FAR *ppvObject);
        };
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_ADO_WRAPCOLUMN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CONNECTIVITY_ADO_WRAP_KEY_HXX
#define CONNECTIVITY_ADO_WRAP_KEY_HXX

#include "ado/WrapTypeDefs.hxx"

namespace connectivity
{
    namespace ado
    {
        class WpADOKey : public WpOLEBase<ADOKey>
        {
        public:
            WpADOKey(ADOKey* pInt=NULL) :   WpOLEBase<ADOKey>(pInt){}
            WpADOKey(const WpADOKey& rhs){operator=(rhs);}

            inline WpADOKey& operator=(const WpADOKey& rhs)
                {WpOLEBase<ADOKey>::operator=(rhs); return *this;}

            void Create();

            ::rtl::OUString get_Name() const;
            void put_Name(const ::rtl::OUString& _rName);
            KeyTypeEnum get_Type() const;
            void put_Type(const KeyTypeEnum& _eNum) ;
            ::rtl::OUString get_RelatedTable() const;
            void put_RelatedTable(const ::rtl::OUString& _rName);
            RuleEnum get_DeleteRule() const;
            void put_DeleteRule(const RuleEnum& _eNum) ;
            RuleEnum get_UpdateRule() const;
            void put_UpdateRule(const RuleEnum& _eNum) ;
            WpADOColumns get_Columns() const;
        };
    }
}

#endif //CONNECTIVITY_ADO_WRAP_KEY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

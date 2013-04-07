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
            WpADOKey(const WpADOKey& rhs) : WpOLEBase<ADOKey>(rhs) {}

            inline WpADOKey& operator=(const WpADOKey& rhs)
                {WpOLEBase<ADOKey>::operator=(rhs); return *this;}

            void Create();

            OUString get_Name() const;
            void put_Name(const OUString& _rName);
            KeyTypeEnum get_Type() const;
            void put_Type(const KeyTypeEnum& _eNum) ;
            OUString get_RelatedTable() const;
            void put_RelatedTable(const OUString& _rName);
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

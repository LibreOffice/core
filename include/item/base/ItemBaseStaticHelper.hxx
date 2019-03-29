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

#ifndef INCLUDED_ITEM_BASE_ITEMBASESTATICHELPER_HXX
#define INCLUDED_ITEM_BASE_ITEMBASESTATICHELPER_HXX

#include <item/base/ItemBase.hxx>
#include <item/base/ItemAdministrator.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // Template class which supports Item implementations using ItemBase.
    // It allows automatic creation of default static methods.
    // It provides implementation of all static typed stuff needed to
    // implement and make available the single global static ItemAdministrator
    // which will be used for the implementation of that new Item-type.
    // Usually one of three predefined/preimplemented IAdministrators
    // will be used, for details see ItemAdministrator.hxx and
    // - IAdministrator_set
    // - IAdministrator_unordered_set
    // - IAdministrator_vector
    // and the explanations/preconditions for their usage.
    template< class TargetType, typename AdminType > class ItemBaseStaticHelper //: public ItemBase
    {
    protected:
        static ItemAdministrator& GetStaticAdmin()
        {
            static AdminType aAdmin(new TargetType());
            return aAdmin;
        }

    public:
        static std::shared_ptr<const TargetType> GetStaticDefault()
        {
            return std::static_pointer_cast<const TargetType>(GetStaticAdmin().GetDefault());
        }

        static bool IsDefault(const std::shared_ptr<const TargetType>& rCandidate)
        {
            return rCandidate && GetStaticAdmin().IsDefault(rCandidate.get());
        }

        // SharedPtr-constructor
        static std::shared_ptr<const TargetType> CreateFromAny(const ItemBase::AnyIDArgs& rArgs)
        {
            TargetType* pNewInstance(new TargetType());
            pNewInstance->PutValues(rArgs);
            return std::shared_ptr<const TargetType>(pNewInstance);
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_ITEMBASESTATICHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

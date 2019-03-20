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

#ifndef INCLUDED_ITEM_BASE_IBASESTATICHELPER_HXX
#define INCLUDED_ITEM_BASE_IBASESTATICHELPER_HXX

#include <item/base/IBase.hxx>
#include <item/base/IAdministrator.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // Template class which supports Item implementations using IBase.
    // It allows automatic creation of default static methods. To keep
    // things simple, it directly derives from ::IBase already.
    // It provides implementation of all static typed stuff needed to
    // implement and make available the single global static IAdministrator
    // which will be used for the implementation of that new Item-type.
    // Usually one of three predefined/preimplemented IAdministrators
    // will be used, for details see IAdministrator.hxx and
    // - IAdministrator_set
    // - IAdministrator_unordered_set
    // - IAdministrator_vector
    // and the explanations/preconditions for their usage.
    // It also defines a convenient SharedPtr type for access for each derived type
    // of Item.
    template< class TargetType, typename AdminType > class IBaseStaticHelper : public IBase
    {
    public:
        // SharedPtr typedef to be used handling instances of given type
        typedef std::shared_ptr<const TargetType> SharedPtr;

    protected:
        static IAdministrator& GetStaticAdmin()
        {
            static AdminType aAdmin(new TargetType());
            return aAdmin;
        }

        virtual IAdministrator* GetIAdministrator() const override
        {
            return &GetStaticAdmin();
        }

    public:
        // we *could* have a method like below that is able to return
        // a non-static_pointer_cast value, thus allowing to return
        // it as const& to the SharedPtr and to avoid constructing the
        // TargetType::SharedPtr (less overhead). This could be used e.g.
        // in Set::SetItem calls.
        // Disadvantage is that the User/Programmer using it would have
        // to be very aware what he is doing - to avoid confusion, I
        // decided to *not* offer this currently.
        //
        // static const IBase::SharedPtr& getStaticDefault()
        // {
        //     return std::static_pointer_cast<const TargetType>(GetStaticAdmin().GetDefault());
        // }

        static std::shared_ptr<const TargetType> GetDefault()
        {
            return std::static_pointer_cast<const TargetType>(GetStaticAdmin().GetDefault());
        }

        static bool IsDefault(const std::shared_ptr<const TargetType>& rCandidate)
        {
            return rCandidate && GetStaticAdmin().IsDefault(rCandidate.get());
        }
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_BASE_IBASESTATICHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

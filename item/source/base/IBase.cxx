/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <item/base/IBase.hxx>
#include <item/base/IAdministrator.hxx>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    IBase::IBase()
    :   std::enable_shared_from_this<IBase>(),
        m_bAdministrated(false)
    {
    }

    bool IBase::CheckSameType(const IBase& rCmp) const
    {
        return typeid(rCmp) == typeid(*this);
    }

    IAdministrator* IBase::GetIAdministrator() const
    {
        // not intended to be used, error
        assert(false && "IBase::GetIAdministrator call not allowed (!)");
        return nullptr;
    }

    IBase::~IBase()
    {
    }

    bool IBase::operator==(const IBase& rCmp) const
    {
        // basic implementation compares type, no data available
        return CheckSameType(rCmp);
    }

    bool IBase::operator<(const IBase& rCmp) const
    {
        // basic implementation uses addresses of instances to
        // deliver a consistent result, but should *not* be used in
        // this form - it will not compare any data
        assert(CheckSameType(rCmp));
        return this < &rCmp;
    }

    size_t IBase::GetUniqueKey() const
    {
        // basic implementation uses adress of instance as hash, to
        // deliver a consistent result, but should *not* be used in
        // this form - it will not compare any data
        return static_cast<size_t>(sal_uInt64(this));
    }

    bool IBase::IsDefault() const
    {
        // callback to static administrator
        return GetIAdministrator()->IsDefault(this);
    }

    const IBase::SharedPtr& IBase::GetDefault() const
    {
        // callback to static administrator
        return GetIAdministrator()->GetDefault();
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

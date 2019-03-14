/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <item/simple/CntOUString.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    CntOUString::CntOUString(const rtl::OUString& rValue)
    :   CntOUStringStaticHelper(),
        m_aValue(rValue)
    {
    }

    CntOUString::~CntOUString()
    {
        // needs to be called from here to have the fully derived implementation type
        // in the helper method - do NOT move to a imaginable general
        // implementation in IBaseStaticHelper (!)
        if(IsAdministrated())
        {
            GetStaticAdmin().HintExpired(this);
        }
    }

    std::shared_ptr<const CntOUString> CntOUString::Create(const rtl::OUString& rValue)
    {
        // use ::Create(...) method with local incarnation, it will handle
        // - detection of being default (will delete local incarnation)
        // - detection of reuse (will delete local incarnation)
        // - detectiomn of new use - will create shared_ptr for local incarnation and buffer
        return std::static_pointer_cast<const CntOUString>(GetStaticAdmin().Create(new CntOUString(rValue)));
    }

    bool CntOUString::operator==(const IBase& rCandidate) const
    {
        assert(IBase::operator==(rCandidate));
        return (GetValue() == static_cast<const CntOUString&>(rCandidate).GetValue());
    }

    size_t CntOUString::GetUniqueKey() const
    {
        return static_cast<const CntOUString*>(this)->GetValue().hashCode();
    }

    const rtl::OUString& CntOUString::GetValue() const
    {
        return m_aValue;
    }
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

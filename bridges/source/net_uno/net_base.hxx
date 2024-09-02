/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/ustring.hxx>
#include <typelib/typedescription.hxx>
#include <sal/log.hxx>

namespace net_uno
{
OUString map_uno_type_to_net(typelib_TypeDescriptionReference* pTDRef);
typelib_TypeDescriptionReference* map_net_type_to_uno(const OUString& sTypeName);

struct BridgeRuntimeError
{
    OUString m_location;
    OUString m_message;

    explicit BridgeRuntimeError(const char* location, const OUString& message)
        : m_location(OUString::createFromAscii(location))
        , m_message(message)
    {
    }
};

class TypeDescHolder
{
public:
    TypeDescHolder(const TypeDescHolder&) = delete;
    TypeDescHolder& operator=(const TypeDescHolder&) = delete;

    explicit TypeDescHolder(typelib_TypeDescriptionReference* td_ref)
        : m_td(nullptr)
    {
        TYPELIB_DANGER_GET(&m_td, td_ref);
        if (!m_td)
        {
            throw BridgeRuntimeError(SAL_WHERE, "could not get type description for "
                                                    + OUString::unacquired(&td_ref->pTypeName));
        }
    }
    ~TypeDescHolder() { TYPELIB_DANGER_RELEASE(m_td); }

    typelib_TypeDescription* get() const { return m_td; }

private:
    typelib_TypeDescription* m_td;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

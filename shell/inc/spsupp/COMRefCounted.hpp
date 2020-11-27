/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SHELL_INC_SPSUPP_COMREFCOUNTED_HPP
#define INCLUDED_SHELL_INC_SPSUPP_COMREFCOUNTED_HPP

#include <objbase.h>
#include <assert.h>

template <class... Interfaces>
class COMRefCounted : public Interfaces...
{
public:
    COMRefCounted() {}
    virtual ~COMRefCounted() {}

    // IUnknown methods

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        return static_cast<ULONG>(::InterlockedIncrement(&m_nRef));
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        assert(m_nRef > 0);
        if (::InterlockedDecrement(&m_nRef) == 0)
        {
            delete this;
            return 0;
        }
        return static_cast<ULONG>(m_nRef);
    }

private:
    LONG m_nRef = 1; // Created referenced
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

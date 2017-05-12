/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_WIN_SCOPEDHDC_HXX
#define INCLUDED_VCL_INC_WIN_SCOPEDHDC_HXX

class ScopedHDC
{
private:
    HDC m_hDC;

public:
    explicit ScopedHDC(HDC hDC)
        : m_hDC(hDC)
    {}

    ScopedHDC(const ScopedHDC&) = delete;
    ScopedHDC& operator=(const ScopedHDC&) = delete;

    ~ScopedHDC()
    {
        if (m_hDC)
            DeleteDC(m_hDC);
    }

    HDC get() const
    {
        return m_hDC;
    }

    explicit operator bool() const
    {
        return m_hDC != nullptr;
    }
};

class SharedHDC
{
private:
    struct Impl
    {
        HDC m_hDC;

        ~Impl()
        {
            DeleteDC(m_hDC);
        }
    };
    std::shared_ptr<Impl> m_pImpl;

public:
    SharedHDC(HDC hDC):
        pImpl(new Impl)
    {
        pImpl->m_hDC = hDC;
    }

    HDC get() const
    {
        if (!m_pImpl)
            return nullptr;

        return m_pImpl->m_hDC;
    }

    explicit operator bool() const
    {
        return m_pImpl && m_pImpl->m_hDC != nullptr;
    }
};

#endif // INCLUDED_VCL_INC_WIN_SCOPEDHDC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

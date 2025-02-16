/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/listener.hxx>
#include <svl/lstner.hxx>
#include "calbck.hxx"

namespace sw
{
template <typename T> class WeakBroadcastingPtr final : public SvtListener
{
private:
    T* m_pBroadcasting;
    void StartListeningIfNonnull()
    {
        if (m_pBroadcasting)
            StartListening(m_pBroadcasting->GetNotifier());
    }

public:
    WeakBroadcastingPtr(T* pBroadcasting)
        : m_pBroadcasting(pBroadcasting)
    {
        StartListeningIfNonnull();
    }
    WeakBroadcastingPtr(const WeakBroadcastingPtr& rOther)
        : m_pBroadcasting(rOther.m_pBroadcasting)
    {
        StartListeningIfNonnull();
    }
    WeakBroadcastingPtr& operator=(const WeakBroadcastingPtr& rOther)
    {
        if (m_pBroadcasting)
            EndListening(m_pBroadcasting->GetNotifier());
        m_pBroadcasting = rOther.m_pBroadcasting;
        StartListeningIfNonnull();
        return *this;
    }

    virtual void Notify(const SfxHint& rHint) override
    {
        if (rHint.GetId() == SfxHintId::Dying)
            m_pBroadcasting = nullptr;
    }

    T* operator->() { return m_pBroadcasting; }
    const T* operator->() const { return m_pBroadcasting; }
    T& operator*() { return *m_pBroadcasting; }
    explicit operator bool() const { return m_pBroadcasting; }
};
}

class SfxDeleteListener final : public SfxListener
{
private:
    bool m_bObjectDeleted;

public:
    explicit SfxDeleteListener(SfxBroadcaster& rNotifier)
        : m_bObjectDeleted(false)
    {
        StartListening(rNotifier);
    }

    virtual void Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint) override
    {
        if (rHint.GetId() == SfxHintId::Dying)
            m_bObjectDeleted = true;
    }

    bool WasDeleted() const { return m_bObjectDeleted; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

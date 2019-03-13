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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTTIMERMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTTIMERMANAGER_HXX

#include <IDocumentTimerAccess.hxx>
#include <SwDocIdle.hxx>

#include <sal/types.h>
#include <tools/link.hxx>

class SwDoc;

namespace sw
{

class DocumentTimerManager : public IDocumentTimerAccess
{
public:
    enum class IdleJob
    {
        None, ///< document has no idle jobs to do
        Busy, ///< document is busy and idle jobs are postponed
        Grammar,
        Layout,
        Fields,
    };

    DocumentTimerManager( SwDoc& i_rSwdoc );
    virtual ~DocumentTimerManager() override;

    void StartIdling() override;

    void StopIdling() override;

    void BlockIdling() override;

    void UnblockIdling() override;

    bool IsDocIdle() const override;

private:
    DocumentTimerManager(DocumentTimerManager const&) = delete;
    DocumentTimerManager& operator=(DocumentTimerManager const&) = delete;

    /// Delay starting idle jobs to allow for post-load activity.
    /// Used by LOK only.
    DECL_LINK( FireIdleJobsTimeout, Timer *, void );

    DECL_LINK( DoIdleJobs, Timer *, void );

    IdleJob GetNextIdleJob() const;

    SwDoc& m_rDoc;

    sal_uInt32 m_nIdleBlockCount; ///< Don't run the Idle, if > 0
    bool m_bStartOnUnblock; ///< true, if the last unblock should start the timer
    SwDocIdle m_aDocIdle;
    Timer m_aFireIdleJobsTimer;
    bool m_bWaitForLokInit; ///< true if we waited for LOK to initialize already.
};

inline bool DocumentTimerManager::IsDocIdle() const
{
    return ((0 == m_nIdleBlockCount) && (GetNextIdleJob() != IdleJob::Busy));
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

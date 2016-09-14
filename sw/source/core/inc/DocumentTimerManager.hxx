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

#include <vcl/idle.hxx>
#include <sal/types.h>
#include <tools/link.hxx>

class SwDoc;

namespace sw
{

class DocumentTimerManager : public IDocumentTimerAccess
{
public:

    DocumentTimerManager( SwDoc& i_rSwdoc );

    void StartIdling() override;

    void StopIdling() override;

    void BlockIdling() override;

    void UnblockIdling() override;

    void StartBackgroundJobs() override;

    // Our own 'IdleTimer' calls the following method
    DECL_LINK_TYPED( DoIdleJobs, Timer *, void );

    virtual ~DocumentTimerManager() override;

private:

    DocumentTimerManager(DocumentTimerManager const&) = delete;
    DocumentTimerManager& operator=(DocumentTimerManager const&) = delete;

    SwDoc& m_rDoc;

    bool mbStartIdleTimer; //< idle timer mode start/stop
    sal_Int32 mIdleBlockCount;
    Timer maDocIdleTimer;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

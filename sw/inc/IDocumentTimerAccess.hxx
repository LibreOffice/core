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

#pragma once

/**
 * Handle the background jobs of a Writer document.
 *
 * Initially it's disabled and unblocked.
 *
 * Jobs include:
 *  * grammar checking
 *  * field updating
 *  * document layouting
 */
class IDocumentTimerAccess
{
public:
    /**
     * Start the idle task.
     *
     * Depends on the block count and various document states.
     */
    virtual void StartIdling() = 0;

    /**
     * Stop idle processing.
     */
    virtual void StopIdling() = 0;

    /**
     * Increment block count.
     *
     * Prevents further background idle processing.
     * This doesn't guarantee the Idle task is not currently running!
     */
    virtual void BlockIdling() = 0;

    /**
     * Decrement block count.
     *
     * May re-start the idle task, if active.
     */
    virtual void UnblockIdling() = 0;

    /**
     * Is the document ready to be processed?
     */
    virtual bool IsDocIdle() const = 0;

    /**
     * Is idling blocked?
     */
    virtual bool IsIdlingBlocked() const = 0;

protected:
    virtual ~IDocumentTimerAccess(){};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_SW_INC_IDOCUMENTTIMERACCESS_HXX
#define INCLUDED_SW_INC_IDOCUMENTTIMERACCESS_HXX

/**
 * Handle the background job of the Writer document.
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
     * Start the idle job depending on the block count.
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
     */
    virtual void BlockIdling() = 0;

    /**
     * Decrement block count.
     *
     * May start the idle job.
     */
    virtual void UnblockIdling() = 0;

    /**
     * Is the document ready to be processed?
     */
    virtual bool IsDocIdle() const = 0;

protected:
    virtual ~IDocumentTimerAccess() {};
};

#endif // INCLUDED_SW_INC_IDOCUMENTTIMERACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

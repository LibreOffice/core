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

/** Manipulate background jobs of the document. It starts with a mode of
 'started' and a block count of 0.
 */
class IDocumentTimerAccess
{
public:
    /**
    Set mode to 'start'.
    */
    virtual void StartIdling() = 0;

    /**
    Set mode to 'stopped'.
    */
    virtual void StopIdling() = 0;

    /**
    Increment block count.
    */
    virtual void BlockIdling() = 0;

    /**
    Decrement block count.
    */
    virtual void UnblockIdling() = 0;

    /**
    Do these jobs asynchronously: do grammar checking,
    do layout, and update fields.
    They will be delayed until mode is start AND block count == 0.
    The implementation might delay them further, for example
    it might wait until the application is idle.
    */
    virtual void StartBackgroundJobs() = 0;

protected:
    virtual ~IDocumentTimerAccess() {};
};

#endif // INCLUDED_SW_INC_IDOCUMENTTIMERACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

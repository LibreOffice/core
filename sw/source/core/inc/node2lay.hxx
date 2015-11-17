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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_NODE2LAY_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_NODE2LAY_HXX

#include <tools/solar.h>
#include <memory>

/**
 * This class connects the Nodes with the Layouts.
 * It provides an intelligent iterator over Frames belonging to the Node or
 * Node Area. Depending on the purpose of iterating (e.g. to insert other
 * Frames before or after the Frames) Master/Follows are recognized and only
 * the relevant ones are returned. Repeated table headers are also taken
 * into account.
 * It's possible to iterate over SectionNodes that are either not directly
 * assigned to a SectionFrm or to multiple ones due to nesting.
 *
 * This class is an interface between the method and a SwClientIter: it
 * chooses the right SwModify depending on the task, creates a SwClientIter
 * and filters its iterations depending on the task.
 * The task is determined by the choice of ctor.
 *
 * 1. Collecting the UpperFrms (so that later on it becomes RestoreUpperFrms)
 *    is called by MakeFrms, if there's no PrevNext (before/after we can insert
 *    the Frames).
 * 2. Inserting the Frames before/after which the new Frames of a Node need to
 *    be inserted, is also called by MakeFrms.
 */

class SwNode2LayImpl;
class SwFrm;
class SwLayoutFrm;
class SwNode;
class SwNodes;
class Point;
struct SwPosition;

class SwNode2Layout
{
    std::unique_ptr<SwNode2LayImpl> pImpl;
public:
    /// Use this ctor for collecting the UpperFrms
    SwNode2Layout( const SwNode& rNd );

    /// Use this ctor for inserting before/after rNd
    /// @param nIdx is the index of the to-be-inserted Node
    SwNode2Layout( const SwNode& rNd, sal_uLong nIdx );
    ~SwNode2Layout();
    SwFrm* NextFrm();
    SwLayoutFrm* UpperFrm( SwFrm* &rpFrm, const SwNode& rNode );
    void RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd );

    SwFrm *GetFrm( const Point* pDocPos = nullptr,
                    const SwPosition *pPos = nullptr,
                    const bool bCalcFrm = true ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

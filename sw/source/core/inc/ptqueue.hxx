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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_PTQUEUE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_PTQUEUE_HXX

/**
 * Unfortunately we have some problems with processing more than one Paint()
 * at a time. This happens especially often during printing.
 *
 * SwRootFrm::Paint() determines that it's called a second time and adds the
 * rectangle and the corresponding Shell to the PaintCollector.
 * The call sites that are causing the double Paint() only need to process the
 * collected Paint()s at the right point in time.
 * Doing this during printing (after having printed one page) is very suitable
 * for doing that.
 *
 * Invalidating windows directly from the RootFrm::Paint was not a successful
 * approach, because the Paint()s arrive at a very unfavourable point in time.
 * Triggering an update for all windows after printing each page does not seem
 * appropriate either: on the one hand we don't have direct access to the edit
 * windows and on the other hand the updates can become very costly on some
 * platforms.
 */

class SwQueuedPaint;
class SwViewShell;
class SwRect;

class SwPaintQueue
{
public:
    static SwQueuedPaint *s_pPaintQueue;

    static void Add( SwViewShell *pSh, const SwRect &rNew );
    static void Remove( SwViewShell *pSh );
    static void Repaint();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

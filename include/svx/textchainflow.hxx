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

#ifndef INCLUDED_SVX_TEXTCHAINFLOW_HXX
#define INCLUDED_SVX_TEXTCHAINFLOW_HXX

class SdrTextObj;
class SdrOutliner;
class NonOverflowingText;
class OverflowingText;
class TextChain;

// XXX: Specialize class for Editing mode and non editing mode?
// XXX: const qualifiers?

class TextChainFlow {

    public:
    TextChainFlow(SdrTextObj *pChainTarget);
    ~TextChainFlow();

    void CheckForFlowEvents(SdrOutliner *, SdrOutliner *);

    bool IsOverflow();
    bool IsUnderflow();

    void ExecuteUnderflow(SdrOutliner *);
    void ExecuteOverflow(SdrOutliner *, SdrOutliner *);

    protected:
    TextChain *GetTextChain();

    void impLeaveOnlyNonOverflowingText(SdrOutliner *);
    void impMoveChainedTextToNextLink(SdrOutliner *);

    OutlinerParaObject *impGetNonOverflowingParaObject(SdrOutliner *pOutliner);
    OutlinerParaObject *impGetOverflowingParaObject(SdrOutliner *pOutliner);

    private:

    void impSetOutlinerToEmptyTxt(SdrOutliner *pOutliner);

    SdrTextObj *mpTargetLink;
    SdrTextObj *mpNextLink;

    TextChain *mpTextChain;

    bool bCheckedFlowEvents;

    bool bUnderflow;
    bool bOverflow;

    OverflowingText *mpOverflowingTxt;
    NonOverflowingText *mpNonOverflowingTxt;

};

#endif // INCLUDED_SVX_TEXTCHAINFLOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


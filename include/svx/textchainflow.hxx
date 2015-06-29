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
class OutlinerParaObject;

// XXX: Specialize class for Editing mode and non editing mode?
// XXX: const qualifiers?

class TextChainFlow {

    public:
    TextChainFlow(SdrTextObj *pChainTarget);
    virtual ~TextChainFlow();

    // Check for flow events in Outliner
    virtual void CheckForFlowEvents(SdrOutliner *);

    bool IsOverflow();
    bool IsUnderflow();

    void ExecuteUnderflow(SdrOutliner *);

    // Uses two outliners: one for the non-overfl text and one for overflowing (might be the same)
    virtual void ExecuteOverflow(SdrOutliner *, SdrOutliner *);

    SdrTextObj *GetLinkTarget();

    protected:

    void impCheckForFlowEvents(SdrOutliner *, SdrOutliner *);

    TextChain *GetTextChain();

    virtual void impLeaveOnlyNonOverflowingText(SdrOutliner *);
    virtual void impMoveChainedTextToNextLink(SdrOutliner *);

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

    OutlinerParaObject *mpUnderflowingPObj;

};


class EditingTextChainFlow : public TextChainFlow
{
    public:
    EditingTextChainFlow(SdrTextObj *);
    virtual void CheckForFlowEvents(SdrOutliner *) SAL_OVERRIDE;

    //virtual void ExecuteOverflow(SdrOutliner *, SdrOutliner *) SAL_OVERRIDE;


    protected:
    virtual void impLeaveOnlyNonOverflowingText(SdrOutliner *) SAL_OVERRIDE;

    virtual void impSetTextForEditingOutliner(OutlinerParaObject *);

};

#endif // INCLUDED_SVX_TEXTCHAINFLOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


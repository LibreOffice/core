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

#ifndef INCLUDED_EDITENG_OVERFLOWINGTXT_HXX
#define INCLUDED_EDITENG_OVERFLOWINGTXT_HXX

#include <editeng/macros.hxx>
#include <editeng/editengdllapi.h>
#include <editeng/editdata.hxx>

class OUString;


class OutlinerParaObject;
class EditTextObject;
class Outliner;

/*
 * The classes OverflowingText and NonOverflowingText handle the
 * actual preparation of the OutlinerParaObjects to be used in destination
 * and source box respectively.
*/

class OverflowingText
{

public:
        OutlinerParaObject *GetJuxtaposedParaObject(Outliner *, OutlinerParaObject *);
        OutlinerParaObject *GetDeeplyMergedParaObject(Outliner *, OutlinerParaObject *);
        ESelection GetInsertionPointSel() const;

        //OUString GetHeadingLines() const;
        //OUString GetEndingLines() const;
        //bool HasOtherParas() const { return !(mTailTxt == "" && mpMidParas == NULL); }

private:
    friend class Outliner;
    // Constructor
    OverflowingText(EditTextObject *pTObj);

    OutlinerParaObject *impMakeOverflowingParaObject(Outliner *pOutliner);

    const EditTextObject *mpContentTextObj;
};

class NonOverflowingText {
public:
    OutlinerParaObject *ToParaObject(Outliner *) const;
    ESelection GetOverflowPointSel() const;
    bool IsLastParaInterrupted() const;

private:
        // Constructor
        NonOverflowingText(const EditTextObject *pTObj, bool bLastParaInterrupted);
        NonOverflowingText(const ESelection &aSel, bool bLastParaInterrupted);

        friend class Outliner;
        const EditTextObject *mpContentTextObj;
        const ESelection maContentSel;
        const bool mbLastParaInterrupted;

};


/*
 * classes OFlowChainedText and UFlowChainedText:
 * contain and handle the state of a broken up text _after_ a flow event
 * (respectively after Overflow and Underflow).
 *
*/

class EDITENG_DLLPUBLIC OFlowChainedText {
    public:
        OFlowChainedText(Outliner *, bool );

        OutlinerParaObject *CreateOverflowingParaObject(Outliner *, OutlinerParaObject *);
        OutlinerParaObject *CreateNonOverflowingParaObject(Outliner *);

        ESelection GetInsertionPointSel() const;
        ESelection GetOverflowPointSel() const;

        bool IsLastParaInterrupted() const;

    protected:
        void impSetOutlinerToEmptyTxt(Outliner *);

    private:

        NonOverflowingText *mpNonOverflowingTxt;
        OverflowingText *mpOverflowingTxt;

        bool mbIsDeepMerge;

};

// UFlowChainedText is a simpler class than OFlowChainedText: it almost only joins para-objects
class EDITENG_DLLPUBLIC UFlowChainedText {
    public:
        UFlowChainedText(Outliner *, bool);

        OutlinerParaObject *CreateMergedUnderflowParaObject(Outliner *, OutlinerParaObject *);
    protected:

    private:
        OutlinerParaObject *mpUnderflowPObj;

        bool mbIsDeepMerge;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

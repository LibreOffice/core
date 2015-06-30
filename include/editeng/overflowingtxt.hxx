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

class OUString;


class OutlinerParaObject;
class EditTextObject;
class Outliner;


class OverflowingText
{

public:
        OUString mHeadTxt;
        const OutlinerParaObject *mpMidParas;
        OUString mTailTxt;
        // NOTE: mpMidParas and mTailTxt might be empty

        // Constructor
        OverflowingText(
            const OUString &headTxt,
            const OutlinerParaObject *pMidParas,
            const OUString &tailTxt)
                : mHeadTxt(headTxt),
                  mpMidParas(pMidParas),
                  mTailTxt(tailTxt)
                { }

        OUString GetHeadingLines() const;
        OUString GetEndingLines() const;
        bool HasOtherParas() const { return !(mTailTxt == "" && mpMidParas == NULL); }
};

class NonOverflowingText {
    public:

        // NOTE: mPreOverflowingTxt might be empty

        // Constructor
        NonOverflowingText(const OutlinerParaObject *pHeadParas,
                           const OUString &preOverflowingTxt)
        : mpHeadParas(pHeadParas),
          mPreOverflowingTxt(preOverflowingTxt), mpContentTextObj(NULL)
        {
            if (pHeadParas == NULL) // Redundant line for debugging
                DBG_ASSERT( pHeadParas != NULL, "pHeadParas is null?! All text is overflowing then" );
        }

        NonOverflowingText(const EditTextObject *pTObj)
        : mpContentTextObj(pTObj)
        { }

        OutlinerParaObject *ToParaObject(Outliner *) const;

    private:
        const OutlinerParaObject *mpHeadParas;
        OUString mPreOverflowingTxt;

        const EditTextObject *mpContentTextObj;
};


/*
 * classes OFlowChainedText and UFlowChainedText:
 * contain and handle the state of a broken up text _after_ a flow event.
 *
*/

class EDITENG_DLLPUBLIC OFlowChainedText {
    public:
        OFlowChainedText(Outliner *);

        OutlinerParaObject *CreateOverflowingParaObject(Outliner *, OutlinerParaObject *);
        OutlinerParaObject *CreateNonOverflowingParaObject(Outliner *);

    protected:
        void impSetOutlinerToEmptyTxt(Outliner *);

    private:

        NonOverflowingText *mpNonOverflowingTxt;
        OverflowingText *mpOverflowingTxt;

};

// UFlowChainedText is a simpler class than OFlowChainedText: it almost only joins para-objects
class EDITENG_DLLPUBLIC UFlowChainedText {
    public:
        UFlowChainedText(Outliner *);

        OutlinerParaObject *CreateMergedUnderflowParaObject(Outliner *, OutlinerParaObject *);
    protected:

    private:
        OutlinerParaObject *mpUnderflowPObj;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

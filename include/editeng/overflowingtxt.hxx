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

#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
  namespace datatransfer {
    class XTransferable;
} } } }

typedef com::sun::star::uno::Reference<
            com::sun::star::datatransfer::XTransferable> TranferableText;

namespace rtl {
    class OUString;
};
using ::rtl::OUString;


class OutlinerParaObject;
class EditTextObject;
class Outliner;


/*
 * A collection of static methods for attaching text.
 * Strongly coupled with some of the classes in this file.
 */
class TextChainingUtils
{
public:
    static TranferableText CreateTransferableFromText(Outliner *);

    static OutlinerParaObject *JuxtaposeParaObject(
            TranferableText xOverflowingContent,
            Outliner *,
            OutlinerParaObject *);
    static OutlinerParaObject *DeeplyMergeParaObject(
            TranferableText xOverflowingContent,
            Outliner *,
            OutlinerParaObject *);
};

/*
 * The classes OverflowingText and NonOverflowingText handle the
 * actual preparation of the OutlinerParaObjects to be used in destination
 * and source box respectively.
*/

class OverflowingText
{
public:
    OutlinerParaObject *JuxtaposeParaObject(Outliner *, OutlinerParaObject *);
    OutlinerParaObject *DeeplyMergeParaObject(Outliner *, OutlinerParaObject *);
    static ESelection GetInsertionPointSel();

private:
    friend class Outliner;
    OverflowingText(TranferableText xOverflowingContent);

    TranferableText mxOverflowingContent;
};

class NonOverflowingText
{
public:
    OutlinerParaObject *RemoveOverflowingText(Outliner *) const;
    ESelection GetOverflowPointSel() const;
    bool IsLastParaInterrupted() const;

private:
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
class EDITENG_DLLPUBLIC OFlowChainedText
{
public:
    OFlowChainedText(Outliner *, bool );

    OutlinerParaObject *InsertOverflowingText(Outliner *, OutlinerParaObject *);
    OutlinerParaObject *RemoveOverflowingText(Outliner *);

    static ESelection GetInsertionPointSel();
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
class EDITENG_DLLPUBLIC UFlowChainedText
{
public:
    UFlowChainedText(Outliner *, bool);
    OutlinerParaObject *CreateMergedUnderflowParaObject(Outliner *, OutlinerParaObject *);

private:
    TranferableText mxUnderflowingTxt;

    bool mbIsDeepMerge;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

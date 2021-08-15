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

#include <config_options.h>
#include <editeng/editengdllapi.h>
#include <editeng/editdata.hxx>

#include <com/sun/star/uno/Reference.h>
#include <memory>
#include <optional>

namespace com::sun::star {
  namespace datatransfer { class XTransferable; }
}
namespace rtl {
    class OUString;
};
using ::rtl::OUString;


class OutlinerParaObject;
class Outliner;


/*
 * A collection of static methods for attaching text.
 * Strongly coupled with some of the classes in this file.
 */
class TextChainingUtils
{
public:
    static css::uno::Reference< css::datatransfer::XTransferable> CreateTransferableFromText(Outliner const *);

    static std::optional<OutlinerParaObject> JuxtaposeParaObject(
            css::uno::Reference< css::datatransfer::XTransferable > const & xOverflowingContent,
            Outliner *,
            OutlinerParaObject const *);
    static std::optional<OutlinerParaObject> DeeplyMergeParaObject(
            css::uno::Reference< css::datatransfer::XTransferable > const & xOverflowingContent,
            Outliner *,
            OutlinerParaObject const *);
};

/*
 * The classes OverflowingText and NonOverflowingText handle the
 * actual preparation of the OutlinerParaObjects to be used in destination
 * and source box respectively.
*/

class OverflowingText
{
public:
    OverflowingText(css::uno::Reference< css::datatransfer::XTransferable > const & xOverflowingContent);

    std::optional<OutlinerParaObject> JuxtaposeParaObject(Outliner *, OutlinerParaObject const *);
    std::optional<OutlinerParaObject> DeeplyMergeParaObject(Outliner *, OutlinerParaObject const *);

private:
    css::uno::Reference< css::datatransfer::XTransferable > mxOverflowingContent;
};

class NonOverflowingText
{
public:
    NonOverflowingText(const ESelection &aSel, bool bLastParaInterrupted);

    std::optional<OutlinerParaObject> RemoveOverflowingText(Outliner *) const;
    ESelection GetOverflowPointSel() const;
    bool IsLastParaInterrupted() const;

private:
    ESelection maContentSel;
    bool mbLastParaInterrupted;
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
    OFlowChainedText(Outliner const *, bool );
    ~OFlowChainedText();

    std::optional<OutlinerParaObject> InsertOverflowingText(Outliner *, OutlinerParaObject const *);
    std::optional<OutlinerParaObject> RemoveOverflowingText(Outliner *);

    ESelection GetOverflowPointSel() const;

    bool IsLastParaInterrupted() const;

private:
    std::optional<NonOverflowingText> mpNonOverflowingTxt;
    std::optional<OverflowingText> mpOverflowingTxt;

    bool mbIsDeepMerge;
};

// UFlowChainedText is a simpler class than OFlowChainedText: it almost only joins para-objects
class EDITENG_DLLPUBLIC UFlowChainedText
{
public:
    UFlowChainedText(Outliner const *, bool);
    std::optional<OutlinerParaObject> CreateMergedUnderflowParaObject(Outliner *, OutlinerParaObject const *);

private:
    css::uno::Reference< css::datatransfer::XTransferable > mxUnderflowingTxt;

    bool mbIsDeepMerge;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

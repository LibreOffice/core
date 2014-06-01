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

#ifndef SW_TOXTEXTGENERATOR_HXX_
#define SW_TOXTEXTGENERATOR_HXX_

#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "swdllapi.h"

#include <boost/shared_ptr.hpp>
#include <vector>

class SfxItemSet;
class SwAttrPool;
class SwFmtAutoFmt;
class SwChapterField;
class SwChapterFieldType;
class SwCntntFrm;
class SwCntntNode;
class SwDoc;
class SwForm;
class SwFormToken;
class SwPageDesc;
class SwTxtAttr;
class SwTxtNode;
struct SwTOXSortTabBase;
class ToxTextGeneratorTest;

namespace sw {

class ToxLinkProcessor;

class SW_DLLPUBLIC ToxTextGenerator
{
public:
    ToxTextGenerator(const SwForm& toxForm);

    virtual ~ToxTextGenerator();

    /** Generate the text for an entry of a table of X (X is, e.g., content).
     *
     * This method will process the entries in @p entries, starting at @p indexOfEntryToProcess and
     * process @p numberOfEntriesToProcess entries.
     */
    void
    GenerateText(SwDoc *doc, const std::vector<SwTOXSortTabBase*>& entries,
                      sal_uInt16 indexOfEntryToProcess, sal_uInt16 numberOfEntriesToProcess,
                      sal_uInt32 _nTOXSectNdIdx, const SwPageDesc* _pDefaultPageDesc);

private:
    const SwForm& mToxForm;
    boost::shared_ptr<ToxLinkProcessor> mLinkProcessor;

    /** A handled text token.
     * It contains the information which should be added to the target text node.
     */
    struct HandledTextToken {
        OUString text;
        std::vector<SwFmtAutoFmt*> autoFormats;
        std::vector<sal_Int32> startPositions;
        std::vector<sal_Int32> endPositions;
    };
    /** Append text (and selected attributes) to a target node.
     *
     * Will take the text of @p source, and return the text and the attributes which should be added to the
     * target text node. @see CollectAttributesForTox() for the criteria of the attributes which are taken.
     */
    static HandledTextToken
    HandleTextToken(const SwTOXSortTabBase& source, SwAttrPool& attrPool);

    /** Applies the result of a handled text token to a target node. */
    static void
    ApplyHandledTextToken(const HandledTextToken& htt, SwTxtNode& targetNode);

    /** Handle a page number token.
     *
     * Will return a string of @p numberOfToxSources concatenated '@' signs, separated by commas, and
     * finished by a '~'.
     * (The '@' sign is the magic character C_NUM_REPL, the '~' sign is the magic character C_END_PAGE_NUM.
     *
     * @internal
     * The count of similar entries, i.e., nodes in aTOXSources of SwTOXSortTabBase gives the PagerNumber
     * pattern.
     */
    static OUString
    ConstructPageNumberPlaceholder(size_t numberOfToxSources);

    /** Collect the attributes of a hint that shall be copied over to the TOX.
     *
     * Some text attributes are used in the TOX entries. This method defines which attributes are used.
     *
     * @param hint The hint from which the attributes are taken
     * @param pool The attribute pool for the new items
     */
    static boost::shared_ptr<SfxItemSet>
    CollectAttributesForTox(const SwTxtAttr& hint, SwAttrPool& pool);

    /** This method will call GetNumStringOfFirstNode() of the first node in the provided SwTOXSortTabBase.
     *
     * The parameters @p bUsePrefix and @p nLevel are passed to SwTxtNode::GetNumString()
     *
     * @internal
     * The method is only called if several preconditions for @p rBase are true. Check the implementation
     * for details.
     */
    static OUString
    GetNumStringOfFirstNode(const SwTOXSortTabBase& rBase, bool bUsePrefix, sal_uInt8 nLevel);

    /** Handle a chapter token.
     */
    OUString
    HandleChapterToken(const SwTOXSortTabBase& rBase, const SwFormToken& aToken, SwDoc* pDoc) const;

    /** Generate the text for a chapter token.
     */
    OUString
    GenerateTextForChapterToken(const SwFormToken& chapterToken, const SwCntntFrm* contentFrame,
            const SwCntntNode *contentNode) const;

    /** Obtain a ChapterField to use for the text generation.
     * @internal
     * This method is overridden in the unittests. Do not override it yourself.
     */
    virtual SwChapterField
    ObtainChapterField(SwChapterFieldType* chapterFieldType, const SwFormToken* chapterToken,
            const SwCntntFrm* contentFrame, const SwCntntNode *contentNode) const;

    friend class ::ToxTextGeneratorTest;
};

}

#endif /* SW_TOXTEXTGENERATOR_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

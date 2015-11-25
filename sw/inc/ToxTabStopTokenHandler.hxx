/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#ifndef TOXTABSTOPTOKENHANDLER_HXX_
#define TOXTABSTOPTOKENHANDLER_HXX_

#include "swdllapi.h"
#include "rtl/ustring.hxx"

#include "editeng/tstpitem.hxx"

struct SwFormToken;
class SwPageDesc;
class SwRootFrame;
class SwTextNode;

namespace sw {

/** This class handles tab stop tokens in the pattern for tox entries.
 *
 * @internal
 * This is an interface class. It allows to mock the class in unit tests
 */
class ToxTabStopTokenHandler
{
public:
    virtual ~ToxTabStopTokenHandler() {}

    /** Return value of HandleTabStopToken */
    struct HandledTabStopToken {
        OUString text;
        SvxTabStop tabStop;
    };

    /** Handle a tab stop token.
     *
     * @returns A HandledTabStopToken. Make sure to append the text field to the text of the target node
     *          and to provide the returned SvxTabStop to the attributes of the node.
     */
    virtual HandledTabStopToken
    HandleTabStopToken(const SwFormToken& aToken, const SwTextNode& targetNode,
            const SwRootFrame *currentLayout) const = 0;
};

/** The default implementation of ToxTabStopTokenHandler */
class DefaultToxTabStopTokenHandler : public ToxTabStopTokenHandler
{
public:

    enum TabStopReferencePolicy {TABSTOPS_RELATIVE_TO_INDENT, TABSTOPS_RELATIVE_TO_PAGE};

   /**
    * @param indexOfSectionNode
    * The index of the section node. It is needed to determine whether a page description was given by a node
    * before the tox section.
    *
    * @param defaultPageDescription
    * Which page description shall be used if we do not find one or the found page description was provided by
    * a node before the tox section
    *
    * @param tabPositionIsRelativeToParagraphIndent
    * Whether the tab position is relative to the paragraph indent. (toxForm.IsRelTabPos() is true or false.)
    *
    * @param tabstopReferencePolicy
    * How tab stops are positioned. (#i21237) The default behavior is to place tab stops relative to the page.
    */
    DefaultToxTabStopTokenHandler(sal_uInt32 indexOfSectionNode, const SwPageDesc& defaultPageDescription,
            bool tabPositionIsRelativeToParagraphIndent,
            TabStopReferencePolicy referencePolicy = TABSTOPS_RELATIVE_TO_PAGE);

    /** Handle a tab stop token.
     *
     * If the token contains tab alignment information, that is used to create the SvxTabStop.
     * Else, the information about the tab stop is taken from a page description.
     * Depending on the TabStopReferencePolicy provided in the constructor, the
     * method behaves differently when deriving the tab stop position.
     */
    ToxTabStopTokenHandler::HandledTabStopToken
    HandleTabStopToken(const SwFormToken& aToken, const SwTextNode& targetNode,
            const SwRootFrame *currentLayout) const override;

private:
    /** Test whether the page layout can be obtained by a layout rectangle.
     *
     * Is used to determine how to find tab stop position.
     */
    static bool
    CanUseLayoutRectangle(const SwTextNode& targetNode, const SwRootFrame *currentLayout);

    /** Calculate the page margin from the page description.
     *
     * This is the fallback method to determine the position of a tab stop.
     */
    long
    CalculatePageMarginFromPageDescription(const SwTextNode& targetNode) const;

    sal_uInt32 mIndexOfSectionNode;
    const SwPageDesc& mDefaultPageDescription;
    bool mTabPositionIsRelativeToParagraphIndent;
    TabStopReferencePolicy mTabStopReferencePolicy;
};

}

#endif /* TOXTABSTOPTOKENHANDLER_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

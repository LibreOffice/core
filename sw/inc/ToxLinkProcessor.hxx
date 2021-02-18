/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SW_TOXLINKPROCESSOR_HXX_
#define SW_TOXLINKPROCESSOR_HXX_

#include "fmtinfmt.hxx"
#include <rtl/ustring.hxx>

#include <memory>
#include <vector>

class SwTextNode;

class ToxLinkProcessorTest;

namespace sw {

/** A helper class for ToxTextGenerator.
 * It collects information about encountered link tokens and allows access in a processed form.
 */
class ToxLinkProcessor {
public:
    ToxLinkProcessor() {}
    virtual ~ToxLinkProcessor() {}

    void
    StartNewLink(sal_Int32 startPosition, const OUString& characterStyle);

    /** Close a link which has been found during processing.
     *
     * @internal
     * If you close more links than were opened, then the method will behave
     * as if a start link was opened at position 0 with the character style
     * STR_POOLCHR_TOXJUMP.
     */
    void
    CloseLink(sal_Int32 endPosition, const OUString& url, bool bRelative);

    /** Insert the found links as attributes to a text node */
    void
    InsertLinkAttributes(SwTextNode& node);

private:
    /** Obtain the pool id which belongs to a character style.
     *
     * @internal
     * This method is overridden in the unittests. You should not override it yourself.
     */
    virtual sal_uInt16
    ObtainPoolId(const OUString& characterStyle) const;

    /** Information about a started link */
    struct StartedLink {
        StartedLink(sal_Int32 startPosition, const OUString& characterStyle) :
                mStartPosition(startPosition), mCharacterStyle(characterStyle) {
        }
        sal_Int32 mStartPosition;
        OUString mCharacterStyle;
    };

    /** A link that has been encountered while parsing a tox.
     * A link is closed if it has both a start and an end token.
     */
    struct ClosedLink {
        ClosedLink(const OUString& url, sal_Int32 startPosition, sal_Int32 endPosition)
            : mINetFormat(url, OUString())
            , mStartTextPos(startPosition)
            , mEndTextPos(endPosition)
        {
        }
        SwFormatINetFormat mINetFormat;
        sal_Int32 mStartTextPos;
        sal_Int32 mEndTextPos;
    };

    std::vector<std::unique_ptr<ClosedLink>> m_ClosedLinks;

    std::unique_ptr<StartedLink> m_pStartedLink;

    friend class ::ToxLinkProcessorTest;
};

}

#endif /* SW_TOXLINKPROCESSOR_HXX_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

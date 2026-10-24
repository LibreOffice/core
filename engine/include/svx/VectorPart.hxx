/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <o3tl/safeint.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/svxdllapi.h>

#include <functional>
#include <map>
#include <optional>
#include <string_view>

namespace drawinglayer
{
class VectorResourceCache;
}

namespace tools
{
class JsonWriter;
}

/// The parts of a document as a client that draws vector primitives asks for them.
namespace svx::vector
{
/** The versions of one part, as the clients that draw vector primitives hold it.

    The version counts up from zero with every change a client can see, and an answer names the
    version it brings the part to. A delta carries what changed after the version a client holds.
    The versions count in the epoch of the document, so two versions say something about each
    other only within one epoch.

    One delta is pushed for a part however many clients hold it, so the part has one mark for
    where its readers stand: the version it was last pushed at. The first answer that serves the
    part sets the mark, since nothing holds the part before that.
 */
class SVXCORE_DLLPUBLIC PartVersions
{
public:
    sal_uInt64 getVersion() const { return mnVersion; }

    /// Counts the version up and returns the new version.
    sal_uInt64 countUp() { return ++mnVersion; }

    /// Counts the version up and keeps the new version as the one at which the set of entries
    /// of the part, or the order they paint in, changed last. Returns the new version.
    sal_uInt64 countUpOrder()
    {
        mnOrderVersion = countUp();
        return mnOrderVersion;
    }

    /// The version at which the set of entries or their order changed last, 0 before it did.
    sal_uInt64 getOrderVersion() const { return mnOrderVersion; }

    /// True once an answer served the part, so a client may hold it. The first whole answer
    /// leaves the version where it found it, which can be zero, so the version alone does not
    /// say whether anyone holds the part.
    bool isServed() const { return mbServed; }

    /** The version an answer steps from, or -1 for an answer that carries the whole part. A push
        steps from the version the part was last pushed at, and a pull from nSinceVersion, the
        version the client holds. A version above the part's own comes from content the part
        never served, so the part is sent whole.
     */
    sal_Int64 stepFrom(sal_Int64 nSinceVersion, bool bPush) const;

    /// True when an answer that steps from nFrom carries a change. The whole part always does,
    /// and a delta does when the version moved after nFrom.
    bool isChangedSince(sal_Int64 nFrom) const
    {
        return nFrom < 0 || mnVersion > o3tl::make_unsigned(nFrom);
    }

    /** Moves the mark once an answer was written. A push moves it to the version the push
        carried. A pull moves it only while nothing served the part: nothing holds the part then,
        and starting the mark at what the pull carried keeps the next delta down to what changed
        after it. A later pull serves one client, while the mark stands for all of them.
     */
    void noteWritten(bool bPush);

private:
    sal_uInt64 mnVersion = 0;
    /// The version the part was last pushed at, or the one the first answer carried.
    sal_uInt64 mnLastSentVersion = 0;
    sal_uInt64 mnOrderVersion = 0;
    bool mbServed = false;
};

/** Writes the fields that every answer about a part opens with: the type, which is
    "vectorprimitivesdelta" for a delta and "vectorprimitives" for the whole part, the mode,
    which names the page list the part sits in, the id of the part, left out when it is empty,
    and the epoch the versions count in.
 */
SVXCORE_DLLPUBLIC void writeAnswerHeader(tools::JsonWriter& rWriter, bool bDelta, sal_Int32 nMode,
                                         std::string_view rPartId, sal_Int32 nEpoch);

/// Writes the version a delta steps from as "from", left out for an nFrom of -1, and the version
/// the answer brings the part to as "version".
SVXCORE_DLLPUBLIC void writeAnswerVersions(tools::JsonWriter& rWriter, sal_Int64 nFrom,
                                           sal_uInt64 nVersion);

/// What a request of a client that draws vector primitives names.
struct PartRequest
{
    /// The id of the part, empty when the request names none.
    OString maPartId;
    /// The page list the part sits in. Nothing when the request names none.
    std::optional<sal_Int32> moMode;
    /// The version the client holds. -1 when it holds none, or when the version counts in
    /// another epoch than the one of the document, or in no named epoch.
    sal_Int64 mnSinceVersion = -1;
    /// True when the request names the epoch the document counts in.
    bool mbSameEpoch = false;
    /// The number the client gave the request, which the answer names back.
    std::optional<sal_Int64> moRequestId;
    /// Every parameter of the request, by name.
    std::map<OUString, OUString> maParameters;
};

/// True for "VectorPrimitives", "VectorRenderingGraphics", "VectorRenderingFont" and
/// "VectorForgetPart", the commands a client that draws vector primitives sends.
SVXCORE_DLLPUBLIC bool isPartCommand(std::u16string_view rCommand);

/// The parameters of rCommand, a command with its parameters, read against nEpoch, the epoch
/// the document counts in.
SVXCORE_DLLPUBLIC PartRequest readPartRequest(std::string_view rCommand, sal_Int32 nEpoch);

/** Answers rCommand, a command with its parameters, when it is one of the commands isPartCommand
    names, and returns false for any other command. A graphic and a font come from rResources.
    rWritePart writes the primitives of the part a request names. rForgetPart drops the state of
    a part, and the answer to it is empty. Without rForgetPart a part is kept.
 */
SVXCORE_DLLPUBLIC bool
answerPartCommand(tools::JsonWriter& rWriter, std::string_view rCommand,
                  const drawinglayer::VectorResourceCache& rResources,
                  const std::function<void(const PartRequest&)>& rWritePart,
                  const std::function<void(std::string_view)>& rForgetPart = {});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

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

#include <algorithm>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

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
class PartVersions;
struct PartCallbacks;

/** Writes one answer about a part, or nothing, and returns true when it wrote one.

    A pull is the answer to a request, and nSinceVersion is the version the request names, -1 when
    it names none. A push is the delta sent to every client that holds the part. The steps run in
    this order:

    - A push of a part that is not served writes nothing.
    - The version the answer steps from is PartVersions::stepFrom.
    - PartCallbacks::maResolve counts the versions up for what changed.
    - A push writes nothing when the version did not move after the one it steps from, unless
      PartCallbacks::maIsPushDue says there is something else to carry.
    - PartCallbacks::maWriteHeader writes the header, then "from" and "version" are written as
      writeAnswerVersions does.
    - PartCallbacks::maWriteOrder runs for a delta when the order version is newer than the
      version stepped from, then PartCallbacks::maWriteEntries.
    - The mark of the readers moves, and a pull makes the part served.
 */
SVXCORE_DLLPUBLIC bool writePart(tools::JsonWriter& rWriter, PartVersions& rVersions,
                                 sal_Int64 nSinceVersion, bool bPush,
                                 const PartCallbacks& rCallbacks);

/** The versions of one part, as the clients that draw vector primitives hold it.

    The version counts up with every change a client can see, and an answer names the version it
    brings the part to. A delta carries what changed after the version a client holds. The
    versions count in the epoch of the document, so two versions say something about each other
    only within one epoch.

    One delta is pushed for a part however many clients hold it, so the part has one mark for
    where its readers stand: the version it was last pushed at. The first answer that serves the
    part sets the mark, since nothing holds the part before that.

    A part is served while a client may hold it. A client holds a part from the first answer to
    its own request that names a version, whole or delta, and until it lets the part go. So the
    part becomes served when an answer to a pull writes its version. A push goes to the clients
    that hold the part already, so it leaves the served mark as it is. The part stops being
    served only when its versions are dropped, which happens when the last client lets the part
    go and when the part is gone from the document.
 */
class SVXCORE_DLLPUBLIC PartVersions
{
public:
    PartVersions() = default;

    /// A part whose versions count up from nStartVersion. No answer steps from a version below
    /// it.
    explicit PartVersions(sal_uInt64 nStartVersion)
        : mnVersion(nStartVersion)
        , mnLastSentVersion(nStartVersion)
        , mnFirstVersion(nStartVersion)
    {
    }

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

    /// True once an answer to a pull wrote a version for the part, so a client may hold it. The
    /// first whole answer can leave the version where it found it, so the version alone does not
    /// say whether anyone holds the part.
    bool isServed() const { return mbServed; }

    /** The version an answer steps from, or -1 for an answer that carries the whole part. A push
        steps from the version the part was last pushed at, and a pull from nSinceVersion, the
        version the client holds. No client holds a part that is not served, so a pull of such a
        part carries it whole. A version above the part's own comes from content the part never
        served, and a version below the one the part started at comes from content the part does
        not know, so in both cases the part is sent whole.
     */
    sal_Int64 stepFrom(sal_Int64 nSinceVersion, bool bPush) const;

    /// True when an answer that steps from nFrom carries a change. The whole part always does,
    /// and a delta does when the version moved after nFrom.
    bool isChangedSince(sal_Int64 nFrom) const
    {
        return nFrom < 0 || mnVersion > o3tl::make_unsigned(nFrom);
    }

private:
    friend SVXCORE_DLLPUBLIC bool writePart(tools::JsonWriter& rWriter, PartVersions& rVersions,
                                            sal_Int64 nSinceVersion, bool bPush,
                                            const PartCallbacks& rCallbacks);

    /** Moves the mark once an answer wrote the version. A push moves it to the version the push
        carried. A pull makes the part served, and moves the mark only when the part was not
        served before: nothing held the part then, and starting the mark at what the pull carried
        keeps the next delta down to what changed after it. A later pull serves one client, while
        the mark stands for all of them.
     */
    void noteWritten(bool bPush);

    sal_uInt64 mnVersion = 0;
    /// The version the part was last pushed at, or the one the first answer carried.
    sal_uInt64 mnLastSentVersion = 0;
    sal_uInt64 mnOrderVersion = 0;
    /// The lowest version a delta can step from: the version the part started at.
    sal_uInt64 mnFirstVersion = 0;
    bool mbServed = false;
};

/** The steps an application supplies to write one answer about a part. maResolve, maWriteHeader
    and maWriteEntries are always set. nFrom is the version the answer steps from, -1 for the
    whole part.
 */
struct PartCallbacks
{
    /// Compares the part with what was written for it last, and counts the versions of the part
    /// up for what differs.
    std::function<void(sal_Int64 nFrom)> maResolve;
    /// True when a push has something to carry while the version of the part did not change. An
    /// unset function counts as false.
    std::function<bool()> maIsPushDue;
    /// Writes the fields the answer opens with, up to the ones that come before "from" and
    /// "version".
    std::function<void(bool bDelta)> maWriteHeader;
    /// Writes the order of the entries, for a delta whose order version is newer than nFrom.
    /// An application that writes its order in another place leaves it unset.
    std::function<void()> maWriteOrder;
    /// Writes the entries that changed after nFrom, or every entry for the whole part, and what
    /// else follows the versions.
    std::function<void(sal_Int64 nFrom)> maWriteEntries;
};

/** The parts a document serves, by the key the application names a part with. State is what the
    application keeps for one part, and it holds the versions of the part in a member maVersions.

    The table keeps a floor for its versions: the lowest version a new part starts at. The floor
    is zero until a part is dropped, and dropping a part raises it above every version that part
    reached. So a part made later, under the same key or another one, starts above every version
    a client can still name from a dropped part, and a request that names such a version is
    answered with the whole part.
 */
template <typename Key, typename State, typename Map = std::unordered_map<Key, State>>
class PartTable
{
public:
    /// The state of the part, or nullptr when the table holds none.
    State* find(const Key& rKey)
    {
        auto aIterator = maParts.find(rKey);
        return aIterator == maParts.end() ? nullptr : &aIterator->second;
    }

    const State* find(const Key& rKey) const
    {
        auto aIterator = maParts.find(rKey);
        return aIterator == maParts.end() ? nullptr : &aIterator->second;
    }

    /// The state of the part. A part the table does not hold yet is made, and its versions
    /// start at the floor.
    State& get(const Key& rKey)
    {
        auto [aIterator, bNew] = maParts.try_emplace(rKey);
        if (bNew)
            aIterator->second.maVersions = PartVersions(mnFloor);
        return aIterator->second;
    }

    /// True when the table holds the part and a client may hold it.
    bool isServed(const Key& rKey) const
    {
        const State* pState = find(rKey);
        return pState && pState->maVersions.isServed();
    }

    /// True when a client may hold any part of the table.
    bool isAnyServed() const
    {
        return std::ranges::any_of(
            maParts, [](const auto& rPart) { return rPart.second.maVersions.isServed(); });
    }

    /// Drops everything the application keeps for the part but its versions, so the versions
    /// keep counting up and a served part stays served.
    void keepVersionsOnly(const Key& rKey)
    {
        State* pState = find(rKey);
        if (!pState)
            return;
        State aKept;
        aKept.maVersions = pState->maVersions;
        *pState = std::move(aKept);
    }

    /// Drops the part with its versions, for a part that no client holds any more and for a
    /// part that is gone from the document. The floor rises above the version the part reached.
    void forget(const Key& rKey)
    {
        auto aIterator = maParts.find(rKey);
        if (aIterator == maParts.end())
            return;
        mnFloor = std::max(mnFloor, aIterator->second.maVersions.getVersion() + 1);
        maParts.erase(aIterator);
    }

    /// Drops every part and the floor. The versions that follow say nothing about the ones
    /// before, so they count in a new epoch.
    void clear()
    {
        maParts.clear();
        mnFloor = 0;
    }

    /// The lowest version a part made now starts at.
    sal_uInt64 getFloor() const { return mnFloor; }

    bool empty() const { return maParts.empty(); }
    size_t size() const { return maParts.size(); }
    typename Map::iterator begin() { return maParts.begin(); }
    typename Map::iterator end() { return maParts.end(); }
    typename Map::const_iterator begin() const { return maParts.begin(); }
    typename Map::const_iterator end() const { return maParts.end(); }

private:
    Map maParts;
    sal_uInt64 mnFloor = 0;
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

/// The commands a client that draws vector primitives sends.
enum class PartCommand
{
    /// Any other command.
    None,
    /// "VectorPrimitives", which asks for the primitives of a part.
    Primitives,
    /// "VectorRenderingGraphics", which asks for a graphic.
    Graphic,
    /// "VectorRenderingFont", which asks for a font.
    Font,
    /// "VectorForgetPart", which drops the state of a part.
    ForgetPart
};

/// The name of rCommand, a command with its parameters like ".uno:Name?key=value": the text
/// between ".uno:" and the "?" that opens the parameters. Empty when rCommand does not start
/// with ".uno:".
SVXCORE_DLLPUBLIC std::string_view commandNameOf(std::string_view rCommand);

/// Which of the commands of PartCommand rCommand is, a command with its parameters. The whole
/// name decides, so a longer name that only starts with one of them is PartCommand::None.
SVXCORE_DLLPUBLIC PartCommand partCommandOf(std::string_view rCommand);

/// The parameters of rCommand, a command with its parameters, read against nEpoch, the epoch
/// the document counts in.
SVXCORE_DLLPUBLIC PartRequest readPartRequest(std::string_view rCommand, sal_Int32 nEpoch);

/** Answers rCommand, a command with its parameters, when it is "VectorPrimitives" or
    "VectorForgetPart", and returns false for any other command.
    rWritePart writes the primitives of the part a request names. rForgetPart drops the state of
    a part, and the answer to it is empty. Without rForgetPart a part is kept.
 */
SVXCORE_DLLPUBLIC bool
answerPartCommand(std::string_view rCommand,
                  const drawinglayer::VectorResourceCache& rResources,
                  const std::function<void(const PartRequest&)>& rWritePart,
                  const std::function<void(std::string_view)>& rForgetPart = {});

/// The answer to rCommand from rResources, when rCommand is a "VectorRenderingGraphics" command
/// with its checksum parameter or a "VectorRenderingFont" command with its id parameter: a line of
/// JSON, then the bytes of the image or of the font file after a newline when there is one. Empty
/// for any other command, and when the command names no checksum or no id.
SVXCORE_DLLPUBLIC std::string
answerResourceCommand(std::string_view rCommand,
                      const drawinglayer::VectorResourceCache& rResources);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

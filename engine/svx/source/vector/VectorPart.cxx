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

#include <svx/VectorPart.hxx>

#include <drawinglayer/processor2d/VectorResourceCache.hxx>
#include <o3tl/safeint.hxx>
#include <sfx2/kit/helper.hxx>
#include <tools/json_writer.hxx>

namespace svx::vector
{
sal_Int64 PartVersions::stepFrom(sal_Int64 nSinceVersion, bool bPush) const
{
    if (bPush)
        nSinceVersion = sal_Int64(mnLastSentVersion);
    else if (!mbServed)
        return -1;
    if (nSinceVersion > sal_Int64(mnVersion) || nSinceVersion < sal_Int64(mnFirstVersion))
        return -1;
    return nSinceVersion;
}

void PartVersions::noteWritten(bool bPush)
{
    if (bPush)
        mnLastSentVersion = mnVersion;
    else if (!mbServed)
    {
        mbServed = true;
        mnLastSentVersion = mnVersion;
    }
}

bool writePart(tools::JsonWriter& rWriter, PartVersions& rVersions, sal_Int64 nSinceVersion,
               bool bPush, const PartCallbacks& rCallbacks)
{
    // A push goes to the clients that hold the part.
    if (bPush && !rVersions.isServed())
        return false;

    const sal_Int64 nFrom = rVersions.stepFrom(nSinceVersion, bPush);
    const bool bDelta = nFrom >= 0;

    // A change only marked what may differ. The comparison decides whether the version moves, so
    // it runs before the version is looked at and written.
    rCallbacks.maResolve(nFrom);

    if (bPush && !rVersions.isChangedSince(nFrom)
        && !(rCallbacks.maIsPushDue && rCallbacks.maIsPushDue()))
        return false;

    rCallbacks.maWriteHeader(bDelta);
    writeAnswerVersions(rWriter, nFrom, rVersions.getVersion());

    // The order names every entry of the part, so a delta carries it only when it changed after
    // the version the delta steps from.
    if (bDelta && rCallbacks.maWriteOrder
        && rVersions.getOrderVersion() > o3tl::make_unsigned(nFrom))
        rCallbacks.maWriteOrder();

    rCallbacks.maWriteEntries(nFrom);

    // The answer named a version, so a client that pulled it holds the part from here on.
    rVersions.noteWritten(bPush);
    return true;
}

void writeAnswerHeader(tools::JsonWriter& rWriter, bool bDelta, sal_Int32 nMode,
                       std::string_view rPartId, sal_Int32 nEpoch)
{
    rWriter.put("type", bDelta ? std::string_view("vectorprimitivesdelta")
                               : std::string_view("vectorprimitives"));
    rWriter.put("mode", nMode);
    if (!rPartId.empty())
        rWriter.put("partId", rPartId);
    rWriter.put("epoch", nEpoch);
}

void writeAnswerVersions(tools::JsonWriter& rWriter, sal_Int64 nFrom, sal_uInt64 nVersion)
{
    if (nFrom >= 0)
        rWriter.put("from", nFrom);
    rWriter.put("version", sal_Int64(nVersion));
}

std::string_view commandNameOf(std::string_view rCommand)
{
    static constexpr std::string_view aPrefix(".uno:");
    if (!rCommand.starts_with(aPrefix))
        return {};
    rCommand.remove_prefix(aPrefix.size());
    return rCommand.substr(0, rCommand.find('?'));
}

PartCommand partCommandOf(std::string_view rCommand)
{
    const std::string_view aName = commandNameOf(rCommand);
    if (aName == "VectorPrimitives")
        return PartCommand::Primitives;
    if (aName == "VectorRenderingGraphics")
        return PartCommand::Graphic;
    if (aName == "VectorRenderingFont")
        return PartCommand::Font;
    if (aName == "VectorForgetPart")
        return PartCommand::ForgetPart;
    return PartCommand::None;
}

PartRequest readPartRequest(std::string_view rCommand, sal_Int32 nEpoch)
{
    PartRequest aRequest;
    aRequest.maParameters = KitHelper::parseCommandParameters(OUString::fromUtf8(rCommand));
    const std::map<OUString, OUString>& rParameters = aRequest.maParameters;

    if (auto it = rParameters.find(u"partid"_ustr); it != rParameters.end())
        aRequest.maPartId = it->second.toUtf8();
    if (auto it = rParameters.find(u"mode"_ustr); it != rParameters.end())
        aRequest.moMode = it->second.toInt32();
    if (auto it = rParameters.find(u"since"_ustr); it != rParameters.end())
        aRequest.mnSinceVersion = it->second.toInt64();
    if (auto it = rParameters.find(u"requestid"_ustr); it != rParameters.end())
        aRequest.moRequestId = it->second.toInt64();

    // A version of another epoch, or one that names no epoch, says nothing about this document,
    // so the client is served the part whole.
    auto itEpoch = rParameters.find(u"epoch"_ustr);
    aRequest.mbSameEpoch = itEpoch != rParameters.end() && itEpoch->second.toInt32() == nEpoch;
    if (!aRequest.mbSameEpoch)
        aRequest.mnSinceVersion = -1;
    return aRequest;
}

std::string answerResourceCommand(std::string_view rCommand,
                                  const drawinglayer::VectorResourceCache& rResources)
{
    const PartCommand eCommand = partCommandOf(rCommand);
    if (eCommand != PartCommand::Graphic && eCommand != PartCommand::Font)
        return {};

    const PartRequest aRequest = readPartRequest(rCommand, rResources.getEpoch());
    const bool bGraphic = eCommand == PartCommand::Graphic;
    auto it = aRequest.maParameters.find(bGraphic ? u"checksum"_ustr : u"id"_ustr);
    if (it == aRequest.maParameters.end())
        return {};
    if (bGraphic)
        return rResources.graphicAnswer(it->second.toInt64());
    // The id of a font is the hash of its file, in hexadecimal.
    return rResources.fontAnswer(it->second.toUInt64(16));
}

bool answerPartCommand(std::string_view rCommand,
                       const drawinglayer::VectorResourceCache& rResources,
                       const std::function<void(const PartRequest&)>& rWritePart,
                       const std::function<void(std::string_view)>& rForgetPart)
{
    const PartCommand eCommand = partCommandOf(rCommand);
    if (eCommand != PartCommand::Primitives && eCommand != PartCommand::ForgetPart)
        return false;

    const PartRequest aRequest = readPartRequest(rCommand, rResources.getEpoch());
    if (eCommand == PartCommand::Primitives)
        rWritePart(aRequest);
    else if (rForgetPart && !aRequest.maPartId.isEmpty())
        rForgetPart(aRequest.maPartId);
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

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
#include <o3tl/string_view.hxx>
#include <sfx2/kit/helper.hxx>
#include <tools/json_writer.hxx>

namespace svx::vector
{
sal_Int64 PartVersions::stepFrom(sal_Int64 nSinceVersion, bool bPush) const
{
    if (bPush)
        nSinceVersion = sal_Int64(mnLastSentVersion);
    if (nSinceVersion > sal_Int64(mnVersion))
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

bool isPartCommand(std::u16string_view rCommand)
{
    return rCommand == u"VectorPrimitives" || rCommand == u"VectorRenderingGraphics"
           || rCommand == u"VectorRenderingFont" || rCommand == u"VectorForgetPart";
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

bool answerPartCommand(tools::JsonWriter& rWriter, std::string_view rCommand,
                       const drawinglayer::VectorResourceCache& rResources,
                       const std::function<void(const PartRequest&)>& rWritePart,
                       const std::function<void(std::string_view)>& rForgetPart)
{
    const bool bGraphic = o3tl::starts_with(rCommand, ".uno:VectorRenderingGraphics");
    const bool bFont = o3tl::starts_with(rCommand, ".uno:VectorRenderingFont");
    const bool bPrimitives = o3tl::starts_with(rCommand, ".uno:VectorPrimitives");
    const bool bForget = o3tl::starts_with(rCommand, ".uno:VectorForgetPart");
    if (!bGraphic && !bFont && !bPrimitives && !bForget)
        return false;

    const PartRequest aRequest = readPartRequest(rCommand, rResources.getEpoch());
    const std::map<OUString, OUString>& rParameters = aRequest.maParameters;
    if (bGraphic)
    {
        if (auto it = rParameters.find(u"checksum"_ustr); it != rParameters.end())
            rResources.writeGraphic(rWriter, it->second.toInt64());
    }
    else if (bFont)
    {
        // The id of a font is the hash of its file, in hexadecimal.
        if (auto it = rParameters.find(u"id"_ustr); it != rParameters.end())
            rResources.writeFont(rWriter, it->second.toUInt64(16));
    }
    else if (bPrimitives)
        rWritePart(aRequest);
    else if (rForgetPart && !aRequest.maPartId.isEmpty())
        rForgetPart(aRequest.maPartId);
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

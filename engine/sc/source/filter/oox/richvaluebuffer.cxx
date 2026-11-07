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

#include <sal/config.h>

#include <richvaluebuffer.hxx>

#include <algorithm>
#include <iterator>
#include <string_view>

#include <o3tl/safeint.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/helper/graphichelper.hxx>
#include <sal/log.hxx>
#include <vcl/graph.hxx>

namespace oox::xls
{
namespace
{
/** Name of the rich value structure that stands for a picture stored in the package. */
constexpr OUString gaLocalImageType = u"_localImage"_ustr;

/** Name of the key of a picture rich value that holds the index into the relationship part. */
constexpr OUString gaLocalImageKey = u"_rvRel:LocalImageIdentifier"_ustr;

/** Name of the rich value structure that stands for an error. */
constexpr OUString gaErrorStructureType = u"_error"_ustr;

/** Name of the key of an error rich value that holds the type of the error. */
constexpr OUString gaErrorTypeKey = u"errorType"_ustr;
}

RichValueBuffer::RichValueBuffer(const WorkbookHelper& rHelper)
    : WorkbookHelper(rHelper)
{
}

void RichValueBuffer::addRichValueBlock(sal_Int32 nBlock, sal_Int32 nRichValue)
{
    if (nBlock >= 0 && nRichValue >= 0)
        maBlockToRichValue[nBlock] = nRichValue;
}

void RichValueBuffer::addValueMetadata(sal_Int32 nEntry, sal_Int32 nBlock)
{
    if (nEntry > 0 && nBlock >= 0)
        maValueMetadataToBlock[nEntry] = nBlock;
}

OUString RichValueBuffer::getRichValueEntry(sal_Int32 nValueMetadataIndex,
                                            std::u16string_view aType,
                                            std::u16string_view aKey) const
{
    const auto aBlock = maValueMetadataToBlock.find(nValueMetadataIndex);
    if (aBlock == maValueMetadataToBlock.end())
        return OUString();

    const auto aRichValue = maBlockToRichValue.find(aBlock->second);
    if (aRichValue == maBlockToRichValue.end())
        return OUString();

    if (aRichValue->second < 0 || o3tl::make_unsigned(aRichValue->second) >= maRichValues.size())
        return OUString();
    const RichValue& rRichValue = maRichValues[aRichValue->second];

    if (rRichValue.mnStructure < 0
        || o3tl::make_unsigned(rRichValue.mnStructure) >= maStructures.size())
        return OUString();
    const RichValueStructure& rStructure = maStructures[rRichValue.mnStructure];
    if (rStructure.maType != aType)
        return OUString();

    // The value sits at the place its key takes among the keys.
    const auto aKeyPosition = std::find(rStructure.maKeys.begin(), rStructure.maKeys.end(), aKey);
    if (aKeyPosition == rStructure.maKeys.end())
        return OUString();

    const size_t nKey = aKeyPosition - rStructure.maKeys.begin();
    if (nKey >= rRichValue.maValues.size())
        return OUString();
    return rRichValue.maValues[nKey];
}

OUString RichValueBuffer::getImagePath(sal_Int32 nValueMetadataIndex) const
{
    const OUString aImage
        = getRichValueEntry(nValueMetadataIndex, gaLocalImageType, gaLocalImageKey);
    if (aImage.isEmpty())
        return OUString();

    const sal_Int32 nImage = aImage.toInt32();
    if (nImage < 0 || o3tl::make_unsigned(nImage) >= maImagePaths.size())
        return OUString();
    return maImagePaths[nImage];
}

OUString RichValueBuffer::getErrorName(sal_Int32 nValueMetadataIndex) const
{
    const OUString aErrorType
        = getRichValueEntry(nValueMetadataIndex, gaErrorStructureType, gaErrorTypeKey);
    if (aErrorType.isEmpty())
        return OUString();

    // The error type is the ERROR.TYPE number of the error minus one. An error the document does
    // not have reads as #VALUE!, which is also the cell error name the cell holds.
    static constexpr std::u16string_view aErrorNames[] = {
        u"#NULL!", u"#DIV/0!", u"#VALUE!", u"#REF!", u"#NAME?", u"#NUM!", u"#N/A",
        u"",       u"#SPILL!", u"",        u"",      u"",       u"",     u"#CALC!",
    };
    const sal_Int32 nErrorType = aErrorType.toInt32();
    if (nErrorType < 0 || o3tl::make_unsigned(nErrorType) >= std::size(aErrorNames)
        || aErrorNames[nErrorType].empty())
        return u"#VALUE!"_ustr;
    return OUString(aErrorNames[nErrorType]);
}

bool RichValueBuffer::isImage(sal_Int32 nValueMetadataIndex) const
{
    return !getImagePath(nValueMetadataIndex).isEmpty();
}

Graphic RichValueBuffer::getImage(sal_Int32 nValueMetadataIndex) const
{
    const OUString aPath = getImagePath(nValueMetadataIndex);
    if (aPath.isEmpty())
        return Graphic();

    const cpo::uno::Reference<css::graphic::XGraphic> xGraphic
        = getBaseFilter().getGraphicHelper().importEmbeddedGraphic(aPath);
    SAL_WARN_IF(!xGraphic.is(), "sc.filter",
                "RichValueBuffer::getImage - cannot read the picture " << aPath);
    return Graphic(xGraphic);
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

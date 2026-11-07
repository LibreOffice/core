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

#include <sal/config.h>

#include <map>
#include <string_view>
#include <vector>

#include <rtl/ustring.hxx>
#include <vcl/graph.hxx>

#include "workbookhelper.hxx"

namespace oox::xls
{
/** One entry of the rich value structure part.

    The type name is the name a rich value refers to, for example "_localImage". The keys are the
    names of the values a rich value of that structure carries, in the order the values appear. */
struct RichValueStructure
{
    OUString maType;
    std::vector<OUString> maKeys;
};

/** The values of one rich value, in the order the keys of its structure list them. */
struct RichValue
{
    sal_Int32 mnStructure = -1;
    std::vector<OUString> maValues;
};

/** The rich value parts of a workbook, and the picture or the error each rich value stands for.

    A cell that carries a picture holds the error value #VALUE! and a value metadata index. That
    index leads through the metadata part to a rich value, and a rich value of the "_localImage"
    structure names a picture in the media folder of the package. A cell with an error that has no
    cell error name of its own, like #SPILL!, also holds #VALUE!, and its rich value of the
    "_error" structure says which error it is. */
class RichValueBuffer final : public WorkbookHelper
{
public:
    explicit RichValueBuffer(const WorkbookHelper& rHelper);

    /** Returns true when the passed value metadata index leads to a picture. The index counts from
        one, the way a cell writes it. */
    bool isImage(sal_Int32 nValueMetadataIndex) const;

    /** Returns the picture a value metadata index stands for, empty when the index names anything
        other than a picture. The index counts from one, the way a cell writes it. */
    Graphic getImage(sal_Int32 nValueMetadataIndex) const;

    /** Returns the formula name of the error a value metadata index stands for, like "#SPILL!",
        and empty when the index names anything other than an error. An error that the document
        does not have is "#VALUE!". The index counts from one, the way a cell writes it. */
    OUString getErrorName(sal_Int32 nValueMetadataIndex) const;

    /** Adds a structure read from the rich value structure part. */
    void addStructure(RichValueStructure aStructure)
    {
        maStructures.push_back(std::move(aStructure));
    }
    /** Adds a rich value read from the rich value part. */
    void addRichValue(RichValue aValue) { maRichValues.push_back(std::move(aValue)); }
    /** Adds the path of a picture named by the rich value relationship part. */
    void addImagePath(const OUString& rPath) { maImagePaths.push_back(rPath); }
    /** Records that the rich value block at the passed position stands for the passed rich
        value. */
    void addRichValueBlock(sal_Int32 nBlock, sal_Int32 nRichValue);
    /** Records that the value metadata entry at the passed position names the passed block of the
        rich value metadata type. The entry position counts from one. */
    void addValueMetadata(sal_Int32 nEntry, sal_Int32 nBlock);

private:
    /** Returns the value under the passed key of the rich value a value metadata index leads to,
        empty when that rich value has a structure of another type or no such key. */
    OUString getRichValueEntry(sal_Int32 nValueMetadataIndex, std::u16string_view aType,
                               std::u16string_view aKey) const;

    /** Returns the path of the picture a value metadata index leads to, empty when the index names
        anything other than a picture. */
    OUString getImagePath(sal_Int32 nValueMetadataIndex) const;

    std::vector<RichValueStructure> maStructures;
    std::vector<RichValue> maRichValues;
    std::vector<OUString> maImagePaths;
    std::map<sal_Int32, sal_Int32> maBlockToRichValue;
    std::map<sal_Int32, sal_Int32> maValueMetadataToBlock;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

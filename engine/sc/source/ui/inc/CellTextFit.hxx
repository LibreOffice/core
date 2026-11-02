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

#include <address.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svl/languageoptions.hxx>

#include <functional>
#include <optional>
#include <string_view>

class ScDocument;
struct ScRefCellValue;

namespace sc
{
/// The longest text a cell draws. Longer text is cut.
inline constexpr sal_Int32 constMaxDrawTextLength = 32767;

/// The width of a text, in the unit of the widths and the space passed along with the measure.
using TextMeasure = std::function<double(const OUString&)>;

/// Whether the protection of the sheet hides the content of the cell. With bPrint the hide when
/// printing flag of the cell counts too.
bool isCellContentHidden(const ScDocument& rDocument, const ScAddress& rAddress, bool bTabProtected,
                         bool bShowFormulas, bool bPrint);

/// Whether text can overflow into the cell: it shows no content and is not part of a merged area.
bool isAvailableForOverflow(ScDocument& rDocument, const ScAddress& rAddress, bool bTabProtected,
                            bool bShowFormulas, bool bPrint);

/// Whether the script of a text is none of Latin, Asian and complex alone, so that the EditEngine
/// lays the text out.
bool isAmbiguousScript(SvtScriptType nScript);

/// The text with the repeat character of its number format inserted at nRepeatPosition as often
/// as it fits into fSpace. Nothing when no character fits. A character narrower than fMinWidth
/// is not repeated.
std::optional<OUString> fillWithRepeatCharacter(const OUString& rText, sal_Int32 nRepeatPosition,
                                                sal_Unicode cRepeat, double fSpace,
                                                double fMinWidth, const TextMeasure& rMeasure);

/// The text repeated as often as it fits into fAvailable, for a cell aligned to repeat. Nothing
/// when it fits only once. fExtraWidth is added to the width of each copy.
std::optional<OUString> repeatToWidth(std::u16string_view rText, double fTextWidth,
                                      double fAvailable, double fExtraWidth,
                                      const TextMeasure& rMeasure);

/// Shrink text to fit fAvailable. rFirstStep shrinks it to the scale that fits by measure, and
/// rNextStep shrinks it by another 10%. Each returns the new size. True when the text fits.
bool shrinkToFit(double fAvailable, const std::function<double()>& rFirstStep,
                 const std::function<double()>& rNextStep);

/// The widths of the characters that a number in the General format is made of.
struct NumberWidths
{
    std::function<double()> maMaxDigit;
    std::function<double()> maSign;
    std::function<double()> maDecimalSeparator;
    std::function<double()> maExponent;
};

/// What fitNumberToWidth did with the text.
enum class NumberFit
{
    /// The cell holds no number value, or its digits cannot be measured. The text is unchanged.
    Unchanged,
    /// The number format could not write the number. The text is what it wrote.
    Failed,
    /// The text holds the number with as many digits as fit.
    Fits,
    /// The number does not fit, or it is an error or a formula shown as text: "###" stands in.
    Hash,
};

/// Write the number of the cell in the General format with as many digits as fit into fWidth.
/// Any other format does not fit and gives NumberFit::Hash.
NumberFit fitNumberToWidth(ScRefCellValue aCell, sal_uInt32 nFormat, bool bShowFormulas,
                           ScDocument& rDocument, double fWidth, const NumberWidths& rWidths,
                           const TextMeasure& rMeasure, OUString& rText);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

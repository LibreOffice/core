/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <editdoc.hxx>
#include <EditSelection.hxx>
#include <rtl/string.hxx>
#include <tools/stream.hxx>
#include <comphelper/errcode.hxx>
#include <functional>
#include <string_view>

class EditEngine;
class ParaPortion;

namespace editeng
{
/** Writes an EditEngine document (or selection) as Markdown text.
 *
 */
class MarkdownWriter
{
public:
    explicit MarkdownWriter(const EditEngine& rEngine);

    /// Write the given selection to rOutput; returns the stream error code.
    ErrCode Write(SvStream& rOutput, EditSelection aSel) const;

    /// Return the whole document as a Markdown OString.
    OString GetString() const;

private:
    void WriteContent(const std::function<void(std::string_view)>& rOut, sal_Int32 nStartNode,
                      sal_Int32 nEndNode, sal_Int32 nStartPos, sal_Int32 nEndPos) const;

    static bool IsMarkdownSpecial(sal_Unicode c);
    static bool IsStartOfLineSpecial(sal_Unicode c);
    static OString EscapeMarkdown(const OUString& rText);

    bool IsAllCodeParagraph(sal_Int32 nNode, const ParaPortion* pPortion, sal_Int32 nStart,
                            sal_Int32 nEnd) const;

    const EditEngine& mrEngine;
};

} // namespace editeng

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

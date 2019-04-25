/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#include <sax/fshelper.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace beans
{
struct PropertyValue;
}
}
}
}
class SwDoc;

/// Handles DOCX export of table styles, based on InteropGrabBag.
class DocxTableStyleExport
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

public:
    void TableStyles(sal_Int32 nCountStylesToWrite);

    /// Writes <w:rPr>...</w:rPr> based on grab-bagged character properties.
    void CharFormat(css::uno::Sequence<css::beans::PropertyValue>& rRPr);

    /// Writes <w:cnfStyle .../> based on grab-bagged para, cell or row properties.
    void CnfStyle(css::uno::Sequence<css::beans::PropertyValue>& rAttributeList);

    void SetSerializer(const sax_fastparser::FSHelperPtr& pSerializer);
    DocxTableStyleExport(SwDoc* pDoc, const sax_fastparser::FSHelperPtr& pSerializer);
    ~DocxTableStyleExport();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

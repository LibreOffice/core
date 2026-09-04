/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_DOCXTABLESTYLEEXPORT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_DOCXTABLESTYLEEXPORT_HXX

#include <memory>

#include <sax/fshelper.hxx>

namespace com::sun::star::beans
{
struct PropertyValue;
}
class SwDoc;

/// Handles DOCX export of table styles, based on InteropGrabBag.
class DocxTableStyleExport
{
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

public:
    void TableStyles(sal_Int32 nCountStylesToWrite);

    /// Writes <w:cnfStyle .../> based on grab-bagged para, cell or row properties.
    void CnfStyle(const cpo::uno::Sequence<css::beans::PropertyValue>& rAttributeList);

    void SetSerializer(const sax_fastparser::FSHelperPtr& pSerializer);
    /// bEcma: the file is written in the first ECMA edition of the format, which spells
    /// alignment as left and right rather than start and end.
    DocxTableStyleExport(SwDoc& rDoc, const sax_fastparser::FSHelperPtr& pSerializer, bool bEcma);
    ~DocxTableStyleExport();

    // This function only returns useful/cached results after EndStyles - i.e. in WriteMainText
    bool FirstRowHasTblHeader(const OUString& rStyleId) const;

    /// The style id a table refers to a table style by: the id the style came into the
    /// document with, when it was loaded from DOCX, else the id its name gives.
    OUString GetDocxStyleId(std::u16string_view rStyleName) const;
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXTABLESTYLEEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

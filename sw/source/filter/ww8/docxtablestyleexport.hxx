/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_DOCXTABLESTYLEEXPORT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_DOCXTABLESTYLEEXPORT_HXX

#include <boost/shared_ptr.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <sax/fshelper.hxx>

class SwDoc;

/// Handles DOCX export of table styles, based on InteropGrabBag.
class DocxTableStyleExport
{
    struct Impl;
    boost::shared_ptr<Impl> m_pImpl;
public:
    void TableStyles(sal_Int32 nCountStylesToWrite);

    /// Writes <w:rPr>...</w:rPr> based on grab-bagged character properties.
    void CharFormat(css::uno::Sequence<css::beans::PropertyValue>& rRPr);

    /// Writes <w:cnfStyle .../> based on grab-bagged para, cell or row properties.
    void CnfStyle(css::uno::Sequence<css::beans::PropertyValue>& rAttributeList);

    void SetSerializer(sax_fastparser::FSHelperPtr pSerializer);
    DocxTableStyleExport(SwDoc* pDoc, sax_fastparser::FSHelperPtr pSerializer);
    ~DocxTableStyleExport();
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_DOCXTABLESTYLEEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

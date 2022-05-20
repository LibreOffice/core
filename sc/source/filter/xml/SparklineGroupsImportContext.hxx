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
#include "importcontext.hxx"
#include "xmlimprt.hxx"
#include <address.hxx>
#include <rangelst.hxx>

namespace sax_fastparser
{
class FastAttributeList;
}

namespace sc
{
class SparklineGroup;

/** Transitional import data of a sparkline */
struct SparklineImportData
{
    ScAddress m_aAddress;
    ScRangeList m_aDataRangeList;
};

/** Handle the import of sparkline groups and sparklines */
class SparklineGroupsImportContext : public ScXMLImportContext
{
private:
    std::shared_ptr<sc::SparklineGroup> m_pCurrentSparklineGroup;
    std::vector<SparklineImportData> m_aCurrentSparklineDataList;

    void
    fillSparklineGroupID(css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList);
    void fillSparklineGroupAttributes(
        css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList);
    void fillSparklineAttributes(
        SparklineImportData& rImportData,
        css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList);

    void insertSparklines();

public:
    SparklineGroupsImportContext(ScXMLImport& rImport);

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList) override;

    void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

} // end sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

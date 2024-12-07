/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <memory>
#include <unordered_map>
#include <tools/color.hxx>
#include <xmloff/xmltoken.hxx>

#include <Sparkline.hxx>
#include <SparklineGroup.hxx>

class ScXMLExport;
namespace model
{
class ComplexColor;
}

namespace sc
{
/** Handle the export of sparkline groups and sparklines */
class SparklineGroupsExport
{
    ScDocument& m_rDoc;
    ScXMLExport& m_rExport;
    SCTAB m_nTable;

    void addSparklineGroupAttributes(sc::SparklineAttributes const& rAttributes);
    void addSparklineGroup(std::shared_ptr<SparklineGroup> const& pSparklineGroup,
                           std::vector<std::shared_ptr<Sparkline>> const& rSparklines);
    void addSparklineAttributes(Sparkline const& rSparkline);

    void insertColor(model::ComplexColor const& rComplexColor, xmloff::token::XMLTokenEnum eToken);
    void insertComplexColor(model::ComplexColor const& rComplexColor,
                            xmloff::token::XMLTokenEnum eToken);

    void insertBool(bool bValue, xmloff::token::XMLTokenEnum eToken);

public:
    SparklineGroupsExport(ScDocument& rDoc, ScXMLExport& rExport, SCTAB nTable);

    void write();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

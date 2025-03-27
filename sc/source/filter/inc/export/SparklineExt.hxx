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
#include <vector>
#include <map>
#include <rangelst.hxx>
#include <Sparkline.hxx>
#include <SparklineAttributes.hxx>

#include <sax/fastattribs.hxx>

#include <xerecord.hxx>
#include <xeroot.hxx>
#include <xeextlst.hxx>

namespace xcl::exp
{
/** Export for sparkline type of <ext> element - top sparkline element. */
class SparklineExt : public XclExpExt
{
public:
    SparklineExt(const XclExpRoot& rRoot);

    void SaveXml(XclExpXmlStream& rStream) override;
    void addSparklineGroup(XclExpXmlStream& rStream, sc::SparklineGroup& rSparklineGroup,
                           std::vector<std::shared_ptr<sc::Sparkline>> const& rSparklines);

    static void
    addSparklineGroupAttributes(const rtl::Reference<sax_fastparser::FastAttributeList>& pAttrList,
                                const sc::SparklineAttributes& rSparklineAttributes);
    static void addSparklineGroupColors(XclExpXmlStream& rStream,
                                        const sc::SparklineAttributes& rSparklineAttributes);

    XclExpExtType GetType() override { return XclExpExtSparklineType; }
};

/** Determines if sparklines needs to be exported and initiates the export. */
class SparklineBuffer : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit SparklineBuffer(const XclExpRoot& rRoot, const XclExtLstRef& xExtLst);
};

} // end namespace xcl::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

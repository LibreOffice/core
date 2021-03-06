/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "transporttypes.hxx"
#include <xmloff/xmlictxt.hxx>
#include <xmloff/SchXMLImportHelper.hxx>

namespace com::sun::star {
    namespace chart2 {
        class XChartDocument;
        class XDataSeries;
    }
}

class SchXMLPropertyMappingContext : public SvXMLImportContext
{
public:

    SchXMLPropertyMappingContext(
                          SvXMLImport& rImport,
                          tSchXMLLSequencesPerIndex& rLSequencesPerIndex,
                              css::uno::Reference<
                              css::chart2::XDataSeries > const & xSeries );

    virtual ~SchXMLPropertyMappingContext() override;

    virtual void SAL_CALL startFastElement (sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs) override;
private:

    css::uno::Reference< css::chart2::XDataSeries > mxDataSeries;

    tSchXMLLSequencesPerIndex& mrLSequencesPerIndex;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

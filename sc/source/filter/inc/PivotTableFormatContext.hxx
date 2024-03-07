/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "excelhandlers.hxx"
#include "worksheethelper.hxx"

namespace oox::xls
{
class PivotTableFormat;
class PivotTableReference;

class PivotTableFormatContext : public WorksheetContextBase
{
public:
    explicit PivotTableFormatContext(WorksheetFragmentBase& rFragment, PivotTableFormat& rFormat);

    oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                 const AttributeList& rAttribs) override;
    void onStartElement(const AttributeList& rAttribs) override;

private:
    PivotTableFormat& mrFormat;
};

class PivotTableReferenceContext : public WorksheetContextBase
{
public:
    explicit PivotTableReferenceContext(PivotTableFormatContext& rFragment,
                                        PivotTableReference& rReference);

    oox::core::ContextHandlerRef onCreateContext(sal_Int32 nElement,
                                                 const AttributeList& rAttribs) override;
    void onStartElement(const AttributeList& rAttribs) override;

private:
    PivotTableReference& mrReference;
};

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

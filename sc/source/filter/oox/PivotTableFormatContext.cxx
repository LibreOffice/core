/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <PivotTableFormatContext.hxx>

#include <pivottablefragment.hxx>
#include <pivottablebuffer.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/helper/attributelist.hxx>

namespace oox::xls
{
PivotTableFormatContext::PivotTableFormatContext(WorksheetFragmentBase& rFragment,
                                                 PivotTableFormat& rFormat)
    : WorksheetContextBase(rFragment)
    , mrFormat(rFormat)
{
}

oox::core::ContextHandlerRef
PivotTableFormatContext::onCreateContext(sal_Int32 nElement, const AttributeList& /*rAttribs*/)
{
    switch (getCurrentElement())
    {
        case XLS_TOKEN(format):
            if (nElement == XLS_TOKEN(pivotArea))
                return this;
            break;
        case XLS_TOKEN(pivotArea):
            if (nElement == XLS_TOKEN(references))
                return this;
            break;
        case XLS_TOKEN(references):
            if (nElement == XLS_TOKEN(reference))
                return new PivotTableReferenceContext(*this, mrFormat.createReference());
            break;
    }
    return nullptr;
}

void PivotTableFormatContext::onStartElement(const AttributeList& rAttribs)
{
    if (isRootElement())
    {
        mrFormat.importFormat(rAttribs);
    }
    else if (isCurrentElement(XLS_TOKEN(pivotArea)))
    {
        mrFormat.importPivotArea(rAttribs);
    }
}

PivotTableReferenceContext::PivotTableReferenceContext(PivotTableFormatContext& rFragment,
                                                       PivotTableReference& rReference)
    : WorksheetContextBase(rFragment)
    , mrReference(rReference)
{
}
oox::core::ContextHandlerRef
PivotTableReferenceContext::onCreateContext(sal_Int32 nElement, const AttributeList& /*rAttribs*/)
{
    switch (getCurrentElement())
    {
        case XLS_TOKEN(reference):
            if (nElement == XLS_TOKEN(x))
                return this;
            break;
    }
    return nullptr;
}

void PivotTableReferenceContext::onStartElement(const AttributeList& rAttribs)
{
    if (isRootElement())
    {
        mrReference.importReference(rAttribs);
    }
    else if (isCurrentElement(XLS_TOKEN(x)))
    {
        mrReference.addFieldItem(rAttribs);
    }
}

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

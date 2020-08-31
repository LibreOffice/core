/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ReportVisitor.hxx>

#include <osl/diagnose.h>

namespace reportdesign
{
using namespace com::sun::star;

OReportVisitor::OReportVisitor(ITraverseReport* _pTraverseReport)
                       :m_pTraverseReport(_pTraverseReport)
{
    OSL_ENSURE(m_pTraverseReport,"ReportDefinition must be not NULL!");
}

void OReportVisitor::start(const uno::Reference< report::XReportDefinition>& _xReportDefinition)
{
    OSL_ENSURE(_xReportDefinition.is(),"ReportDefinition is NULL!");
    if ( !_xReportDefinition.is() )
        return;

    m_pTraverseReport->traverseReport(_xReportDefinition);
    m_pTraverseReport->traverseReportFunctions(_xReportDefinition->getFunctions());
    if ( _xReportDefinition->getPageHeaderOn() )
        m_pTraverseReport->traversePageHeader(_xReportDefinition->getPageHeader());
    if ( _xReportDefinition->getReportHeaderOn() )
        m_pTraverseReport->traverseReportHeader(_xReportDefinition->getReportHeader());

    uno::Reference< report::XGroups > xGroups = _xReportDefinition->getGroups();
    m_pTraverseReport->traverseGroups(xGroups);
    const sal_Int32 nCount = xGroups->getCount();
    sal_Int32 i = 0;
    for (;i<nCount ; ++i)
    {
        uno::Reference< report::XGroup > xGroup(xGroups->getByIndex(i),uno::UNO_QUERY);
        m_pTraverseReport->traverseGroup(xGroup);
        m_pTraverseReport->traverseGroupFunctions(xGroup->getFunctions());
        if ( xGroup->getHeaderOn() )
            m_pTraverseReport->traverseGroupHeader(xGroup->getHeader());
    }

    m_pTraverseReport->traverseDetail(_xReportDefinition->getDetail());

    for (i = 0;i<nCount ; ++i)
    {
        uno::Reference< report::XGroup > xGroup(xGroups->getByIndex(i),uno::UNO_QUERY);
        if ( xGroup->getFooterOn() )
            m_pTraverseReport->traverseGroupFooter(xGroup->getFooter());
    }

    if ( _xReportDefinition->getPageFooterOn() )
        m_pTraverseReport->traversePageFooter(_xReportDefinition->getPageFooter());
    if ( _xReportDefinition->getReportFooterOn() )
        m_pTraverseReport->traverseReportFooter(_xReportDefinition->getReportFooter());
}

void OReportVisitor::start(const uno::Reference< report::XGroup>& _xGroup)
{
    OSL_ENSURE(_xGroup.is(),"Group is NULL!");
    if ( !_xGroup.is() )
        return;
    m_pTraverseReport->traverseGroup(_xGroup);
    m_pTraverseReport->traverseGroupFunctions(_xGroup->getFunctions());
    if ( _xGroup->getHeaderOn() )
        m_pTraverseReport->traverseGroupHeader(_xGroup->getHeader());
    if ( _xGroup->getFooterOn() )
        m_pTraverseReport->traverseGroupFooter(_xGroup->getFooter());
}

} // namespace reportdesign


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportVisitor.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "ReportVisitor.hxx"
namespace reportdesign
{
using namespace com::sun::star;

OReportVisitor::OReportVisitor(ITraverseReport* _pTraverseReport)
                       :m_pTraverseReport(_pTraverseReport)
{
    OSL_ENSURE(m_pTraverseReport,"ReportDefintion must be not NULL!");
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
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
// =============================================================================
} // namespace reportdesign
// =============================================================================

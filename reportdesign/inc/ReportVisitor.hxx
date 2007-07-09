/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportVisitor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:02 $
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
#ifndef INCLUDED_REPORT_VISTITOR_HXX
#define INCLUDED_REPORT_VISTITOR_HXX

#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#include "dllapi.h"

namespace reportdesign
{
    class SAL_NO_VTABLE ITraverseReport
    {
    public:
        virtual void traverseReport(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xReport) = 0;
        virtual void traverseReportFunctions(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions>& _xFunctions) = 0;
        virtual void traverseReportHeader(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) = 0;
        virtual void traverseReportFooter(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) = 0;
        virtual void traversePageHeader(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) = 0;
        virtual void traversePageFooter(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) = 0;

        virtual void traverseGroups(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroups>& _xGroups) = 0;
        virtual void traverseGroup(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup) = 0;
        virtual void traverseGroupFunctions(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFunctions>& _xFunctions) = 0;
        virtual void traverseGroupHeader(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) = 0;
        virtual void traverseGroupFooter(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) = 0;

        virtual void traverseDetail(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection) = 0;
    };

    class REPORTDESIGN_DLLPUBLIC OReportVisitor
    {
        ITraverseReport*                                                                m_pTraverseReport;
    public:
        OReportVisitor(ITraverseReport* _pTraverseReport);
        void start(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition>& _xReportDefinition);
        void start(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup);
    };
// =============================================================================
} // namespace reportdesign
// =============================================================================
#endif // INCLUDED_REPORT_VISTITOR_HXX


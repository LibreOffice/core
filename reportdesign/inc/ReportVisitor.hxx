/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_REPORT_VISTITOR_HXX
#define INCLUDED_REPORT_VISTITOR_HXX

#include "dllapi.h"
#include <com/sun/star/report/XReportDefinition.hpp>


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

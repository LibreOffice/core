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

    protected:
        ~ITraverseReport() {}
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

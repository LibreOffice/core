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

#ifndef REPORTDRAWPAGE_HXX_INCLUDED
#define REPORTDRAWPAGE_HXX_INCLUDED

#include <svx/unopage.hxx>
#include <com/sun/star/report/XSection.hpp>

namespace reportdesign
{
    class OReportDrawPage : public SvxDrawPage
    {
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XSection > m_xSection;
        OReportDrawPage(const OReportDrawPage&);
        void operator =(const OReportDrawPage&);
    protected:
        virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape )throw (std::exception);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw (std::exception);
    public:
        OReportDrawPage(SdrPage* pPage,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
        virtual ~OReportDrawPage() throw(){}
    };
}
#endif //REPORTDRAWPAGE_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

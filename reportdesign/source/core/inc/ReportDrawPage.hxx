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
        virtual SdrObject *_CreateSdrObject( const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > & xShape )throw ();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >  _CreateShape( SdrObject *pObj ) const throw ();
    public:
        OReportDrawPage(SdrPage* pPage,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
        virtual ~OReportDrawPage() throw(){}
    };
}
#endif //REPORTDRAWPAGE_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

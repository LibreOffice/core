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

#ifndef INCLUDED_FORMATTEDFIELDBEAUTIFIER_HXX
#define INCLUDED_FORMATTEDFIELDBEAUTIFIER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
/** === end UNO includes === **/

#include <vector>
#include <IReportControllerObserver.hxx>

namespace rptui
{
    class OReportController;

    class FormattedFieldBeautifier : public IReportControllerObserver
    {
        const OReportController& m_rReportController;
        sal_Int32 m_nTextColor;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer > getVclWindowPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent >& _xComponent) throw(::com::sun::star::uno::RuntimeException);

        void setPlaceholderText( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );
        void setPlaceholderText( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >& _xVclWindowPeer, const ::rtl::OUString& _rText );

        sal_Int32 getTextColor();

    public:
        FormattedFieldBeautifier(const OReportController & _aObserver);
        virtual ~FormattedFieldBeautifier();

        void    notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent );
        void    notifyElementInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );
        void    handle( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );
    };

} // namespace rptui


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

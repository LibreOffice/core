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

#ifndef INCLUDED_FIXEDTEXTCOLOR_HXX
#define INCLUDED_FIXEDTEXTCOLOR_HXX

/** === begin UNO includes === **/
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/report/XFixedText.hpp>
/** === end UNO includes === **/

#include <IReportControllerObserver.hxx>
#include <vector>

namespace rptui
{
    class OReportController;

    class FixedTextColor : public IReportControllerObserver
    {
        const OReportController& m_rReportController;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer > getVclWindowPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFixedText >& _xComponent) throw(::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > getXControl(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFixedText >& _xFixedText) throw(::com::sun::star::uno::RuntimeException);

        void setPropertyTextColor(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >& _xVclWindowPeer, sal_Int32 _nFormatKey);

        // sal_Int32 getTextColor();

    public:
        FixedTextColor(const OReportController & _aObserver);
        virtual ~FixedTextColor();

        void    notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent );
        void    notifyElementInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );
        void    handle( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    };

} // namespace rptui


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FIXEDTEXTCOLOR_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_FIXEDTEXTCOLOR_HXX

#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/report/XFixedText.hpp>

#include "IReportControllerObserver.hxx"
#include <vector>

namespace rptui
{
    class OReportController;

    class FixedTextColor : public IReportControllerObserver
    {
        const OReportController& m_rReportController;

        css::uno::Reference< css::awt::XVclWindowPeer > getVclWindowPeer(const css::uno::Reference< css::report::XFixedText >& _xComponent) throw(css::uno::RuntimeException);
        css::uno::Reference< css::awt::XControl > getXControl(const css::uno::Reference< css::report::XFixedText >& _xFixedText) throw(css::uno::RuntimeException);

        static void setPropertyTextColor(const css::uno::Reference< css::awt::XVclWindowPeer >& _xVclWindowPeer, sal_Int32 _nFormatKey);

    public:
        FixedTextColor(const OReportController & _aObserver);
        virtual ~FixedTextColor();

        void    notifyPropertyChange( const css::beans::PropertyChangeEvent& _rEvent ) override;
        void    notifyElementInserted( const css::uno::Reference< css::uno::XInterface >& _rxElement ) override;
        void    handle( const css::uno::Reference< css::uno::XInterface >& _rxElement ) override;

    };

} // namespace rptui


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

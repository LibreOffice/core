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
#ifndef RPTUI_RULER_HXX
#define RPTUI_RULER_HXX

#include <vcl/window.hxx>
#include <com/sun/star/report/XSection.hpp>

namespace rptui
{
    class OReportSection;
    class OReportWindow;
    class OReportRuler : public Window
    {
        OReportSection*     m_pSection;
        OReportWindow*      m_pParent;
        sal_Bool            m_bShow;
        OReportRuler(OReportRuler&);
        void operator =(OReportRuler&);
    public:
        OReportRuler(Window* _pParent,OReportWindow* _pReportWindowt,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
        virtual ~OReportRuler();

        // windows overloads
        virtual void Resize();

        inline OReportSection*      getSection()        const { return m_pSection; }
        inline OReportWindow*       getView()           const { return m_pParent; }

        /** makes the grid visible
        *
        * \param _bVisible when <TRUE/> the grid is made visible
        */
        void SetGridVisible(sal_Bool _bVisible);
    };

} // rptui

#endif // RPTUI_RULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

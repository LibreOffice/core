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
//==============================================================================
} // rptui
//==============================================================================
#endif // RPTUI_RULER_HXX

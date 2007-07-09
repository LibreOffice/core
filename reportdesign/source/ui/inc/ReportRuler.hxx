#ifndef RPTUI_RULER_HXX
#define RPTUI_RULER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportRuler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:30 $
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

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSECTION_HPP_
#include <com/sun/star/report/XSection.hpp>
#endif

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
        void SetGridVisible(BOOL _bVisible);
    };
//==============================================================================
} // rptui
//==============================================================================
#endif // RPTUI_RULER_HXX

#ifndef RPTUI_COLORLISTENER_HXX
#define RPTUI_COLORLISTENER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColorListener.hxx,v $
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
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <svtools/colorcfg.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_EXTCOLORCFG_HXX
#include <svtools/extcolorcfg.hxx>
#endif
#ifndef _RPTUI_MODULE_HELPER_RPT_HXX_
#include "ModuleHelper.hxx"
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

namespace rptui
{
    class OColorListener : public Window, public SfxListener
    {
        OColorListener(const OColorListener&);
        void operator =(const OColorListener&);
    protected:
        OModuleClient                       m_aModuleClient;
        Link                                m_aCollapsedLink;
        svtools::ColorConfig                m_aColorConfig;
        svtools::ExtendedColorConfig        m_aExtendedColorConfig;
        ::rtl::OUString                     m_sColorEntry;
        sal_Int32                           m_nColor;
        sal_Int32                           m_nTextBoundaries;
        sal_Bool                            m_bCollapsed;
        sal_Bool                            m_bMarked;

        virtual void ImplInitSettings() = 0;
    protected:
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
    public:
        OColorListener(Window* _pParent,const ::rtl::OUString& _sColorEntry);
        virtual ~OColorListener();

        using Window::Notify;
        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint);

        /** set the marker as marked or not marked
            @param  _bMark  set the new state of the marker
        */
        void    setMarked(sal_Bool _bMark);

        /** returns if the section is marked
        */
        inline sal_Bool isMarked() const { return m_bMarked; }

        inline void     setCollapsedHdl(const Link& _aLink ){ m_aCollapsedLink = _aLink; }
        inline sal_Bool isCollapsed() const { return m_bCollapsed; }

        /** collapse or expand
         *
         * \param _bCollapsed
         */
        virtual void    setCollapsed(sal_Bool _bCollapsed);
    };
}
#endif // RPTUI_COLORLISTENER_HXX


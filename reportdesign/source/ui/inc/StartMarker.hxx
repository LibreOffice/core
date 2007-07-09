#ifndef RPTUI_STARTMARKER_HXX
#define RPTUI_STARTMARKER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StartMarker.hxx,v $
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

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RULER_HXX //autogen
#include <svtools/ruler.hxx>
#endif
#ifndef RPTUI_COLORLISTENER_HXX
#include "ColorListener.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif


namespace rptui
{
    class OSectionsWindow;
    class OStartMarker : public OColorListener
    {

        Ruler                       m_aVRuler;
        FixedText                   m_aText;
        FixedImage                  m_aImage;
        OSectionsWindow*            m_pParent;
        static Image*               s_pDefCollapsed;
        static Image*               s_pDefExpanded;
        static Image*               s_pDefCollapsedHC;
        static Image*               s_pDefExpandedHC;
        static oslInterlockedCount  s_nImageRefCount; /// When 0 all static images will be destroyed
        sal_Int32                   m_nCornerSize;

        sal_Bool                    m_bShowRuler;

        void initDefaultNodeImages();
        void setColor();
        virtual void ImplInitSettings();
        OStartMarker(OStartMarker&);
        void operator =(OStartMarker&);
    public:
        OStartMarker(OSectionsWindow* _pParent,const ::rtl::OUString& _sColorEntry);
        virtual ~OStartMarker();

        // SfxListener
        virtual void    Notify(SfxBroadcaster & rBc, SfxHint const & rHint);
        // window overloads
        virtual void    Paint( const Rectangle& rRect );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
        virtual void    Resize();
        virtual void    RequestHelp( const HelpEvent& rHEvt );
        using Window::Notify;

        void            setTitle(const String& _sTitle);
        sal_Int32       getWidth() const;
        sal_Int32       getMinHeight() const;

        /** returns the offset where the horizontal ruler must start
        */
        sal_Int32       getRulerOffset() const;

        /** shows or hides the ruler.
        */
        void            showRuler(sal_Bool _bShow);

        virtual void    setCollapsed(sal_Bool _bCollapsed);
    };
}
#endif // RPTUI_STARTMARKER_HXX


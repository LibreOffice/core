#ifndef RPTUI_ENDMARKER_HXX
#define RPTUI_ENDMARKER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EndMarker.hxx,v $
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

#ifndef RPTUI_COLORLISTENER_HXX
#include "ColorListener.hxx"
#endif

namespace rptui
{
    /** \class OEndMarker
     *  \brief Defines the right side of a graphical section.
     */
    class OEndMarker : public OColorListener
    {
        OEndMarker(OEndMarker&);
        void operator =(OEndMarker&);
    protected:
        virtual void ImplInitSettings();
    public:
        OEndMarker(Window* _pParent,const ::rtl::OUString& _sColorEntry);
        virtual ~OEndMarker();

        // windows
        virtual void    Paint( const Rectangle& rRect );
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    };
}
#endif // RPTUI_ENDMARKER_HXX


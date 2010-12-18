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
#ifndef RPTUI_STARTMARKER_HXX
#define RPTUI_STARTMARKER_HXX

#include <osl/interlck.h>
#include <svtools/ruler.hxx>
#include "ColorListener.hxx"
#include <vcl/fixed.hxx>


namespace rptui
{
    class OSectionWindow;
    class OStartMarker : public OColorListener
    {

        Ruler                       m_aVRuler;
        FixedText                   m_aText;
        FixedImage                  m_aImage;
        OSectionWindow*             m_pParent;
        static Image*               s_pDefCollapsed;
        static Image*               s_pDefExpanded;
        static Image*               s_pDefCollapsedHC;
        static Image*               s_pDefExpandedHC;
        static oslInterlockedCount  s_nImageRefCount; /// When 0 all static images will be destroyed

        sal_Bool                    m_bShowRuler;

        void changeImage();
        void initDefaultNodeImages();
        void setColor();
        virtual void ImplInitSettings();
        OStartMarker(OStartMarker&);
        void operator =(OStartMarker&);
    public:
        OStartMarker(OSectionWindow* _pParent,const ::rtl::OUString& _sColorEntry);
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
        sal_Int32       getMinHeight() const;

        /** shows or hides the ruler.
        */
        void            showRuler(sal_Bool _bShow);

        virtual void    setCollapsed(sal_Bool _bCollapsed);

        /** zoom the ruler and view windows
        */
        void            zoom(const Fraction& _aZoom);
    };
}
#endif // RPTUI_STARTMARKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

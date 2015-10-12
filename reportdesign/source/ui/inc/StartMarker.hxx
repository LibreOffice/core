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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_STARTMARKER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_STARTMARKER_HXX

#include <osl/interlck.h>
#include <svtools/ruler.hxx>
#include "ColorListener.hxx"
#include <vcl/fixed.hxx>


namespace rptui
{
    class OSectionWindow;
    class OStartMarker : public OColorListener
    {

        VclPtr<Ruler>               m_aVRuler;
        VclPtr<FixedText>           m_aText;
        VclPtr<FixedImage>          m_aImage;
        VclPtr<OSectionWindow>      m_pParent;
        static Image*               s_pDefCollapsed;
        static Image*               s_pDefExpanded;
        static oslInterlockedCount  s_nImageRefCount; /// When 0 all static images will be destroyed

        bool m_bShowRuler;

        void changeImage();
        void initDefaultNodeImages();
        void setColor();

        virtual void ImplInitSettings() SAL_OVERRIDE;
        virtual void ApplySettings(vcl::RenderContext& rRenderContext) SAL_OVERRIDE;

        OStartMarker(OStartMarker&) = delete;
        void operator =(OStartMarker&) = delete;
    public:
        OStartMarker(OSectionWindow* _pParent,const OUString& _sColorEntry);
        virtual ~OStartMarker();
        virtual void dispose() SAL_OVERRIDE;

        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint) SAL_OVERRIDE;
        // Window overrides
        virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) SAL_OVERRIDE;
        virtual void MouseButtonUp(const MouseEvent& rMEvt) SAL_OVERRIDE;
        virtual void Resize() SAL_OVERRIDE;
        virtual void RequestHelp(const HelpEvent& rHEvt) SAL_OVERRIDE;
        using Window::Notify;

        void setTitle(const OUString& _sTitle);
        sal_Int32 getMinHeight() const;

        /** shows or hides the ruler.
        */
        void showRuler(bool _bShow);

        virtual void setCollapsed(bool _bCollapsed) SAL_OVERRIDE;

        /** zoom the ruler and view windows
        */
        void zoom(const Fraction& _aZoom);
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_STARTMARKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

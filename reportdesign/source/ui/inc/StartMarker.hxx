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
#include <vcl/image.hxx>

namespace rptui
{
    class OSectionWindow;
    class OStartMarker : public OColorListener
    {

        VclPtr<Ruler>               m_aVRuler;
        OUString                    m_aText;
        tools::Rectangle            m_aTextRect;
        Image                       m_aImage;
        tools::Rectangle            m_aImageRect;
        VclPtr<OSectionWindow>      m_pParent;
        static Image*               s_pDefCollapsed;
        static Image*               s_pDefExpanded;
        static oslInterlockedCount  s_nImageRefCount; /// When 0 all static images will be destroyed

        bool m_bShowRuler;

        void changeImage();
        void initDefaultNodeImages();

        virtual void ImplInitSettings() override;
        virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

        OStartMarker(OStartMarker const &) = delete;
        void operator =(OStartMarker const &) = delete;
    public:
        OStartMarker(OSectionWindow* _pParent,const OUString& _sColorEntry);
        virtual ~OStartMarker() override;
        virtual void dispose() override;

        // SfxListener
        virtual void Notify(SfxBroadcaster & rBc, SfxHint const & rHint) override;
        // Window overrides
        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
        virtual void MouseButtonUp(const MouseEvent& rMEvt) override;
        virtual void Resize() override;
        virtual void RequestHelp(const HelpEvent& rHEvt) override;

        void setTitle(const OUString& _sTitle);
        sal_Int32 getMinHeight() const;

        /** shows or hides the ruler.
        */
        void showRuler(bool _bShow);

        virtual void setCollapsed(bool _bCollapsed) override;

        /** zoom the ruler and view windows
        */
        void zoom(const Fraction& _aZoom);
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_STARTMARKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

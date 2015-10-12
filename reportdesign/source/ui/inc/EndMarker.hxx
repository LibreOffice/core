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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_ENDMARKER_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_ENDMARKER_HXX

#include "ColorListener.hxx"

namespace rptui
{
    /** \class OEndMarker
     *  \brief Defines the right side of a graphical section.
     */
    class OEndMarker : public OColorListener
    {
        OEndMarker(OEndMarker&) = delete;
        void operator =(OEndMarker&) = delete;
    protected:
        virtual void ImplInitSettings() SAL_OVERRIDE;
    public:
        OEndMarker(vcl::Window* _pParent,const OUString& _sColorEntry);
        virtual ~OEndMarker();

        // windows
        virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;
        virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_ENDMARKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

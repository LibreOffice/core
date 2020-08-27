/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DASHEDLINE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DASHEDLINE_HXX

#include <vcl/ctrl.hxx>

/** Class for displaying a dashed line in the Writer GUI.
  */
class SwDashedLine : public Control
{
    Color& (*m_pColorFn)();

public:
    SwDashedLine( vcl::Window* pParent, Color& ( *pColorFn )() );
    virtual ~SwDashedLine( ) override;

    virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

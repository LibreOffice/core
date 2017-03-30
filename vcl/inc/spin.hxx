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

#ifndef INCLUDED_VCL_INC_SPIN_HXX
#define INCLUDED_VCL_INC_SPIN_HXX

#include <vcl/window.hxx>

namespace tools { class Rectangle; }

// Draw Spinners as found in a SpinButton. Some themes like gtk3 will draw +- elements here,
// so these are only suitable in the context of SpinButtons
void ImplDrawSpinButton(vcl::RenderContext& rRenderContext, vcl::Window* pWindow,
                        const tools::Rectangle& rUpperRect, const tools::Rectangle& rLowerRect,
                        bool bUpperIn, bool bLowerIn, bool bUpperEnabled = true, bool bLowerEnabled = true,
                        bool bHorz = false, bool bMirrorHorz = false);

// Draw Up/Down buttons suitable for use in any context
void ImplDrawUpDownButtons(vcl::RenderContext& rRenderContext,
                           const tools::Rectangle& rUpperRect, const tools::Rectangle& rLowerRect,
                           bool bUpperIn, bool bLowerIn, bool bUpperEnabled, bool bLowerEnabled,
                           bool bHorz, bool bMirrorHorz = false);


#endif // INCLUDED_VCL_INC_SPIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

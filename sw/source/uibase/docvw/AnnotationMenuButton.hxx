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

#pragma once

#include <vcl/menubtn.hxx>

namespace sw::annotation {
    class SwAnnotationWin;
}

namespace sw::annotation {

class AnnotationMenuButton : public MenuButton
{
    public:
        AnnotationMenuButton( sw::annotation::SwAnnotationWin& rSidebarWin );
        virtual ~AnnotationMenuButton() override;
        virtual void dispose() override;

        // override MenuButton methods
        virtual void Select() override;

        // override vcl::Window methods
        virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
        virtual void KeyInput( const KeyEvent& rKeyEvt ) override;

    private:
        sw::annotation::SwAnnotationWin& mrSidebarWin;
};

} // end of namespace sw::annotation

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

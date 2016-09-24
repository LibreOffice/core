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

#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <vcl/builderfactory.hxx>
#include <svtools/miscopt.hxx>

namespace sfx2 { namespace notebookbar {

class NotebookbarToolBox : public sfx2::sidebar::SidebarToolBox
{
public:

    NotebookbarToolBox(vcl::Window* pParentWindow)
    : sfx2::sidebar::SidebarToolBox(pParentWindow)
    {
        ToolBoxButtonSize eSize = ToolBoxButtonSize::Small;
        eSize = GetIconSize();
        SetToolboxButtonSize(eSize);
    }

    virtual ToolBoxButtonSize GetIconSize() const override
    {
        SvtMiscOptions aMiscOptions;
        return aMiscOptions.GetNotebookbarIconSize();
    }
};

VCL_BUILDER_FACTORY(NotebookbarToolBox)

} } // end of namespace sfx2::notebookbar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

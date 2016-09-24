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

#ifndef INCLUDED_VCL_POPUPMENUWINDOW_HXX
#define INCLUDED_VCL_POPUPMENUWINDOW_HXX

#include <vcl/floatwin.hxx>

class VCL_DLLPUBLIC PopupMenuFloatingWindow : public FloatingWindow
{
private:
    struct ImplData;
    ImplData* mpImplData;
public:
    PopupMenuFloatingWindow( vcl::Window* pParent );
    virtual ~PopupMenuFloatingWindow() override;
    virtual void dispose() override;

    sal_uInt16      GetMenuStackLevel() const;
    void            SetMenuStackLevel( sal_uInt16 nLevel );
    bool            IsPopupMenu() const;

    //determine if a given window is an activated PopupMenuFloatingWindow
    static bool isPopupMenu(const vcl::Window *pWindow);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

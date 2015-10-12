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
#ifndef INCLUDED_SFX2_SOURCE_INC_PARTWND_HXX
#define INCLUDED_SFX2_SOURCE_INC_PARTWND_HXX

#include <com/sun/star/uno/Reference.h>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace frame
            {
                class XFrame;
            }
        }
    }
}

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>

// forward ---------------------------------------------------------------

// class SfxPartChildWnd_Impl -----------------------------------

class SfxPartChildWnd_Impl : public SfxChildWindow
{
public:
    SfxPartChildWnd_Impl( vcl::Window* pParent, sal_uInt16 nId,
                                   SfxBindings* pBindings,
                                   SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SfxPartChildWnd_Impl);
                            virtual ~SfxPartChildWnd_Impl();

    virtual bool            QueryClose() override;
};

// class SfxExplorerDockWnd_Impl -----------------------------------------

class SfxPartDockWnd_Impl : public SfxDockingWindow
{
protected:
    virtual void            Resize() override;
    virtual bool            Notify( NotifyEvent& rNEvt ) override;

public:
                            SfxPartDockWnd_Impl( SfxBindings* pBindings,
                                  SfxChildWindow* pChildWin,
                                  vcl::Window* pParent,
                                  WinBits nBits );

    bool                    QueryClose();
    virtual void            FillInfo(SfxChildWinInfo&) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

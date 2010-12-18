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
#ifndef _PARTWND_HXX
#define _PARTWND_HXX

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
    SfxPartChildWnd_Impl( Window* pParent, sal_uInt16 nId,
                                   SfxBindings* pBindings,
                                   SfxChildWinInfo* pInfo );

    SFX_DECL_CHILDWINDOW(SfxPartChildWnd_Impl);
                            ~SfxPartChildWnd_Impl();

    virtual sal_Bool        QueryClose();
};

// class SfxExplorerDockWnd_Impl -----------------------------------------

class SfxPartDockWnd_Impl : public SfxDockingWindow
{
private:
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > m_xFrame;

protected:
    virtual void            Resize();
    virtual long            Notify( NotifyEvent& rNEvt );

public:
                            SfxPartDockWnd_Impl( SfxBindings* pBindings,
                                  SfxChildWindow* pChildWin,
                                  Window* pParent,
                                  WinBits nBits );

                            ~SfxPartDockWnd_Impl();
    sal_Bool                    QueryClose();
    virtual void            FillInfo(SfxChildWinInfo&) const;
    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame > ForgetFrame();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef _RECFLOAT_HXX
#define _RECFLOAT_HXX

// includes *******************************************************************

#include <sfx2/childwin.hxx>
#include <sfx2/basedlgs.hxx>

class SfxRecordingFloatWrapper_Impl : public SfxChildWindow
{
    SfxBindings*        pBindings;
public:
                        SfxRecordingFloatWrapper_Impl( Window* pParent ,
                                                sal_uInt16 nId ,
                                                SfxBindings* pBindings ,
                                                SfxChildWinInfo* pInfo );
                        ~SfxRecordingFloatWrapper_Impl();

                        SFX_DECL_CHILDWINDOW(SfxRecordingFloatWrapper_Impl);
    virtual sal_Bool    QueryClose();
};

class SfxRecordingFloat_Impl : public SfxFloatingWindow
{
    SfxChildWindow*         pWrapper;
    ToolBox                 aTbx;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XToolbarController > xStopRecTbxCtrl;
public:
                        SfxRecordingFloat_Impl( SfxBindings* pBindings ,
                            SfxChildWindow* pChildWin ,
                            Window* pParent );
    virtual             ~SfxRecordingFloat_Impl();
    virtual sal_Bool        Close();
    virtual void        FillInfo( SfxChildWinInfo& rInfo ) const;
    virtual void        StateChanged( StateChangedType nStateChange );

    DECL_LINK( Select, ToolBox * );
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

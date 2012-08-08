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

#ifndef _RECFLOAT_HXX
#define _RECFLOAT_HXX

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

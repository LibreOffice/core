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

#ifndef INCLUDED_VCL_INC_IOS_IOSINST_HXX
#define INCLUDED_VCL_INC_IOS_IOSINST_HXX

#include <pthread.h>

#include <premac.h>
#include <CoreGraphics/CoreGraphics.h>
#include <postmac.h>

#include <tools/link.hxx>

#include "headless/svpinst.hxx"
#include "headless/svpframe.hxx"

class IosSalFrame;
class IosSalInstance : public SvpSalInstance
{
public:
    IosSalInstance( SalYieldMutex *pMutex );
    virtual ~IosSalInstance();
    static IosSalInstance *getInstance();

    virtual SalSystem* CreateSalSystem() SAL_OVERRIDE;

    void GetWorkArea( Rectangle& rRect );
    SalFrame* CreateFrame( SalFrame* pParent, sal_uLong nStyle );
    SalFrame* CreateChildFrame( SystemParentData* pParent, sal_uLong nStyle );

    SalFrame *getFocusFrame() const;

    void damaged( IosSalFrame *frame,
                  const basegfx::B2IBox& rDamageRect);

    // Functions scheduled to be run as "user events" in the LO thread
    typedef struct {
        bool done;
        CGContextRef context;
        CGRect rect;
    } RenderWindowsArg;
    DECL_LINK( RenderWindows, RenderWindowsArg* );

    DECL_LINK( DisplayConfigurationChanged, void* );

    typedef struct {
        const void *documentHandle;
        int x, y;
    } SelectionStartMoveArg;
    DECL_LINK( SelectionStartMove, SelectionStartMoveArg* );

    typedef struct {
        const void *documentHandle;
        int x, y;
    } SelectionEndMoveArg;
    DECL_LINK( SelectionEndMove, SelectionEndMoveArg* );

    pthread_mutex_t m_aRenderMutex;
    pthread_cond_t m_aRenderCond;
};

#endif // INCLUDED_VCL_INC_IOS_IOSINST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

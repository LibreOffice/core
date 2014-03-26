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

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKOBJECT_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKOBJECT_HXX

#include <tools/solar.h>
#include <vcl/sysdata.hxx>
#include <salobj.hxx>
#include <unx/gtk/gtkframe.hxx>

class GtkSalObject : public SalObject
{
    SystemEnvData     m_aSystemData;
    GtkWidget*          m_pSocket;
#if GTK_CHECK_VERSION(3,0,0)
    cairo_region_t*     m_pRegion;
#else
    GdkRegion*          m_pRegion;
#endif

    // signals
    static gboolean     signalButton( GtkWidget*, GdkEventButton*, gpointer );
    static gboolean     signalFocus( GtkWidget*, GdkEventFocus*, gpointer );
    static void         signalDestroy( GtkWidget*, gpointer );
public:
    GtkSalObject( GtkSalFrame* pParent, bool bShow = true );
    virtual ~GtkSalObject();

    // overload all pure virtual methods
     virtual void                   ResetClipRegion() SAL_OVERRIDE;
    virtual sal_uInt16              GetClipRegionType() SAL_OVERRIDE;
    virtual void                    BeginSetClipRegion( sal_uLong nRects ) SAL_OVERRIDE;
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                    EndSetClipRegion() SAL_OVERRIDE;

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                    Show( bool bVisible ) SAL_OVERRIDE;

    virtual void                                    SetForwardKey( bool bEnable ) SAL_OVERRIDE;

    virtual const SystemEnvData*    GetSystemData() const SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKOBJECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

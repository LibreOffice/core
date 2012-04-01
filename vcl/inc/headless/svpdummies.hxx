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

#ifndef _SVP_SVPDUMMIES_HXX

#include <vcl/sysdata.hxx>
#include <generic/gensys.h>
#include <salobj.hxx>
#include <salimestatus.hxx>
#include <salsys.hxx>

class SalGraphics;

class SvpSalObject : public SalObject
{
public:
    SystemChildData m_aSystemChildData;

    SvpSalObject();
    virtual ~SvpSalObject();

    // overload all pure virtual methods
     virtual void                   ResetClipRegion();
    virtual sal_uInt16              GetClipRegionType();
    virtual void                    BeginSetClipRegion( sal_uLong nRects );
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                    EndSetClipRegion();

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight );
    virtual void                    Show( sal_Bool bVisible );

    virtual const SystemEnvData*    GetSystemData() const;

    virtual void InterceptChildWindowKeyDown( sal_Bool bIntercept );
};

class SvpImeStatus : public SalI18NImeStatus
{
  public:
        SvpImeStatus() {}
        virtual ~SvpImeStatus();

        virtual bool canToggle();
        virtual void toggle();
};

class SvpSalSystem : public SalGenericSystem
{
public:
    SvpSalSystem() {}
    virtual ~SvpSalSystem();
    // get info about the display
    virtual unsigned int GetDisplayScreenCount();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle GetDisplayScreenWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetDisplayScreenName( unsigned int nScreen );

    virtual int ShowNativeDialog( const rtl::OUString& rTitle,
                                  const rtl::OUString& rMessage,
                                  const std::list< rtl::OUString >& rButtons,
                                  int nDefButton );
};


#endif // _SVP_SVPDUMMIES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

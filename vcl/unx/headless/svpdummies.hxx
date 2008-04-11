/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svpdummies.hxx,v $
 * $Revision: 1.4 $
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

#include <vcl/salogl.hxx>
#include <vcl/salobj.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/salimestatus.hxx>
#include <vcl/salsys.hxx>

class SalGraphics;

class SvpSalOpenGL : public SalOpenGL
{
public:
    SvpSalOpenGL() {}
    virtual ~SvpSalOpenGL();

    // overload all pure virtual methods
    virtual bool        IsValid();
    virtual oglFunction GetOGLFnc( const char * );
    virtual void        OGLEntry( SalGraphics* pGraphics );
    virtual void        OGLExit( SalGraphics* pGraphics );
    virtual void        StartScene( SalGraphics* pGraphics );
    virtual void        StopScene();
};

class SvpSalObject : public SalObject
{
public:
    SystemChildData m_aSystemChildData;

    SvpSalObject();
    virtual ~SvpSalObject();

    // overload all pure virtual methods
     virtual void                   ResetClipRegion();
    virtual USHORT                  GetClipRegionType();
    virtual void                    BeginSetClipRegion( ULONG nRects );
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    virtual void                    EndSetClipRegion();

    virtual void                    SetPosSize( long nX, long nY, long nWidth, long nHeight );
    virtual void                    Show( BOOL bVisible );
    virtual void                    Enable( BOOL nEnable );
    virtual void                    GrabFocus();

    virtual void                    SetBackground();
    virtual void                    SetBackground( SalColor nSalColor );

    virtual const SystemEnvData*    GetSystemData() const;
};

class SvpImeStatus : public SalI18NImeStatus
{
  public:
        SvpImeStatus() {}
        virtual ~SvpImeStatus();

        virtual bool canToggle();
        virtual void toggle();
};

class SvpSalSystem : public SalSystem
{
    public:
    SvpSalSystem() {}
    virtual ~SvpSalSystem();
    // get info about the display
    virtual unsigned int GetDisplayScreenCount();
    virtual bool IsMultiDisplay();
    virtual unsigned int GetDefaultDisplayNumber();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle GetDisplayWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetScreenName( unsigned int nScreen );


    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
};


#endif // _SVP_SVPDUMMIES_H

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
    SystemEnvData m_aSystemChildData;

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
    virtual OUString GetDisplayScreenName( unsigned int nScreen );

    virtual int ShowNativeDialog( const OUString& rTitle,
                                  const OUString& rMessage,
                                  const std::list< OUString >& rButtons,
                                  int nDefButton );
};


#endif // _SVP_SVPDUMMIES_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

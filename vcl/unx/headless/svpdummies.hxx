/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SVP_SVPDUMMIES_HXX

#include <vcl/sysdata.hxx>

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
    virtual void                    Enable( sal_Bool nEnable );
    virtual void                    GrabFocus();

    virtual void                    SetBackground();
    virtual void                    SetBackground( SalColor nSalColor );

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

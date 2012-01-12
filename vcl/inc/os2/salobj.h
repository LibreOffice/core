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



#ifndef _SV_SALOBJ_H
#define _SV_SALOBJ_H

#include <salobj.hxx>

// -----------------
// - SalObjectData -
// -----------------

class Os2SalObject : public SalObject
{
public:
    HWND                    mhWnd;                  // Window handle
    HWND                    mhWndChild;             // Child Window handle
    HWND                    mhLastFocusWnd;         // Child-Window, welches als letztes den Focus hatte
    SystemChildData         maSysData;              // SystemEnvData
    HWND                    mhLastClipWnd;          // LastClip-Window
    HWND                    mhOldLastClipWnd;       // LastClip-Window befor BeginSetClipRegion
    long                    mnHeight;               // Fenster-Hoehe fuer Positionsumrechnung
    Os2SalObject*               mpNextObject;           // pointer to next object
    void*                   mpInst;                 // instance handle for callback
    SALOBJECTPROC           mpProc;                 // callback proc

    Os2SalObject();
    virtual ~Os2SalObject();

    virtual void                    ResetClipRegion();
    virtual USHORT                  GetClipRegionType();
    virtual void                    BeginSetClipRegion( ULONG nRects );
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

#endif // _SV_SALOBJ_H

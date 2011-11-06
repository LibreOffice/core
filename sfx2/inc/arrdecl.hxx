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


#ifndef _SFX_ARRDECL_HXX
#define _SFX_ARRDECL_HXX

#include <tools/list.hxx>
#include <svl/svarray.hxx>
#include <sfx2/minarray.hxx>

struct CntUpdateResult;

SV_DECL_PTRARR_DEL(CntUpdateResults_Impl, CntUpdateResult*, 4, 4)

class SfxObjectShell;
SV_DECL_PTRARR( SfxObjectShellArr_Impl, SfxObjectShell*, 4, 4 )

class SfxViewFrame;
SV_DECL_PTRARR( SfxViewFrameArr_Impl, SfxViewFrame*, 4, 4 )

class SfxViewShell;
SV_DECL_PTRARR( SfxViewShellArr_Impl, SfxViewShell*, 4, 4 )

class SfxObjectFactory;
typedef SfxObjectFactory* SfxObjectFactoryPtr;
SV_DECL_PTRARR( SfxObjectFactoryArr_Impl, SfxObjectFactoryPtr, 3, 3 )

struct SfxTbxCtrlFactory;
SV_DECL_PTRARR_DEL( SfxTbxCtrlFactArr_Impl, SfxTbxCtrlFactory*, 8, 4 )

struct SfxStbCtrlFactory;
SV_DECL_PTRARR_DEL( SfxStbCtrlFactArr_Impl, SfxStbCtrlFactory*, 8, 4 )

struct SfxMenuCtrlFactory;
SV_DECL_PTRARR_DEL( SfxMenuCtrlFactArr_Impl, SfxMenuCtrlFactory*, 2, 2 )

struct SfxChildWinFactory;
SV_DECL_PTRARR_DEL( SfxChildWinFactArr_Impl, SfxChildWinFactory*, 2, 2 )

class SfxModule;
SV_DECL_PTRARR( SfxModuleArr_Impl, SfxModule*, 2, 2 )

class SfxFilter;
DECL_PTRARRAY( SfxFilterArr_Impl, SfxFilter*, 4, 4 )

class SfxFrame;
typedef SfxFrame* SfxFramePtr;
SV_DECL_PTRARR( SfxFrameArr_Impl, SfxFramePtr, 4, 4 )

DECLARE_LIST( SfxFilterList_Impl, SfxFilter* )

struct SfxExternalLib_Impl;
typedef SfxExternalLib_Impl* SfxExternalLibPtr;
SV_DECL_PTRARR_DEL( SfxExternalLibArr_Impl, SfxExternalLibPtr, 2, 2 )

//class XEventListenerRef;
//typedef XEventListenerRef* XEventListenerPtr;
//SV_DECL_PTRARR_DEL( XEventListenerArr_Impl, XEventListenerPtr, 4, 4 )

//class XFrameRef;
//typedef XFrameRef* XFramePtr;
//SV_DECL_PTRARR_DEL( XFrameArr_Impl, XFramePtr, 4, 4 )

class SfxSlot;
typedef SfxSlot* SfxSlotPtr;
SV_DECL_PTRARR( SfxSlotArr_Impl, SfxSlotPtr, 20, 20 )

#endif

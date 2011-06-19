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
#ifndef _SFX_MACROCONF_HXX
#define _SFX_MACROCONF_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/errcode.hxx>
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>     // SvUShorts
#include <sfx2/evntconf.hxx>

class SfxMacroInfo;
class SfxSlot;
class SfxMacroInfoItem;
class SfxObjectShell;
class BasicManager;
struct SfxMacroConfig_Impl;
class SbMethod;
class SbxValue;
class SbxObject;
class SbxArray;
class SvStream;
class SvxMacro;

typedef SfxMacroInfo* SfxMacroInfoPtr;
SV_DECL_PTRARR(SfxMacroInfoArr_Impl, SfxMacroInfoPtr, 5, 5)

class SFX2_DLLPUBLIC SfxMacroInfo
{
friend class SfxMacroConfig;
friend class SfxEventConfiguration;

    String*                 pHelpText;
    sal_uInt16                  nRefCnt;
    sal_Bool                    bAppBasic;
    String                  aLibName;
    String                  aModuleName;
    String                  aMethodName;
    sal_uInt16                  nSlotId;
    SfxSlot*                pSlot;

public:
    SfxMacroInfo( const String& rURL );
    SfxMacroInfo( bool _bAppBasic = true );
    SfxMacroInfo( bool _bAppBasic, const String& rQualifiedName );
    SfxMacroInfo(SfxMacroInfo& rOther);
    SfxMacroInfo(bool _bAppBasic, const String& rLibName,
                    const String& rModuleName, const String& rMethodName);
    ~SfxMacroInfo();
    sal_Bool operator==(const SfxMacroInfo& rOther) const;
    String              GetMacroName() const;
    String              GetQualifiedName() const;
    String              GetFullQualifiedName() const;
    BasicManager*       GetBasicManager() const;
    String              GetBasicName() const;
    String              GetHelpText() const;
    sal_Bool                IsAppMacro() const
                        { return bAppBasic; }
    const String&       GetModuleName() const
                        { return aModuleName; }
    const String&       GetLibName() const
                        { return aLibName; }
    const String&       GetMethodName() const
                        { return aMethodName; }
    sal_uInt16              GetSlotId() const
                        { return nSlotId; }
    SfxSlot*            GetSlot() const
                        { return pSlot; }

    sal_Bool                Compare( const SvxMacro& ) const;
    void                SetHelpText( const String& rText );
    String              GetURL() const;
};

class SFX2_DLLPUBLIC SfxMacroConfig
{
friend class SfxEventConfiguration;

    SAL_DLLPRIVATE static SfxMacroConfig* pMacroConfig;

    SfxMacroConfig_Impl*    pImp;
    SvUShorts               aIdArray;

public:
                            SfxMacroConfig();
                            ~SfxMacroConfig();

    static SfxMacroConfig*  GetOrCreate();

    static String           RequestHelp( sal_uInt16 nId );
    static sal_Bool             IsMacroSlot( sal_uInt16 nId );
    static sal_Bool             IsBasic( SbxObject*, const String&, BasicManager* );
    static ErrCode          Call( SbxObject*, const String&, BasicManager*,
                                SbxArray *pArgs=NULL, SbxValue *pRet=NULL );
    static SbMethod*        GetMethod_Impl( const String&, BasicManager* );

    sal_uInt16                  GetSlotId(SfxMacroInfoPtr);
    void                    ReleaseSlotId(sal_uInt16 nId);
    void                    RegisterSlotId(sal_uInt16 nId);
    SfxMacroInfo*           GetMacroInfo(sal_uInt16 nId) const;
    sal_Bool                    ExecuteMacro(sal_uInt16 nId, const String& rArgs ) const;
    sal_Bool                    ExecuteMacro( SfxObjectShell*, const SvxMacro*, const String& ) const;
    sal_Bool                    CheckMacro(sal_uInt16 nId) const;
    sal_Bool                    CheckMacro( SfxObjectShell*, const SvxMacro* ) const;

    SAL_DLLPRIVATE static void Release_Impl();
    SAL_DLLPRIVATE const SfxMacroInfo* GetMacroInfo_Impl( const SvxMacro *pMacro ) const;
    DECL_DLLPRIVATE_LINK( CallbackHdl_Impl, SfxMacroConfig*);
    DECL_DLLPRIVATE_LINK( EventHdl_Impl, SfxMacroInfo*);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <tools/errcode.hxx>
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>		// SvUShorts
#include <bf_sfx2/evntconf.hxx>
class SvStream;
namespace binfilter {

class SvxMacro;
class BasicManager;
class SbMethod;
class SbxValue;
class SbxObject;
class SbxArray;
class SfxMacroInfo;
class SfxSlot;
class SfxMacroInfoItem;
class SfxObjectShell;
struct SfxMacroConfig_Impl;

typedef SfxMacroInfo* SfxMacroInfoPtr;
#if _SOLAR__PRIVATE
SV_DECL_PTRARR(SfxMacroInfoArr_Impl, SfxMacroInfoPtr, 5, 5)//STRIP008 ;
#else
class SfxMacroInfoArr_Impl;
#endif

class SfxMacroInfo
{
friend class SfxMacroConfig;
friend class SfxEventConfiguration;
friend SvStream& operator >> (SvStream& rStream, SfxMacroInfo& rInfo);
friend SvStream& operator << (SvStream& rStream, const SfxMacroInfo& rInfo);

    SfxObjectShell* 		pDocShell;			// nur noch wg. Kompatib. drin
    String*   				pHelpText;
    sal_uInt16                  nRefCnt;
    sal_Bool					bAppBasic;
    String					aLibName;
    String					aModuleName;
    String					aMethodName;
    sal_uInt16					nSlotId;
    SfxSlot*				pSlot;

public:
    SfxMacroInfo(SfxObjectShell *pDoc=NULL);
    ~SfxMacroInfo();
    sal_Bool operator==(const SfxMacroInfo& rOther) const;
    int Store (SvStream&);
    String				GetQualifiedName() const;
    String				GetBasicName() const;
    sal_Bool				IsAppMacro() const
                        { return bAppBasic; }
    const String&		GetModuleName() const
                        { return aModuleName; }
    const String&		GetLibName() const
                        { return aLibName; }
    const String&		GetMethodName() const
                        { return aMethodName; }
    sal_uInt16				GetSlotId() const
                        { return nSlotId; }
    SfxSlot*			GetSlot() const
                        { return pSlot; }
};

//ASDBG obsolete >= 582
//ASDBG class ::com::sun::star::uno::Reference< ::com::sun::star::script::XEngine > ;
//ASDBG class ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ;

class SfxMacroConfig
{
friend class SfxEventConfiguration;

    static SfxMacroConfig*	pMacroConfig;

    SfxMacroConfig_Impl* 	pImp;
    SvUShorts               aIdArray;

public:
                            ~SfxMacroConfig();

    static ErrCode			Call( SbxObject*, const String&, BasicManager*,
                                SbxArray *pArgs=NULL, SbxValue *pRet=NULL );
//ASDBG obsolete >= 582
//ASDBG 	static void	CallStarScript( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEngine > & rxEngine, const String & rCode,
//ASDBG     const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rSource, void *pArgs, void *pRet );
    static SbMethod*		GetMethod_Impl( const String&, BasicManager* );

#if _SOLAR__PRIVATE
    static void				Release_Impl();
    DECL_LINK(				CallbackHdl_Impl, SfxMacroConfig*);
    DECL_LINK(				EventHdl_Impl, SfxMacroInfo*);
#endif
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

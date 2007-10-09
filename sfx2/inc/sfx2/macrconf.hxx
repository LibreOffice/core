/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macrconf.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:30:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFX_MACROCONF_HXX
#define _SFX_MACROCONF_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_SFX2_DLLAPI_H
#include "sfx2/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif
#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>     // SvUShorts
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
//#if 0 // _SOLAR__PRIVATE
SV_DECL_PTRARR(SfxMacroInfoArr_Impl, SfxMacroInfoPtr, 5, 5)
//#else
//class SfxMacroInfoArr_Impl;
//#endif

class SFX2_DLLPUBLIC SfxMacroInfo
{
friend class SfxMacroConfig;
friend class SfxEventConfiguration;
friend SvStream& operator >> (SvStream& rStream, SfxMacroInfo& rInfo);
friend SvStream& operator << (SvStream& rStream, const SfxMacroInfo& rInfo);

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
    int Load (SvStream&);
    int Store (SvStream&);
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

//ASDBG obsolete >= 582
//ASDBG class ::com::sun::star::uno::Reference< ::com::sun::star::script::XEngine > ;
//ASDBG class ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > ;

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
//ASDBG obsolete >= 582
//ASDBG     static void CallStarScript( const ::com::sun::star::uno::Reference< ::com::sun::star::script::XEngine > & rxEngine, const String & rCode,
//ASDBG     const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & rSource, void *pArgs, void *pRet );
    static SbMethod*        GetMethod_Impl( const String&, BasicManager* );

    sal_uInt16                  GetSlotId(SfxMacroInfoPtr);
    void                    ReleaseSlotId(sal_uInt16 nId);
    void                    RegisterSlotId(sal_uInt16 nId);
    const SfxMacroInfoPtr   GetMacroInfo(sal_uInt16 nId) const;
    sal_Bool                    ExecuteMacro(sal_uInt16 nId, const String& rArgs ) const;
    sal_Bool                    ExecuteMacro( SfxObjectShell*, const SvxMacro*, const String& ) const;
    sal_Bool                    CheckMacro(sal_uInt16 nId) const;
    sal_Bool                    CheckMacro( SfxObjectShell*, const SvxMacro* ) const;

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE static void Release_Impl();
    SAL_DLLPRIVATE const SfxMacroInfoPtr GetMacroInfo_Impl( const SvxMacro *pMacro ) const;
    DECL_DLLPRIVATE_LINK( CallbackHdl_Impl, SfxMacroConfig*);
    DECL_DLLPRIVATE_LINK( EventHdl_Impl, SfxMacroInfo*);
//#endif
};

#endif

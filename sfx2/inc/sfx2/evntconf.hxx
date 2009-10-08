/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: evntconf.hxx,v $
 * $Revision: 1.3.28.1 $
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
#ifndef _SFX_EVENTCONF_HXX
#define _SFX_EVENTCONF_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"

//#if 0 // _SOLAR__PRIVATE

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

//#endif

#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>     // SvUShorts
#include <sfx2/event.hxx>

#include <sfx2/sfxsids.hrc>
#ifdef ITEMID_MACRO
#undef ITEMID_MACRO
#endif
#define ITEMID_MACRO SID_ATTR_MACROITEM
#include <svtools/macitem.hxx>

class SfxMacroInfo;
class SfxMacroTabPage;
class SfxMacroInfoArr_Impl;
class SfxEventConfigItem_Impl;
class SfxEventInfoArr_Impl;
class SfxObjectShell;
class SvxMacroTableDtor;

#define SFX_NO_EVENT USHRT_MAX

//#if 0 // _SOLAR__PRIVATE
struct SfxEvent_Impl
{
    String                      aEventName;
    USHORT                      nEventId;

    SfxEvent_Impl(const String& rName, USHORT nId) :
        aEventName(rName),
        nEventId(nId)
    {}
};

SV_DECL_PTRARR(SfxEventArr_Impl, SfxEvent_Impl*, 5, 5)

// -----------------------------------------------------------------------

#define PROP_EVENT_TYPE     "EventType"
#define PROP_LIBRARY        "Library"
#define PROP_SCRIPT         "Script"
#define PROP_MACRO_NAME     "MacroName"
#define STAR_BASIC          "StarBasic"

// -----------------------------------------------------------------------
//#else
//typedef SvPtrarr SfxEventArr_Impl;
//#endif

class SFX2_DLLPUBLIC SfxEventConfiguration
{
friend class SfxEventConfigItem_Impl;

    SfxMacroInfoArr_Impl*   pArr;
    SfxEventArr_Impl*       pEventArr;
    SvxMacroTableDtor*      pAppTable;
    SvxMacroTableDtor*      pDocTable;
    sal_Bool                bIgnoreConfigure;

public:
                            SfxEventConfiguration();
                            ~SfxEventConfiguration();

    void                    RegisterEvent(USHORT nId, const String& rName);
    String                  GetEventName(USHORT nId) const;
    USHORT                  GetEventId(const String& rName) const;
    USHORT                  GetEventCount() const
                            { return pEventArr->Count()-1; }
  //  const SvxMacro*         GetMacroForEventId( USHORT nEventId, SfxObjectShell *pObjSh );
    void                    ConfigureEvent(USHORT nId, const SvxMacro&,
                                SfxObjectShell* pObjSh);
    void                    ConfigureEvent(USHORT nId, const String& rMacro,
                                SfxObjectShell* pObjSh);

    //const SfxMacroInfo*     GetMacroInfo(USHORT nEventId, SfxObjectShell* pObjSh) const;

    // void                    AddEvents( SfxMacroTabPage* ) const;
    //SvxMacroTableDtor*      GetAppEventTable();
    SvxMacroTableDtor*      GetDocEventTable(SfxObjectShell*);
    //void                    SetAppEventTable( const SvxMacroTableDtor& );
    //void                    SetDocEventTable( SfxObjectShell*, const SvxMacroTableDtor& );

    static void             RegisterEvent( USHORT nId, const String& rName,
                                           const String& rMacroName );

    //static BOOL             Import( SvStream& rInStream, SvStream* pOutStream, SfxObjectShell* pDoc=NULL );
    //static BOOL             Export( SvStream* pInStream, SvStream& rOutStream, SfxObjectShell* pDoc=NULL );

//#if 0 // _SOLAR__PRIVATE
    SAL_DLLPRIVATE const SfxEvent_Impl* GetEvent_Impl(USHORT nPos) const
                                    { return (*pEventArr)[nPos+1]; }

    SAL_DLLPRIVATE BOOL Warn_Impl( SfxObjectShell *pDoc, const SvxMacro* pMacro );
    SAL_DLLPRIVATE void PropagateEvent_Impl( SfxObjectShell *pDoc,
                                                     USHORT nId,
                                                     const SvxMacro* pMacro );
    SAL_DLLPRIVATE static rtl::OUString GetEventName_Impl( ULONG nID );
    SAL_DLLPRIVATE static ULONG GetEventId_Impl( const rtl::OUString& rEventName );
    SAL_DLLPRIVATE ::com::sun::star::uno::Any CreateEventData_Impl( const SvxMacro *pMacro );

    SAL_DLLPRIVATE static ULONG GetPos_Impl( USHORT nID, sal_Bool &rFound );
    SAL_DLLPRIVATE static ULONG GetPos_Impl( const String& rName, sal_Bool &rFound );
//#endif
};
/*
//#if 0 // _SOLAR__PRIVATE
class SfxEventConfigItem_Impl : public SfxConfigItem
{
friend class SfxEventConfiguration;

    SvxMacroTableDtor       aMacroTable;
    SfxEventConfiguration   *pEvConfig;
    SfxObjectShell          *pObjShell;
    BOOL                    bWarning;
    BOOL                    bAlwaysWarning;
    BOOL                    bInitialized;

    void                    ConstructMacroTable();
    int                     Load(SvStream&);
    BOOL                    Store(SvStream&);
    BOOL                    LoadXML(SvStream&);
    BOOL                    StoreXML(SvStream&);

public:
                            SfxEventConfigItem_Impl( USHORT,
                                SfxEventConfiguration*,
                                SfxObjectShell* pObjSh = NULL );
                            ~SfxEventConfigItem_Impl();

    void                    Init( SfxConfigManager* );
    virtual int             Load(SotStorage&);
    virtual BOOL            Store(SotStorage&);
    virtual String          GetStreamName() const;
    virtual void            UseDefault();
    void                    ConfigureEvent( USHORT nPos, SvxMacro* );
};

//#endif
*/
#endif

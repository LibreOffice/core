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
#include <tools/rtti.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

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
class SfxMacroInfoArr_Impl;
class SfxEventConfigItem_Impl;
class SfxEventInfoArr_Impl;
class SfxObjectShell;
class SvxMacroTableDtor;

#define SFX_NO_EVENT USHRT_MAX

struct SFX2_DLLPUBLIC SfxEventName
{
    USHORT  mnId;
    String  maEventName;
    String  maUIName;

            SfxEventName( USHORT nId,
                             const String& rEventName,
                             const String& rUIName )
                : mnId( nId )
                , maEventName( rEventName )
                , maUIName( rUIName ) {}
};

DECLARE_TABLE( _SfxEventNamesList, SfxEventName* )

class SFX2_DLLPUBLIC SfxEventNamesList : public _SfxEventNamesList
{
public:
    SfxEventNamesList( const USHORT nInitSz = 0, const USHORT nReSz = 1 ): _SfxEventNamesList( nInitSz, nReSz ) {}
    SfxEventNamesList( const SfxEventNamesList &rCpy ) : _SfxEventNamesList() { *this = rCpy; }
    ~SfxEventNamesList() { DelDtor(); }
    SfxEventNamesList& operator=( const SfxEventNamesList &rCpy );
    void DelDtor();
};

class SFX2_DLLPUBLIC SfxEventNamesItem : public SfxPoolItem
{
    SfxEventNamesList aEventsList;

public:
    TYPEINFO();

    SfxEventNamesItem ( const USHORT nId ) : SfxPoolItem( nId ) {}

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT) const;
    virtual SvStream&       Store(SvStream &, USHORT nItemVersion ) const;
    virtual USHORT          GetVersion( USHORT nFileFormatVersion ) const;

    const SfxEventNamesList& GetEvents() const { return aEventsList;}
    void SetEvents( const SfxEventNamesList& rList ) { aEventsList = rList; }
    void                    AddEvent( const String&, const String&, USHORT );
};

// -----------------------------------------------------------------------

#define PROP_EVENT_TYPE     "EventType"
#define PROP_LIBRARY        "Library"
#define PROP_SCRIPT         "Script"
#define PROP_MACRO_NAME     "MacroName"
#define STAR_BASIC          "StarBasic"

class SFX2_DLLPUBLIC SfxEventConfiguration
{
friend class SfxEventConfigItem_Impl;

    SvxMacroTableDtor*      pAppTable;
    SvxMacroTableDtor*      pDocTable;
    sal_Bool                bIgnoreConfigure;

public:
                            SfxEventConfiguration();
                            ~SfxEventConfiguration();

    void                    ConfigureEvent(USHORT nId, const SvxMacro&,
                                SfxObjectShell* pObjSh);
    void                    ConfigureEvent(USHORT nId, const String& rMacro,
                                SfxObjectShell* pObjSh);

    SvxMacroTableDtor*      GetDocEventTable(SfxObjectShell*);

    static void             RegisterEvent( USHORT nId, const String& rName,
                                           const String& rMacroName );

    SAL_DLLPRIVATE BOOL Warn_Impl( SfxObjectShell *pDoc, const SvxMacro* pMacro );
    SAL_DLLPRIVATE void PropagateEvent_Impl( SfxObjectShell *pDoc,
                                                     USHORT nId,
                                                     const SvxMacro* pMacro );
    SAL_DLLPRIVATE static rtl::OUString GetEventName_Impl( ULONG nID );
    SAL_DLLPRIVATE static ULONG GetEventId_Impl( const rtl::OUString& rEventName );
    SAL_DLLPRIVATE ::com::sun::star::uno::Any CreateEventData_Impl( const SvxMacro *pMacro );

    SAL_DLLPRIVATE static ULONG GetPos_Impl( USHORT nID, sal_Bool &rFound );
    SAL_DLLPRIVATE static ULONG GetPos_Impl( const String& rName, sal_Bool &rFound );
};

#endif

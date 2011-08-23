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
#ifndef _SFX_EVENTCONF_HXX
#define _SFX_EVENTCONF_HXX

#if _SOLAR__PRIVATE

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

#endif

#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>		// SvUShorts
#include <bf_sfx2/cfgitem.hxx>
#include <bf_sfx2/event.hxx>

#include <bf_sfx2/sfxsids.hrc>
#ifdef ITEMID_MACRO
#undef ITEMID_MACRO
#endif
#define ITEMID_MACRO SID_ATTR_MACROITEM
#include <bf_svtools/macitem.hxx>
namespace binfilter {

class SfxMacroInfo;
class SfxMacroTabPage;
class SfxMacroInfoArr_Impl;
class SfxEventConfigItem_Impl;
class SfxEventInfoArr_Impl;
class SfxObjectShell;

#define SFX_NO_EVENT USHRT_MAX

#if _SOLAR__PRIVATE
struct SfxEvent_Impl
{
    String						aEventName;
    USHORT						nEventId;

    SfxEvent_Impl(const String& rName, USHORT nId) :
        aEventName(rName),
        nEventId(nId)
    {}
};

SV_DECL_PTRARR(SfxEventArr_Impl, SfxEvent_Impl*, 5, 5)//STRIP008 ;

// -----------------------------------------------------------------------

#define	PROP_EVENT_TYPE		"EventType"
#define PROP_LIBRARY		"Library"
#define PROP_SCRIPT			"Script"
#define PROP_SCRIPT_URL		"URL"
#define PROP_MACRO_NAME		"MacroName"
#define STAR_BASIC			"StarBasic"
#define JAVA_SCRIPT			"JavaScript"

// -----------------------------------------------------------------------
#else
typedef SvPtrarr SfxEventArr_Impl;
#endif

class SfxEventConfiguration
{
friend class SfxEventConfigItem_Impl;

    SfxMacroInfoArr_Impl*	pArr;
    SfxEventArr_Impl*		pEventArr;
    SfxEventConfigItem_Impl* pAppEventConfig;
    SfxEventConfigItem_Impl* pDocEventConfig;
    sal_Bool				bIgnoreConfigure;

public:
                            SfxEventConfiguration();
                            ~SfxEventConfiguration();

    void					RegisterEvent(USHORT nId, const String& rName);
    USHORT					GetEventCount() const
                            { return pEventArr->Count()-1; }
    void					ConfigureEvent(USHORT nId, const String& rMacro,
                                SfxObjectShell* pObjSh);

    SvxMacroTableDtor*		GetAppEventTable();

    static void				RegisterEvent( USHORT nId, const String& rName,
                                           const String& rMacroName );

    static BOOL             Import( SvStream& rInStream, SvStream* pOutStream, SfxObjectShell* pDoc=NULL );
    static BOOL             Export( SvStream* pInStream, SvStream& rOutStream, SfxObjectShell* pDoc=NULL );

#if _SOLAR__PRIVATE
    SfxEventConfigItem_Impl* 	GetAppEventConfig_Impl();
    const SfxEvent_Impl*		GetEvent_Impl(USHORT nPos) const
                                    { return (*pEventArr)[nPos+1]; }
    BOOL						Warn_Impl( SfxObjectShell *pDoc, const SvxMacro* pMacro );
    void						PropagateEvents_Impl( SfxObjectShell *pDoc,
                                                      const SvxMacroTableDtor& rTable );
    static ::rtl::OUString		GetEventName_Impl( ULONG nID );
    static ULONG				GetEventId_Impl( const ::rtl::OUString& rEventName );
    ::com::sun::star::uno::Any	CreateEventData_Impl( const SvxMacro *pMacro );

    static ULONG				GetPos_Impl( USHORT nID, sal_Bool &rFound );
    static ULONG				GetPos_Impl( const String& rName, sal_Bool &rFound );
#endif
};

#if _SOLAR__PRIVATE
class SfxEventConfigItem_Impl : public SfxConfigItem
{
friend class SfxEventConfiguration;

    SvxMacroTableDtor		aMacroTable;
    SfxEventConfiguration	*pEvConfig;
    SfxObjectShell			*pObjShell;
    BOOL					bWarning;
    BOOL					bAlwaysWarning;
    BOOL					bInitialized;

    void					ConstructMacroTable();
    int                     Load(SvStream&);

public:
                            SfxEventConfigItem_Impl( USHORT,
                                SfxEventConfiguration*,
                                SfxObjectShell* pObjSh = NULL );
                            ~SfxEventConfigItem_Impl();

    void					Init( SfxConfigManager* );
    virtual int             Load(SotStorage&);
    virtual BOOL            Store(SotStorage&);
    virtual String          GetStreamName() const;
    virtual void			UseDefault();
    void					ConfigureEvent( USHORT nPos, SvxMacro* );
};

#endif

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

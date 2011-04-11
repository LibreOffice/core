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

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/rtti.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>     // SvUShorts
#include <sfx2/event.hxx>

#include <sfx2/sfxsids.hrc>
#ifdef ITEMID_MACRO
#undef ITEMID_MACRO
#endif
#define ITEMID_MACRO SID_ATTR_MACROITEM
#include <svl/macitem.hxx>
#include <vector>

class SfxObjectShell;
class SvxMacroTableDtor;

#define SFX_NO_EVENT USHRT_MAX

struct SFX2_DLLPUBLIC SfxEventName
{
    sal_uInt16  mnId;
    String  maEventName;
    String  maUIName;

            SfxEventName( sal_uInt16 nId,
                             const String& rEventName,
                             const String& rUIName )
                : mnId( nId )
                , maEventName( rEventName )
                , maUIName( rUIName ) {}
};

typedef ::std::vector< SfxEventName* > _SfxEventNamesList;

class SFX2_DLLPUBLIC SfxEventNamesList
{
private:
    _SfxEventNamesList  aEventNamesList;
    void DelDtor();

public:
    SfxEventNamesList() {}
    SfxEventNamesList( const SfxEventNamesList &rCpy ) { *this = rCpy; }
    ~SfxEventNamesList() { DelDtor(); }
    SfxEventNamesList& operator=( const SfxEventNamesList &rCpy );

    size_t size() const { return aEventNamesList.size(); };

    SfxEventName* at( size_t Index ) const
        { return Index < aEventNamesList.size() ? aEventNamesList[ Index ] : NULL; }

    void push_back( SfxEventName* Item ) { aEventNamesList.push_back( Item ); }
};

class SFX2_DLLPUBLIC SfxEventNamesItem : public SfxPoolItem
{
    SfxEventNamesList aEventsList;

public:
    TYPEINFO();

    SfxEventNamesItem ( const sal_uInt16 nId ) : SfxPoolItem( nId ) {}

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    XubString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileFormatVersion ) const;

    const SfxEventNamesList& GetEvents() const { return aEventsList;}
    void SetEvents( const SfxEventNamesList& rList ) { aEventsList = rList; }
    void                    AddEvent( const String&, const String&, sal_uInt16 );
};

// -----------------------------------------------------------------------

#define PROP_EVENT_TYPE     "EventType"
#define PROP_LIBRARY        "Library"
#define PROP_SCRIPT         "Script"
#define PROP_MACRO_NAME     "MacroName"
#define STAR_BASIC          "StarBasic"

class SFX2_DLLPUBLIC SfxEventConfiguration
{
public:
    static void                         ConfigureEvent( ::rtl::OUString aName, const SvxMacro&, SfxObjectShell* pObjSh);
    static SvxMacro*                    ConvertToMacro( const com::sun::star::uno::Any& rElement, SfxObjectShell* pDoc, sal_Bool bBlowUp );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

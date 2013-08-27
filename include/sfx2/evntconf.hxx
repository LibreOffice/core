/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _SFX_EVENTCONF_HXX
#define _SFX_EVENTCONF_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/rtti.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <sfx2/event.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/macitem.hxx>
#include <vector>

class SfxObjectShell;
class SvxMacroTableDtor;

struct SFX2_DLLPUBLIC SfxEventName
{
    sal_uInt16  mnId;
    OUString    maEventName;
    OUString    maUIName;

            SfxEventName( sal_uInt16 nId,
                             const OUString& rEventName,
                             const OUString& rUIName )
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
                                    OUString &rText,
                                    const IntlWrapper * = 0 ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const;
    virtual sal_uInt16      GetVersion( sal_uInt16 nFileFormatVersion ) const;

    const SfxEventNamesList& GetEvents() const { return aEventsList;}
    void SetEvents( const SfxEventNamesList& rList ) { aEventsList = rList; }
    void                    AddEvent( const OUString&, const OUString&, sal_uInt16 );
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
    static void                         ConfigureEvent( OUString aName, const SvxMacro&, SfxObjectShell* pObjSh);
    static SvxMacro*                    ConvertToMacro( const com::sun::star::uno::Any& rElement, SfxObjectShell* pDoc, sal_Bool bBlowUp );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SFX2_EVNTCONF_HXX
#define INCLUDED_SFX2_EVNTCONF_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
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

class SFX2_DLLPUBLIC SfxEventNamesList
{
private:
    ::std::vector< SfxEventName* > aEventNamesList;
    void DelDtor();

public:
    SfxEventNamesList() {}
    SfxEventNamesList( const SfxEventNamesList &rCpy ) { *this = rCpy; }
    ~SfxEventNamesList() { DelDtor(); }
    SfxEventNamesList& operator=( const SfxEventNamesList &rCpy );

    size_t size() const { return aEventNamesList.size(); };

    SfxEventName* at( size_t Index ) const
        { return Index < aEventNamesList.size() ? aEventNamesList[ Index ] : nullptr; }

    void push_back( SfxEventName* Item ) { aEventNamesList.push_back( Item ); }
};

class SFX2_DLLPUBLIC SfxEventNamesItem : public SfxPoolItem
{
    SfxEventNamesList aEventsList;

public:

    SfxEventNamesItem ( const sal_uInt16 nId ) : SfxPoolItem( nId ) {}

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper * = nullptr ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const override;
    virtual SvStream&       Store(SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual sal_uInt16      GetVersion( sal_uInt16 nFileFormatVersion ) const override;

    const SfxEventNamesList& GetEvents() const { return aEventsList;}
    void                    AddEvent( const OUString&, const OUString&, sal_uInt16 );
};



#define PROP_EVENT_TYPE     "EventType"
#define PROP_LIBRARY        "Library"
#define PROP_SCRIPT         "Script"
#define PROP_MACRO_NAME     "MacroName"
#define STAR_BASIC          "StarBasic"

class SFX2_DLLPUBLIC SfxEventConfiguration
{
public:
    static void                         ConfigureEvent( const OUString& aName, const SvxMacro&, SfxObjectShell* pObjSh);
    static SvxMacro*                    ConvertToMacro( const css::uno::Any& rElement, SfxObjectShell* pDoc, bool bBlowUp );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

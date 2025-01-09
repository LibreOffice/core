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
#include <sfx2/event.hxx>
#include <svl/poolitem.hxx>
#include <memory>
#include <utility>
#include <vector>

class SfxObjectShell;
class SvxMacro;

struct SFX2_DLLPUBLIC SfxEventName
{
    SvMacroItemId mnId;
    OUString      maEventName;
    OUString      maUIName;

            SfxEventName( SvMacroItemId nId,
                             OUString aEventName,
                             OUString aUIName )
                : mnId( nId )
                , maEventName(std::move( aEventName ))
                , maUIName(std::move( aUIName )) {}
};

class SFX2_DLLPUBLIC SfxEventNamesList
{
private:
    ::std::vector< SfxEventName > aEventNamesList;

public:
    SfxEventNamesList() {}
    SfxEventNamesList( const SfxEventNamesList &rCpy ) { *this = rCpy; }
    ~SfxEventNamesList();
    SfxEventNamesList& operator=( const SfxEventNamesList &rCpy );

    size_t size() const { return aEventNamesList.size(); };

    SfxEventName& at( size_t Index ) { return aEventNamesList[ Index ]; }
    SfxEventName const & at( size_t Index ) const { return aEventNamesList[ Index ]; }

    void push_back( SfxEventName Item ) { aEventNamesList.push_back( std::move(Item) ); }
};

class SFX2_DLLPUBLIC SfxEventNamesItem final : public SfxPoolItem
{
    SfxEventNamesList aEventsList;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SfxEventNamesItem)
    SfxEventNamesItem ( const sal_uInt16 nId )
        : SfxPoolItem( nId ) {}

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& ) const override;
    virtual SfxEventNamesItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    const SfxEventNamesList& GetEvents() const { return aEventsList;}
    void                    AddEvent( const OUString&, const OUString&, SvMacroItemId );
};


inline constexpr OUString PROP_EVENT_TYPE = u"EventType"_ustr;
inline constexpr OUString PROP_LIBRARY = u"Library"_ustr;
inline constexpr OUString PROP_SCRIPT = u"Script"_ustr;
inline constexpr OUString PROP_MACRO_NAME = u"MacroName"_ustr;
#define STAR_BASIC          "StarBasic"

class SFX2_DLLPUBLIC SfxEventConfiguration
{
public:
    static void                         ConfigureEvent( const OUString& aName, const SvxMacro&, SfxObjectShell const * pObjSh);
    static std::unique_ptr<SvxMacro>    ConvertToMacro( const css::uno::Any& rElement, SfxObjectShell* pDoc );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

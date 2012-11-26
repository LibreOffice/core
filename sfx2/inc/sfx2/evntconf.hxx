/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFX_EVENTCONF_HXX
#define _SFX_EVENTCONF_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
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

DECLARE_LIST( _SfxEventNamesList, SfxEventName* )

class SFX2_DLLPUBLIC SfxEventNamesList : public _SfxEventNamesList
{
public:
    SfxEventNamesList( const sal_uInt16 nInitSz = 0, const sal_uInt16 nReSz = 1 ): _SfxEventNamesList( nInitSz, nReSz ) {}
    SfxEventNamesList( const SfxEventNamesList &rCpy ) : _SfxEventNamesList() { *this = rCpy; }
    ~SfxEventNamesList() { DelDtor(); }
    SfxEventNamesList& operator=( const SfxEventNamesList &rCpy );
    void DelDtor();
};

class SFX2_DLLPUBLIC SfxEventNamesItem : public SfxPoolItem
{
    SfxEventNamesList aEventsList;

public:
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

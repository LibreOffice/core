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
#ifndef _SFXEVENT_HXX
#define _SFXEVENT_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sfx2/sfx.hrc"
#include <svl/hint.hxx>
#include <unotools/eventcfg.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/view/PrintableState.hpp>

namespace css = ::com::sun::star;

class SfxObjectShell;

//-------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxEventHint : public SfxHint
{
    SfxObjectShell*     pObjShell;
    OUString     aEventName;
    sal_uInt16              nEventId;

public:
    TYPEINFO();
    SfxEventHint( sal_uInt16 nId, const OUString& aName, SfxObjectShell *pObj = 0 )
                        :   pObjShell(pObj),
                            aEventName(aName),
                            nEventId(nId)
                        {}

    sal_uInt16              GetEventId() const
                        { return nEventId; }

    OUString     GetEventName() const
                        { return aEventName; }

    SfxObjectShell*     GetObjShell() const
                        { return pObjShell; }
};

//-------------------------------------------------------------------

class SFX2_DLLPUBLIC SfxViewEventHint : public SfxEventHint
{
    ::com::sun::star::uno::Reference< css::frame::XController2 > xViewController;

public:
    TYPEINFO();

    SfxViewEventHint( sal_uInt16 nId, const OUString& aName, SfxObjectShell *pObj, const css::uno::Reference< css::frame::XController >& xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController( xController, css::uno::UNO_QUERY )
                        {}

    SfxViewEventHint( sal_uInt16 nId, const OUString& aName, SfxObjectShell *pObj, const css::uno::Reference< css::frame::XController2 >& xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController( xController )
                        {}

    css::uno::Reference< css::frame::XController2 > GetController() const
                        { return xViewController; }
};

//-------------------------------------------------------------------

class SfxNamedHint : public SfxHint
{
    OUString            _aEventName;
    SfxObjectShell*     _pObjShell;
    OUString            _aArgs;

public:
                        TYPEINFO();

                        SfxNamedHint( const OUString& rName,
                                      const OUString& rArgs,
                                      SfxObjectShell *pObj = 0  )
                        :   _aEventName( rName ),
                            _pObjShell( pObj),
                            _aArgs( rArgs )
                        {}

                        SfxNamedHint( const OUString& rName,
                                      SfxObjectShell *pObj = 0 )
                        :   _aEventName( rName ),
                            _pObjShell( pObj )
                        {}

    const OUString&     GetArgs() const { return _aArgs;}
    const OUString&     GetName() const { return _aEventName; }
    SfxObjectShell*     GetObjShell() const { return _pObjShell; }
};

class Printer;

class SfxPrintingHint : public SfxViewEventHint
{
    sal_Int32 mnPrintableState;
    com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aOpts;
public:
        TYPEINFO();

        SfxPrintingHint(
                sal_Int32 nEvent,
                const css::uno::Sequence < css::beans::PropertyValue >& rOpts,
                SfxObjectShell *pObj,
                const css::uno::Reference< css::frame::XController2 >& xController )
        : SfxViewEventHint(
            SFX_EVENT_PRINTDOC,
            GlobalEventConfig::GetEventName( STR_EVENT_PRINTDOC ),
            pObj,
            xController )
        , mnPrintableState( nEvent )
        , aOpts( rOpts )
        {}

        SfxPrintingHint( sal_Int32 nEvent )
        : SfxViewEventHint( SFX_EVENT_PRINTDOC, rtl::OUString(), 0, css::uno::Reference< css::frame::XController >() )
        , mnPrintableState( nEvent )
        {}

    sal_Int32 GetWhich() const { return mnPrintableState; }
    const css::uno::Sequence < css::beans::PropertyValue >& GetOptions() { return aOpts; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SFX2_EVENT_HXX
#define INCLUDED_SFX2_EVENT_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <svl/hint.hxx>
#include <unotools/eventcfg.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/view/PrintableState.hpp>

/**
  these values get stored in streams in a 16-bit value
*/
enum class SvMacroItemId : sal_uInt16 {
    NONE                 = 0,

    // used by SwHTMLForm_Impl
    HtmlOnSubmitForm,
    HtmlOnResetForm,
    HtmlOnGetFocus,
    HtmlOnLoseFocus,
    HtmlOnClick,
    HtmlOnClickItem,
    HtmlOnChange,
    HtmlOnSelect,

    // used by SwHTMLParser
    OpenDoc,
    PrepareCloseDoc,
    ActivateDoc,
    DeactivateDoc,

    // Events for Controls etc.
    OnMouseOver          =  5100,
    OnClick              =  5101,
    OnMouseOut           =  5102,

    OnImageLoadDone      = 10000,
    OnImageLoadCancel    = 10001,
    OnImageLoadError     = 10002,

    SwObjectSelect       = 20000,
    SwStartInsGlossary   = 20001,
    SwEndInsGlossary     = 20002,
    SwFrmKeyInputAlpha   = 20004,
    SwFrmKeyInputNoAlpha = 20005,
    SwFrmResize          = 20006,
    SwFrmMove            = 20007,
};

class SfxObjectShell;

enum class SfxEventHintId {
    NONE = 0,
    ActivateDoc,
    CloseDoc,
    CloseView,
    CreateDoc,
    DeactivateDoc,
    DocCreated,
    LoadFinished,
    ModifyChanged,
    OpenDoc,
    PrepareCloseDoc,
    PrepareCloseView,
    PrintDoc,
    SaveAsDoc,
    SaveAsDocDone,
    SaveAsDocFailed,
    SaveDoc,
    SaveDocDone,
    SaveDocFailed,
    SaveToDoc,
    SaveToDocDone,
    SaveToDocFailed,
    StorageChanged,
    ViewCreated,
    VisAreaChanged,
    // SW events
    SwMailMerge,
    SwMailMergeEnd,
    SwEventPageCount,
    SwEventFieldMerge,
    SwEventFieldMergeFinished,
    SwEventLayoutFinished,
};

class SfxEventHint : public SfxHint
{
    SfxObjectShell*     pObjShell;
    OUString            aEventName;
    SfxEventHintId      nEventId;

public:
    SfxEventHint( SfxEventHintId nId, const OUString& aName, SfxObjectShell *pObj )
                        :   pObjShell(pObj),
                            aEventName(aName),
                            nEventId(nId)
                        {}

    SfxEventHintId      GetEventId() const
                        { return nEventId; }

    const OUString&     GetEventName() const
                        { return aEventName; }

    SfxObjectShell*     GetObjShell() const
                        { return pObjShell; }
};


class SfxViewEventHint : public SfxEventHint
{
    css::uno::Reference< css::frame::XController2 > xViewController;

public:
    SfxViewEventHint( SfxEventHintId nId, const OUString& aName, SfxObjectShell *pObj, const css::uno::Reference< css::frame::XController >& xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController( xController, css::uno::UNO_QUERY )
                        {}

    SfxViewEventHint( SfxEventHintId nId, const OUString& aName, SfxObjectShell *pObj, const css::uno::Reference< css::frame::XController2 >& xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController( xController )
                        {}

    const css::uno::Reference< css::frame::XController2 >& GetController() const
                        { return xViewController; }
};

class Printer;

class SfxPrintingHint : public SfxViewEventHint
{
    css::view::PrintableState mnPrintableState;
    css::uno::Sequence < css::beans::PropertyValue > aOpts;
public:
        SfxPrintingHint(
                css::view::PrintableState nState,
                const css::uno::Sequence < css::beans::PropertyValue >& rOpts,
                SfxObjectShell *pObj,
                const css::uno::Reference< css::frame::XController2 >& xController )
        : SfxViewEventHint(
            SfxEventHintId::PrintDoc,
            GlobalEventConfig::GetEventName( GlobalEventId::PRINTDOC ),
            pObj,
            xController )
        , mnPrintableState( nState )
        , aOpts( rOpts )
        {}

        SfxPrintingHint( css::view::PrintableState nState )
        : SfxViewEventHint( SfxEventHintId::PrintDoc, rtl::OUString(), nullptr, css::uno::Reference< css::frame::XController >() )
        , mnPrintableState( nState )
        {}

    css::view::PrintableState GetWhich() const { return mnPrintableState; }
    const css::uno::Sequence < css::beans::PropertyValue >& GetOptions() const { return aOpts; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

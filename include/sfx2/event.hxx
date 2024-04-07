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

#include <ostream>

#include <unotools/weakref.hxx>
#include <sfx2/dllapi.h>
#include <svl/hint.hxx>
#include <unotools/eventcfg.hxx>
#include <rtl/ustring.hxx>
#include <sfx2/objsh.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/frame/XController2.hpp>
#include <com/sun/star/view/PrintableState.hpp>
#include <utility>

namespace com::sun::star::beans { struct PropertyValue; }

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

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const SvMacroItemId& id )
{
    switch(id)
    {
    case SvMacroItemId::NONE: return stream << "NONE";
    case SvMacroItemId::HtmlOnSubmitForm: return stream << "HtmlOnSubmitForm";
    case SvMacroItemId::HtmlOnResetForm: return stream << "HtmlOnResetForm";
    case SvMacroItemId::HtmlOnGetFocus: return stream << "HtmlOnGetFocus";
    case SvMacroItemId::HtmlOnLoseFocus: return stream << "HtmlOnLoseFocus";
    case SvMacroItemId::HtmlOnClick: return stream << "HtmlOnClick";
    case SvMacroItemId::HtmlOnClickItem: return stream << "HtmlOnClickItem";
    case SvMacroItemId::HtmlOnChange: return stream << "HtmlOnChange";
    case SvMacroItemId::HtmlOnSelect: return stream << "HtmlOnSelect";
    case SvMacroItemId::OpenDoc: return stream << "OpenDoc";
    case SvMacroItemId::PrepareCloseDoc: return stream << "PrepareCloseDoc";
    case SvMacroItemId::ActivateDoc: return stream << "ActivateDoc";
    case SvMacroItemId::DeactivateDoc: return stream << "DeactivateDoc";
    case SvMacroItemId::OnMouseOver: return stream << "OnMouseOver";
    case SvMacroItemId::OnClick: return stream << "OnClick";
    case SvMacroItemId::OnMouseOut: return stream << "OnMouseOut";
    case SvMacroItemId::OnImageLoadDone: return stream << "OnImageLoadDone";
    case SvMacroItemId::OnImageLoadCancel: return stream << "OnImageLoadCancel";
    case SvMacroItemId::OnImageLoadError: return stream << "OnImageLoadError";
    case SvMacroItemId::SwObjectSelect: return stream << "SwObjectSelect";
    case SvMacroItemId::SwStartInsGlossary: return stream << "SwStartInsGlossary";
    case SvMacroItemId::SwEndInsGlossary: return stream << "SwEndInsGlossary";
    case SvMacroItemId::SwFrmKeyInputAlpha: return stream << "SwFrmKeyInputAlpha";
    case SvMacroItemId::SwFrmKeyInputNoAlpha: return stream << "SwFrmKeyInputNoAlpha";
    case SvMacroItemId::SwFrmResize: return stream << "SwFrmResize";
    case SvMacroItemId::SwFrmMove: return stream << "SwFrmMove";
    default: return stream << "unk(" << std::to_string(int(id)) << ")";
    }
}

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

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const SfxEventHintId& id )
{
    switch(id)
    {
    case SfxEventHintId::NONE: return stream << "NONE";
    case SfxEventHintId::ActivateDoc: return stream << "ActivateDoc";
    case SfxEventHintId::CloseDoc: return stream << "CloseDoc";
    case SfxEventHintId::CloseView: return stream << "CloseView";
    case SfxEventHintId::CreateDoc: return stream << "CreateDoc";
    case SfxEventHintId::DeactivateDoc: return stream << "DeactivateDoc";
    case SfxEventHintId::DocCreated: return stream << "DocCreated";
    case SfxEventHintId::LoadFinished: return stream << "LoadFinished";
    case SfxEventHintId::ModifyChanged: return stream << "ModifyChanged";
    case SfxEventHintId::OpenDoc: return stream << "OpenDoc";
    case SfxEventHintId::PrepareCloseDoc: return stream << "PrepareCloseDoc";
    case SfxEventHintId::PrepareCloseView: return stream << "PrepareCloseView";
    case SfxEventHintId::PrintDoc: return stream << "PrintDoc";
    case SfxEventHintId::SaveAsDoc: return stream << "SaveAsDoc";
    case SfxEventHintId::SaveAsDocDone: return stream << "SaveAsDocDone";
    case SfxEventHintId::SaveAsDocFailed: return stream << "SaveAsDocFailed";
    case SfxEventHintId::SaveDoc: return stream << "SaveDoc";
    case SfxEventHintId::SaveDocDone: return stream << "SaveDocDone";
    case SfxEventHintId::SaveDocFailed: return stream << "SaveDocFailed";
    case SfxEventHintId::SaveToDoc: return stream << "SaveToDoc";
    case SfxEventHintId::SaveToDocDone: return stream << "SaveToDocDone";
    case SfxEventHintId::SaveToDocFailed: return stream << "SaveToDocFailed";
    case SfxEventHintId::StorageChanged: return stream << "StorageChanged";
    case SfxEventHintId::ViewCreated: return stream << "ViewCreated";
    case SfxEventHintId::VisAreaChanged: return stream << "VisAreaChanged";
    case SfxEventHintId::SwMailMerge: return stream << "SwMailMerge";
    case SfxEventHintId::SwMailMergeEnd: return stream << "SwMailMergeEnd";
    case SfxEventHintId::SwEventPageCount: return stream << "SwEventPageCount";
    case SfxEventHintId::SwEventFieldMerge: return stream << "SwEventFieldMerge";
    case SfxEventHintId::SwEventFieldMergeFinished: return stream << "SwEventFieldMergeFinished";
    case SfxEventHintId::SwEventLayoutFinished: return stream << "SwEventLayoutFinished";
    default: return stream << "unk(" << std::to_string(int(id)) << ")";
    }
}

class SFX2_DLLPUBLIC SfxEventHint : public SfxHint
{
    unotools::WeakReference<SfxObjectShell> pObjShell;
    OUString            aEventName;
    SfxEventHintId      nEventId;

public:
    SfxEventHint( SfxEventHintId nId, OUString aName, SfxObjectShell *pObj )
                        :   pObjShell(pObj),
                            aEventName(std::move(aName)),
                            nEventId(nId)
                        {}

    SfxEventHintId      GetEventId() const
                        { return nEventId; }

    const OUString&     GetEventName() const
                        { return aEventName; }

    rtl::Reference<SfxObjectShell> GetObjShell() const { return pObjShell.get(); }
};


class SFX2_DLLPUBLIC SfxViewEventHint : public SfxEventHint
{
    css::uno::Reference< css::frame::XController2 > xViewController;

public:
    SfxViewEventHint( SfxEventHintId nId, const OUString& aName, SfxObjectShell *pObj, const css::uno::Reference< css::frame::XController >& xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController( xController, css::uno::UNO_QUERY )
                        {}

    SfxViewEventHint( SfxEventHintId nId, const OUString& aName, SfxObjectShell *pObj, css::uno::Reference< css::frame::XController2 > xController )
                        : SfxEventHint( nId, aName, pObj )
                        , xViewController(std::move( xController ))
                        {}

    const css::uno::Reference< css::frame::XController2 >& GetController() const
                        { return xViewController; }
};

class SfxPrintingHint final : public SfxViewEventHint
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
        : SfxViewEventHint(
            SfxEventHintId::PrintDoc,
            GlobalEventConfig::GetEventName( GlobalEventId::PRINTDOC ),
            nullptr,
            css::uno::Reference< css::frame::XController >() )
        , mnPrintableState( nState )
        {}

    css::view::PrintableState GetWhich() const { return mnPrintableState; }
    const css::uno::Sequence < css::beans::PropertyValue >& GetOptions() const { return aOpts; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

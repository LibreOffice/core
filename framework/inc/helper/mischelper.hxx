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

#pragma once

#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <i18nlangtag/lang.h>
#include <svl/languageoptions.hxx>
#include <rtl/ustring.hxx>

#include <functional>
#include <set>

namespace framework
{

// menu ids for language status bar control
enum LangMenuIDs
{
    MID_LANG_SEL_1 = 1,     // need to start with 1 since xPopupMenu->execute will return 0 if the menu is cancelled
    MID_LANG_SEL_2,
    MID_LANG_SEL_3,
    MID_LANG_SEL_4,
    MID_LANG_SEL_5,
    MID_LANG_SEL_6,
    MID_LANG_SEL_7,
    MID_LANG_SEL_8,
    MID_LANG_SEL_9,
    MID_LANG_SEL_NONE,
    MID_LANG_SEL_RESET,
    MID_LANG_SEL_MORE,
    MID_LANG_DEF_NONE,
    MID_LANG_DEF_RESET,
    MID_LANG_DEF_MORE,

    MID_LANG_PARA_SEPARATOR,
    MID_LANG_PARA_STRING,

    MID_LANG_PARA_1,
    MID_LANG_PARA_2,
    MID_LANG_PARA_3,
    MID_LANG_PARA_4,
    MID_LANG_PARA_5,
    MID_LANG_PARA_6,
    MID_LANG_PARA_7,
    MID_LANG_PARA_8,
    MID_LANG_PARA_9,
    MID_LANG_PARA_NONE,
    MID_LANG_PARA_RESET,
    MID_LANG_PARA_MORE,
};

inline bool IsScriptTypeMatchingToLanguage( SvtScriptType nScriptType, LanguageType nLang )
{
    return bool(nScriptType & SvtLanguageOptions::GetScriptTypeOfLanguage( nLang ));
}

inline void RetrieveTypeNameFromResourceURL( const OUString& aResourceURL, OUString& aType, OUString& aName )
{
    static const char      RESOURCEURL_PREFIX[] = "private:resource/";
    static const sal_Int32 RESOURCEURL_PREFIX_SIZE = strlen(RESOURCEURL_PREFIX);

    if (aResourceURL.startsWith( RESOURCEURL_PREFIX ))
    {
        sal_Int32 nIdx{ RESOURCEURL_PREFIX_SIZE };
        while (nIdx<aResourceURL.getLength() && aResourceURL[nIdx]=='/') ++nIdx;
        if (nIdx>=aResourceURL.getLength())
            return;
        aType = aResourceURL.getToken(0, '/', nIdx);
        if (nIdx<0)
            return;
        while (nIdx<aResourceURL.getLength() && aResourceURL[nIdx]=='/') ++nIdx;
        if (nIdx>=aResourceURL.getLength())
            return;
        aName = aResourceURL.getToken(0, '/', nIdx);
    }
}

class LanguageGuessingHelper
{
    mutable css::uno::Reference< css::linguistic2::XLanguageGuessing >    m_xLanguageGuesser;
    css::uno::Reference< css::uno::XComponentContext >                    m_xContext;

public:
    LanguageGuessingHelper(const css::uno::Reference< css::uno::XComponentContext >& _xContext) : m_xContext(_xContext){}

    css::uno::Reference< css::linguistic2::XLanguageGuessing > const &  GetGuesser() const;
};

void FillLangItems( std::set< OUString > &rLangItems,
        const css::uno::Reference< css::frame::XFrame > &rxFrame,
        const LanguageGuessingHelper & rLangGuessHelper,
        SvtScriptType    nScriptType,
        const OUString & rCurLang,
        const OUString & rKeyboardLang,
        const OUString & rGuessedTextLang );

//It's common for an object to want to create and own a Broadcaster and set
//itself as a Listener on its own Broadcaster member.

//However, calling addListener on a Broadcaster means that the Broadcaster adds
//a reference to the Listener leading to an ownership cycle where the Listener
//owns the Broadcaster which "owns" the Listener.

//The WeakContainerListener allows breaking this cycle and retrofitting
//afflicted implementations fairly easily.

//OriginalListener owns the Broadcaster which "owns" the WeakContainerListener
//which forwards the events to the OriginalListener without taking ownership of
//it.
class WeakContainerListener final : public ::cppu::WeakImplHelper<css::container::XContainerListener>
{
    private:
        css::uno::WeakReference<css::container::XContainerListener> mxOwner;

    public:
        WeakContainerListener(css::uno::Reference<css::container::XContainerListener> const & xOwner)
            : mxOwner(xOwner)
        {
        }

        // container.XContainerListener
        virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& rEvent) override
        {
            css::uno::Reference<css::container::XContainerListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->elementInserted(rEvent);
        }

        virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& rEvent) override
        {
            css::uno::Reference<css::container::XContainerListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->elementRemoved(rEvent);
        }

        virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& rEvent) override
        {
            css::uno::Reference<css::container::XContainerListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->elementReplaced(rEvent);
        }

        // lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override
        {
            css::uno::Reference<css::container::XContainerListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->disposing(rEvent);

        }
};

class WeakChangesListener final : public ::cppu::WeakImplHelper<css::util::XChangesListener>
{
    private:
        css::uno::WeakReference<css::util::XChangesListener> mxOwner;

    public:
        WeakChangesListener(css::uno::Reference<css::util::XChangesListener> const & xOwner)
            : mxOwner(xOwner)
        {
        }

        // util.XChangesListener
        virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& rEvent) override
        {
            css::uno::Reference<css::util::XChangesListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->changesOccurred(rEvent);
        }

        // lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override
        {
            css::uno::Reference<css::util::XChangesListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->disposing(rEvent);

        }
};

class WeakDocumentEventListener final : public ::cppu::WeakImplHelper<css::document::XDocumentEventListener>
{
    private:
        css::uno::WeakReference<css::document::XDocumentEventListener> mxOwner;

    public:
        WeakDocumentEventListener(css::uno::Reference<css::document::XDocumentEventListener> const & xOwner)
            : mxOwner(xOwner)
        {
        }

        virtual void SAL_CALL documentEventOccured(const css::document::DocumentEvent& rEvent) override
        {
            css::uno::Reference<css::document::XDocumentEventListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->documentEventOccured(rEvent);

        }

        // lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override
        {
            css::uno::Reference<css::document::XDocumentEventListener> xOwner(mxOwner.get(),
                css::uno::UNO_QUERY);
            if (xOwner.is())
                xOwner->disposing(rEvent);

        }
};

css::uno::Reference<css::ui::XContextChangeEventListener>
GetFirstListenerWith_Impl(
    css::uno::Reference<css::uno::XComponentContext> const & xComponentContext,
    css::uno::Reference<css::uno::XInterface> const& xEventFocus,
    std::function<bool (css::uno::Reference<css::ui::XContextChangeEventListener> const&)> const& rPredicate);

extern auto (*g_pGetMultiplexerListener)(
    css::uno::Reference<css::uno::XComponentContext> const & xComponentContext,
    css::uno::Reference<css::uno::XInterface> const&,
    std::function<bool (css::uno::Reference<css::ui::XContextChangeEventListener> const&)> const&)
    -> css::uno::Reference<css::ui::XContextChangeEventListener>;

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

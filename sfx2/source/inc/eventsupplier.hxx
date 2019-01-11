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

#ifndef INCLUDED_SFX2_SOURCE_INC_EVENTSUPPLIER_HXX
#define INCLUDED_SFX2_SOURCE_INC_EVENTSUPPLIER_HXX

#include <sal/types.h>

#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <cppuhelper/implbase.hxx>
#include <sfx2/sfxuno.hxx>

#include <cppuhelper/interfacecontainer.hxx>
#include <svl/lstner.hxx>
#include <unotools/eventcfg.hxx>

namespace comphelper
{
    class NamedValueCollection;
}

class SfxObjectShell;
class SvxMacro;


class SfxEvents_Impl : public ::cppu::WeakImplHelper< css::container::XNameReplace, css::document::XEventListener  >
{
    css::uno::Sequence< OUString >     maEventNames;
    css::uno::Sequence< css::uno::Any >                 maEventData;
    css::uno::Reference< css::document::XEventBroadcaster >  mxBroadcaster;
    ::osl::Mutex                    maMutex;
    SfxObjectShell                 *mpObjShell;

public:
                                SfxEvents_Impl( SfxObjectShell* pShell,
                                                css::uno::Reference< css::document::XEventBroadcaster > const & xBroadcaster );
                               virtual ~SfxEvents_Impl() override;

    //  --- XNameReplace ---
    virtual void SAL_CALL       replaceByName( const OUString & aName, const css::uno::Any & aElement ) override;

    //  --- XNameAccess ( parent of XNameReplace ) ---
    virtual css::uno::Any SAL_CALL        getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL   hasByName( const OUString& aName ) override;

    //  --- XElementAccess ( parent of XNameAccess ) ---
    virtual css::uno::Type SAL_CALL    getElementType() override;
    virtual sal_Bool SAL_CALL   hasElements() override;

    // --- ::document::XEventListener ---
    virtual void SAL_CALL       notifyEvent( const css::document::EventObject& aEvent ) override;

    // --- ::lang::XEventListener ---
    virtual void SAL_CALL       disposing( const css::lang::EventObject& Source ) override;

    // convert and normalize
    static std::unique_ptr<SvxMacro>  ConvertToMacro( const css::uno::Any& rElement, SfxObjectShell* pDoc );
    static void                 NormalizeMacro( const css::uno::Any& rIn, css::uno::Any& rOut, SfxObjectShell* pDoc );
    static void                 NormalizeMacro(
                                    const ::comphelper::NamedValueCollection& i_eventDescriptor,
                                    ::comphelper::NamedValueCollection& o_normalizedDescriptor,
                                    SfxObjectShell* i_document );
    static void Execute( css::uno::Any const & aEventData, const css::document::DocumentEvent& aTrigger, SfxObjectShell* pDoc );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

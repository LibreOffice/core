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

#ifndef INCLUDED_UNOTOOLS_INTLWRAPPER_HXX
#define INCLUDED_UNOTOOLS_INTLWRAPPER_HXX

#include <unotools/unotoolsdllapi.h>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/uno/Reference.h>

#include <i18nlangtag/languagetag.hxx>
#include <memory>
#include <optional>

namespace com::sun::star::uno { class XComponentContext; }

class LocaleDataWrapper;

/**
    A wrapper of I18N wrappers. Using this is more expensive than using some
    single wrapper classes so use it only if you must pass a single pointer
    without knowing in advance what is needed, e.g. for
    SfxPoolItem::GetPresentation(). Remember that this wrapper was only created
    for convenience to bypass some oddities, if possible don't use it. <p>

    Implemented are only the const get...() methods of the wrappers, which are
    loaded on demand, for consistency reasons no change of locale is possible.
    Only default calendar and default collator are supported. <p>

    One exception though is the calendar wrapper: to be able to set a value and
    retrieve calendar values it is not const, so methods using this should
    reset the calendar to the previous value if it isn't sure where the
    IntlWrapper did come from. <p>
 */
class UNOTOOLS_DLLPUBLIC IntlWrapper
{
private:
    LanguageTag         maLanguageTag;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    const LocaleDataWrapper*  pLocaleData { nullptr };
    std::optional<CollatorWrapper>    moCollator;
    std::optional<CollatorWrapper>    moCaseCollator;

    void                ImplNewLocaleData() const;
    void                ImplNewCollator( bool bCaseSensitive ) const;

public:
    IntlWrapper(LanguageTag aLanguageTag);
    ~IntlWrapper();

    const LocaleDataWrapper*    getLocaleData() const
                                    {
                                        if ( !pLocaleData )
                                            ImplNewLocaleData();
                                        return pLocaleData;
                                    }
    /// case insensitive collator, simple IGNORE_CASE
    const CollatorWrapper*      getCollator() const
                                    {
                                        if ( !moCollator )
                                            ImplNewCollator( false );
                                        return &*moCollator;
                                    }
    /// case sensitive collator
    const CollatorWrapper*      getCaseCollator() const
                                    {
                                        if ( !moCaseCollator )
                                            ImplNewCollator( true );
                                        return &*moCaseCollator;
                                    }
};

#endif // INCLUDED_UNOTOOLS_INTLWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

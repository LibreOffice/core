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
#ifndef INCLUDED_SVX_NUMVSET_HXX
#define INCLUDED_SVX_NUMVSET_HXX

#include <vcl/idle.hxx>
#include <svtools/valueset.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include <svx/svxdllapi.h>

namespace com::sun::star {
    namespace container{
        class XIndexAccess;
    }
    namespace beans{
        struct PropertyValue;
    }
    namespace text{
        class XNumberingFormatter;
    }
}

enum class NumberingPageType
{
    DOCBULLET,
    BULLET,
    SINGLENUM,
    OUTLINE,
    BITMAP
};

class SVX_DLLPUBLIC SvxNumValueSet : public ValueSet
{
    NumberingPageType ePageType;
    tools::Rectangle       aOrgRect;
    VclPtr<VirtualDevice> pVDev;

    css::uno::Reference<css::text::XNumberingFormatter> xFormatter;
    css::lang::Locale aLocale;

    // Pair of bullet chars (first), and their respective font (second)
    std::vector<std::pair<OUString, OUString>> maCustomBullets;

    css::uno::Sequence<
        css::uno::Sequence<
            css::beans::PropertyValue> > aNumSettings;

    css::uno::Sequence<
        css::uno::Reference<
            css::container::XIndexAccess> > aOutlineSettings;

public:
    SvxNumValueSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow);
    void init(NumberingPageType eType);
    virtual ~SvxNumValueSet() override;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;

    void            SetNumberingSettings(
        const css::uno::Sequence<
                  css::uno::Sequence<css::beans::PropertyValue> >& aNum,
        css::uno::Reference<css::text::XNumberingFormatter> const & xFormatter,
        const css::lang::Locale& rLocale );

    void            SetOutlineNumberingSettings(
            css::uno::Sequence<
                css::uno::Reference<css::container::XIndexAccess> > const & rOutline,
            css::uno::Reference<css::text::XNumberingFormatter> const & xFormatter,
            const css::lang::Locale& rLocale);

    std::vector<std::pair<OUString, OUString>> GetCustomBullets() { return maCustomBullets; }
    void SetCustomBullets(std::vector<std::pair<OUString, OUString>> aCustomBullets);

    virtual FactoryFunction GetUITestFactory() const override;

};


class SVX_DLLPUBLIC SvxBmpNumValueSet final : public SvxNumValueSet
{
    Idle        aFormatIdle;
    bool        bGrfNotFound;

    DECL_DLLPRIVATE_LINK(FormatHdl_Impl, Timer *, void);

public:
    SvxBmpNumValueSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow);
    void init();
    virtual ~SvxBmpNumValueSet() override;

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

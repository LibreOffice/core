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

#include <svtools/valueset.hxx>
#include <limits.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/Locale.hpp>
#include <svx/svxdllapi.h>

namespace com{namespace sun{ namespace star{
    namespace container{
        class XIndexAccess;
    }
    namespace beans{
        struct PropertyValue;
    }
    namespace text{
        class XNumberingFormatter;
    }
}}}

class SvxBrushItem;
class SvxNumRule;
struct SvxBmpItemInfo
{
    SvxBrushItem*   pBrushItem;
    sal_uInt16          nItemId;
};

class SVX_DLLPUBLIC SvxNumValueSet : public ValueSet
{
    Color           aLineColor;
    sal_uInt16      nPageType;
    bool            bHTMLMode;
    Rectangle       aOrgRect;
    VirtualDevice*  pVDev;

    com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter> xFormatter;
    com::sun::star::lang::Locale aLocale;

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Sequence<
            com::sun::star::beans::PropertyValue> > aNumSettings;

    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference<
            com::sun::star::container::XIndexAccess> > aOutlineSettings;

    public:
    SvxNumValueSet(vcl::Window* pParent, WinBits nWinBits = WB_TABSTOP);
    void init(sal_uInt16 nType);
    virtual ~SvxNumValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;

    void            SetHTMLMode(bool bSet) {bHTMLMode = bSet;}
    void            SetNumberingSettings(
        const com::sun::star::uno::Sequence<
            com::sun::star::uno::Sequence<
                com::sun::star::beans::PropertyValue> >& aNum,
        com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter>& xFormatter,
        const com::sun::star::lang::Locale& rLocale );

    void            SetOutlineNumberingSettings(
            com::sun::star::uno::Sequence<
                com::sun::star::uno::Reference<
                    com::sun::star::container::XIndexAccess> >& rOutline,
            com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter>& xFormatter,
            const com::sun::star::lang::Locale& rLocale);
};

class SVX_DLLPUBLIC SvxBmpNumValueSet : public SvxNumValueSet
{
    OUString    sBullets;
    Timer       aFormatTimer;
    bool        bGrfNotFound;

    void init();

    protected:
        DECL_LINK(FormatHdl_Impl, void *);

    void            SetGrfNotFound(bool bSet) {bGrfNotFound = bSet;}
    bool            IsGrfNotFound()const {return bGrfNotFound;}

    Timer&          GetFormatTimer() {return aFormatTimer;}

    public:
    SvxBmpNumValueSet(vcl::Window* pParent, WinBits nWinBits = WB_TABSTOP);
    virtual ~SvxBmpNumValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef _SVX_HTMLCFG_HXX
#define _SVX_HTMLCFG_HXX

// -----------------------------------------------------------------------

#include <tools/solar.h>
#include <unotools/configitem.hxx>
#include <svtools/svtdllapi.h>

#define HTML_FONT_COUNT 7

// !!!be aware!!!: the following defines are _not_ used as values in the configuration file
//                  this is because of compatibility reasons
#define HTML_CFG_MSIE       1   // Internet Explorer
#define HTML_CFG_WRITER     2   // Writer
#define HTML_CFG_NS40       3   // Netscape 4.0

#define HTML_CFG_MAX HTML_CFG_NS40

class Link;
struct HtmlOptions_Impl;
// -----------------------------------------------------------------------

class SVT_DLLPUBLIC SvxHtmlOptions : public utl::ConfigItem
{
    HtmlOptions_Impl*pImp;
    const com::sun::star::uno::Sequence<OUString>& GetPropertyNames();
    void Load( const com::sun::star::uno::Sequence< OUString >& rPropertyNames );
    void CallListeners();

public:
    SvxHtmlOptions();
    ~SvxHtmlOptions();

    virtual void    Commit();
    virtual void Notify( const com::sun::star::uno::Sequence< OUString >& _rPropertyNames);

    sal_uInt16      GetFontSize(sal_uInt16 nPos) const;
    void        SetFontSize(sal_uInt16 nPos, sal_uInt16 nSize);

    sal_Bool        IsImportUnknown() const;
    void        SetImportUnknown(sal_Bool bSet);

    sal_uInt16      GetExportMode() const;
    void        SetExportMode(sal_uInt16 nSet);

    sal_Bool        IsStarBasic() const;
    void        SetStarBasic(sal_Bool bSet);

    sal_Bool        IsStarBasicWarning() const;
    void        SetStarBasicWarning(sal_Bool bSet);

    sal_Bool        IsSaveGraphicsLocal() const;
    void        SetSaveGraphicsLocal(sal_Bool bSet);

    sal_Bool        IsPrintLayoutExtension() const;
    void        SetPrintLayoutExtension(sal_Bool bSet);

    sal_Bool        IsIgnoreFontFamily() const;
    void        SetIgnoreFontFamily(sal_Bool bSet);

    sal_Bool    IsDefaultTextEncoding() const;
    rtl_TextEncoding GetTextEncoding() const;
    void        SetTextEncoding( rtl_TextEncoding );
    static SvxHtmlOptions& Get();

    sal_Bool        IsNumbersEnglishUS() const;
    void        SetNumbersEnglishUS(sal_Bool bSet);
};

#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SVTOOLS_HTMLCFG_HXX
#define INCLUDED_SVTOOLS_HTMLCFG_HXX

#include <unotools/configitem.hxx>
#include <svtools/svtdllapi.h>

#define HTML_FONT_COUNT 7

// !!!be aware!!!: the following defines are _not_ used as values in the configuration file
//                  this is because of compatibility reasons
#define HTML_CFG_MSIE       1   // Internet Explorer
#define HTML_CFG_WRITER     2   // Writer
#define HTML_CFG_NS40       3   // Netscape 4.0

#define HTML_CFG_MAX HTML_CFG_NS40

struct HtmlOptions_Impl;

class SVT_DLLPUBLIC SvxHtmlOptions : public utl::ConfigItem
{
    HtmlOptions_Impl*pImp;
    static const css::uno::Sequence<OUString>& GetPropertyNames();
    void Load( const css::uno::Sequence< OUString >& rPropertyNames );

    virtual void    ImplCommit() override;

public:
    SvxHtmlOptions();
    virtual ~SvxHtmlOptions();

    virtual void Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;

    sal_uInt16      GetFontSize(sal_uInt16 nPos) const;
    void        SetFontSize(sal_uInt16 nPos, sal_uInt16 nSize);

    bool        IsImportUnknown() const;
    void        SetImportUnknown(bool bSet);

    sal_uInt16  GetExportMode() const;

    bool        IsStarBasic() const;
    void        SetStarBasic(bool bSet);

    bool        IsStarBasicWarning() const;
    void        SetStarBasicWarning(bool bSet);

    bool        IsSaveGraphicsLocal() const;
    void        SetSaveGraphicsLocal(bool bSet);

    bool        IsPrintLayoutExtension() const;
    void        SetPrintLayoutExtension(bool bSet);

    bool        IsIgnoreFontFamily() const;
    void        SetIgnoreFontFamily(bool bSet);

    bool        IsDefaultTextEncoding() const;
    rtl_TextEncoding GetTextEncoding() const;
    void        SetTextEncoding( rtl_TextEncoding );
    static SvxHtmlOptions& Get();

    bool        IsNumbersEnglishUS() const;
    void        SetNumbersEnglishUS(bool bSet);
};

#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

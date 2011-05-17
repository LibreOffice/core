/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_HTMLCFG_HXX
#define _SVX_HTMLCFG_HXX

// -----------------------------------------------------------------------

#include <tools/solar.h>
#include <unotools/configitem.hxx>
#include <svtools/svtdllapi.h>

#define HTML_FONT_COUNT 7

// !!!be aware!!!: the following defines are _not_ used as values in the configuration file
//                  this is because of compatibility reasons
#define HTML_CFG_HTML32     0   // Html 3.2
#define HTML_CFG_MSIE_40    1   // Internet Explorer 4.0
#define HTML_CFG_MSIE       HTML_CFG_MSIE_40
#define HTML_CFG_WRITER     2   // Writer
#define HTML_CFG_NS40       3   // Netscape 4.0
#define HTML_CFG_MSIE_40_OLD 4   // Internet Explorer 4.0 - alter Wert

#define HTML_CFG_MAX HTML_CFG_NS40

class Link;
struct HtmlOptions_Impl;
// -----------------------------------------------------------------------

class SVT_DLLPUBLIC SvxHtmlOptions : public utl::ConfigItem
{
    HtmlOptions_Impl*pImp;
    const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();
    void Load( const com::sun::star::uno::Sequence< rtl::OUString >& rPropertyNames );
    void CallListeners();

public:
                    SvxHtmlOptions();
                    ~SvxHtmlOptions();

    virtual void    Commit();
    virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames);

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
    static SvxHtmlOptions* Get();

    sal_Bool        IsNumbersEnglishUS() const;
    void        SetNumbersEnglishUS(sal_Bool bSet);

    void AddListenerLink( const Link& rLink );
    void RemoveListenerLink( const Link& rLink );
};

#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

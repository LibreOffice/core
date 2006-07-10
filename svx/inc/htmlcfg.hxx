/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlcfg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 11:55:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_HTMLCFG_HXX
#define _SVX_HTMLCFG_HXX

// -----------------------------------------------------------------------

#include <tools/solar.h>

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

struct HtmlOptions_Impl;
// -----------------------------------------------------------------------

class SVX_DLLPUBLIC SvxHtmlOptions : public utl::ConfigItem
{
    HtmlOptions_Impl*pImp;
    const com::sun::star::uno::Sequence<rtl::OUString>& GetPropertyNames();

public:
                    SvxHtmlOptions();
                    ~SvxHtmlOptions();

    virtual void            Commit();

    USHORT      GetFontSize(USHORT nPos) const;
    void        SetFontSize(USHORT nPos, USHORT nSize);

    BOOL        IsImportUnknown() const;
    void        SetImportUnknown(BOOL bSet);

    USHORT      GetExportMode() const;
    void        SetExportMode(USHORT nSet);

    BOOL        IsStarBasic() const;
    void        SetStarBasic(BOOL bSet);

    BOOL        IsStarBasicWarning() const;
    void        SetStarBasicWarning(BOOL bSet);

    BOOL        IsSaveGraphicsLocal() const;
    void        SetSaveGraphicsLocal(BOOL bSet);

    BOOL        IsPrintLayoutExtension() const;
    void        SetPrintLayoutExtension(BOOL bSet);

    BOOL        IsIgnoreFontFamily() const;
    void        SetIgnoreFontFamily(BOOL bSet);

    sal_Bool    IsDefaultTextEncoding() const;
    rtl_TextEncoding GetTextEncoding() const;
    void        SetTextEncoding( rtl_TextEncoding );
    static SvxHtmlOptions* Get();

    BOOL        IsNumbersEnglishUS() const;
    void        SetNumbersEnglishUS(BOOL bSet);
};

#endif





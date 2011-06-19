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
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#define INCLUDED_SVTOOLS_COLORCFG_HXX

#include "svtools/svtdllapi.h"
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/brdcst.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>

//-----------------------------------------------------------------------------
namespace svtools{
enum ColorConfigEntry
{
    DOCCOLOR            ,
    DOCBOUNDARIES       ,
    APPBACKGROUND       ,
    OBJECTBOUNDARIES    ,
    TABLEBOUNDARIES     ,
    FONTCOLOR           ,
    LINKS               ,
    LINKSVISITED        ,
    ANCHOR              ,
    SPELL         ,
    SMARTTAGS     ,
    SHADOWCOLOR         ,
    WRITERTEXTGRID      ,
    WRITERFIELDSHADINGS ,
    WRITERIDXSHADINGS         ,
    WRITERDIRECTCURSOR        ,
    WRITERSCRIPTINDICATOR,
    WRITERSECTIONBOUNDARIES,
    WRITERPAGEBREAKS,
    HTMLSGML            ,
    HTMLCOMMENT         ,
    HTMLKEYWORD         ,
    HTMLUNKNOWN         ,
    CALCGRID            ,
    CALCPAGEBREAK       ,
    CALCPAGEBREAKMANUAL,
    CALCPAGEBREAKAUTOMATIC,
    CALCDETECTIVE       ,
    CALCDETECTIVEERROR       ,
    CALCREFERENCE       ,
    CALCNOTESBACKGROUND     ,
    DRAWGRID            ,
    DRAWDRAWING         ,
    DRAWFILL            ,
    BASICIDENTIFIER,
    BASICCOMMENT   ,
    BASICNUMBER    ,
    BASICSTRING    ,
    BASICOPERATOR  ,
    BASICKEYWORD   ,
    BASICERROR   ,
    SQLIDENTIFIER,
    SQLNUMBER,
    SQLSTRING,
    SQLOPERATOR,
    SQLKEYWORD,
    SQLPARAMETER,
    SQLCOMMENT,
    ColorConfigEntryCount
};

class ColorConfig_Impl;
struct ColorConfigValue
{

    sal_Bool    bIsVisible; //validity depends on the element type
    sal_Int32   nColor;
    ColorConfigValue() : bIsVisible(sal_False), nColor(0) {}
    sal_Bool operator !=(const ColorConfigValue& rCmp) const
        { return nColor != rCmp.nColor || bIsVisible != rCmp.bIsVisible;}
};

class SVT_DLLPUBLIC ColorConfig:
    public utl::detail::Options
{
    friend class ColorConfig_Impl;
private:
    static ColorConfig_Impl* m_pImpl;
public:
    ColorConfig();
    virtual ~ColorConfig();

    // get the configured value - if bSmart is set the default color setting is provided
    // instead of the automatic color
    ColorConfigValue        GetColorValue(ColorConfigEntry eEntry, sal_Bool bSmart = sal_True)const;
    static Color            GetDefaultColor(ColorConfigEntry eEntry);
};

class SVT_DLLPUBLIC EditableColorConfig
{
    ColorConfig_Impl*   m_pImpl;
    sal_Bool            m_bModified;
public:
    EditableColorConfig();
    ~EditableColorConfig();

    ::com::sun::star::uno::Sequence< ::rtl::OUString >  GetSchemeNames() const;
    void                                                DeleteScheme(const ::rtl::OUString& rScheme );
    void                                                AddScheme(const ::rtl::OUString& rScheme );
    sal_Bool                                            LoadScheme(const ::rtl::OUString& rScheme );
    const ::rtl::OUString&                              GetCurrentSchemeName()const;
    void                        SetCurrentSchemeName(const ::rtl::OUString& rScheme);

    const ColorConfigValue&     GetColorValue(ColorConfigEntry eEntry)const;
    void                        SetColorValue(ColorConfigEntry eEntry, const ColorConfigValue& rValue);
    void                        SetModified();
    void                        ClearModified(){m_bModified = sal_False;}
    sal_Bool                    IsModified()const{return m_bModified;}
    void                        Commit();

    void                        DisableBroadcast();
    void                        EnableBroadcast();
};
}//namespace svtools
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

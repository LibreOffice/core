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

#ifndef INCLUDED_SVTDLLAPI_H
#include "bf_svtools/svtdllapi.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _SFXBRDCST_HXX
#include <bf_svtools/brdcst.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <bf_svtools/lstner.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <bf_svtools/options.hxx>
#endif


//-----------------------------------------------------------------------------
namespace binfilter {
enum ColorConfigEntry
{
    DOCCOLOR            ,
    DOCBOUNDARIES       ,
    APPBACKGROUND       ,
    OBJECTBOUNDARIES    ,
    TABLEBOUNDARIES     ,
    FONTCOLOR			,
    LINKS               ,
    LINKSVISITED        ,
    ANCHOR              ,
    SPELL         ,
    SMARTTAGS     ,
    WRITERTEXTGRID      ,
    WRITERFIELDSHADINGS ,
    WRITERIDXSHADINGS         ,
    WRITERDIRECTCURSOR        ,
    WRITERNOTESINDICATOR,
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
    ColorConfigEntryCount
};
/* -----------------------------22.03.2002 15:36------------------------------

 ---------------------------------------------------------------------------*/
class ColorConfig_Impl;
struct ColorConfigValue
{

    sal_Bool    bIsVisible; //validity depends on the element type
    sal_Int32   nColor;
    ColorConfigValue() : bIsVisible(sal_False), nColor(0) {}
    sal_Bool operator !=(const ColorConfigValue& rCmp) const
        { return nColor != rCmp.nColor || bIsVisible != rCmp.bIsVisible;}
};
/* -----------------------------22.03.2002 15:36------------------------------

 ---------------------------------------------------------------------------*/
class  ColorConfig:
    public Options, public SfxBroadcaster, public SfxListener
{
    friend class ColorConfig_Impl;
private:
    static ColorConfig_Impl* m_pImpl;
public:
    ColorConfig();
    virtual ~ColorConfig();

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    // get the configured value - if bSmart is set the default color setting is provided
    // instead of the automatic color
    ColorConfigValue        GetColorValue(ColorConfigEntry eEntry, sal_Bool bSmart = sal_True)const;

    static Color            GetDefaultColor(ColorConfigEntry eEntry);
};
}
#endif


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

#include <bf_svtools/colorcfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configpathes.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <bf_svtools/poolitem.hxx> //Any2Bool
#include <bf_svtools/smplhint.hxx>
#include <osl/mutex.hxx>
#include <itemholder2.hxx>

#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <rtl/instance.hxx>

//-----------------------------------------------------------------------------
using namespace utl;
using namespace rtl;
using namespace com::sun::star;

namespace binfilter
{

#define C2U(cChar) OUString::createFromAscii(cChar)
static const sal_Char cColor[] = "/Color";
static const sal_Char cColorSchemes[] = "ColorSchemes/";
sal_Int32            nColorRefCount_Impl = 0;
namespace 
{
    struct ColorMutex_Impl 
        : public rtl::Static< ::osl::Mutex, ColorMutex_Impl > {}; 
}

ColorConfig_Impl*    ColorConfig::m_pImpl = NULL;

/* -----------------------------16.01.01 15:36--------------------------------
 ---------------------------------------------------------------------------*/
class ColorConfig_Impl : public utl::ConfigItem, public SfxBroadcaster
{
    ColorConfigValue    m_aConfigValues[ColorConfigEntryCount];
    sal_Bool            m_bEditMode;
    rtl::OUString       m_sIsVisible;
    rtl::OUString       m_sLoadedScheme;
    sal_Bool			m_bIsBroadcastEnabled;
    static sal_Bool     m_bLockBroadcast;
    static sal_Bool     m_bBroadcastWhenUnlocked;

    uno::Sequence< ::rtl::OUString> GetPropertyNames(const rtl::OUString& rScheme);
public:
    ColorConfig_Impl(sal_Bool bEditMode = sal_False);
    virtual ~ColorConfig_Impl();

    void                            Load(const rtl::OUString& rScheme);
    void                            CommitCurrentSchemeName();
    //changes the name of the current scheme but doesn't load it!
    void                            SetCurrentSchemeName(const rtl::OUString& rSchemeName) {m_sLoadedScheme = rSchemeName;}
    virtual void                    Commit();
    virtual void                    Notify( const uno::Sequence<rtl::OUString>& aPropertyNames);

    const ColorConfigValue&         GetColorConfigValue(ColorConfigEntry eValue)
                                                            {return m_aConfigValues[eValue];}

    const rtl::OUString&            GetLoadedScheme() const {return m_sLoadedScheme;}

    void                            SetModified(){ConfigItem::SetModified();}
    void                            ClearModified(){ConfigItem::ClearModified();}
    void                            SettingsChanged();

    // #100822#
    DECL_LINK( DataChangedEventListener, VclWindowEvent* );

    void ImplUpdateApplicationSettings();
};

/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString> ColorConfig_Impl::GetPropertyNames(const rtl::OUString& rScheme)
{
    uno::Sequence<OUString> aNames(2 * ColorConfigEntryCount);
    OUString* pNames = aNames.getArray();
    struct ColorConfigEntryData_Impl
    {
        const sal_Char* cName;
        sal_Int32       nLength;
        rtl_TextEncoding eEncoding;
        sal_Bool bCanBeVisible;
    };
    static const ColorConfigEntryData_Impl cNames[] =
    {
        { RTL_CONSTASCII_USTRINGPARAM("/DocColor")        ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/DocBoundaries")   ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/AppBackground")   ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/ObjectBoundaries"),sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/TableBoundaries") ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/FontColor")		,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/Links")           ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/LinksVisited")    ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/Anchor")          ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/Spell")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SmartTags")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterTextGrid")  ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterFieldShadings"),sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterIdxShadings")     ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterDirectCursor")    ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterNotesIndicator")    ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterScriptIndicator")    ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterSectionBoundaries")    ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterPageBreaks")    ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLSGML")        ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLComment")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLKeyword")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLUnknown")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcGrid")        ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreak"), sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreakManual"), sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreakAutomatic"), sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcDetective")   ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcDetectiveError")   ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcReference")   ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcNotesBackground") ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/DrawGrid")        ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/DrawDrawing")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/DrawFill")        ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICIdentifier"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICComment")   ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICNumber")    ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICString")    ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICOperator")  ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICKeyword")   ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICError"),  sal_False }
    };
    int nIndex = 0;
    OUString sColor = C2U(cColor);
    OUString sBase(C2U(cColorSchemes));
    sBase += utl::wrapConfigurationElementName(rScheme);
    const int nCount = ColorConfigEntryCount;
    for(sal_Int32 i = 0; i < 4 * nCount; i+= 4)
    {
        rtl::OUString sBaseName(sBase);
        sal_Int32 nPos = i / 4;
        sBaseName += OUString(cNames[nPos].cName, cNames[nPos].nLength, cNames[nPos].eEncoding);
        pNames[nIndex] += sBaseName;
        pNames[nIndex++] += sColor;
        if(cNames[nPos].bCanBeVisible)
        {
            pNames[nIndex] += sBaseName;
            pNames[nIndex++] += m_sIsVisible;
        }
    }
    aNames.realloc(nIndex);
    return aNames;
}
/* -----------------------------22.03.2002 14:37------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ColorConfig_Impl::m_bLockBroadcast = sal_False;
sal_Bool ColorConfig_Impl::m_bBroadcastWhenUnlocked = sal_False;
ColorConfig_Impl::ColorConfig_Impl(sal_Bool bEditMode) :
    ConfigItem(C2U("Office.UI/ColorScheme")),
    m_bEditMode(bEditMode),
    m_sIsVisible(C2U("/IsVisible")),
    m_bIsBroadcastEnabled(sal_True)
{
    if(!m_bEditMode)
    {
        //try to register on the root node - if possible
        uno::Sequence < ::rtl::OUString > aNames(1);
        EnableNotification( aNames );
    }
    Load(::rtl::OUString());

    ImplUpdateApplicationSettings();

    // #100822#
    ::Application::AddEventListener( LINK(this, ColorConfig_Impl, DataChangedEventListener) );

}
/* -----------------------------25.03.2002 12:28------------------------------

 ---------------------------------------------------------------------------*/
ColorConfig_Impl::~ColorConfig_Impl()
{
    // #100822#
    ::Application::RemoveEventListener( LINK(this, ColorConfig_Impl, DataChangedEventListener) );
}
/* -----------------------------22.03.2002 14:38------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfig_Impl::Load(const rtl::OUString& rScheme)
{
    rtl::OUString sScheme(rScheme);
    if(!sScheme.getLength())
    {
        //detect current scheme name
        uno::Sequence < ::rtl::OUString > aCurrent(1);
        aCurrent.getArray()[0] = C2U("CurrentColorScheme");
        uno::Sequence< uno::Any > aCurrentVal = GetProperties( aCurrent );
        aCurrentVal.getConstArray()[0] >>= sScheme;
    }
    m_sLoadedScheme = sScheme;
    //
    uno::Sequence < ::rtl::OUString > aColorNames = GetPropertyNames(sScheme);
    uno::Sequence< uno::Any > aColors = GetProperties( aColorNames );
    const uno::Any* pColors = aColors.getConstArray();
    const ::rtl::OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    for(int i = 0; i < 2 * ColorConfigEntryCount && aColors.getLength() > nIndex; i+= 2)
    {
        if(pColors[nIndex].hasValue())
            pColors[nIndex] >>= m_aConfigValues[i / 2].nColor;
        else
            m_aConfigValues[i/2].nColor = COL_AUTO;
        nIndex++;
        if(nIndex >= aColors.getLength())
            break;
        //test for visibility property
        if(pColorNames[nIndex].match(m_sIsVisible, pColorNames[nIndex].getLength() - m_sIsVisible.getLength()))
             m_aConfigValues[i / 2].bIsVisible = Any2Bool(pColors[nIndex++]);
    }
}
/* -----------------------------22.03.2002 14:38------------------------------

 ---------------------------------------------------------------------------*/
void    ColorConfig_Impl::Notify( const uno::Sequence<OUString>& )
{
    //loading via notification always uses the default setting
    Load(::rtl::OUString());

    SolarMutexGuard aVclGuard;

    if(m_bLockBroadcast)
    {
        m_bBroadcastWhenUnlocked = sal_True;
        ImplUpdateApplicationSettings();
    }
    else
        Broadcast(SfxSimpleHint(SFX_HINT_COLORS_CHANGED));
}
/* -----------------------------22.03.2002 14:38------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfig_Impl::Commit()
{
    uno::Sequence < ::rtl::OUString > aColorNames = GetPropertyNames(m_sLoadedScheme);
    uno::Sequence < beans::PropertyValue > aPropValues(aColorNames.getLength());
    beans::PropertyValue* pPropValues = aPropValues.getArray();
    const ::rtl::OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    const uno::Type& rBoolType = ::getBooleanCppuType();
    for(int i = 0; i < 2 * ColorConfigEntryCount && aColorNames.getLength() > nIndex; i+= 2)
    {
        pPropValues[nIndex].Name = pColorNames[nIndex];
        //save automatic colors as void value
        if(COL_AUTO != sal::static_int_cast<ColorData>(m_aConfigValues[i/2].nColor))
            pPropValues[nIndex].Value <<= m_aConfigValues[i/2].nColor;

        nIndex++;
        if(nIndex >= aColorNames.getLength())
            break;
        //test for visibility property
        if(pColorNames[nIndex].match(m_sIsVisible, pColorNames[nIndex].getLength() - m_sIsVisible.getLength()))
        {
             pPropValues[nIndex].Name = pColorNames[nIndex];
             pPropValues[nIndex].Value.setValue(&m_aConfigValues[i/2].bIsVisible, rBoolType);
             nIndex++;
        }
    }
    rtl::OUString sNode(C2U("ColorSchemes"));
    SetSetProperties(sNode, aPropValues);

    CommitCurrentSchemeName();
}
/* -----------------11.12.2002 10:42-----------------
 *
 * --------------------------------------------------*/
void ColorConfig_Impl::CommitCurrentSchemeName()
{
    //save current scheme name
    uno::Sequence < ::rtl::OUString > aCurrent(1);
    aCurrent.getArray()[0] = C2U("CurrentColorScheme");
    uno::Sequence< uno::Any > aCurrentVal(1);
    aCurrentVal.getArray()[0] <<= m_sLoadedScheme;
    PutProperties(aCurrent, aCurrentVal);
}
/* -----------------------------2002/06/20 13:03------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfig_Impl::SettingsChanged()
{
    SolarMutexGuard aVclGuard;

    ImplUpdateApplicationSettings();

    Broadcast( SfxSimpleHint( SFX_HINT_COLORS_CHANGED ) );
}
/* -----------------------------2002/08/16 12:07 -----------------------------
   #100822#
 --------------------------------------------------------------------------- */
IMPL_LINK( ColorConfig_Impl, DataChangedEventListener, VclWindowEvent*, pEvent )
{
    if ( pEvent->GetId() == VCLEVENT_APPLICATION_DATACHANGED )
    {
        DataChangedEvent* pData = (DataChangedEvent*)(pEvent->GetData());
        if ( (pData->GetType() == DATACHANGED_SETTINGS) &&
             (pData->GetFlags() & SETTINGS_STYLE) )
        {
            SettingsChanged();
            return 1L;
        } else
            return 0L;
    } else
        return 0L;
}

// ---------------------------------------------------------------------------

/** updates the font color in the vcl window settings */
void ColorConfig_Impl::ImplUpdateApplicationSettings()
{
    Application* pApp = GetpApp();
    if( pApp )
    {
        AllSettings aSettings = pApp->GetSettings();
        StyleSettings aStyleSettings( aSettings.GetStyleSettings() );

        ColorConfigValue aRet = GetColorConfigValue(FONTCOLOR);
        if(COL_AUTO == sal::static_int_cast<ColorData>(aRet.nColor))
            aRet.nColor = ColorConfig::GetDefaultColor(FONTCOLOR).GetColor();

        Color aFontColor(aRet.nColor);

        if( aStyleSettings.GetFontColor() != aFontColor )
        {
            aStyleSettings.SetFontColor( aFontColor );

            aSettings.SetStyleSettings( aStyleSettings );
            pApp->SetSettings( aSettings );
        }
    }
}

// ---------------------------------------------------------------------------

ColorConfig::ColorConfig()
{
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    if ( !m_pImpl )
    {
        m_pImpl = new ColorConfig_Impl;
        ItemHolder2::holdConfigItem(E_COLORCFG);
    }
    ++nColorRefCount_Impl;
    StartListening( *m_pImpl);
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
ColorConfig::~ColorConfig()
{
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    EndListening( *m_pImpl);
    if(!--nColorRefCount_Impl)
    {
        delete m_pImpl;
        m_pImpl = 0;
    }
}
/* -----------------------------11.04.2002 11:49------------------------------

 ---------------------------------------------------------------------------*/
Color ColorConfig::GetDefaultColor(ColorConfigEntry eEntry)
{
    static const sal_Int32 aAutoColors[] =
    {
        0, // DOCCOLOR
        0xc0c0c0, // DOCBOUNDARIES
        0x808080, // APPBACKGROUND
        0xc0c0c0, // OBJECTBOUNDARIES
        0xc0c0c0, // TABLEBOUNDARIES
        0, // FONTCOLOR
        0xcc, // LINKS
        0x80, // LINKSVISITED
        0, // ANCHOR
        0xff0000, // SPELL
        COL_LIGHTMAGENTA,// SMARTTAGS
        0xc0c0c0, // WRITERTEXTGRID
        0xc0c0c0, // WRITERFIELDSHADIN
        0xc0c0c0, // WRITERIDXSHADINGS
        0, // WRITERDIRECTCURSOR
        COL_YELLOW, //WRITERNOTESINDICATOR
        COL_GREEN,  //WRITERSCRIPTINDICATOR
        0xc0c0c0, //WRITERSECTIONBOUNDARIES
        COL_BLUE, //WRITERPAGEBREAKS,
        COL_LIGHTBLUE, // HTMLSGML
        COL_LIGHTGREEN, // HTMLCOMMENT
        COL_LIGHTRED, // HTMLKEYWORD
        COL_GRAY, // HTMLUNKNOWN
        COL_LIGHTGRAY, // CALCGRID
        COL_BLUE, //CALCPAGEBREAK
        0x2300dc, //CALCPAGEBREAKMANUAL
        COL_GRAY, //CALCPAGEBREAKAUTOMATIC
        COL_LIGHTBLUE, // CALCDETECTIVE
        COL_LIGHTRED, // CALCDETECTIVEERROR
        COL_LIGHTRED, // CALCREFERENCE
        0xffffc0, // CALCNOTESBACKGROUND
        0xc0c0c0, // DRAWGRID
        0, // DRAWDRAWING
        0xb8ff, // DRAWFILL
        COL_GREEN, // BASICIDENTIFIER,
        COL_GRAY,// BASICCOMMENT   ,
        COL_LIGHTRED,// BASICNUMBER    ,
        COL_LIGHTRED,// BASICSTRING    ,
        COL_BLUE, // BASICOPERATOR  ,
        COL_BLUE, // BASICKEYWORD   ,
        COL_RED, //BASICERROR
    };
    Color aRet;
    switch(eEntry)
    {
        case DOCCOLOR :
            aRet = Application::GetSettings().GetStyleSettings().GetWindowColor();
            break;

        case APPBACKGROUND :
            aRet = Application::GetSettings().GetStyleSettings().GetWorkspaceColor();
            break;

        case SPELL :
        case DRAWDRAWING :
        case SMARTTAGS :            
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                aRet = rStyleSettings.GetHighContrastMode() ?
                    rStyleSettings.GetDialogTextColor().GetColor() : aAutoColors[eEntry];
        }
        break;

        case DRAWFILL            :
                aRet = /*rStyleSettings.GetHighContrastMode() ?
                    rStyleSettings.OutlineMode??? : */	aAutoColors[eEntry];
        break;

        case FONTCOLOR :
            aRet = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
            break;

        default:
            aRet = aAutoColors[eEntry];
    }
    return aRet;
}
/* -----------------------------11.04.2002 11:49------------------------------

 ---------------------------------------------------------------------------*/
ColorConfigValue ColorConfig::GetColorValue(ColorConfigEntry eEntry, sal_Bool bSmart)const
{
    ColorConfigValue aRet = m_pImpl->GetColorConfigValue(eEntry);
    if(bSmart)
    {
        if(COL_AUTO == sal::static_int_cast<ColorData>(aRet.nColor))
            aRet.nColor = ColorConfig::GetDefaultColor(eEntry).GetColor();
        //#103495# don't allow grey between 40% and 60% as application background
        const UINT8 nRed = COLORDATA_RED( aRet.nColor);
        if(eEntry == APPBACKGROUND &&
                (nRed == COLORDATA_GREEN( aRet.nColor)) &&
                    (nRed == COLORDATA_BLUE( aRet.nColor)) &&
                nRed > 102 && nRed < 153 )
        {
            aRet.nColor = RGB_COLORDATA(153, 153, 153);
        }
    }

    return aRet;
}
/* -----------------------------12.04.2002 09:25------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfig::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    SolarMutexGuard aVclGuard;

    Broadcast( rHint );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

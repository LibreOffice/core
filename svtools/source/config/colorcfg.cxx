/*************************************************************************
 *
 *  $RCSfile: colorcfg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-08-07 11:48:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <colorcfg.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <unotools/configpathes.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _SFXPOOLITEM_HXX
#include <poolitem.hxx> //Any2Bool
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX
#include <smplhint.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

 /* #100822# ----
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
 ------------- */
#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif

//-----------------------------------------------------------------------------
using namespace utl;
using namespace rtl;
using namespace com::sun::star;

namespace svtools
{

#define C2U(cChar) OUString::createFromAscii(cChar)
static const sal_Char cColor[] = "/Color";
static const sal_Char cColorSchemes[] = "ColorSchemes/";
sal_Int32            nColorRefCount_Impl = 0;
::osl::Mutex         aColorMutex_Impl;
ColorConfig_Impl*    ColorConfig::m_pImpl = NULL;

/* -----------------------------16.01.01 15:36--------------------------------
 ---------------------------------------------------------------------------*/
class ColorConfig_Impl : public utl::ConfigItem, public SfxBroadcaster
{
    ColorConfigValue    m_aConfigValues[ColorConfigEntryCount];
    sal_Bool            m_bEditMode;
    rtl::OUString       m_sIsVisible;
    rtl::OUString       m_sLoadedScheme;
    sal_Bool            m_bIsBroadcastEnabled;
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
    void                            SetColorConfigValue(ColorConfigEntry eValue,
                                                            const ColorConfigValue& rValue );

    const rtl::OUString&            GetLoadedScheme() const {return m_sLoadedScheme;}

    uno::Sequence< ::rtl::OUString> GetSchemeNames();

    sal_Bool                        AddScheme(const rtl::OUString& rNode);
    sal_Bool                        RemoveScheme(const rtl::OUString& rNode);
    void                            SetModified(){ConfigItem::SetModified();}
    void                            ClearModified(){ConfigItem::ClearModified();}
    void                            SettingsChanged();

    static void                     DisableBroadcast();
    static void                     EnableBroadcast();
    static sal_Bool                 IsEnableBroadcast();

    static void                     LockBroadcast();
    static void                     UnlockBroadcast();

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
        RTL_CONSTASCII_USTRINGPARAM("/DocColor")        ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/DocBoundaries")   ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/AppBackground")   ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/ObjectBoundaries"),sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/TableBoundaries") ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/FontColor")       ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/Links")           ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/LinksVisited")    ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/Anchor")          ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/Spell")     ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/WriterTextGrid")  ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/WriterFieldShadings"),sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/WriterIdxShadings")     ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/WriterDirectCursor")    ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/WriterNotesIndicator")    ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/WriterScriptIndicator")    ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/WriterSectionBoundaries")    ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/WriterPageBreaks")    ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/HTMLSGML")        ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/HTMLComment")     ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/HTMLKeyword")     ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/HTMLUnknown")     ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcGrid")        ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreak"), sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreakManual"), sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreakAutomatic"), sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcDetective")   ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcDetectiveError")   ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcReference")   ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/CalcNotesBackground") ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/DrawGrid")        ,sal_True,
        RTL_CONSTASCII_USTRINGPARAM("/DrawDrawing")     ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/DrawFill")        ,sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/BASICIdentifier"),  sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/BASICComment")   ,  sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/BASICNumber")    ,  sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/BASICString")    ,  sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/BASICOperator")  ,  sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/BASICKeyword")   ,  sal_False,
        RTL_CONSTASCII_USTRINGPARAM("/BASICError"),  sal_False
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
    m_sIsVisible(C2U("/IsVisible")),
    m_bEditMode(bEditMode),
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
// -----------------------------------------------------------------------------
void ColorConfig_Impl::DisableBroadcast()
{
    ColorConfig::m_pImpl->m_bIsBroadcastEnabled = sal_False;
}
// -----------------------------------------------------------------------------
void ColorConfig_Impl::EnableBroadcast()
{
    ColorConfig::m_pImpl->m_bIsBroadcastEnabled = sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool ColorConfig_Impl::IsEnableBroadcast()
{
    return ColorConfig::m_pImpl->m_bIsBroadcastEnabled;
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
void    ColorConfig_Impl::Notify( const uno::Sequence<OUString>& rPropertyNames)
{
    //loading via notification always uses the default setting
    Load(::rtl::OUString());

    vos::OGuard aVclGuard( Application::GetSolarMutex() );

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
        if(COL_AUTO != m_aConfigValues[i/2].nColor)
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
/* -----------------------------25.03.2002 12:19------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfig_Impl::SetColorConfigValue(ColorConfigEntry eValue, const ColorConfigValue& rValue )
{
    if(rValue != m_aConfigValues[eValue])
    {
        m_aConfigValues[eValue] = rValue;
        SetModified();
    }
}
/* -----------------------------25.03.2002 15:22------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< ::rtl::OUString> ColorConfig_Impl::GetSchemeNames()
{
    return GetNodeNames(C2U("ColorSchemes"));
}
/* -----------------------------09.04.2002 17:19------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ColorConfig_Impl::AddScheme(const rtl::OUString& rScheme)
{
    if(ConfigItem::AddNode(C2U("ColorSchemes"), rScheme))
    {
        m_sLoadedScheme = rScheme;
        Commit();
        return sal_True;
    }
    return sal_False;
}
/* -----------------------------09.04.2002 17:19------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ColorConfig_Impl::RemoveScheme(const rtl::OUString& rScheme)
{
    uno::Sequence< rtl::OUString > aElements(1);
    aElements.getArray()[0] = rScheme;
    return ClearNodeElements(C2U("ColorSchemes"), aElements);
}
/* -----------------------------2002/06/20 13:03------------------------------

 ---------------------------------------------------------------------------*/
void ColorConfig_Impl::SettingsChanged()
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    ImplUpdateApplicationSettings();

    Broadcast( SfxSimpleHint( SFX_HINT_COLORS_CHANGED ) );
}
/* -----------------11.12.2002 09:21-----------------
 *
 * --------------------------------------------------*/
void ColorConfig_Impl::LockBroadcast()
{
    m_bLockBroadcast = sal_True;
}
/* -----------------11.12.2002 09:21-----------------
 *
 * --------------------------------------------------*/
void ColorConfig_Impl::UnlockBroadcast()
{
    if ( m_bBroadcastWhenUnlocked )
    {
        if ( m_bBroadcastWhenUnlocked = ColorConfig::m_pImpl != NULL )
        {
            ColorConfig::m_pImpl->ImplUpdateApplicationSettings();
            if ( ColorConfig::m_pImpl->IsEnableBroadcast() )
            {
                m_bBroadcastWhenUnlocked = sal_False;
                ColorConfig::m_pImpl->Broadcast(SfxSimpleHint(SFX_HINT_COLORS_CHANGED));
            }
        }
    }
    m_bLockBroadcast = sal_False;
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

        ColorConfigValue aRet = GetColorConfigValue(svtools::FONTCOLOR);
        if(COL_AUTO == aRet.nColor)
            aRet.nColor = ColorConfig::GetDefaultColor(svtools::FONTCOLOR).GetColor();

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
    ::osl::MutexGuard aGuard( aColorMutex_Impl );
    if ( !m_pImpl )
        m_pImpl = new ColorConfig_Impl;
    ++nColorRefCount_Impl;
    StartListening( *m_pImpl);
}
/* -----------------------------16.01.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
ColorConfig::~ColorConfig()
{
    ::osl::MutexGuard aGuard( aColorMutex_Impl );
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
        COL_RED //BASICERROR
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
        {
            const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                aRet = rStyleSettings.GetHighContrastMode() ?
                    rStyleSettings.GetDialogTextColor().GetColor() : aAutoColors[eEntry];
        }
        break;

        case DRAWFILL            :
                aRet = /*rStyleSettings.GetHighContrastMode() ?
                    rStyleSettings.OutlineMode??? : */  aAutoColors[eEntry];
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
        if(COL_AUTO == aRet.nColor)
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
void ColorConfig::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    vos::OGuard aVclGuard( Application::GetSolarMutex() );

    Broadcast( rHint );
}
/* -----------------------------25.03.2002 12:01------------------------------

 ---------------------------------------------------------------------------*/
EditableColorConfig::EditableColorConfig() :
    m_pImpl(new ColorConfig_Impl),
    m_bModified(sal_False)
{
    m_pImpl->LockBroadcast();
}
/*-- 25.03.2002 12:03:08---------------------------------------------------

  -----------------------------------------------------------------------*/
EditableColorConfig::~EditableColorConfig()
{
    m_pImpl->UnlockBroadcast();
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    delete m_pImpl;
}

/*-- 25.03.2002 12:03:15---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< ::rtl::OUString >  EditableColorConfig::GetSchemeNames() const
{
    return m_pImpl->GetSchemeNames();
}
/*-- 25.03.2002 12:03:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void EditableColorConfig::DeleteScheme(const ::rtl::OUString& rScheme )
{
    m_pImpl->RemoveScheme(rScheme);
}
/*-- 25.03.2002 12:03:16---------------------------------------------------

  -----------------------------------------------------------------------*/
void EditableColorConfig::AddScheme(const ::rtl::OUString& rScheme )
{
    m_pImpl->AddScheme(rScheme);
}
/*-- 25.03.2002 12:03:16---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool EditableColorConfig::LoadScheme(const ::rtl::OUString& rScheme )
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = sal_False;
    m_pImpl->Load(rScheme);
    //the name of the loaded scheme has to be committed separately
    m_pImpl->CommitCurrentSchemeName();
    return sal_True;
}
/*-- 25.03.2002 12:03:16---------------------------------------------------

  -----------------------------------------------------------------------*/
const ::rtl::OUString& EditableColorConfig::GetCurrentSchemeName()const
{
    return m_pImpl->GetLoadedScheme();
}
/* -----------------11.12.2002 10:56-----------------
 *  changes the name of the current scheme but doesn't load it!
 * --------------------------------------------------*/
void EditableColorConfig::SetCurrentSchemeName(const ::rtl::OUString& rScheme)
{
    m_pImpl->SetCurrentSchemeName(rScheme);
    m_pImpl->CommitCurrentSchemeName();
}
/*-- 25.03.2002 12:03:17---------------------------------------------------

  -----------------------------------------------------------------------*/
const ColorConfigValue& EditableColorConfig::GetColorValue(
    ColorConfigEntry eEntry)const
{
    return m_pImpl->GetColorConfigValue(eEntry);
}
/*-- 25.03.2002 12:03:17---------------------------------------------------

  -----------------------------------------------------------------------*/
void EditableColorConfig::SetColorValue(
    ColorConfigEntry eEntry, const ColorConfigValue& rValue)
{
    m_pImpl->SetColorConfigValue(eEntry, rValue);
    m_pImpl->ClearModified();
    m_bModified = sal_True;
}
/* -----------------------------10.04.2002 13:22------------------------------

 ---------------------------------------------------------------------------*/
void EditableColorConfig::SetModified()
{
    m_bModified = sal_True;
}
/* -----------------15.10.2002 14:51-----------------
 *
 * --------------------------------------------------*/
void EditableColorConfig::Commit()
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = sal_False;
}
// -----------------------------------------------------------------------------
void EditableColorConfig::DisableBroadcast()
{
    m_pImpl->DisableBroadcast();
}
// -----------------------------------------------------------------------------
void EditableColorConfig::EnableBroadcast()
{
    m_pImpl->EnableBroadcast();
}
// -----------------------------------------------------------------------------

}//namespace svtools

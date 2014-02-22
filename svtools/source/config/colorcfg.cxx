/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <svtools/colorcfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/configitem.hxx>
#include <unotools/confignode.hxx>
#include <unotools/configpaths.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/poolitem.hxx>
#include <svl/smplhint.hxx>
#include <osl/mutex.hxx>

#include <itemholder2.hxx>

 /* #100822# ----
#include <vcl/wrkwin.hxx>
 ------------- */
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <rtl/instance.hxx>


using namespace utl;
using namespace com::sun::star;


namespace svtools
{

static const sal_Char cColor[] = "/Color";
static const sal_Char cColorSchemes[] = "ColorSchemes/";
sal_Int32            nColorRefCount_Impl = 0;
namespace
{
    struct ColorMutex_Impl
        : public rtl::Static< ::osl::Mutex, ColorMutex_Impl > {};
}

ColorConfig_Impl*    ColorConfig::m_pImpl = NULL;

class ColorConfig_Impl : public utl::ConfigItem
{
    ColorConfigValue    m_aConfigValues[ColorConfigEntryCount];
    sal_Bool            m_bEditMode;
    OUString       m_sIsVisible;
    OUString       m_sLoadedScheme;
    bool m_bAutoDetectSystemHC;

    uno::Sequence< OUString> GetPropertyNames(const OUString& rScheme);
public:
    ColorConfig_Impl(sal_Bool bEditMode = sal_False);
    virtual ~ColorConfig_Impl();

    void                            Load(const OUString& rScheme);
    void                            CommitCurrentSchemeName();
    
    void                            SetCurrentSchemeName(const OUString& rSchemeName) {m_sLoadedScheme = rSchemeName;}
    virtual void                    Commit();
    virtual void                    Notify( const uno::Sequence<OUString>& aPropertyNames);

    const ColorConfigValue&         GetColorConfigValue(ColorConfigEntry eValue)
                                                            {return m_aConfigValues[eValue];}
    void                            SetColorConfigValue(ColorConfigEntry eValue,
                                                            const ColorConfigValue& rValue );

    const OUString&            GetLoadedScheme() const {return m_sLoadedScheme;}

    uno::Sequence< OUString> GetSchemeNames();

    sal_Bool                        AddScheme(const OUString& rNode);
    sal_Bool                        RemoveScheme(const OUString& rNode);
    void                            SetModified(){ConfigItem::SetModified();}
    void                            ClearModified(){ConfigItem::ClearModified();}
    void                            SettingsChanged();
    bool GetAutoDetectSystemHC() {return m_bAutoDetectSystemHC;}

    
    DECL_LINK( DataChangedEventListener, VclWindowEvent* );

    void ImplUpdateApplicationSettings();
};

uno::Sequence< OUString> ColorConfig_Impl::GetPropertyNames(const OUString& rScheme)
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
        { RTL_CONSTASCII_USTRINGPARAM("/FontColor")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/Links")           ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/LinksVisited")    ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/Spell")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SmartTags")     ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/Shadow")        , sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterTextGrid")  ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterFieldShadings"),sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterIdxShadings")     ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterDirectCursor")    ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterScriptIndicator")    ,sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterSectionBoundaries")    ,sal_True },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterHeaderFooterMark")    ,sal_False },
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
        { RTL_CONSTASCII_USTRINGPARAM("/BASICIdentifier"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICComment")   ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICNumber")    ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICString")    ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICOperator")  ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICKeyword")   ,  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICError"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLIdentifier"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLNumber"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLString"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLOperator"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLKeyword"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLParameter"),  sal_False },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLComment"),  sal_False }
    };
    int nIndex = 0;
    OUString sColor = cColor;
    OUString sBase(cColorSchemes);
    sBase += utl::wrapConfigurationElementName(rScheme);
    const int nCount = ColorConfigEntryCount;
    for(sal_Int32 i = 0; i < 4 * nCount; i+= 4)
    {
        OUString sBaseName(sBase);
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

ColorConfig_Impl::ColorConfig_Impl(sal_Bool bEditMode) :
    ConfigItem("Office.UI/ColorScheme"),
    m_bEditMode(bEditMode),
    m_sIsVisible("/IsVisible"),
    m_bAutoDetectSystemHC(true)
{
    if(!m_bEditMode)
    {
        
        uno::Sequence < OUString > aNames(1);
        EnableNotification( aNames );
    }
    Load(OUString());

    ImplUpdateApplicationSettings();

    
    ::Application::AddEventListener( LINK(this, ColorConfig_Impl, DataChangedEventListener) );

}

ColorConfig_Impl::~ColorConfig_Impl()
{
    
    ::Application::RemoveEventListener( LINK(this, ColorConfig_Impl, DataChangedEventListener) );
}

void ColorConfig_Impl::Load(const OUString& rScheme)
{
    OUString sScheme(rScheme);
    if(sScheme.isEmpty())
    {
        
        uno::Sequence < OUString > aCurrent(1);
        aCurrent.getArray()[0] = "CurrentColorScheme";
        uno::Sequence< uno::Any > aCurrentVal = GetProperties( aCurrent );
        aCurrentVal.getConstArray()[0] >>= sScheme;
    }
    m_sLoadedScheme = sScheme;
    //
    uno::Sequence < OUString > aColorNames = GetPropertyNames(sScheme);
    uno::Sequence< uno::Any > aColors = GetProperties( aColorNames );
    const uno::Any* pColors = aColors.getConstArray();
    const OUString* pColorNames = aColorNames.getConstArray();
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
        
        if(pColorNames[nIndex].match(m_sIsVisible, pColorNames[nIndex].getLength() - m_sIsVisible.getLength()))
             m_aConfigValues[i / 2].bIsVisible = Any2Bool(pColors[nIndex++]);
    }
    
    {
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(comphelper::getProcessComponentContext(),OUString("org.openoffice.Office.Common/Accessibility") );
        if(aNode.isValid())
        {
            uno::Any aValue = aNode.getNodeValue(OUString("AutoDetectSystemHC"));
            aValue >>= m_bAutoDetectSystemHC;
        }
    }
}

void    ColorConfig_Impl::Notify( const uno::Sequence<OUString>& )
{
    
    Load(OUString());
    NotifyListeners(0);
}

void ColorConfig_Impl::Commit()
{
    uno::Sequence < OUString > aColorNames = GetPropertyNames(m_sLoadedScheme);
    uno::Sequence < beans::PropertyValue > aPropValues(aColorNames.getLength());
    beans::PropertyValue* pPropValues = aPropValues.getArray();
    const OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    const uno::Type& rBoolType = ::getBooleanCppuType();
    for(int i = 0; i < 2 * ColorConfigEntryCount && aColorNames.getLength() > nIndex; i+= 2)
    {
        pPropValues[nIndex].Name = pColorNames[nIndex];
        
        if(COL_AUTO != sal::static_int_cast<ColorData>(m_aConfigValues[i/2].nColor))
            pPropValues[nIndex].Value <<= m_aConfigValues[i/2].nColor;

        nIndex++;
        if(nIndex >= aColorNames.getLength())
            break;
        
        if(pColorNames[nIndex].match(m_sIsVisible, pColorNames[nIndex].getLength() - m_sIsVisible.getLength()))
        {
             pPropValues[nIndex].Name = pColorNames[nIndex];
             pPropValues[nIndex].Value.setValue(&m_aConfigValues[i/2].bIsVisible, rBoolType);
             nIndex++;
        }
    }
    OUString sNode("ColorSchemes");
    SetSetProperties(sNode, aPropValues);

    CommitCurrentSchemeName();
}

void ColorConfig_Impl::CommitCurrentSchemeName()
{
    
    uno::Sequence < OUString > aCurrent(1);
    aCurrent.getArray()[0] = "CurrentColorScheme";
    uno::Sequence< uno::Any > aCurrentVal(1);
    aCurrentVal.getArray()[0] <<= m_sLoadedScheme;
    PutProperties(aCurrent, aCurrentVal);
}

void ColorConfig_Impl::SetColorConfigValue(ColorConfigEntry eValue, const ColorConfigValue& rValue )
{
    if(rValue != m_aConfigValues[eValue])
    {
        m_aConfigValues[eValue] = rValue;
        SetModified();
    }
}

uno::Sequence< OUString> ColorConfig_Impl::GetSchemeNames()
{
    return GetNodeNames("ColorSchemes");
}

sal_Bool ColorConfig_Impl::AddScheme(const OUString& rScheme)
{
    if(ConfigItem::AddNode("ColorSchemes", rScheme))
    {
        m_sLoadedScheme = rScheme;
        Commit();
        return sal_True;
    }
    return sal_False;
}

sal_Bool ColorConfig_Impl::RemoveScheme(const OUString& rScheme)
{
    uno::Sequence< OUString > aElements(1);
    aElements.getArray()[0] = rScheme;
    return ClearNodeElements("ColorSchemes", aElements);
}

void ColorConfig_Impl::SettingsChanged()
{
    SolarMutexGuard aVclGuard;

    ImplUpdateApplicationSettings();

    NotifyListeners(0);
}

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



/** updates the font color in the vcl window settings */
void ColorConfig_Impl::ImplUpdateApplicationSettings()
{
    Application* pApp = GetpApp();
    if( pApp )
    {
        AllSettings aSettings = pApp->GetSettings();
        StyleSettings aStyleSettings( aSettings.GetStyleSettings() );

        ColorConfigValue aRet = GetColorConfigValue(svtools::FONTCOLOR);
        if(COL_AUTO == sal::static_int_cast<ColorData>(aRet.nColor))
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



ColorConfig::ColorConfig()
{
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    if ( !m_pImpl )
    {
        m_pImpl = new ColorConfig_Impl;
        svtools::ItemHolder2::holdConfigItem(E_COLORCFG);
    }
    ++nColorRefCount_Impl;
    m_pImpl->AddListener(this);
}

ColorConfig::~ColorConfig()
{
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    m_pImpl->RemoveListener(this);
    if(!--nColorRefCount_Impl)
    {
        delete m_pImpl;
        m_pImpl = 0;
    }
}

Color ColorConfig::GetDefaultColor(ColorConfigEntry eEntry)
{
    static const sal_Int32 aAutoColors[] =
    {
        COL_WHITE, 
        0xc0c0c0, 
        0x808080, 
        0xc0c0c0, 
        0xc0c0c0, 
        COL_BLACK, 
        0xcc, 
        0x80, 
        0xff0000, 
        COL_LIGHTMAGENTA,
        COL_GRAY, 
        0xc0c0c0, 
        0xc0c0c0, 
        0xc0c0c0, 
        0, 
        COL_GREEN,  
        0xc0c0c0, 
        0x0369a3, 
        COL_BLUE, 
        COL_LIGHTBLUE, 
        COL_LIGHTGREEN, 
        COL_LIGHTRED, 
        COL_GRAY, 
        COL_LIGHTGRAY, 
        COL_BLUE, 
        0x2300dc, 
        COL_GRAY, 
        COL_LIGHTBLUE, 
        COL_LIGHTRED, 
        0xef0fff, 
        0xffffc0, 
        0xc0c0c0, 
        COL_GREEN, 
        COL_GRAY,
        COL_LIGHTRED,
        COL_LIGHTRED,
        COL_BLUE, 
        COL_BLUE, 
        COL_RED, 
        0x009900, 
        0x000000, 
        0xCE7B00, 
        0x000000, 
        0x0000E6, 
        0x259D9D, 
        0x969696,
    };
    Color aRet;
    switch(eEntry)
    {
        case APPBACKGROUND :
            aRet = Application::GetSettings().GetStyleSettings().GetWorkspaceColor();
            break;

        case LINKS :
            aRet = Application::GetSettings().GetStyleSettings().GetLinkColor();
            break;

        case LINKSVISITED :
            aRet = Application::GetSettings().GetStyleSettings().GetVisitedLinkColor();
            break;

        default:
            aRet = aAutoColors[eEntry];
    }
    
    if(m_pImpl &&  m_pImpl->GetAutoDetectSystemHC())
    {
        switch(eEntry)
        {
            case DOCCOLOR :
                aRet = Application::GetSettings().GetStyleSettings().GetWindowColor();
                break;
            case FONTCOLOR :
                aRet = Application::GetSettings().GetStyleSettings().GetWindowTextColor();
                break;
            default:
                break;
        }
    }
    return aRet;
}

ColorConfigValue ColorConfig::GetColorValue(ColorConfigEntry eEntry, sal_Bool bSmart)const
{
    ColorConfigValue aRet = m_pImpl->GetColorConfigValue(eEntry);
    if(bSmart)
    {
        if(COL_AUTO == sal::static_int_cast<ColorData>(aRet.nColor))
            aRet.nColor = ColorConfig::GetDefaultColor(eEntry).GetColor();
    }

    return aRet;
}

void ColorConfig::Reload()
{
    m_pImpl->Load(OUString());
}

EditableColorConfig::EditableColorConfig() :
    m_pImpl(new ColorConfig_Impl),
    m_bModified(sal_False)
{
    m_pImpl->BlockBroadcasts(true);
}

EditableColorConfig::~EditableColorConfig()
{
    m_pImpl->BlockBroadcasts(false);
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    delete m_pImpl;
}

uno::Sequence< OUString >  EditableColorConfig::GetSchemeNames() const
{
    return m_pImpl->GetSchemeNames();
}

void EditableColorConfig::DeleteScheme(const OUString& rScheme )
{
    m_pImpl->RemoveScheme(rScheme);
}

void EditableColorConfig::AddScheme(const OUString& rScheme )
{
    m_pImpl->AddScheme(rScheme);
}

sal_Bool EditableColorConfig::LoadScheme(const OUString& rScheme )
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = sal_False;
    m_pImpl->Load(rScheme);
    
    m_pImpl->CommitCurrentSchemeName();
    return sal_True;
}

const OUString& EditableColorConfig::GetCurrentSchemeName()const
{
    return m_pImpl->GetLoadedScheme();
}


void EditableColorConfig::SetCurrentSchemeName(const OUString& rScheme)
{
    m_pImpl->SetCurrentSchemeName(rScheme);
    m_pImpl->CommitCurrentSchemeName();
}

const ColorConfigValue& EditableColorConfig::GetColorValue(
    ColorConfigEntry eEntry)const
{
    return m_pImpl->GetColorConfigValue(eEntry);
}

void EditableColorConfig::SetColorValue(
    ColorConfigEntry eEntry, const ColorConfigValue& rValue)
{
    m_pImpl->SetColorConfigValue(eEntry, rValue);
    m_pImpl->ClearModified();
    m_bModified = sal_True;
}

void EditableColorConfig::SetModified()
{
    m_bModified = sal_True;
}

void EditableColorConfig::Commit()
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = sal_False;
}

void EditableColorConfig::DisableBroadcast()
{
    m_pImpl->BlockBroadcasts(true);
}

void EditableColorConfig::EnableBroadcast()
{
    m_pImpl->BlockBroadcasts(false);
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

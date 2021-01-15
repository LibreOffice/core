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

#include <sal/config.h>

#include <string_view>

#include <svtools/colorcfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/configitem.hxx>
#include <unotools/confignode.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configpaths.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/poolitem.hxx>
#include <osl/mutex.hxx>

#include "itemholder2.hxx"

#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <rtl/instance.hxx>


using namespace utl;
using namespace com::sun::star;

const char g_sIsVisible[] = "/IsVisible";


namespace svtools
{

static sal_Int32            nColorRefCount_Impl = 0;
namespace
{
    struct ColorMutex_Impl
        : public rtl::Static< ::osl::Mutex, ColorMutex_Impl > {};
}

ColorConfig_Impl*    ColorConfig::m_pImpl = nullptr;

class ColorConfig_Impl : public utl::ConfigItem
{
    ColorConfigValue m_aConfigValues[ColorConfigEntryCount];
    OUString         m_sLoadedScheme;
    bool             m_bAutoDetectSystemHC;

    virtual void                    ImplCommit() override;

public:
    explicit ColorConfig_Impl();
    virtual ~ColorConfig_Impl() override;

    void                            Load(const OUString& rScheme);
    void                            CommitCurrentSchemeName();
    //changes the name of the current scheme but doesn't load it!
    void                            SetCurrentSchemeName(const OUString& rSchemeName) {m_sLoadedScheme = rSchemeName;}
    virtual void                    Notify( const uno::Sequence<OUString>& aPropertyNames) override;

    const ColorConfigValue&         GetColorConfigValue(ColorConfigEntry eValue)
                                                            {return m_aConfigValues[eValue];}
    void                            SetColorConfigValue(ColorConfigEntry eValue,
                                                            const ColorConfigValue& rValue );

    const OUString&            GetLoadedScheme() const {return m_sLoadedScheme;}

    uno::Sequence< OUString> GetSchemeNames();

    void                            AddScheme(const OUString& rNode);
    void                            RemoveScheme(const OUString& rNode);
    using ConfigItem::SetModified;
    using ConfigItem::ClearModified;
    void                            SettingsChanged();
    bool GetAutoDetectSystemHC() const {return m_bAutoDetectSystemHC;}

    DECL_LINK( DataChangedEventListener, VclSimpleEvent&, void );

    void ImplUpdateApplicationSettings();
};

namespace {

uno::Sequence< OUString> GetPropertyNames(const OUString& rScheme)
{
    struct ColorConfigEntryData_Impl
    {
        std::u16string_view cName;
        bool            bCanBeVisible;
    };
    static const ColorConfigEntryData_Impl cNames[] =
    {
        { std::u16string_view(u"/DocColor")        ,false },
        { std::u16string_view(u"/DocBoundaries")   ,true },
        { std::u16string_view(u"/AppBackground")   ,false },
        { std::u16string_view(u"/ObjectBoundaries"),true },
        { std::u16string_view(u"/TableBoundaries") ,true },
        { std::u16string_view(u"/FontColor")     ,false },
        { std::u16string_view(u"/Links")           ,true },
        { std::u16string_view(u"/LinksVisited")    ,true },
        { std::u16string_view(u"/Spell")     ,false },
        { std::u16string_view(u"/SmartTags")     ,false },
        { std::u16string_view(u"/Shadow")        , true },
        { std::u16string_view(u"/WriterTextGrid")  ,false },
        { std::u16string_view(u"/WriterFieldShadings"),true },
        { std::u16string_view(u"/WriterIdxShadings")     ,true },
        { std::u16string_view(u"/WriterDirectCursor")    ,true },
        { std::u16string_view(u"/WriterScriptIndicator")    ,false },
        { std::u16string_view(u"/WriterSectionBoundaries")    ,true },
        { std::u16string_view(u"/WriterHeaderFooterMark")    ,false },
        { std::u16string_view(u"/WriterPageBreaks")    ,false },
        { std::u16string_view(u"/HTMLSGML")        ,false },
        { std::u16string_view(u"/HTMLComment")     ,false },
        { std::u16string_view(u"/HTMLKeyword")     ,false },
        { std::u16string_view(u"/HTMLUnknown")     ,false },
        { std::u16string_view(u"/CalcGrid")        ,false },
        { std::u16string_view(u"/CalcPageBreak"), false },
        { std::u16string_view(u"/CalcPageBreakManual"), false },
        { std::u16string_view(u"/CalcPageBreakAutomatic"), false },
        { std::u16string_view(u"/CalcDetective")   ,false },
        { std::u16string_view(u"/CalcDetectiveError")   ,false },
        { std::u16string_view(u"/CalcReference")   ,false },
        { std::u16string_view(u"/CalcNotesBackground") ,false },
        { std::u16string_view(u"/CalcValue") ,false },
        { std::u16string_view(u"/CalcFormula") ,false },
        { std::u16string_view(u"/CalcText") ,false },
        { std::u16string_view(u"/CalcProtectedBackground") ,false },
        { std::u16string_view(u"/DrawGrid")        ,true },
        { std::u16string_view(u"/BASICIdentifier"),  false },
        { std::u16string_view(u"/BASICComment")   ,  false },
        { std::u16string_view(u"/BASICNumber")    ,  false },
        { std::u16string_view(u"/BASICString")    ,  false },
        { std::u16string_view(u"/BASICOperator")  ,  false },
        { std::u16string_view(u"/BASICKeyword")   ,  false },
        { std::u16string_view(u"/BASICError"),  false },
        { std::u16string_view(u"/SQLIdentifier"),  false },
        { std::u16string_view(u"/SQLNumber"),  false },
        { std::u16string_view(u"/SQLString"),  false },
        { std::u16string_view(u"/SQLOperator"),  false },
        { std::u16string_view(u"/SQLKeyword"),  false },
        { std::u16string_view(u"/SQLParameter"),  false },
        { std::u16string_view(u"/SQLComment"),  false }
    };

    uno::Sequence<OUString> aNames(2 * ColorConfigEntryCount);
    OUString* pNames = aNames.getArray();
    int nIndex = 0;
    OUString sBase = "ColorSchemes/"
                   + utl::wrapConfigurationElementName(rScheme);
    for(sal_Int32 i = 0; i < ColorConfigEntryCount; ++i)
    {
        OUString sBaseName = sBase + cNames[i].cName;
        pNames[nIndex++] = sBaseName + "/Color";
        if(cNames[i].bCanBeVisible)
        {
            pNames[nIndex++] = sBaseName + g_sIsVisible;
        }
    }
    aNames.realloc(nIndex);
    return aNames;
}

}

ColorConfig_Impl::ColorConfig_Impl() :
    ConfigItem("Office.UI/ColorScheme"),
    m_bAutoDetectSystemHC(true)
{
    //try to register on the root node - if possible
    uno::Sequence < OUString > aNames(1);
    EnableNotification( aNames );

    if (!utl::ConfigManager::IsFuzzing())
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
        //detect current scheme name
        uno::Sequence < OUString > aCurrent { "CurrentColorScheme" };
        uno::Sequence< uno::Any > aCurrentVal = GetProperties( aCurrent );
        aCurrentVal.getConstArray()[0] >>= sScheme;
    }
    m_sLoadedScheme = sScheme;

    uno::Sequence < OUString > aColorNames = GetPropertyNames(sScheme);
    uno::Sequence< uno::Any > aColors = GetProperties( aColorNames );
    const uno::Any* pColors = aColors.getConstArray();
    const OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    for(int i = 0; i < ColorConfigEntryCount && aColors.getLength() > nIndex; ++i)
    {
        if(pColors[nIndex].hasValue())
        {
            Color nTmp;
            pColors[nIndex] >>= nTmp;
            m_aConfigValues[i].nColor = nTmp;
        }
        else
            m_aConfigValues[i].nColor = COL_AUTO;
        nIndex++;
        if(nIndex >= aColors.getLength())
            break;
        //test for visibility property
        if(pColorNames[nIndex].endsWith(g_sIsVisible))
             m_aConfigValues[i].bIsVisible = Any2Bool(pColors[nIndex++]);
    }
    // fdo#71511: check if we are running in a11y autodetect
    {
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(comphelper::getProcessComponentContext(),"org.openoffice.Office.Common/Accessibility" );
        if(aNode.isValid())
        {
            uno::Any aValue = aNode.getNodeValue(OUString("AutoDetectSystemHC"));
            aValue >>= m_bAutoDetectSystemHC;
        }
    }
}

void    ColorConfig_Impl::Notify( const uno::Sequence<OUString>& )
{
    //loading via notification always uses the default setting
    Load(OUString());
    NotifyListeners(ConfigurationHints::NONE);
}

void ColorConfig_Impl::ImplCommit()
{
    uno::Sequence < OUString > aColorNames = GetPropertyNames(m_sLoadedScheme);
    uno::Sequence < beans::PropertyValue > aPropValues(aColorNames.getLength());
    beans::PropertyValue* pPropValues = aPropValues.getArray();
    const OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    for(int i = 0; i < ColorConfigEntryCount && aColorNames.getLength() > nIndex; ++i)
    {
        pPropValues[nIndex].Name = pColorNames[nIndex];
        //save automatic colors as void value
        if(m_aConfigValues[i].nColor != COL_AUTO)
            pPropValues[nIndex].Value <<= m_aConfigValues[i].nColor;

        nIndex++;
        if(nIndex >= aColorNames.getLength())
            break;
        //test for visibility property
        if(pColorNames[nIndex].endsWith(g_sIsVisible))
        {
             pPropValues[nIndex].Name = pColorNames[nIndex];
             pPropValues[nIndex].Value <<= m_aConfigValues[i].bIsVisible;
             nIndex++;
        }
    }
    SetSetProperties("ColorSchemes", aPropValues);

    CommitCurrentSchemeName();
}

void ColorConfig_Impl::CommitCurrentSchemeName()
{
    //save current scheme name
    uno::Sequence < OUString > aCurrent { "CurrentColorScheme" };
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

void ColorConfig_Impl::AddScheme(const OUString& rScheme)
{
    if(ConfigItem::AddNode("ColorSchemes", rScheme))
    {
        m_sLoadedScheme = rScheme;
        Commit();
    }
}

void ColorConfig_Impl::RemoveScheme(const OUString& rScheme)
{
    uno::Sequence< OUString > aElements { rScheme };
    ClearNodeElements("ColorSchemes", aElements);
}

void ColorConfig_Impl::SettingsChanged()
{
    SolarMutexGuard aVclGuard;

    ImplUpdateApplicationSettings();

    NotifyListeners(ConfigurationHints::NONE);
}

IMPL_LINK( ColorConfig_Impl, DataChangedEventListener, VclSimpleEvent&, rEvent, void )
{
    if ( rEvent.GetId() == VclEventId::ApplicationDataChanged )
    {
        DataChangedEvent* pData = static_cast<DataChangedEvent*>(static_cast<VclWindowEvent&>(rEvent).GetData());
        if ( (pData->GetType() == DataChangedEventType::SETTINGS) &&
             (pData->GetFlags() & AllSettingsFlags::STYLE) )
        {
            SettingsChanged();
        }
    }
}


/** updates the font color in the vcl window settings */
void ColorConfig_Impl::ImplUpdateApplicationSettings()
{
    Application* pApp = GetpApp();
    if( !pApp )
        return;

    AllSettings aSettings = Application::GetSettings();
    StyleSettings aStyleSettings( aSettings.GetStyleSettings() );

    ColorConfigValue aRet = GetColorConfigValue(svtools::FONTCOLOR);
    if(COL_AUTO == aRet.nColor)
        aRet.nColor = ColorConfig::GetDefaultColor(svtools::FONTCOLOR);

    Color aFontColor(aRet.nColor);

    if( aStyleSettings.GetFontColor() != aFontColor )
    {
        aStyleSettings.SetFontColor( aFontColor );

        aSettings.SetStyleSettings( aStyleSettings );
        Application::SetSettings( aSettings );
    }
}

ColorConfig::ColorConfig()
{
    if (utl::ConfigManager::IsFuzzing())
        return;
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    if ( !m_pImpl )
    {
        m_pImpl = new ColorConfig_Impl;
        svtools::ItemHolder2::holdConfigItem(EItem::ColorConfig);
    }
    ++nColorRefCount_Impl;
    m_pImpl->AddListener(this);
}

ColorConfig::~ColorConfig()
{
    if (utl::ConfigManager::IsFuzzing())
        return;
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    m_pImpl->RemoveListener(this);
    if(!--nColorRefCount_Impl)
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

Color ColorConfig::GetDefaultColor(ColorConfigEntry eEntry)
{
    static const Color aAutoColors[] =
    {
        COL_WHITE, // DOCCOLOR
        COL_LIGHTGRAY, // DOCBOUNDARIES
        Color(0xDFDFDE), // APPBACKGROUND
        COL_LIGHTGRAY, // OBJECTBOUNDARIES
        COL_LIGHTGRAY, // TABLEBOUNDARIES
        COL_BLACK, // FONTCOLOR
        COL_BLUE, // LINKS
        Color(0x0000cc), // LINKSVISITED
        COL_LIGHTRED, // SPELL
        COL_LIGHTMAGENTA, // SMARTTAGS
        COL_GRAY, // SHADOWCOLOR
        COL_LIGHTGRAY, // WRITERTEXTGRID
        COL_LIGHTGRAY, // WRITERFIELDSHADIN
        COL_LIGHTGRAY, // WRITERIDXSHADINGS
        COL_BLACK, // WRITERDIRECTCURSOR
        COL_GREEN, //WRITERSCRIPTINDICATOR
        COL_LIGHTGRAY, //WRITERSECTIONBOUNDARIES
        Color(0x0369a3), //WRITERHEADERFOOTERMARK,
        COL_BLUE, //WRITERPAGEBREAKS,
        COL_LIGHTBLUE, // HTMLSGML
        COL_LIGHTGREEN, // HTMLCOMMENT
        COL_LIGHTRED, // HTMLKEYWORD
        COL_GRAY, // HTMLUNKNOWN
        COL_GRAY3, // CALCGRID
        COL_BLUE, //CALCPAGEBREAK
        Color(0x2300dc), //CALCPAGEBREAKMANUAL
        COL_GRAY7, //CALCPAGEBREAKAUTOMATIC
        COL_LIGHTBLUE, // CALCDETECTIVE
        COL_LIGHTRED, // CALCDETECTIVEERROR
        Color(0xef0fff), // CALCREFERENCE
        Color(0xffffc0), // CALCNOTESBACKGROUND
        COL_LIGHTBLUE, // CALCVALUE
        COL_GREEN, // CALCFORMULA
        COL_BLACK, // CALCTEXT
        COL_LIGHTGRAY, // CALCPROTECTEDBACKGROUND
        COL_GRAY7, // DRAWGRID
        COL_GREEN, // BASICIDENTIFIER,
        COL_GRAY, // BASICCOMMENT,
        COL_LIGHTRED, // BASICNUMBER,
        COL_LIGHTRED, // BASICSTRING,
        COL_BLUE, // BASICOPERATOR,
        COL_BLUE, // BASICKEYWORD,
        COL_RED, //BASICERROR
        Color(0x009900), // SQLIDENTIFIER
        COL_BLACK, // SQLNUMBER
        Color(0xCE7B00), // SQLSTRING
        COL_BLACK, // SQLOPERATOR
        Color(0x0000E6), // SQLKEYWORD
        Color(0x259D9D), // SQLPARAMETER
        COL_GRAY, // SQLCOMMENT
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
    // fdo#71511: if in autodetected a11y HC mode, do pull background color from theme
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

ColorConfigValue ColorConfig::GetColorValue(ColorConfigEntry eEntry, bool bSmart) const
{
    ColorConfigValue aRet;

    if (m_pImpl)
        aRet = m_pImpl->GetColorConfigValue(eEntry);

    if (bSmart && aRet.nColor == COL_AUTO)
        aRet.nColor = ColorConfig::GetDefaultColor(eEntry);

    return aRet;
}

EditableColorConfig::EditableColorConfig() :
    m_pImpl(new ColorConfig_Impl),
    m_bModified(false)
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

void EditableColorConfig::LoadScheme(const OUString& rScheme )
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = false;
    m_pImpl->Load(rScheme);
    //the name of the loaded scheme has to be committed separately
    m_pImpl->CommitCurrentSchemeName();
}

const OUString& EditableColorConfig::GetCurrentSchemeName()const
{
    return m_pImpl->GetLoadedScheme();
}

// Changes the name of the current scheme but doesn't load it!
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
    m_bModified = true;
}

void EditableColorConfig::SetModified()
{
    m_bModified = true;
}

void EditableColorConfig::Commit()
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = false;
}

void EditableColorConfig::DisableBroadcast()
{
    m_pImpl->BlockBroadcasts(true);
}

void EditableColorConfig::EnableBroadcast()
{
    m_pImpl->BlockBroadcasts(false);
}


}//namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

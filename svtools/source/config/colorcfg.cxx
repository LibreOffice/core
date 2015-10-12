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


#include <svtools/colorcfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/configitem.hxx>
#include <unotools/confignode.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configpaths.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/poolitem.hxx>
#include <svl/smplhint.hxx>
#include <osl/mutex.hxx>

#include "itemholder2.hxx"

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
    ColorConfigValue m_aConfigValues[ColorConfigEntryCount];
    bool             m_bEditMode;
    OUString         m_sIsVisible;
    OUString         m_sLoadedScheme;
    bool             m_bAutoDetectSystemHC;

    uno::Sequence< OUString> GetPropertyNames(const OUString& rScheme);

    virtual void                    ImplCommit() override;

public:
    explicit ColorConfig_Impl(bool bEditMode = false);
    virtual ~ColorConfig_Impl();

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

    bool                            AddScheme(const OUString& rNode);
    bool                            RemoveScheme(const OUString& rNode);
    void                            SetModified(){ConfigItem::SetModified();}
    void                            ClearModified(){ConfigItem::ClearModified();}
    void                            SettingsChanged();
    bool GetAutoDetectSystemHC() {return m_bAutoDetectSystemHC;}

    DECL_LINK_TYPED( DataChangedEventListener, VclSimpleEvent&, void );

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
        bool            bCanBeVisible;
    };
    static const ColorConfigEntryData_Impl cNames[] =
    {
        { RTL_CONSTASCII_USTRINGPARAM("/DocColor")        ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/DocBoundaries")   ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/AppBackground")   ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/ObjectBoundaries"),true },
        { RTL_CONSTASCII_USTRINGPARAM("/TableBoundaries") ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/FontColor")     ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/Links")           ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/LinksVisited")    ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/Spell")     ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/SmartTags")     ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/Shadow")        , true },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterTextGrid")  ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterFieldShadings"),true },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterIdxShadings")     ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterDirectCursor")    ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterScriptIndicator")    ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterSectionBoundaries")    ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterHeaderFooterMark")    ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/WriterPageBreaks")    ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLSGML")        ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLComment")     ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLKeyword")     ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/HTMLUnknown")     ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcGrid")        ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreak"), false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreakManual"), false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcPageBreakAutomatic"), false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcDetective")   ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcDetectiveError")   ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcReference")   ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/CalcNotesBackground") ,false },
        { RTL_CONSTASCII_USTRINGPARAM("/DrawGrid")        ,true },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICIdentifier"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICComment")   ,  false },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICNumber")    ,  false },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICString")    ,  false },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICOperator")  ,  false },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICKeyword")   ,  false },
        { RTL_CONSTASCII_USTRINGPARAM("/BASICError"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLIdentifier"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLNumber"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLString"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLOperator"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLKeyword"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLParameter"),  false },
        { RTL_CONSTASCII_USTRINGPARAM("/SQLComment"),  false }
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

ColorConfig_Impl::ColorConfig_Impl(bool bEditMode) :
    ConfigItem("Office.UI/ColorScheme"),
    m_bEditMode(bEditMode),
    m_sIsVisible("/IsVisible"),
    m_bAutoDetectSystemHC(true)
{
    if(!m_bEditMode)
    {
        //try to register on the root node - if possible
        uno::Sequence < OUString > aNames(1);
        EnableNotification( aNames );
    }

    if (!utl::ConfigManager::IsAvoidConfig())
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
        uno::Sequence < OUString > aCurrent(1);
        aCurrent.getArray()[0] = "CurrentColorScheme";
        uno::Sequence< uno::Any > aCurrentVal = GetProperties( aCurrent );
        aCurrentVal.getConstArray()[0] >>= sScheme;
    }
    m_sLoadedScheme = sScheme;

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
        //test for visibility property
        if(pColorNames[nIndex].endsWith(m_sIsVisible))
             m_aConfigValues[i / 2].bIsVisible = Any2Bool(pColors[nIndex++]);
    }
    // fdo#71511: check if we are running in a11y autodetect
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
    //loading via notification always uses the default setting
    Load(OUString());
    NotifyListeners(0);
}

void ColorConfig_Impl::ImplCommit()
{
    uno::Sequence < OUString > aColorNames = GetPropertyNames(m_sLoadedScheme);
    uno::Sequence < beans::PropertyValue > aPropValues(aColorNames.getLength());
    beans::PropertyValue* pPropValues = aPropValues.getArray();
    const OUString* pColorNames = aColorNames.getConstArray();
    sal_Int32 nIndex = 0;
    const uno::Type& rBoolType = cppu::UnoType<bool>::get();
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
        if(pColorNames[nIndex].endsWith(m_sIsVisible))
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
    //save current scheme name
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

bool ColorConfig_Impl::AddScheme(const OUString& rScheme)
{
    if(ConfigItem::AddNode("ColorSchemes", rScheme))
    {
        m_sLoadedScheme = rScheme;
        Commit();
        return true;
    }
    return false;
}

bool ColorConfig_Impl::RemoveScheme(const OUString& rScheme)
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

IMPL_LINK_TYPED( ColorConfig_Impl, DataChangedEventListener, VclSimpleEvent&, rEvent, void )
{
    if ( rEvent.GetId() == VCLEVENT_APPLICATION_DATACHANGED )
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
    if( pApp )
    {
        AllSettings aSettings = Application::GetSettings();
        StyleSettings aStyleSettings( aSettings.GetStyleSettings() );

        ColorConfigValue aRet = GetColorConfigValue(svtools::FONTCOLOR);
        if(COL_AUTO == sal::static_int_cast<ColorData>(aRet.nColor))
            aRet.nColor = ColorConfig::GetDefaultColor(svtools::FONTCOLOR).GetColor();

        Color aFontColor(aRet.nColor);

        if( aStyleSettings.GetFontColor() != aFontColor )
        {
            aStyleSettings.SetFontColor( aFontColor );

            aSettings.SetStyleSettings( aStyleSettings );
            Application::SetSettings( aSettings );
        }
    }
}

ColorConfig::ColorConfig()
{
    if (utl::ConfigManager::IsAvoidConfig())
        return;
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
    if (utl::ConfigManager::IsAvoidConfig())
        return;
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
        COL_WHITE, // DOCCOLOR
        0xc0c0c0, // DOCBOUNDARIES
        0xDFDFDE, // APPBACKGROUND
        COL_LIGHTGRAY, // OBJECTBOUNDARIES
        COL_LIGHTGRAY, // TABLEBOUNDARIES
        COL_BLACK, // FONTCOLOR
        COL_BLUE, // LINKS
        0x0000cc, // LINKSVISITED
        COL_LIGHTRED, // SPELL
        COL_LIGHTMAGENTA,// SMARTTAGS
        COL_GRAY, // SHADOWCOLOR
        COL_LIGHTGRAY, // WRITERTEXTGRID
        COL_LIGHTGRAY, // WRITERFIELDSHADIN
        COL_LIGHTGRAY, // WRITERIDXSHADINGS
        0, // WRITERDIRECTCURSOR
        COL_GREEN,  //WRITERSCRIPTINDICATOR
        COL_LIGHTGRAY, //WRITERSECTIONBOUNDARIES
        0x0369a3, //WRITERHEADERFOOTERMARK,
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
        0xef0fff, // CALCREFERENCE
        0xffffc0, // CALCNOTESBACKGROUND
        COL_LIGHTGRAY, // DRAWGRID
        COL_GREEN, // BASICIDENTIFIER,
        COL_GRAY,// BASICCOMMENT   ,
        COL_LIGHTRED,// BASICNUMBER    ,
        COL_LIGHTRED,// BASICSTRING    ,
        COL_BLUE, // BASICOPERATOR  ,
        COL_BLUE, // BASICKEYWORD   ,
        COL_RED, //BASICERROR
        0x009900, // SQLIDENTIFIER
        0x000000, // SQLNUMBER
        0xCE7B00, // SQLSTRING
        0x000000, // SQLOPERATOR
        0x0000E6, // SQLKEYWORD
        0x259D9D, // SQLPARAMTER
        0x969696,// SQLCOMMENT
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

    if (bSmart)
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

bool EditableColorConfig::LoadScheme(const OUString& rScheme )
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = false;
    m_pImpl->Load(rScheme);
    //the name of the loaded scheme has to be committed separately
    m_pImpl->CommitCurrentSchemeName();
    return true;
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

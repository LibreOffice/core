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

#include <map>

#include <svtools/extcolorcfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/color.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configpaths.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/poolitem.hxx>
#include <svl/hint.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/event.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>


using namespace utl;
using namespace com::sun::star;


namespace svtools
{

static sal_Int32            nExtendedColorRefCount_Impl = 0;
namespace
{
    struct ColorMutex_Impl
        : public rtl::Static< ::osl::Mutex, ColorMutex_Impl > {};
}

ExtendedColorConfig_Impl*    ExtendedColorConfig::m_pImpl = nullptr;

class ExtendedColorConfig_Impl : public utl::ConfigItem, public SfxBroadcaster
{
    typedef std::map<OUString, OUString> TDisplayNames;
    typedef std::map<OUString, ExtendedColorConfigValue> TConfigValues;
    typedef ::std::vector<TConfigValues::iterator> TMapPos;
    typedef ::std::pair< TConfigValues, TMapPos > TComponentMapping;
    typedef std::map<OUString, TComponentMapping> TComponents;
    TComponents         m_aConfigValues;
    TDisplayNames       m_aComponentDisplayNames;
    ::std::vector<TComponents::iterator> m_aConfigValuesPos;

    OUString        m_sLoadedScheme;
    bool            m_bIsBroadcastEnabled;
    static bool     m_bLockBroadcast;
    static bool     m_bBroadcastWhenUnlocked;

    uno::Sequence< OUString> GetPropertyNames(const OUString& rScheme);
    void FillComponentColors(uno::Sequence < OUString >& _rComponents,const TDisplayNames& _rDisplayNames);

    virtual void                    ImplCommit() override;

public:
    explicit ExtendedColorConfig_Impl();
    virtual ~ExtendedColorConfig_Impl() override;

    void                            Load(const OUString& rScheme);
    void                            CommitCurrentSchemeName();
    //changes the name of the current scheme but doesn't load it!
    void                            SetCurrentSchemeName(const OUString& rSchemeName) {m_sLoadedScheme = rSchemeName;}
    bool                            ExistsScheme(const OUString& _sSchemeName);
    virtual void                    Notify( const uno::Sequence<OUString>& aPropertyNames) override;

    sal_Int32                       GetComponentCount() const;
    OUString                 GetComponentName(sal_uInt32 _nPos) const;
    OUString                 GetComponentDisplayName(const OUString& _sComponentName) const;
    sal_Int32                       GetComponentColorCount(const OUString& _sName) const;
    ExtendedColorConfigValue        GetComponentColorConfigValue(const OUString& _sName,sal_uInt32 _nPos) const;

    ExtendedColorConfigValue GetColorConfigValue(const OUString& _sComponentName,const OUString& _sName)
    {
        TComponents::iterator aFind = m_aConfigValues.find(_sComponentName);
        if ( aFind != m_aConfigValues.end() )
        {
            TConfigValues::iterator aFind2 = aFind->second.first.find(_sName);
            if ( aFind2 != aFind->second.first.end() )
                return aFind2->second;
        }
#if OSL_DEBUG_LEVEL > 0
        SAL_WARN( "svtools", "Could find the required config:\n"
                  "component: " << _sComponentName
                  << "\nname: " << _sName );
#endif
        return ExtendedColorConfigValue();
    }
    void                            SetColorConfigValue(const OUString& _sName,
                                                            const ExtendedColorConfigValue& rValue );

    void                            AddScheme(const OUString& rNode);
    void                            RemoveScheme(const OUString& rNode);
    using ConfigItem::SetModified;
    using ConfigItem::ClearModified;
    void                            SettingsChanged();

    static void                     DisableBroadcast();
    static void                     EnableBroadcast();

    static void                     LockBroadcast();
    static void                     UnlockBroadcast();

    DECL_LINK( DataChangedEventListener, VclSimpleEvent&, void );
};

uno::Sequence< OUString> ExtendedColorConfig_Impl::GetPropertyNames(const OUString& rScheme)
{
    uno::Sequence< OUString> aNames(GetNodeNames(rScheme));
    for(OUString & i : aNames)
    {
        i = rScheme + "/" + i;
    }
    return aNames;
}

sal_Int32 ExtendedColorConfig_Impl::GetComponentCount() const
{
    return m_aConfigValues.size();
}

sal_Int32 ExtendedColorConfig_Impl::GetComponentColorCount(const OUString& _sName) const
{
    sal_Int32 nSize = 0;
    TComponents::const_iterator aFind = m_aConfigValues.find(_sName);
    if ( aFind != m_aConfigValues.end() )
    {
        nSize = aFind->second.first.size();
    }
    return nSize;
}

ExtendedColorConfigValue ExtendedColorConfig_Impl::GetComponentColorConfigValue(const OUString& _sName,sal_uInt32 _nPos) const
{
    TComponents::const_iterator aFind = m_aConfigValues.find(_sName);
    if ( aFind != m_aConfigValues.end() )
    {
        if ( _nPos < aFind->second.second.size() )
        {
            return aFind->second.second[_nPos]->second;
        }
    }
    return ExtendedColorConfigValue();
}

OUString ExtendedColorConfig_Impl::GetComponentDisplayName(const OUString& _sComponentName) const
{
    OUString sRet;
    TDisplayNames::const_iterator aFind = m_aComponentDisplayNames.find(_sComponentName);
    if ( aFind != m_aComponentDisplayNames.end() )
        sRet = aFind->second;
    return sRet;
}

OUString ExtendedColorConfig_Impl::GetComponentName(sal_uInt32 _nPos) const
{
    OUString sRet;
    if ( _nPos < m_aConfigValuesPos.size() )
        sRet = m_aConfigValuesPos[_nPos]->first;
    return sRet;
}

bool ExtendedColorConfig_Impl::m_bLockBroadcast = false;
bool ExtendedColorConfig_Impl::m_bBroadcastWhenUnlocked = false;
ExtendedColorConfig_Impl::ExtendedColorConfig_Impl() :
    ConfigItem("Office.ExtendedColorScheme"),
    m_bIsBroadcastEnabled(true)
{
    //try to register on the root node - if possible
    uno::Sequence < OUString > aNames(1);
    EnableNotification( aNames );
    Load(OUString());

    ::Application::AddEventListener( LINK(this, ExtendedColorConfig_Impl, DataChangedEventListener) );

}

ExtendedColorConfig_Impl::~ExtendedColorConfig_Impl()
{
    ::Application::RemoveEventListener( LINK(this, ExtendedColorConfig_Impl, DataChangedEventListener) );
}

void ExtendedColorConfig_Impl::DisableBroadcast()
{
    if ( ExtendedColorConfig::m_pImpl )
        ExtendedColorConfig::m_pImpl->m_bIsBroadcastEnabled = false;
}

void ExtendedColorConfig_Impl::EnableBroadcast()
{
    if ( ExtendedColorConfig::m_pImpl )
        ExtendedColorConfig::m_pImpl->m_bIsBroadcastEnabled = true;
}

static void lcl_addString(uno::Sequence < OUString >& _rSeq,const OUString& _sAdd)
{
    for(OUString & i : _rSeq)
        i += _sAdd;
}

void ExtendedColorConfig_Impl::Load(const OUString& rScheme)
{
    m_aComponentDisplayNames.clear();
    m_aConfigValuesPos.clear();
    m_aConfigValues.clear();

    // fill display names
    TDisplayNames aDisplayNameMap;
    uno::Sequence < OUString > aComponentNames = GetPropertyNames("EntryNames");
    OUString sDisplayName("/DisplayName");
    for(OUString & componentName : aComponentNames)
    {
        uno::Sequence < OUString > aComponentDisplayNames(1);
        aComponentDisplayNames[0] = componentName + sDisplayName;
        uno::Sequence< uno::Any > aComponentDisplayNamesValue = GetProperties( aComponentDisplayNames );
        OUString sComponentDisplayName;
        if ( aComponentDisplayNamesValue.getLength() && (aComponentDisplayNamesValue[0] >>= sComponentDisplayName) )
        {
            m_aComponentDisplayNames.emplace(componentName.getToken(1, '/'),sComponentDisplayName);
        }

        componentName += "/Entries";
        uno::Sequence < OUString > aDisplayNames = GetPropertyNames(componentName);
        lcl_addString(aDisplayNames,sDisplayName);

        uno::Sequence< uno::Any > aDisplayNamesValue = GetProperties( aDisplayNames );

        const OUString* pDispIter = aDisplayNames.getConstArray();
        const OUString* pDispEnd  = pDispIter + aDisplayNames.getLength();
        for(sal_Int32 j = 0;pDispIter != pDispEnd;++pDispIter,++j)
        {
            sal_Int32 nIndex = 0;
            pDispIter->getToken(0,'/',nIndex);
            OUString sName = pDispIter->copy(nIndex);
            sName = sName.copy(0,sName.lastIndexOf(sDisplayName));
            OUString sCurrentDisplayName;
            aDisplayNamesValue[j] >>= sCurrentDisplayName;
            aDisplayNameMap.emplace(sName,sCurrentDisplayName);
        }
    }

    // load color settings
    OUString sScheme(rScheme);

    if(sScheme.isEmpty())
    {
        //detect current scheme name
        uno::Sequence < OUString > aCurrent { "ExtendedColorScheme/CurrentColorScheme" };
        uno::Sequence< uno::Any > aCurrentVal = GetProperties( aCurrent );
        aCurrentVal.getConstArray()[0] >>= sScheme;
    } // if(!sScheme.getLength())

    m_sLoadedScheme = sScheme;
    OUString sBase = "ExtendedColorScheme/ColorSchemes/"
                   + sScheme;

    bool bFound = ExistsScheme(sScheme);
    if ( bFound )
    {
        aComponentNames = GetPropertyNames(sBase);
        FillComponentColors(aComponentNames,aDisplayNameMap);
    }

    if ( m_sLoadedScheme.isEmpty() )
        m_sLoadedScheme = "default";

    if ( sScheme != "default" )
    {
        if ( ExistsScheme("default") )
        {
            aComponentNames = GetPropertyNames("ExtendedColorScheme/ColorSchemes/default");
            FillComponentColors(aComponentNames,aDisplayNameMap);
        }
    }
    if ( !bFound && !sScheme.isEmpty() )
    {
        AddScheme(sScheme);
        CommitCurrentSchemeName();
    }
}

void ExtendedColorConfig_Impl::FillComponentColors(uno::Sequence < OUString >& _rComponents,const TDisplayNames& _rDisplayNames)
{
    const OUString sColorEntries("/Entries");
    for(OUString const & component : _rComponents)
    {
        OUString sComponentName = component.copy(component.lastIndexOf('/')+1);
        if ( m_aConfigValues.find(sComponentName) == m_aConfigValues.end() )
        {
            OUString sEntry = component + sColorEntries;

            uno::Sequence < OUString > aColorNames = GetPropertyNames(sEntry);
            uno::Sequence < OUString > aDefaultColorNames = aColorNames;

            const OUString sColor("/Color");
            const OUString sDefaultColor("/DefaultColor");
            lcl_addString(aColorNames,sColor);
            lcl_addString(aDefaultColorNames,sDefaultColor);
            uno::Sequence< uno::Any > aColors = GetProperties( aColorNames );
            const uno::Any* pColors = aColors.getConstArray();

            uno::Sequence< uno::Any > aDefaultColors = GetProperties( aDefaultColorNames );
            bool bDefaultColorFound = aDefaultColors.getLength() != 0;
            const uno::Any* pDefaultColors = aDefaultColors.getConstArray();

            OUString* pColorIter = aColorNames.getArray();
            OUString* pColorEnd  = pColorIter + aColorNames.getLength();

            m_aConfigValuesPos.push_back(m_aConfigValues.emplace(sComponentName,TComponentMapping(TConfigValues(),TMapPos())).first);
            TConfigValues& aConfigValues = (*m_aConfigValuesPos.rbegin())->second.first;
            TMapPos& aConfigValuesPos = (*m_aConfigValuesPos.rbegin())->second.second;
            for(int i = 0; pColorIter != pColorEnd; ++pColorIter ,++i)
            {
                if ( aConfigValues.find(*pColorIter) == aConfigValues.end() )
                {
                    sal_Int32 nIndex = 0;
                    pColorIter->getToken(2,'/',nIndex);
                    OUString sName(pColorIter->copy(nIndex)),sDisplayName;
                    OUString sTemp = sName.copy(0,sName.lastIndexOf(sColor));

                    TDisplayNames::const_iterator aFind = _rDisplayNames.find(sTemp);
                    sName = sName.getToken(2, '/');
                    OSL_ENSURE(aFind != _rDisplayNames.end(),"DisplayName is not in EntryNames config list!");
                    if ( aFind != _rDisplayNames.end() )
                        sDisplayName = aFind->second;

                    OSL_ENSURE(pColors[i].hasValue(),"Color config entry has NIL as color value set!");
                    OSL_ENSURE(pDefaultColors[i].hasValue(),"Color config entry has NIL as color value set!");
                    Color nColor, nDefaultColor;
                    pColors[i] >>= nColor;
                    if ( bDefaultColorFound )
                        pDefaultColors[i] >>= nDefaultColor;
                    else
                        nDefaultColor = nColor;
                    ExtendedColorConfigValue aValue(sName,sDisplayName,nColor,nDefaultColor);
                    aConfigValuesPos.push_back(aConfigValues.emplace(sName,aValue).first);
                }
            } // for(int i = 0; pColorIter != pColorEnd; ++pColorIter ,++i)
        }
    }
}

void    ExtendedColorConfig_Impl::Notify( const uno::Sequence<OUString>& /*rPropertyNames*/)
{
    //loading via notification always uses the default setting
    Load(OUString());

    SolarMutexGuard aVclGuard;

    if(m_bLockBroadcast)
    {
        m_bBroadcastWhenUnlocked = true;
    }
    else
        Broadcast(SfxHint(SfxHintId::ColorsChanged));
}

void ExtendedColorConfig_Impl::ImplCommit()
{
    if ( m_sLoadedScheme.isEmpty() )
        return;
    const OUString sColorEntries("Entries");
    const OUString sColor("/Color");
    OUString sBase = "ExtendedColorScheme/ColorSchemes/"
                   + m_sLoadedScheme;
    const OUString s_sSep("/");

    for (auto const& configValue : m_aConfigValues)
    {
        if ( ConfigItem::AddNode(sBase, configValue.first) )
        {
            OUString sNode = sBase
                           + s_sSep
                           + configValue.first
            //ConfigItem::AddNode(sNode, sColorEntries);
                           + s_sSep
                           + sColorEntries;

            uno::Sequence < beans::PropertyValue > aPropValues(configValue.second.first.size());
            beans::PropertyValue* pPropValues = aPropValues.getArray();
            for (auto const& elem : configValue.second.first)
            {
                pPropValues->Name = sNode + s_sSep + elem.first;
                ConfigItem::AddNode(sNode, elem.first);
                pPropValues->Name += sColor;
                pPropValues->Value <<= elem.second.getColor();
                // the default color will never be changed
                ++pPropValues;
            }
            SetSetProperties("ExtendedColorScheme/ColorSchemes", aPropValues);
        }
    }

    CommitCurrentSchemeName();
}

void ExtendedColorConfig_Impl::CommitCurrentSchemeName()
{
    //save current scheme name
    uno::Sequence < OUString > aCurrent { "ExtendedColorScheme/CurrentColorScheme" };
    uno::Sequence< uno::Any > aCurrentVal(1);
    aCurrentVal.getArray()[0] <<= m_sLoadedScheme;
    PutProperties(aCurrent, aCurrentVal);
}

bool ExtendedColorConfig_Impl::ExistsScheme(const OUString& _sSchemeName)
{
    OUString sBase("ExtendedColorScheme/ColorSchemes");

    uno::Sequence < OUString > aComponentNames = GetPropertyNames(sBase);
    sBase += "/" + _sSchemeName;
    const OUString* pCompIter = aComponentNames.getConstArray();
    const OUString* pCompEnd  = pCompIter + aComponentNames.getLength();
    for(;pCompIter != pCompEnd && *pCompIter != sBase;++pCompIter)
        ;
    return pCompIter != pCompEnd;
}

void ExtendedColorConfig_Impl::SetColorConfigValue(const OUString& _sName, const ExtendedColorConfigValue& rValue )
{
    TComponents::iterator aFind = m_aConfigValues.find(_sName);
    if ( aFind != m_aConfigValues.end() )
    {
        TConfigValues::iterator aFind2 = aFind->second.first.find(rValue.getName());
        if ( aFind2 != aFind->second.first.end() )
            aFind2->second = rValue;
        SetModified();
    }
}

void ExtendedColorConfig_Impl::AddScheme(const OUString& rScheme)
{
    if(ConfigItem::AddNode("ExtendedColorScheme/ColorSchemes", rScheme))
    {
        m_sLoadedScheme = rScheme;
        Commit();
    }
}

void ExtendedColorConfig_Impl::RemoveScheme(const OUString& rScheme)
{
    uno::Sequence< OUString > aElements { rScheme };
    ClearNodeElements("ExtendedColorScheme/ColorSchemes", aElements);
}

void ExtendedColorConfig_Impl::SettingsChanged()
{
    SolarMutexGuard aVclGuard;

    Broadcast( SfxHint( SfxHintId::ColorsChanged ) );
}

void ExtendedColorConfig_Impl::LockBroadcast()
{
    m_bLockBroadcast = true;
}

void ExtendedColorConfig_Impl::UnlockBroadcast()
{
    if ( m_bBroadcastWhenUnlocked )
    {
        m_bBroadcastWhenUnlocked = ExtendedColorConfig::m_pImpl != nullptr;
        if ( m_bBroadcastWhenUnlocked )
        {
            if (ExtendedColorConfig::m_pImpl->m_bIsBroadcastEnabled)
            {
                m_bBroadcastWhenUnlocked = false;
                ExtendedColorConfig::m_pImpl->Broadcast(SfxHint(SfxHintId::ColorsChanged));
            }
        }
    }
    m_bLockBroadcast = false;
}

IMPL_LINK( ExtendedColorConfig_Impl, DataChangedEventListener, VclSimpleEvent&, rEvent, void )
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


ExtendedColorConfig::ExtendedColorConfig()
{
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    if ( !m_pImpl )
        m_pImpl = new ExtendedColorConfig_Impl;
    ++nExtendedColorRefCount_Impl;
    StartListening( *m_pImpl);
}

ExtendedColorConfig::~ExtendedColorConfig()
{
    ::osl::MutexGuard aGuard( ColorMutex_Impl::get() );
    EndListening( *m_pImpl);
    if(!--nExtendedColorRefCount_Impl)
    {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

ExtendedColorConfigValue ExtendedColorConfig::GetColorValue(const OUString& _sComponentName,const OUString& _sName)const
{
    return m_pImpl->GetColorConfigValue(_sComponentName,_sName);
}

sal_Int32 ExtendedColorConfig::GetComponentCount() const
{
    return m_pImpl->GetComponentCount();
}

sal_Int32 ExtendedColorConfig::GetComponentColorCount(const OUString& _sName) const
{
    return m_pImpl->GetComponentColorCount(_sName);
}

ExtendedColorConfigValue ExtendedColorConfig::GetComponentColorConfigValue(const OUString& _sName,sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentColorConfigValue(_sName,_nPos);
}

OUString ExtendedColorConfig::GetComponentName(sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentName(_nPos);
}

OUString ExtendedColorConfig::GetComponentDisplayName(const OUString& _sComponentName) const
{
    return m_pImpl->GetComponentDisplayName(_sComponentName);
}

void ExtendedColorConfig::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
{
    SolarMutexGuard aVclGuard;

    Broadcast( rHint );
}

EditableExtendedColorConfig::EditableExtendedColorConfig() :
    m_pImpl(new ExtendedColorConfig_Impl),
    m_bModified(false)
{
    ExtendedColorConfig_Impl::LockBroadcast();
}

EditableExtendedColorConfig::~EditableExtendedColorConfig()
{
    ExtendedColorConfig_Impl::UnlockBroadcast();
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
}

void EditableExtendedColorConfig::DeleteScheme(const OUString& rScheme )
{
    m_pImpl->RemoveScheme(rScheme);
}

void EditableExtendedColorConfig::AddScheme(const OUString& rScheme )
{
    m_pImpl->AddScheme(rScheme);
}

void EditableExtendedColorConfig::LoadScheme(const OUString& rScheme )
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

// Changes the name of the current scheme but doesn't load it!
void EditableExtendedColorConfig::SetCurrentSchemeName(const OUString& rScheme)
{
    m_pImpl->SetCurrentSchemeName(rScheme);
    m_pImpl->CommitCurrentSchemeName();
}

void EditableExtendedColorConfig::SetColorValue(
    const OUString& _sName, const ExtendedColorConfigValue& rValue)
{
    m_pImpl->SetColorConfigValue(_sName, rValue);
    m_pImpl->ClearModified();
    m_bModified = true;
}

void EditableExtendedColorConfig::SetModified()
{
    m_bModified = true;
}

void EditableExtendedColorConfig::Commit()
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = false;
}

void EditableExtendedColorConfig::DisableBroadcast()
{
    ExtendedColorConfig_Impl::DisableBroadcast();
}

void EditableExtendedColorConfig::EnableBroadcast()
{
    ExtendedColorConfig_Impl::EnableBroadcast();
}

sal_Int32 EditableExtendedColorConfig::GetComponentCount() const
{
    return m_pImpl->GetComponentCount();
}

sal_Int32 EditableExtendedColorConfig::GetComponentColorCount(const OUString& _sName) const
{
    return m_pImpl->GetComponentColorCount(_sName);
}

ExtendedColorConfigValue EditableExtendedColorConfig::GetComponentColorConfigValue(const OUString& _sName,sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentColorConfigValue(_sName,_nPos);
}

OUString EditableExtendedColorConfig::GetComponentName(sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentName(_nPos);
}
}//namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

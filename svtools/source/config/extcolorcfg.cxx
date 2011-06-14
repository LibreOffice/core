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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/extcolorcfg.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <unotools/configitem.hxx>
#include <unotools/configpathes.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svl/poolitem.hxx> //Any2Bool
#include <svl/smplhint.hxx>
#include <osl/mutex.hxx>

 /* #100822# ----
#include <vcl/wrkwin.hxx>
 ------------- */
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>
#include <comphelper/stl_types.hxx>


//-----------------------------------------------------------------------------
using namespace utl;
using namespace com::sun::star;

using ::rtl::OUString;

namespace svtools
{

#define C2U(cChar) OUString::createFromAscii(cChar)
sal_Int32            nExtendedColorRefCount_Impl = 0;
namespace
{
    struct ColorMutex_Impl
        : public rtl::Static< ::osl::Mutex, ColorMutex_Impl > {};
}

ExtendedColorConfig_Impl*    ExtendedColorConfig::m_pImpl = NULL;

class ExtendedColorConfig_Impl : public utl::ConfigItem, public SfxBroadcaster
{
    DECLARE_STL_USTRINGACCESS_MAP( ::rtl::OUString, TDisplayNames);
    DECLARE_STL_USTRINGACCESS_MAP(ExtendedColorConfigValue,TConfigValues);
    typedef ::std::vector<TConfigValues::iterator> TMapPos;
    typedef ::std::pair< TConfigValues, TMapPos > TComponentMapping;
    DECLARE_STL_USTRINGACCESS_MAP(TComponentMapping,TComponents);
    TComponents         m_aConfigValues;
    TDisplayNames       m_aComponentDisplayNames;
    ::std::vector<TComponents::iterator> m_aConfigValuesPos;

    sal_Bool            m_bEditMode;
    rtl::OUString       m_sLoadedScheme;
    sal_Bool            m_bIsBroadcastEnabled;
    static sal_Bool     m_bLockBroadcast;
    static sal_Bool     m_bBroadcastWhenUnlocked;

    uno::Sequence< ::rtl::OUString> GetPropertyNames(const rtl::OUString& rScheme);
    void FillComponentColors(uno::Sequence < ::rtl::OUString >& _rComponents,const TDisplayNames& _rDisplayNames);
public:
    ExtendedColorConfig_Impl(sal_Bool bEditMode = sal_False);
    virtual ~ExtendedColorConfig_Impl();

    void                            Load(const rtl::OUString& rScheme);
    void                            CommitCurrentSchemeName();
    //changes the name of the current scheme but doesn't load it!
    void                            SetCurrentSchemeName(const rtl::OUString& rSchemeName) {m_sLoadedScheme = rSchemeName;}
    sal_Bool                        ExistsScheme(const ::rtl::OUString& _sSchemeName);
    virtual void                    Commit();
    virtual void                    Notify( const uno::Sequence<rtl::OUString>& aPropertyNames);

    sal_Int32                       GetComponentCount() const;
    ::rtl::OUString                 GetComponentName(sal_uInt32 _nPos) const;
    ::rtl::OUString                 GetComponentDisplayName(const ::rtl::OUString& _sComponentName) const;
    sal_Int32                       GetComponentColorCount(const ::rtl::OUString& _sName) const;
    ExtendedColorConfigValue        GetComponentColorConfigValue(const ::rtl::OUString& _sName,sal_uInt32 _nPos) const;

    ExtendedColorConfigValue GetColorConfigValue(const ::rtl::OUString& _sComponentName,const ::rtl::OUString& _sName)
    {
        TComponents::iterator aFind = m_aConfigValues.find(_sComponentName);
        if ( aFind != m_aConfigValues.end() )
        {
            TConfigValues::iterator aFind2 = aFind->second.first.find(_sName);
            if ( aFind2 != aFind->second.first.end() )
                return aFind2->second;
        }
#if OSL_DEBUG_LEVEL > 0
        ::rtl::OStringBuffer aMessage( "Could find the required config:\n" );
        aMessage.append( "component: " );
        aMessage.append( ::rtl::OUStringToOString( _sComponentName, RTL_TEXTENCODING_UTF8 ) );
        aMessage.append( "\nname: " );
        aMessage.append( ::rtl::OUStringToOString( _sName, RTL_TEXTENCODING_UTF8 ) );
        OSL_FAIL( aMessage.makeStringAndClear().getStr() );
#endif
        return ExtendedColorConfigValue();
    }
    void                            SetColorConfigValue(const ::rtl::OUString& _sName,
                                                            const ExtendedColorConfigValue& rValue );

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
};

uno::Sequence< OUString> ExtendedColorConfig_Impl::GetPropertyNames(const rtl::OUString& rScheme)
{
    uno::Sequence< OUString> aNames(GetNodeNames(rScheme));
    ::rtl::OUString* pIter = aNames.getArray();
    ::rtl::OUString* pEnd    = pIter + aNames.getLength();
    ::rtl::OUString sSep(RTL_CONSTASCII_USTRINGPARAM("/"));
    for(;pIter != pEnd;++pIter)
    {
        *pIter = rScheme + sSep + *pIter;
    }
    return aNames;
}
// -----------------------------------------------------------------------------
sal_Int32 ExtendedColorConfig_Impl::GetComponentCount() const
{
    return m_aConfigValues.size();
}
// -----------------------------------------------------------------------------
sal_Int32 ExtendedColorConfig_Impl::GetComponentColorCount(const ::rtl::OUString& _sName) const
{
    sal_Int32 nSize = 0;
    TComponents::const_iterator aFind = m_aConfigValues.find(_sName);
    if ( aFind != m_aConfigValues.end() )
    {
        nSize = aFind->second.first.size();
    }
    return nSize;
}
// -----------------------------------------------------------------------------
ExtendedColorConfigValue ExtendedColorConfig_Impl::GetComponentColorConfigValue(const ::rtl::OUString& _sName,sal_uInt32 _nPos) const
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
// -----------------------------------------------------------------------------
::rtl::OUString ExtendedColorConfig_Impl::GetComponentDisplayName(const ::rtl::OUString& _sComponentName) const
{
    ::rtl::OUString sRet;
    TDisplayNames::const_iterator aFind = m_aComponentDisplayNames.find(_sComponentName);
    if ( aFind != m_aComponentDisplayNames.end() )
        sRet = aFind->second;
    return sRet;
}
// -----------------------------------------------------------------------------
::rtl::OUString ExtendedColorConfig_Impl::GetComponentName(sal_uInt32 _nPos) const
{
    ::rtl::OUString sRet;
    if ( _nPos < m_aConfigValuesPos.size() )
        sRet = m_aConfigValuesPos[_nPos]->first;
    return sRet;
}

sal_Bool ExtendedColorConfig_Impl::m_bLockBroadcast = sal_False;
sal_Bool ExtendedColorConfig_Impl::m_bBroadcastWhenUnlocked = sal_False;
ExtendedColorConfig_Impl::ExtendedColorConfig_Impl(sal_Bool bEditMode) :
    ConfigItem(C2U("Office.ExtendedColorScheme")),
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

    // #100822#
    ::Application::AddEventListener( LINK(this, ExtendedColorConfig_Impl, DataChangedEventListener) );

}

ExtendedColorConfig_Impl::~ExtendedColorConfig_Impl()
{
    // #100822#
    ::Application::RemoveEventListener( LINK(this, ExtendedColorConfig_Impl, DataChangedEventListener) );
}
// -----------------------------------------------------------------------------
void ExtendedColorConfig_Impl::DisableBroadcast()
{
    if ( ExtendedColorConfig::m_pImpl )
        ExtendedColorConfig::m_pImpl->m_bIsBroadcastEnabled = sal_False;
}
// -----------------------------------------------------------------------------
void ExtendedColorConfig_Impl::EnableBroadcast()
{
    if ( ExtendedColorConfig::m_pImpl )
        ExtendedColorConfig::m_pImpl->m_bIsBroadcastEnabled = sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool ExtendedColorConfig_Impl::IsEnableBroadcast()
{
    return ExtendedColorConfig::m_pImpl ? ExtendedColorConfig::m_pImpl->m_bIsBroadcastEnabled : sal_False;
}

void lcl_addString(uno::Sequence < ::rtl::OUString >& _rSeq,const ::rtl::OUString& _sAdd)
{
    ::rtl::OUString* pIter = _rSeq.getArray();
    ::rtl::OUString* pEnd  = pIter + _rSeq.getLength();
    for(;pIter != pEnd;++pIter)
        *pIter += _sAdd;
}
// -----------------------------------------------------------------------------
void ExtendedColorConfig_Impl::Load(const rtl::OUString& rScheme)
{
    m_aComponentDisplayNames.clear();
    m_aConfigValuesPos.clear();
    m_aConfigValues.clear();

    // fill display names
    TDisplayNames aDisplayNameMap;
    ::rtl::OUString sEntryNames(RTL_CONSTASCII_USTRINGPARAM("EntryNames"));
    uno::Sequence < ::rtl::OUString > aComponentNames = GetPropertyNames(sEntryNames);
    ::rtl::OUString sDisplayName(RTL_CONSTASCII_USTRINGPARAM("/DisplayName"));
    ::rtl::OUString* pIter = aComponentNames.getArray();
    ::rtl::OUString* pEnd  = pIter + aComponentNames.getLength();
    for(sal_Int32 i = 0;pIter != pEnd;++pIter,++i)
    {
        uno::Sequence < ::rtl::OUString > aComponentDisplayNames(1);
        aComponentDisplayNames[0] = *pIter;
        aComponentDisplayNames[0] += sDisplayName;
        uno::Sequence< uno::Any > aComponentDisplayNamesValue = GetProperties( aComponentDisplayNames );
        ::rtl::OUString sComponentDisplayName;
        if ( aComponentDisplayNamesValue.getLength() && (aComponentDisplayNamesValue[0] >>= sComponentDisplayName) )
        {
            sal_Int32 nIndex = 0;
            m_aComponentDisplayNames.insert(TDisplayNames::value_type(pIter->getToken(1,'/',nIndex),sComponentDisplayName));
        }

        *pIter += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/Entries"));
        uno::Sequence < ::rtl::OUString > aDisplayNames = GetPropertyNames(*pIter);
        lcl_addString(aDisplayNames,sDisplayName);

        uno::Sequence< uno::Any > aDisplayNamesValue = GetProperties( aDisplayNames );

        const ::rtl::OUString* pDispIter = aDisplayNames.getConstArray();
        const ::rtl::OUString* pDispEnd  = pDispIter + aDisplayNames.getLength();
        for(sal_Int32 j = 0;pDispIter != pDispEnd;++pDispIter,++j)
        {
            sal_Int32 nIndex = 0;
            pDispIter->getToken(0,'/',nIndex);
            ::rtl::OUString sName = pDispIter->copy(nIndex);
            sName = sName.copy(0,sName.lastIndexOf(sDisplayName));
            ::rtl::OUString sCurrentDisplayName;
            aDisplayNamesValue[j] >>= sCurrentDisplayName;
            aDisplayNameMap.insert(TDisplayNames::value_type(sName,sCurrentDisplayName));
        }
    }

    // load color settings
    rtl::OUString sScheme(rScheme);

    if(!sScheme.getLength())
    {
        //detect current scheme name
        uno::Sequence < ::rtl::OUString > aCurrent(1);
        aCurrent.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ExtendedColorScheme/CurrentColorScheme"));
        uno::Sequence< uno::Any > aCurrentVal = GetProperties( aCurrent );
        aCurrentVal.getConstArray()[0] >>= sScheme;
    } // if(!sScheme.getLength())

    m_sLoadedScheme = sScheme;
    ::rtl::OUString sBase(RTL_CONSTASCII_USTRINGPARAM("ExtendedColorScheme/ColorSchemes/"));
    sBase += sScheme;

    sal_Bool bFound = ExistsScheme(sScheme);
    if ( bFound )
    {
        aComponentNames = GetPropertyNames(sBase);
        FillComponentColors(aComponentNames,aDisplayNameMap);
    }

    if ( !m_sLoadedScheme.getLength() )
        m_sLoadedScheme = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("default"));

    if ( !sScheme.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("default")) )
    {
        ::rtl::OUString sDefault(RTL_CONSTASCII_USTRINGPARAM("default"));
        if ( ExistsScheme(sDefault) )
        {
            ::rtl::OUString sBaseDefault(RTL_CONSTASCII_USTRINGPARAM("ExtendedColorScheme/ColorSchemes/default"));
            aComponentNames = GetPropertyNames(sBaseDefault);
            FillComponentColors(aComponentNames,aDisplayNameMap);
        }
    }
    if ( !bFound && sScheme.getLength() )
    {
        AddScheme(sScheme);
        CommitCurrentSchemeName();
    }
}
// -----------------------------------------------------------------------------
void ExtendedColorConfig_Impl::FillComponentColors(uno::Sequence < ::rtl::OUString >& _rComponents,const TDisplayNames& _rDisplayNames)
{
    const ::rtl::OUString sColorEntries(RTL_CONSTASCII_USTRINGPARAM("/Entries"));
    ::rtl::OUString* pIter = _rComponents.getArray();
    ::rtl::OUString* pEnd  = pIter + _rComponents.getLength();
    for(;pIter != pEnd;++pIter)
    {
        ::rtl::OUString sComponentName = pIter->copy(pIter->lastIndexOf('/')+1);
        if ( m_aConfigValues.find(sComponentName) == m_aConfigValues.end() )
        {
            ::rtl::OUString sEntry = *pIter;
            sEntry += sColorEntries;

            uno::Sequence < ::rtl::OUString > aColorNames = GetPropertyNames(sEntry);
            uno::Sequence < ::rtl::OUString > aDefaultColorNames = aColorNames;

            const ::rtl::OUString sColor(RTL_CONSTASCII_USTRINGPARAM("/Color"));
            const ::rtl::OUString sDefaultColor(RTL_CONSTASCII_USTRINGPARAM("/DefaultColor"));
            lcl_addString(aColorNames,sColor);
            lcl_addString(aDefaultColorNames,sDefaultColor);
            uno::Sequence< uno::Any > aColors = GetProperties( aColorNames );
            const uno::Any* pColors = aColors.getConstArray();

            uno::Sequence< uno::Any > aDefaultColors = GetProperties( aDefaultColorNames );
            bool bDefaultColorFound = aDefaultColors.getLength() != 0;
            const uno::Any* pDefaultColors = aDefaultColors.getConstArray();

            ::rtl::OUString* pColorIter = aColorNames.getArray();
            ::rtl::OUString* pColorEnd  = pColorIter + aColorNames.getLength();

            m_aConfigValuesPos.push_back(m_aConfigValues.insert(TComponents::value_type(sComponentName,TComponentMapping(TConfigValues(),TMapPos()))).first);
            TConfigValues& aConfigValues = (*m_aConfigValuesPos.rbegin())->second.first;
            TMapPos& aConfigValuesPos = (*m_aConfigValuesPos.rbegin())->second.second;
            for(int i = 0; pColorIter != pColorEnd; ++pColorIter ,++i)
            {
                if ( aConfigValues.find(*pColorIter) == aConfigValues.end() )
                {
                    sal_Int32 nIndex = 0;
                    pColorIter->getToken(2,'/',nIndex);
                    ::rtl::OUString sName(pColorIter->copy(nIndex)),sDisplayName;
                    ::rtl::OUString sTemp = sName.copy(0,sName.lastIndexOf(sColor));

                    TDisplayNames::const_iterator aFind = _rDisplayNames.find(sTemp);
                    nIndex = 0;
                    sName = sName.getToken(2,'/',nIndex);
                    OSL_ENSURE(aFind != _rDisplayNames.end(),"DisplayName is not in EntryNames config list!");
                    if ( aFind != _rDisplayNames.end() )
                        sDisplayName = aFind->second;

                    OSL_ENSURE(pColors[i].hasValue(),"Color config entry has NIL as color value set!");
                    OSL_ENSURE(pDefaultColors[i].hasValue(),"Color config entry has NIL as color value set!");
                    sal_Int32 nColor = 0,nDefaultColor = 0;
                    pColors[i] >>= nColor;
                    if ( bDefaultColorFound )
                        pDefaultColors[i] >>= nDefaultColor;
                    else
                        nDefaultColor = nColor;
                    ExtendedColorConfigValue aValue(sName,sDisplayName,nColor,nDefaultColor);
                    aConfigValuesPos.push_back(aConfigValues.insert(TConfigValues::value_type(sName,aValue)).first);
                }
            } // for(int i = 0; pColorIter != pColorEnd; ++pColorIter ,++i)
        }
    }
}

void    ExtendedColorConfig_Impl::Notify( const uno::Sequence<OUString>& /*rPropertyNames*/)
{
    //loading via notification always uses the default setting
    Load(::rtl::OUString());

    SolarMutexGuard aVclGuard;

    if(m_bLockBroadcast)
    {
        m_bBroadcastWhenUnlocked = sal_True;
    }
    else
        Broadcast(SfxSimpleHint(SFX_HINT_COLORS_CHANGED));
}

void ExtendedColorConfig_Impl::Commit()
{
    if ( !m_sLoadedScheme.getLength() )
        return;
    const ::rtl::OUString sColorEntries(RTL_CONSTASCII_USTRINGPARAM("Entries"));
    const ::rtl::OUString sColor(RTL_CONSTASCII_USTRINGPARAM("/Color"));
    const ::rtl::OUString sDefaultColor(RTL_CONSTASCII_USTRINGPARAM("/DefaultColor"));
    ::rtl::OUString sBase(RTL_CONSTASCII_USTRINGPARAM("ExtendedColorScheme/ColorSchemes/"));
    const ::rtl::OUString s_sSep(RTL_CONSTASCII_USTRINGPARAM("/"));
    sBase += m_sLoadedScheme;

    TComponents::iterator aIter = m_aConfigValues.begin();
    TComponents::iterator aEnd = m_aConfigValues.end();
    for( ;aIter != aEnd;++aIter )
    {
        ::rtl::OUString sEntry = aIter->first;
        sEntry += sColorEntries;

        if ( ConfigItem::AddNode(sBase, aIter->first) )
        {
            rtl::OUString sNode = sBase;
            sNode += s_sSep;
            sNode += aIter->first;
            //ConfigItem::AddNode(sNode, sColorEntries);
            sNode += s_sSep;
            sNode += sColorEntries;

            uno::Sequence < beans::PropertyValue > aPropValues(aIter->second.first.size());
            beans::PropertyValue* pPropValues = aPropValues.getArray();
            TConfigValues::iterator aConIter = aIter->second.first.begin();
            TConfigValues::iterator aConEnd  = aIter->second.first.end();
            for (; aConIter != aConEnd; ++aConIter,++pPropValues)
            {
                pPropValues->Name = sNode + s_sSep + aConIter->first;
                ConfigItem::AddNode(sNode, aConIter->first);
                pPropValues->Name += sColor;
                pPropValues->Value <<= aConIter->second.getColor();
                // the default color will never be changed
            }
            ::rtl::OUString s(RTL_CONSTASCII_USTRINGPARAM("ExtendedColorScheme/ColorSchemes"));
            SetSetProperties(s, aPropValues);
        }
    }

    CommitCurrentSchemeName();
}

void ExtendedColorConfig_Impl::CommitCurrentSchemeName()
{
    //save current scheme name
    uno::Sequence < ::rtl::OUString > aCurrent(1);
    aCurrent.getArray()[0] = C2U("ExtendedColorScheme/CurrentColorScheme");
    uno::Sequence< uno::Any > aCurrentVal(1);
    aCurrentVal.getArray()[0] <<= m_sLoadedScheme;
    PutProperties(aCurrent, aCurrentVal);
}
// -----------------------------------------------------------------------------
sal_Bool ExtendedColorConfig_Impl::ExistsScheme(const ::rtl::OUString& _sSchemeName)
{
    ::rtl::OUString sBase(RTL_CONSTASCII_USTRINGPARAM("ExtendedColorScheme/ColorSchemes"));

    uno::Sequence < ::rtl::OUString > aComponentNames = GetPropertyNames(sBase);
    sBase += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + _sSchemeName;
    const ::rtl::OUString* pCompIter = aComponentNames.getConstArray();
    const ::rtl::OUString* pCompEnd  = pCompIter + aComponentNames.getLength();
    for(;pCompIter != pCompEnd && *pCompIter != sBase;++pCompIter)
        ;
    return pCompIter != pCompEnd;
}

void ExtendedColorConfig_Impl::SetColorConfigValue(const ::rtl::OUString& _sName, const ExtendedColorConfigValue& rValue )
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

uno::Sequence< ::rtl::OUString> ExtendedColorConfig_Impl::GetSchemeNames()
{
    return GetNodeNames(C2U("ExtendedColorScheme/ColorSchemes"));
}

sal_Bool ExtendedColorConfig_Impl::AddScheme(const rtl::OUString& rScheme)
{
    if(ConfigItem::AddNode(C2U("ExtendedColorScheme/ColorSchemes"), rScheme))
    {
        m_sLoadedScheme = rScheme;
        Commit();
        return sal_True;
    }
    return sal_False;
}

sal_Bool ExtendedColorConfig_Impl::RemoveScheme(const rtl::OUString& rScheme)
{
    uno::Sequence< rtl::OUString > aElements(1);
    aElements.getArray()[0] = rScheme;
    return ClearNodeElements(C2U("ExtendedColorScheme/ColorSchemes"), aElements);
}

void ExtendedColorConfig_Impl::SettingsChanged()
{
    SolarMutexGuard aVclGuard;

    Broadcast( SfxSimpleHint( SFX_HINT_COLORS_CHANGED ) );
}

void ExtendedColorConfig_Impl::LockBroadcast()
{
    m_bLockBroadcast = sal_True;
}

void ExtendedColorConfig_Impl::UnlockBroadcast()
{
    if ( m_bBroadcastWhenUnlocked )
    {
        m_bBroadcastWhenUnlocked = ExtendedColorConfig::m_pImpl != NULL;
        if ( m_bBroadcastWhenUnlocked )
        {
            if ( ExtendedColorConfig::m_pImpl->IsEnableBroadcast() )
            {
                m_bBroadcastWhenUnlocked = sal_False;
                ExtendedColorConfig::m_pImpl->Broadcast(SfxSimpleHint(SFX_HINT_COLORS_CHANGED));
            }
        }
    }
    m_bLockBroadcast = sal_False;
}

IMPL_LINK( ExtendedColorConfig_Impl, DataChangedEventListener, VclWindowEvent*, pEvent )
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

// ---------------------------------------------------------------------------

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
        m_pImpl = 0;
    }
}

ExtendedColorConfigValue ExtendedColorConfig::GetColorValue(const ::rtl::OUString& _sComponentName,const ::rtl::OUString& _sName)const
{
    return m_pImpl->GetColorConfigValue(_sComponentName,_sName);
}
// -----------------------------------------------------------------------------
sal_Int32 ExtendedColorConfig::GetComponentCount() const
{
    return m_pImpl->GetComponentCount();
}
// -----------------------------------------------------------------------------
sal_Int32 ExtendedColorConfig::GetComponentColorCount(const ::rtl::OUString& _sName) const
{
    return m_pImpl->GetComponentColorCount(_sName);
}
// -----------------------------------------------------------------------------
ExtendedColorConfigValue ExtendedColorConfig::GetComponentColorConfigValue(const ::rtl::OUString& _sName,sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentColorConfigValue(_sName,_nPos);
}
// -----------------------------------------------------------------------------
::rtl::OUString ExtendedColorConfig::GetComponentName(sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentName(_nPos);
}
// -----------------------------------------------------------------------------
::rtl::OUString ExtendedColorConfig::GetComponentDisplayName(const ::rtl::OUString& _sComponentName) const
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
    m_bModified(sal_False)
{
    m_pImpl->LockBroadcast();
}

EditableExtendedColorConfig::~EditableExtendedColorConfig()
{
    m_pImpl->UnlockBroadcast();
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    delete m_pImpl;
}


uno::Sequence< ::rtl::OUString >  EditableExtendedColorConfig::GetSchemeNames() const
{
    return m_pImpl->GetSchemeNames();
}

void EditableExtendedColorConfig::DeleteScheme(const ::rtl::OUString& rScheme )
{
    m_pImpl->RemoveScheme(rScheme);
}

void EditableExtendedColorConfig::AddScheme(const ::rtl::OUString& rScheme )
{
    m_pImpl->AddScheme(rScheme);
}

sal_Bool EditableExtendedColorConfig::LoadScheme(const ::rtl::OUString& rScheme )
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

const ::rtl::OUString& EditableExtendedColorConfig::GetCurrentSchemeName()const
{
    return m_pImpl->GetLoadedScheme();
}

// Changes the name of the current scheme but doesn't load it!
void EditableExtendedColorConfig::SetCurrentSchemeName(const ::rtl::OUString& rScheme)
{
    m_pImpl->SetCurrentSchemeName(rScheme);
    m_pImpl->CommitCurrentSchemeName();
}

ExtendedColorConfigValue EditableExtendedColorConfig::GetColorValue(const ::rtl::OUString& _sComponentName,
    const ::rtl::OUString& _sName)const
{
    return m_pImpl->GetColorConfigValue(_sComponentName,_sName);
}

void EditableExtendedColorConfig::SetColorValue(
    const ::rtl::OUString& _sName, const ExtendedColorConfigValue& rValue)
{
    m_pImpl->SetColorConfigValue(_sName, rValue);
    m_pImpl->ClearModified();
    m_bModified = sal_True;
}

void EditableExtendedColorConfig::SetModified()
{
    m_bModified = sal_True;
}

void EditableExtendedColorConfig::Commit()
{
    if(m_bModified)
        m_pImpl->SetModified();
    if(m_pImpl->IsModified())
        m_pImpl->Commit();
    m_bModified = sal_False;
}
// -----------------------------------------------------------------------------
void EditableExtendedColorConfig::DisableBroadcast()
{
    m_pImpl->DisableBroadcast();
}
// -----------------------------------------------------------------------------
void EditableExtendedColorConfig::EnableBroadcast()
{
    m_pImpl->EnableBroadcast();
}
// -----------------------------------------------------------------------------
sal_Int32 EditableExtendedColorConfig::GetComponentCount() const
{
    return m_pImpl->GetComponentCount();
}
// -----------------------------------------------------------------------------
sal_Int32 EditableExtendedColorConfig::GetComponentColorCount(const ::rtl::OUString& _sName) const
{
    return m_pImpl->GetComponentColorCount(_sName);
}
// -----------------------------------------------------------------------------
ExtendedColorConfigValue EditableExtendedColorConfig::GetComponentColorConfigValue(const ::rtl::OUString& _sName,sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentColorConfigValue(_sName,_nPos);
}
// -----------------------------------------------------------------------------
::rtl::OUString EditableExtendedColorConfig::GetComponentName(sal_uInt32 _nPos) const
{
    return m_pImpl->GetComponentName(_nPos);
}
// -----------------------------------------------------------------------------
::rtl::OUString EditableExtendedColorConfig::GetComponentDisplayName(const ::rtl::OUString& _sComponentName) const
{
    return m_pImpl->GetComponentDisplayName(_sComponentName);
}
}//namespace svtools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

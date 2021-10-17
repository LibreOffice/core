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

#include <svtools/miscopt.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <comphelper/sequence.hxx>
#include <tools/link.hxx>
#include <osl/diagnose.h>

#include <rtl/instance.hxx>
#include "itemholder2.hxx"

#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <vector>

using namespace ::utl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star;

constexpr OUStringLiteral ROOTNODE_MISC = u"Office.Common/Misc";

// PROPERTYHANDLE defines must be sequential from zero for Commit/Load
constexpr OUStringLiteral PROPERTYNAME_SYMBOLSET = u"SymbolSet";
#define PROPERTYHANDLE_SYMBOLSET                0
constexpr OUStringLiteral PROPERTYNAME_ICONTHEME = u"SymbolStyle";
#define PROPERTYHANDLE_SYMBOLSTYLE              1

class SvtMiscOptions_Impl : public ConfigItem
{
private:
    ::std::vector<Link<LinkParamNone*,void>> aList;
    sal_Int16   m_nSymbolsSize;
    bool        m_bIsSymbolsSizeRO;
    bool        m_bIsSymbolsStyleRO;
    bool        m_bIconThemeWasSetAutomatically;

        virtual void ImplCommit() override;

public:

         SvtMiscOptions_Impl();
        virtual ~SvtMiscOptions_Impl() override;

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      This method is called from the ConfigManager before the application ends or from the
                        PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames ) override;

        /** loads required data from the configuration. It's called in the constructor to
         read all entries and form ::Notify to re-read changed settings

         */
        void Load( const Sequence< OUString >& rPropertyNames );

        //  public interface

        sal_Int16 GetSymbolsSize() const
        { return m_nSymbolsSize; }

        void SetSymbolsSize( sal_Int16 nSet );

        static OUString GetIconTheme();

        enum class SetModifiedFlag { SET, DONT_SET };

        /** Set the icon theme
         *
         * @param theme
         * The name of the icon theme to use.
         *
         * @param setModified
         * Whether to call SetModified() and CallListeners().
         *
         * @internal
         * The @p setModified flag was introduced because the unittests fail if we call SetModified()
         * during initialization in the constructor.
         */
        void
        SetIconTheme(const OUString &theme, SetModifiedFlag setModified );

        bool IconThemeWasSetAutomatically() const
        {return m_bIconThemeWasSetAutomatically;}

        void AddListenerLink( const Link<LinkParamNone*,void>& rLink );
        void RemoveListenerLink( const Link<LinkParamNone*,void>& rLink );
        void CallListeners();


    //  private methods


private:

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent our module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.
            @return     A list of needed configuration keys is returned.
        *//*-*****************************************************************************************************/

        static Sequence< OUString > GetPropertyNames();
};


//  constructor

SvtMiscOptions_Impl::SvtMiscOptions_Impl()
    // Init baseclasses first
    : ConfigItem( ROOTNODE_MISC )

    , m_nSymbolsSize( 0 )
    , m_bIsSymbolsSizeRO( false )
    , m_bIsSymbolsStyleRO( false )
    , m_bIconThemeWasSetAutomatically( false )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = GetPropertyNames  (           );
    Load( seqNames );
    Sequence< Any >         seqValues   = GetProperties     ( seqNames  );
    Sequence< sal_Bool >    seqRO       = GetReadOnlyStates ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtMiscOptions_Impl::SvtMiscOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if (!seqValues[nProperty].hasValue())
            continue;
        switch( nProperty )
        {
            case PROPERTYHANDLE_SYMBOLSET :
            {
                if( !(seqValues[nProperty] >>= m_nSymbolsSize) )
                {
                    OSL_FAIL("Wrong type of \"Misc\\SymbolSet\"!" );
                }
                m_bIsSymbolsSizeRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_SYMBOLSTYLE :
            {
                OUString aIconTheme;
                if (seqValues[nProperty] >>= aIconTheme)
                    SetIconTheme(aIconTheme, SetModifiedFlag::DONT_SET);
                else
                    OSL_FAIL("Wrong type of \"Misc\\SymbolStyle\"!" );

                m_bIsSymbolsStyleRO = seqRO[nProperty];
                break;
            }

        }
    }

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!
    EnableNotification( seqNames );
}


//  destructor

SvtMiscOptions_Impl::~SvtMiscOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

void SvtMiscOptions_Impl::Load( const Sequence< OUString >& rPropertyNames )
{
    const uno::Sequence< OUString> aInternalPropertyNames( GetPropertyNames());
    Sequence< Any > seqValues = GetProperties( rPropertyNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(rPropertyNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if (!seqValues[nProperty].hasValue())
            continue;
        switch( comphelper::findValue(aInternalPropertyNames, rPropertyNames[nProperty]) )
        {
            case PROPERTYHANDLE_SYMBOLSET           :   {
                                                            if( !(seqValues[nProperty] >>= m_nSymbolsSize) )
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\SymbolSet\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSTYLE         :   {
                                                            OUString aIconTheme;
                                                            if (seqValues[nProperty] >>= aIconTheme)
                                                                SetIconTheme(aIconTheme, SetModifiedFlag::DONT_SET);
                                                            else
                                                                OSL_FAIL("Wrong type of \"Misc\\SymbolStyle\"!" );
                                                        }
                                                    break;
        }
    }
}

void SvtMiscOptions_Impl::AddListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    aList.push_back( rLink );
}

void SvtMiscOptions_Impl::RemoveListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    aList.erase(std::remove(aList.begin(), aList.end(), rLink), aList.end());
}

void SvtMiscOptions_Impl::CallListeners()
{
    for (auto const& elem : aList)
        elem.Call( nullptr );
}

void SvtMiscOptions_Impl::SetSymbolsSize( sal_Int16 nSet )
{
    m_nSymbolsSize = nSet;
    SetModified();
    CallListeners();
}

OUString SvtMiscOptions_Impl::GetIconTheme()
{
    return Application::GetSettings().GetStyleSettings().DetermineIconTheme();
}

void
SvtMiscOptions_Impl::SetIconTheme(const OUString &rName, SetModifiedFlag setModified)
{
    OUString aTheme(rName);
    if (aTheme.isEmpty() || aTheme == "auto")
    {
        aTheme = Application::GetSettings().GetStyleSettings().GetAutomaticallyChosenIconTheme();
        m_bIconThemeWasSetAutomatically = true;
    }
    else
        m_bIconThemeWasSetAutomatically = false;

    AllSettings aAllSettings = Application::GetSettings();
    StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();
    aStyleSettings.SetIconTheme(aTheme);

    aAllSettings.SetStyleSettings(aStyleSettings);
    Application::MergeSystemSettings( aAllSettings );
    Application::SetSettings(aAllSettings);

    if (setModified == SetModifiedFlag::SET) {
        SetModified();
    }
    CallListeners();
}


//  public method

void SvtMiscOptions_Impl::Notify( const Sequence< OUString >& rPropertyNames )
{
    Load( rPropertyNames );
    CallListeners();
}


//  public method

void SvtMiscOptions_Impl::ImplCommit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = GetPropertyNames  ();
    sal_Int32               nCount      = seqNames.getLength();
    Sequence< Any >         seqValues   ( nCount );
    auto seqValuesRange = asNonConstRange(seqValues);
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_SYMBOLSET :
            {
                if ( !m_bIsSymbolsSizeRO )
                   seqValuesRange[nProperty] <<= m_nSymbolsSize;
                break;
            }

            case PROPERTYHANDLE_SYMBOLSTYLE :
            {
                if ( !m_bIsSymbolsStyleRO ) {
                    OUString value;
                    if (m_bIconThemeWasSetAutomatically) {
                        value = "auto";
                    }
                    else {
                        value = GetIconTheme();
                    }
                    seqValuesRange[nProperty] <<= value;
                }
                break;
            }

        }
    }
    // Set properties in configuration.
    PutProperties( seqNames, seqValues );
}


//  private method

Sequence< OUString > SvtMiscOptions_Impl::GetPropertyNames()
{
    return Sequence<OUString>
    {
        PROPERTYNAME_SYMBOLSET,
        PROPERTYNAME_ICONTHEME,
    };
}

namespace {

std::weak_ptr<SvtMiscOptions_Impl> g_pMiscOptions;

}

SvtMiscOptions::SvtMiscOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl = g_pMiscOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtMiscOptions_Impl>();
        g_pMiscOptions = m_pImpl;
        svtools::ItemHolder2::holdConfigItem(EItem::MiscOptions);
    }
}

SvtMiscOptions::~SvtMiscOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl.reset();
}


sal_Int16 SvtMiscOptions::GetSymbolsSize() const
{
    return m_pImpl->GetSymbolsSize();
}

void SvtMiscOptions::SetSymbolsSize( sal_Int16 nSet )
{
    m_pImpl->SetSymbolsSize( nSet );
}

sal_Int16 SvtMiscOptions::GetCurrentSymbolsSize() const
{
    sal_Int16 eOptSymbolsSize = m_pImpl->GetSymbolsSize();

    if ( eOptSymbolsSize == SFX_SYMBOLS_SIZE_AUTO )
    {
        // Use system settings, we have to retrieve the toolbar icon size from the
        // Application class
        ToolbarIconSize nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
        if (nStyleIconSize == ToolbarIconSize::Size32)
            eOptSymbolsSize = SFX_SYMBOLS_SIZE_32;
        else if (nStyleIconSize == ToolbarIconSize::Large)
            eOptSymbolsSize = SFX_SYMBOLS_SIZE_LARGE;
        else
            eOptSymbolsSize = SFX_SYMBOLS_SIZE_SMALL;
    }

    return eOptSymbolsSize;
}

bool SvtMiscOptions::AreCurrentSymbolsLarge() const
{
    return ( GetCurrentSymbolsSize() == SFX_SYMBOLS_SIZE_LARGE || GetCurrentSymbolsSize() == SFX_SYMBOLS_SIZE_32);
}

OUString SvtMiscOptions::GetIconTheme() const
{
    return SvtMiscOptions_Impl::GetIconTheme();
}

void SvtMiscOptions::SetIconTheme(const OUString& iconTheme)
{
    m_pImpl->SetIconTheme(iconTheme, SvtMiscOptions_Impl::SetModifiedFlag::SET);
}

namespace
{
    class theSvtMiscOptionsMutex :
        public rtl::Static< osl::Mutex, theSvtMiscOptionsMutex > {};
}

Mutex & SvtMiscOptions::GetInitMutex()
{
    return theSvtMiscOptionsMutex::get();
}

void SvtMiscOptions::AddListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    m_pImpl->AddListenerLink( rLink );
}

void SvtMiscOptions::RemoveListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    m_pImpl->RemoveListenerLink( rLink );
}

bool
SvtMiscOptions::IconThemeWasSetAutomatically() const
{
    return m_pImpl->IconThemeWasSetAutomatically();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

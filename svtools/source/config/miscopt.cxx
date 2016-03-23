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
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <tools/link.hxx>

#include <rtl/instance.hxx>
#include "itemholder2.hxx"

#include <svtools/imgdef.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <list>

using namespace ::utl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star;

#define ROOTNODE_MISC                       "Office.Common/Misc"

// PROPERTYHANDLE defines must be sequential from zero for Commit/Load
#define PROPERTYNAME_PLUGINSENABLED         "PluginsEnabled"
#define PROPERTYHANDLE_PLUGINSENABLED           0
#define PROPERTYNAME_SYMBOLSET              "SymbolSet"
#define PROPERTYHANDLE_SYMBOLSET                1
#define PROPERTYNAME_TOOLBOXSTYLE           "ToolboxStyle"
#define PROPERTYHANDLE_TOOLBOXSTYLE             2
#define PROPERTYNAME_USESYSTEMFILEDIALOG    "UseSystemFileDialog"
#define PROPERTYHANDLE_USESYSTEMFILEDIALOG      3
#define PROPERTYNAME_ICONTHEME              "SymbolStyle"
#define PROPERTYHANDLE_SYMBOLSTYLE              4
#define PROPERTYNAME_USESYSTEMPRINTDIALOG   "UseSystemPrintDialog"
#define PROPERTYHANDLE_USESYSTEMPRINTDIALOG     5
#define PROPERTYNAME_SHOWLINKWARNINGDIALOG  "ShowLinkWarningDialog"
#define PROPERTYHANDLE_SHOWLINKWARNINGDIALOG    6
#define PROPERTYNAME_DISABLEUICUSTOMIZATION "DisableUICustomization"
#define PROPERTYHANDLE_DISABLEUICUSTOMIZATION   7
#define PROPERTYNAME_EXPERIMENTALMODE       "ExperimentalMode"
#define PROPERTYHANDLE_EXPERIMENTALMODE         8
#define PROPERTYNAME_MACRORECORDERMODE       "MacroRecorderMode"
#define PROPERTYHANDLE_MACRORECORDERMODE        9

#define VCL_TOOLBOX_STYLE_FLAT              ((sal_uInt16)0x0004) // from <vcl/toolbox.hxx>

class SvtMiscOptions_Impl : public ConfigItem
{
private:
    ::std::list<Link<LinkParamNone*,void>> aList;
    bool        m_bUseSystemFileDialog;
    bool        m_bIsUseSystemFileDialogRO;
    bool        m_bPluginsEnabled;
    bool        m_bIsPluginsEnabledRO;
    sal_Int16   m_nSymbolsSize;
    bool        m_bIsSymbolsSizeRO;
    bool        m_bIsSymbolsStyleRO;
    sal_Int16   m_nToolboxStyle;
    bool        m_bIsToolboxStyleRO;
    bool        m_bUseSystemPrintDialog;
    bool        m_bIsUseSystemPrintDialogRO;
    bool        m_bShowLinkWarningDialog;
    bool        m_bIsShowLinkWarningDialogRO;
    bool        m_bDisableUICustomization;
    bool        m_bExperimentalMode;
    bool        m_bMacroRecorderMode;
    bool        m_bIconThemeWasSetAutomatically;

        virtual void ImplCommit() override;

public:

         SvtMiscOptions_Impl();
        virtual ~SvtMiscOptions_Impl();

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
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


        inline bool UseSystemFileDialog() const
        { return m_bUseSystemFileDialog; }

        inline void SetUseSystemFileDialog( bool bSet )
        {  m_bUseSystemFileDialog = bSet; SetModified(); }

        inline bool IsUseSystemFileDialogReadOnly() const
        { return m_bIsUseSystemFileDialogRO; }

        inline bool DisableUICustomization() const
        { return m_bDisableUICustomization; }

        inline void SetExperimentalMode( bool bSet )
        { m_bExperimentalMode = bSet; SetModified(); }

        inline bool IsExperimentalMode() const
        { return m_bExperimentalMode; }

        inline void SetMacroRecorderMode( bool bSet )
        { m_bMacroRecorderMode = bSet; SetModified(); }

        inline bool IsMacroRecorderMode() const
        { return m_bMacroRecorderMode; }

        inline bool IsPluginsEnabled() const
        { return m_bPluginsEnabled; }

        inline sal_Int16 GetSymbolsSize()
        { return m_nSymbolsSize; }

        void SetSymbolsSize( sal_Int16 nSet );

        static OUString GetIconTheme();

        enum SetModifiedFlag { SET_MODIFIED, DONT_SET_MODIFIED };

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
        SetIconTheme(const OUString &theme, SetModifiedFlag setModified = SET_MODIFIED );

        bool IconThemeWasSetAutomatically()
        {return m_bIconThemeWasSetAutomatically;}

        // translate to VCL settings ( "0" = 3D, "1" = FLAT )
        inline sal_Int16 GetToolboxStyle()
        { return m_nToolboxStyle ? VCL_TOOLBOX_STYLE_FLAT : 0; }

        // translate from VCL settings
        void SetToolboxStyle( sal_Int16 nStyle, bool _bSetModified );

        inline bool UseSystemPrintDialog() const
        { return m_bUseSystemPrintDialog; }

        inline void SetUseSystemPrintDialog( bool bSet )
        {  m_bUseSystemPrintDialog = bSet; SetModified(); }

        inline bool ShowLinkWarningDialog() const
        { return m_bShowLinkWarningDialog; }

        void SetShowLinkWarningDialog( bool bSet )
        {  m_bShowLinkWarningDialog = bSet; SetModified(); }

        bool IsShowLinkWarningDialogReadOnly() const
        { return m_bIsShowLinkWarningDialogRO; }

        void AddListenerLink( const Link<LinkParamNone*,void>& rLink );
        void RemoveListenerLink( const Link<LinkParamNone*,void>& rLink );
        void CallListeners();


    //  private methods


private:

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent oue module tree
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

    , m_bUseSystemFileDialog( false )
    , m_bIsUseSystemFileDialogRO( false )
    , m_bPluginsEnabled( false )
    , m_bIsPluginsEnabledRO( false )
    , m_nSymbolsSize( 0 )
    , m_bIsSymbolsSizeRO( false )
    , m_bIsSymbolsStyleRO( false )
    , m_nToolboxStyle( 1 )
    , m_bIsToolboxStyleRO( false )
    , m_bUseSystemPrintDialog( false )
    , m_bIsUseSystemPrintDialogRO( false )
    , m_bShowLinkWarningDialog( true )
    , m_bIsShowLinkWarningDialogRO( false )
    , m_bExperimentalMode( false )
    , m_bMacroRecorderMode( false )
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
            case PROPERTYHANDLE_PLUGINSENABLED :
            {
                if( !(seqValues[nProperty] >>= m_bPluginsEnabled) )
                {
                    OSL_FAIL("Wrong type of \"Misc\\PluginsEnabled\"!" );
                }
                m_bIsPluginsEnabledRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_SYMBOLSET :
            {
                if( !(seqValues[nProperty] >>= m_nSymbolsSize) )
                {
                    OSL_FAIL("Wrong type of \"Misc\\SymbolSet\"!" );
                }
                m_bIsSymbolsSizeRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_TOOLBOXSTYLE :
            {
                if( !(seqValues[nProperty] >>= m_nToolboxStyle) )
                {
                    OSL_FAIL("Wrong type of \"Misc\\ToolboxStyle\"!" );
                }
                m_bIsToolboxStyleRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_USESYSTEMFILEDIALOG :
            {
                if( !(seqValues[nProperty] >>= m_bUseSystemFileDialog) )
                {
                    OSL_FAIL("Wrong type of \"Misc\\UseSystemFileDialog\"!" );
                }
                m_bIsUseSystemFileDialogRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_USESYSTEMPRINTDIALOG :
            {
                if( !(seqValues[nProperty] >>= m_bUseSystemPrintDialog) )
                {
                    OSL_FAIL("Wrong type of \"Misc\\UseSystemPrintDialog\"!" );
                }
                m_bIsUseSystemPrintDialogRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_SHOWLINKWARNINGDIALOG :
            {
                if( !(seqValues[nProperty] >>= m_bShowLinkWarningDialog) )
                {
                    OSL_FAIL("Wrong type of \"Misc\\ShowLinkWarningDialog\"!" );
                }
                m_bIsShowLinkWarningDialogRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_SYMBOLSTYLE :
            {
                OUString aIconTheme;
                if (seqValues[nProperty] >>= aIconTheme)
                    SetIconTheme(aIconTheme, DONT_SET_MODIFIED);
                else
                    OSL_FAIL("Wrong type of \"Misc\\SymbolStyle\"!" );

                m_bIsSymbolsStyleRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_DISABLEUICUSTOMIZATION :
            {
                if( !(seqValues[nProperty] >>= m_bDisableUICustomization) )
                    OSL_FAIL("Wrong type of \"Misc\\DisableUICustomization\"!" );
                break;
            }
            case PROPERTYHANDLE_EXPERIMENTALMODE :
            {
                if( !(seqValues[nProperty] >>= m_bExperimentalMode) )
                    OSL_FAIL("Wrong type of \"Misc\\ExperimentalMode\"!" );
                break;
            }
            case PROPERTYHANDLE_MACRORECORDERMODE :
            {
                if( !(seqValues[nProperty] >>= m_bMacroRecorderMode) )
                    OSL_FAIL("Wrong type of \"Misc\\MacroRecorderMode\"!" );
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

static int lcl_MapPropertyName( const OUString& rCompare,
                const uno::Sequence< OUString>& aInternalPropertyNames)
{
    for(int nProp = 0; nProp < aInternalPropertyNames.getLength(); ++nProp)
    {
        if( aInternalPropertyNames[nProp] == rCompare )
            return nProp;
    }
    return -1;
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
        switch( lcl_MapPropertyName(rPropertyNames[nProperty], aInternalPropertyNames) )
        {
            case PROPERTYHANDLE_PLUGINSENABLED      :   {
                                                            if( !(seqValues[nProperty] >>= m_bPluginsEnabled) )
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\PluginsEnabled\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSET           :   {
                                                            if( !(seqValues[nProperty] >>= m_nSymbolsSize) )
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\SymbolSet\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_TOOLBOXSTYLE        :   {
                                                            if( !(seqValues[nProperty] >>= m_nToolboxStyle) )
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\ToolboxStyle\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_USESYSTEMFILEDIALOG      :   {
                                                            if( !(seqValues[nProperty] >>= m_bUseSystemFileDialog) )
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\UseSystemFileDialog\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_USESYSTEMPRINTDIALOG     :   {
                                                            if( !(seqValues[nProperty] >>= m_bUseSystemPrintDialog) )
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\UseSystemPrintDialog\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_SHOWLINKWARNINGDIALOG     :   {
                                                            if( !(seqValues[nProperty] >>= m_bShowLinkWarningDialog) )
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\ShowLinkWarningDialog\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSTYLE         :   {
                                                            OUString aIconTheme;
                                                            if (seqValues[nProperty] >>= aIconTheme)
                                                                SetIconTheme(aIconTheme, DONT_SET_MODIFIED);
                                                            else
                                                                OSL_FAIL("Wrong type of \"Misc\\SymbolStyle\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_DISABLEUICUSTOMIZATION      :   {
                                                            if( !(seqValues[nProperty] >>= m_bDisableUICustomization) )
                                                                OSL_FAIL("Wrong type of \"Misc\\DisableUICustomization\"!" );
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
    for ( ::std::list<Link<LinkParamNone*,void>>::iterator iter = aList.begin(); iter != aList.end(); ++iter )
    {
        if ( *iter == rLink )
        {
            aList.erase(iter);
            break;
        }
    }
}

void SvtMiscOptions_Impl::CallListeners()
{
    for ( ::std::list<Link<LinkParamNone*,void>>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
        iter->Call( nullptr );
}

void SvtMiscOptions_Impl::SetToolboxStyle( sal_Int16 nStyle, bool _bSetModified )
{
    m_nToolboxStyle = nStyle ? 1 : 0;
    if ( _bSetModified )
        SetModified();
    CallListeners();
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

    if (setModified == SET_MODIFIED) {
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
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_PLUGINSENABLED :
            {
                if ( !m_bIsPluginsEnabledRO )
                    seqValues[nProperty] <<= m_bPluginsEnabled;
                break;
            }

            case PROPERTYHANDLE_SYMBOLSET :
            {
                if ( !m_bIsSymbolsSizeRO )
                   seqValues[nProperty] <<= m_nSymbolsSize;
                break;
            }

            case PROPERTYHANDLE_TOOLBOXSTYLE :
            {
                if ( !m_bIsToolboxStyleRO )
                    seqValues[nProperty] <<= m_nToolboxStyle;
                break;
            }

            case PROPERTYHANDLE_USESYSTEMFILEDIALOG :
            {
                if ( !m_bIsUseSystemFileDialogRO )
                    seqValues[nProperty] <<= m_bUseSystemFileDialog;
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
                    seqValues[nProperty] <<= value;
                }
                break;
            }

            case PROPERTYHANDLE_USESYSTEMPRINTDIALOG :
            {
                if ( !m_bIsUseSystemPrintDialogRO )
                    seqValues[nProperty] <<= m_bUseSystemPrintDialog;
                break;
            }

            case PROPERTYHANDLE_SHOWLINKWARNINGDIALOG :
            {
                if ( !m_bIsShowLinkWarningDialogRO )
                    seqValues[nProperty] <<= m_bShowLinkWarningDialog;
                break;
            }

            case PROPERTYHANDLE_DISABLEUICUSTOMIZATION :
            {
                seqValues[nProperty] <<= m_bDisableUICustomization;
                break;
            }
            case PROPERTYHANDLE_EXPERIMENTALMODE :
            {
                seqValues[nProperty] <<= m_bExperimentalMode;
                break;
            }
            case PROPERTYHANDLE_MACRORECORDERMODE :
            {
                seqValues[nProperty] <<= m_bMacroRecorderMode;
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
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        OUString(PROPERTYNAME_PLUGINSENABLED),
        OUString(PROPERTYNAME_SYMBOLSET),
        OUString(PROPERTYNAME_TOOLBOXSTYLE),
        OUString(PROPERTYNAME_USESYSTEMFILEDIALOG),
        OUString(PROPERTYNAME_ICONTHEME),
        OUString(PROPERTYNAME_USESYSTEMPRINTDIALOG),
        OUString(PROPERTYNAME_SHOWLINKWARNINGDIALOG),
        OUString(PROPERTYNAME_DISABLEUICUSTOMIZATION),
        OUString(PROPERTYNAME_EXPERIMENTALMODE),
        OUString(PROPERTYNAME_MACRORECORDERMODE)
    };

    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, SAL_N_ELEMENTS( pProperties ) );
    // ... and return it.
    return seqPropertyNames;
}


//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further information

SvtMiscOptions_Impl*    SvtMiscOptions::m_pDataContainer    = nullptr  ;
sal_Int32               SvtMiscOptions::m_nRefCount = 0     ;


//  constructor

SvtMiscOptions::SvtMiscOptions()
{
    // SvtMiscOptions_Impl ctor indirectly calls code that requires locked
    // SolarMutex; lock it first:
    SolarMutexGuard g;
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already exist!
    if( m_pDataContainer == nullptr )
    {
       m_pDataContainer = new SvtMiscOptions_Impl;
       svtools::ItemHolder2::holdConfigItem(E_MISCOPTIONS);
    }
}


//  destructor

SvtMiscOptions::~SvtMiscOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );
    // Decrease our refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy our static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = nullptr;
    }
}

bool SvtMiscOptions::UseSystemFileDialog() const
{
    return m_pDataContainer->UseSystemFileDialog();
}

void SvtMiscOptions::SetUseSystemFileDialog( bool bEnable )
{
    m_pDataContainer->SetUseSystemFileDialog( bEnable );
}

bool SvtMiscOptions::IsUseSystemFileDialogReadOnly() const
{
    return m_pDataContainer->IsUseSystemFileDialogReadOnly();
}

bool SvtMiscOptions::IsPluginsEnabled() const
{
    return m_pDataContainer->IsPluginsEnabled();
}

sal_Int16 SvtMiscOptions::GetSymbolsSize() const
{
    return m_pDataContainer->GetSymbolsSize();
}

void SvtMiscOptions::SetSymbolsSize( sal_Int16 nSet )
{
    m_pDataContainer->SetSymbolsSize( nSet );
}

sal_Int16 SvtMiscOptions::GetCurrentSymbolsSize() const
{
    sal_Int16 eOptSymbolsSize = m_pDataContainer->GetSymbolsSize();

    if ( eOptSymbolsSize == SFX_SYMBOLS_SIZE_AUTO )
    {
        // Use system settings, we have to retrieve the toolbar icon size from the
        // Application class
        ToolbarIconSize nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
        if ( nStyleIconSize == ToolbarIconSize::Large )
            eOptSymbolsSize = SFX_SYMBOLS_SIZE_LARGE;
        else
            eOptSymbolsSize = SFX_SYMBOLS_SIZE_SMALL;
    }

    return eOptSymbolsSize;
}

bool SvtMiscOptions::AreCurrentSymbolsLarge() const
{
    return ( GetCurrentSymbolsSize() == SFX_SYMBOLS_SIZE_LARGE );
}

OUString SvtMiscOptions::GetIconTheme() const
{
    return SvtMiscOptions_Impl::GetIconTheme();
}

void SvtMiscOptions::SetIconTheme(const OUString& iconTheme)
{
    m_pDataContainer->SetIconTheme(iconTheme);
}

bool SvtMiscOptions::DisableUICustomization() const
{
    return m_pDataContainer->DisableUICustomization();
}

sal_Int16 SvtMiscOptions::GetToolboxStyle() const
{
    return m_pDataContainer->GetToolboxStyle();
}

void SvtMiscOptions::SetToolboxStyle( sal_Int16 nStyle )
{
    m_pDataContainer->SetToolboxStyle( nStyle, true );
}

bool SvtMiscOptions::UseSystemPrintDialog() const
{
    return m_pDataContainer->UseSystemPrintDialog();
}

void SvtMiscOptions::SetUseSystemPrintDialog( bool bEnable )
{
    m_pDataContainer->SetUseSystemPrintDialog( bEnable );
}

bool SvtMiscOptions::ShowLinkWarningDialog() const
{
    return m_pDataContainer->ShowLinkWarningDialog();
}

void SvtMiscOptions::SetShowLinkWarningDialog( bool bSet )
{
    m_pDataContainer->SetShowLinkWarningDialog( bSet );
}

bool SvtMiscOptions::IsShowLinkWarningDialogReadOnly() const
{
    return m_pDataContainer->IsShowLinkWarningDialogReadOnly();
}

void SvtMiscOptions::SetExperimentalMode( bool bSet )
{
    m_pDataContainer->SetExperimentalMode( bSet );
}

bool SvtMiscOptions::IsExperimentalMode() const
{
    return m_pDataContainer->IsExperimentalMode();
}

void SvtMiscOptions::SetMacroRecorderMode( bool bSet )
{
    m_pDataContainer->SetMacroRecorderMode( bSet );
}

bool SvtMiscOptions::IsMacroRecorderMode() const
{
    return m_pDataContainer->IsMacroRecorderMode();
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
    m_pDataContainer->AddListenerLink( rLink );
}

void SvtMiscOptions::RemoveListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    m_pDataContainer->RemoveListenerLink( rLink );
}

bool
SvtMiscOptions::IconThemeWasSetAutomatically()
{
    return m_pDataContainer->IconThemeWasSetAutomatically();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

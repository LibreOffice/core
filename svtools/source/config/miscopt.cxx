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

#include <list>

using namespace ::utl                   ;
using namespace ::rtl                   ;
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
#define PROPERTYNAME_SYMBOLSTYLE            "SymbolStyle"
#define PROPERTYHANDLE_SYMBOLSTYLE              4
#define PROPERTYNAME_USESYSTEMPRINTDIALOG   "UseSystemPrintDialog"
#define PROPERTYHANDLE_USESYSTEMPRINTDIALOG     5
#define PROPERTYNAME_SHOWLINKWARNINGDIALOG  "ShowLinkWarningDialog"
#define PROPERTYHANDLE_SHOWLINKWARNINGDIALOG    6
#define PROPERTYNAME_DISABLEUICUSTOMIZATION "DisableUICustomization"
#define PROPERTYHANDLE_DISABLEUICUSTOMIZATION   7
#define PROPERTYNAME_ALWAYSALLOWSAVE        "AlwaysAllowSave"
#define PROPERTYHANDLE_ALWAYSALLOWSAVE          8
#define PROPERTYNAME_EXPERIMENTALMODE       "ExperimentalMode"
#define PROPERTYHANDLE_EXPERIMENTALMODE         9
#define PROPERTYNAME_MACRORECORDERMODE       "MacroRecorderMode"
#define PROPERTYHANDLE_MACRORECORDERMODE        10

#define VCL_TOOLBOX_STYLE_FLAT              ((sal_uInt16)0x0004) // from <vcl/toolbox.hxx>

class SvtMiscOptions_Impl : public ConfigItem
{
    private:
    ::std::list<Link> aList;
    sal_Bool    m_bUseSystemFileDialog;
    sal_Bool    m_bIsUseSystemFileDialogRO;
    sal_Bool    m_bPluginsEnabled;
    sal_Bool    m_bIsPluginsEnabledRO;
    sal_Int16   m_nSymbolsSize;
    sal_Bool    m_bIsSymbolsSizeRO;
    sal_Bool    m_bIsSymbolsStyleRO;
    sal_Int16   m_nToolboxStyle;
    sal_Bool    m_bIsToolboxStyleRO;
    sal_Bool    m_bUseSystemPrintDialog;
    sal_Bool    m_bIsUseSystemPrintDialogRO;
    sal_Bool    m_bShowLinkWarningDialog;
    sal_Bool    m_bIsShowLinkWarningDialogRO;
    sal_Bool    m_bDisableUICustomization;
    sal_Bool    m_bAlwaysAllowSave;
    sal_Bool    m_bExperimentalMode;
    sal_Bool    m_bMacroRecorderMode;

    public:

         SvtMiscOptions_Impl();
        ~SvtMiscOptions_Impl();

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );

        /** loads required data from the configuration. It's called in the constructor to
         read all entries and form ::Notify to re-read changed settings

         */
        void Load( const Sequence< OUString >& rPropertyNames );

        /*-****************************************************************************************************//**
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Commit();

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------

        inline sal_Bool UseSystemFileDialog() const
        { return m_bUseSystemFileDialog; }

        inline void SetUseSystemFileDialog( sal_Bool bSet )
        {  m_bUseSystemFileDialog = bSet; SetModified(); }

        inline sal_Bool IsUseSystemFileDialogReadOnly() const
        { return m_bIsUseSystemFileDialogRO; }

        inline sal_Bool DisableUICustomization() const
        { return m_bDisableUICustomization; }

        inline void SetSaveAlwaysAllowed( sal_Bool bSet )
        { m_bAlwaysAllowSave = bSet; SetModified(); }

        inline sal_Bool IsSaveAlwaysAllowed() const
        { return m_bAlwaysAllowSave; }

        inline void SetExperimentalMode( sal_Bool bSet )
        { m_bExperimentalMode = bSet; SetModified(); }

        inline sal_Bool IsExperimentalMode() const
        { return m_bExperimentalMode; }

        inline void SetMacroRecorderMode( sal_Bool bSet )
        { m_bMacroRecorderMode = bSet; SetModified(); }

        inline sal_Bool IsMacroRecorderMode() const
        { return m_bMacroRecorderMode; }

        inline sal_Bool IsPluginsEnabled() const
        { return m_bPluginsEnabled; }

        inline sal_Bool IsPluginsEnabledReadOnly() const
        { return m_bIsPluginsEnabledRO; }

        inline sal_Int16 GetSymbolsSize()
        { return m_nSymbolsSize; }

        void SetSymbolsSize( sal_Int16 nSet );

        inline sal_Bool IsGetSymbolsSizeReadOnly()
        { return m_bIsSymbolsSizeRO; }

        sal_Int16 GetSymbolsStyle() const;
        OUString GetSymbolsStyleName() const;
        sal_Int16 GetCurrentSymbolsStyle() const;

        inline void SetSymbolsStyle( sal_Int16 nSet )
        { ImplSetSymbolsStyle( true, nSet, OUString() ); }

        inline void SetSymbolsStyleName( OUString &rName )
        { ImplSetSymbolsStyle( false, 0, rName ); }

        inline sal_Bool IsGetSymbolsStyleReadOnly()
        { return m_bIsSymbolsStyleRO; }

        // translate to VCL settings ( "0" = 3D, "1" = FLAT )
        inline sal_Int16 GetToolboxStyle()
        { return m_nToolboxStyle ? VCL_TOOLBOX_STYLE_FLAT : 0; }

        // translate from VCL settings
        void SetToolboxStyle( sal_Int16 nStyle, bool _bSetModified );

        inline sal_Bool IsGetToolboxStyleReadOnly()
        { return m_bIsToolboxStyleRO; }

        inline sal_Bool UseSystemPrintDialog() const
        { return m_bUseSystemPrintDialog; }

        inline void SetUseSystemPrintDialog( sal_Bool bSet )
        {  m_bUseSystemPrintDialog = bSet; SetModified(); }

        inline sal_Bool IsUseSystemPrintDialogReadOnly() const
        { return m_bIsUseSystemPrintDialogRO; }

        inline sal_Bool ShowLinkWarningDialog() const
        { return m_bShowLinkWarningDialog; }

        void SetShowLinkWarningDialog( sal_Bool bSet )
        {  m_bShowLinkWarningDialog = bSet; SetModified(); }

        sal_Bool IsShowLinkWarningDialogReadOnly() const
        { return m_bIsShowLinkWarningDialogRO; }

        void AddListenerLink( const Link& rLink );
        void RemoveListenerLink( const Link& rLink );
        void CallListeners();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of our configuration management which represent oue module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        static Sequence< OUString > GetPropertyNames();

    protected:
        void ImplSetSymbolsStyle( bool bValue, sal_Int16 nSet, const OUString &rName );
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtMiscOptions_Impl::SvtMiscOptions_Impl()
    // Init baseclasses first
    : ConfigItem( ROOTNODE_MISC )

    , m_bUseSystemFileDialog( sal_False )
    , m_bIsUseSystemFileDialogRO( sal_False )
    , m_bPluginsEnabled( sal_False )
    , m_bIsPluginsEnabledRO( sal_False )
    , m_nSymbolsSize( 0 )
    , m_bIsSymbolsSizeRO( sal_False )
    , m_bIsSymbolsStyleRO( sal_False )
    , m_nToolboxStyle( 1 )
    , m_bIsToolboxStyleRO( sal_False )
    , m_bUseSystemPrintDialog( sal_False )
    , m_bIsUseSystemPrintDialogRO( sal_False )
    , m_bShowLinkWarningDialog( sal_True )
    , m_bIsShowLinkWarningDialogRO( sal_False )
    , m_bAlwaysAllowSave( sal_False )
    , m_bExperimentalMode( sal_False )
    , m_bMacroRecorderMode( sal_False )

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
        if (seqValues[nProperty].hasValue()==sal_False)
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
                OUString aSymbolsStyle;
                if( seqValues[nProperty] >>= aSymbolsStyle )
                    SetSymbolsStyleName( aSymbolsStyle );
                else
                {
                    OSL_FAIL("Wrong type of \"Misc\\SymbolStyle\"!" );
                }
                m_bIsSymbolsStyleRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_DISABLEUICUSTOMIZATION :
            {
                if( !(seqValues[nProperty] >>= m_bDisableUICustomization) )
                    OSL_FAIL("Wrong type of \"Misc\\DisableUICustomization\"!" );
                break;
            }
            case PROPERTYHANDLE_ALWAYSALLOWSAVE :
            {
                if( !(seqValues[nProperty] >>= m_bAlwaysAllowSave) )
                    OSL_FAIL("Wrong type of \"Misc\\AlwaysAllowSave\"!" );
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

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtMiscOptions_Impl::~SvtMiscOptions_Impl()
{
    // We must save our current values .. if user forgets it!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

static int lcl_MapPropertyName( const OUString rCompare,
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
        if (seqValues[nProperty].hasValue()==sal_False)
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
                                                            OUString aSymbolsStyle;
                                                            if( seqValues[nProperty] >>= aSymbolsStyle )
                                                                SetSymbolsStyleName( aSymbolsStyle );
                                                            else
                                                            {
                                                                OSL_FAIL("Wrong type of \"Misc\\SymbolStyle\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_DISABLEUICUSTOMIZATION      :   {
                                                            if( !(seqValues[nProperty] >>= m_bDisableUICustomization) )
                                                                OSL_FAIL("Wrong type of \"Misc\\DisableUICustomization\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_ALWAYSALLOWSAVE:
            {
                if( !(seqValues[nProperty] >>= m_bAlwaysAllowSave) )
                    OSL_FAIL("Wrong type of \"Misc\\AlwaysAllowSave\"!" );
            }
            break;
        }
    }
}

void SvtMiscOptions_Impl::AddListenerLink( const Link& rLink )
{
    aList.push_back( rLink );
}

void SvtMiscOptions_Impl::RemoveListenerLink( const Link& rLink )
{
    for ( ::std::list<Link>::iterator iter = aList.begin(); iter != aList.end(); ++iter )
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
    for ( ::std::list<Link>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
        iter->Call( this );
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

sal_Int16 SvtMiscOptions_Impl::GetSymbolsStyle() const
{
    return (sal_Int16)Application::GetSettings().GetStyleSettings().GetSymbolsStyle();
}

OUString SvtMiscOptions_Impl::GetSymbolsStyleName() const
{
    return Application::GetSettings().GetStyleSettings().GetSymbolsStyleName();
}

sal_Int16 SvtMiscOptions_Impl::GetCurrentSymbolsStyle() const
{
    return (sal_Int16)Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyle();
}

void SvtMiscOptions_Impl::ImplSetSymbolsStyle( bool bValue, sal_Int16 nSet, const OUString &rName )
{
    if ( ( bValue && ( nSet != GetSymbolsStyle() ) ) ||
         ( !bValue && ( rName != GetSymbolsStyleName() ) ) )
    {
        AllSettings aAllSettings = Application::GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();

        if ( bValue )
            aStyleSettings.SetSymbolsStyle( nSet );
        else
            aStyleSettings.SetSymbolsStyleName( rName );

        aAllSettings.SetStyleSettings(aStyleSettings);
        Application::MergeSystemSettings( aAllSettings );
        Application::SetSettings(aAllSettings);

        SetModified();
        CallListeners();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtMiscOptions_Impl::Notify( const Sequence< OUString >& rPropertyNames )
{
    Load( rPropertyNames );
    CallListeners();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtMiscOptions_Impl::Commit()
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
                if ( !m_bIsSymbolsStyleRO )
                    seqValues[nProperty] <<= GetSymbolsStyleName();
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
            case PROPERTYHANDLE_ALWAYSALLOWSAVE :
            {
                seqValues[nProperty] <<= m_bAlwaysAllowSave;
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

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtMiscOptions_Impl::GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        OUString(PROPERTYNAME_PLUGINSENABLED),
        OUString(PROPERTYNAME_SYMBOLSET),
        OUString(PROPERTYNAME_TOOLBOXSTYLE),
        OUString(PROPERTYNAME_USESYSTEMFILEDIALOG),
        OUString(PROPERTYNAME_SYMBOLSTYLE),
        OUString(PROPERTYNAME_USESYSTEMPRINTDIALOG),
        OUString(PROPERTYNAME_SHOWLINKWARNINGDIALOG),
        OUString(PROPERTYNAME_DISABLEUICUSTOMIZATION),
        OUString(PROPERTYNAME_ALWAYSALLOWSAVE),
        OUString(PROPERTYNAME_EXPERIMENTALMODE),
        OUString(PROPERTYNAME_MACRORECORDERMODE)
    };

    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, SAL_N_ELEMENTS( pProperties ) );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further information
//*****************************************************************************************************************
SvtMiscOptions_Impl*    SvtMiscOptions::m_pDataContainer    = NULL  ;
sal_Int32               SvtMiscOptions::m_nRefCount = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtMiscOptions::SvtMiscOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
       m_pDataContainer = new SvtMiscOptions_Impl;
       svtools::ItemHolder2::holdConfigItem(E_MISCOPTIONS);
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
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
        m_pDataContainer = NULL;
    }
}

sal_Bool SvtMiscOptions::UseSystemFileDialog() const
{
    return m_pDataContainer->UseSystemFileDialog();
}

void SvtMiscOptions::SetUseSystemFileDialog( sal_Bool bEnable )
{
    m_pDataContainer->SetUseSystemFileDialog( bEnable );
}

sal_Bool SvtMiscOptions::IsUseSystemFileDialogReadOnly() const
{
    return m_pDataContainer->IsUseSystemFileDialogReadOnly();
}

sal_Bool SvtMiscOptions::IsPluginsEnabled() const
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
        sal_uLong nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
        if ( nStyleIconSize == STYLE_TOOLBAR_ICONSIZE_LARGE )
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

sal_Int16 SvtMiscOptions::GetSymbolsStyle() const
{
    return m_pDataContainer->GetSymbolsStyle();
}

sal_Int16 SvtMiscOptions::GetCurrentSymbolsStyle() const
{
    return m_pDataContainer->GetCurrentSymbolsStyle();
}

void SvtMiscOptions::SetSymbolsStyle( sal_Int16 nSet )
{
    m_pDataContainer->SetSymbolsStyle( nSet );
}

sal_Bool SvtMiscOptions::DisableUICustomization() const
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

sal_Bool SvtMiscOptions::UseSystemPrintDialog() const
{
    return m_pDataContainer->UseSystemPrintDialog();
}

void SvtMiscOptions::SetUseSystemPrintDialog( sal_Bool bEnable )
{
    m_pDataContainer->SetUseSystemPrintDialog( bEnable );
}

sal_Bool SvtMiscOptions::ShowLinkWarningDialog() const
{
    return m_pDataContainer->ShowLinkWarningDialog();
}

void SvtMiscOptions::SetShowLinkWarningDialog( sal_Bool bSet )
{
    m_pDataContainer->SetShowLinkWarningDialog( bSet );
}

sal_Bool SvtMiscOptions::IsShowLinkWarningDialogReadOnly() const
{
    return m_pDataContainer->IsShowLinkWarningDialogReadOnly();
}

void SvtMiscOptions::SetSaveAlwaysAllowed( sal_Bool bSet )
{
    m_pDataContainer->SetSaveAlwaysAllowed( bSet );
}

sal_Bool SvtMiscOptions::IsSaveAlwaysAllowed() const
{
    return m_pDataContainer->IsSaveAlwaysAllowed();
}

void SvtMiscOptions::SetExperimentalMode( sal_Bool bSet )
{
    m_pDataContainer->SetExperimentalMode( bSet );
}

sal_Bool SvtMiscOptions::IsExperimentalMode() const
{
    return m_pDataContainer->IsExperimentalMode();
}

void SvtMiscOptions::SetMacroRecorderMode( sal_Bool bSet )
{
    m_pDataContainer->SetMacroRecorderMode( bSet );
}

sal_Bool SvtMiscOptions::IsMacroRecorderMode() const
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

void SvtMiscOptions::AddListenerLink( const Link& rLink )
{
    m_pDataContainer->AddListenerLink( rLink );
}

void SvtMiscOptions::RemoveListenerLink( const Link& rLink )
{
    m_pDataContainer->RemoveListenerLink( rLink );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

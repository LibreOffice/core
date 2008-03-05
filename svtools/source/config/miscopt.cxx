/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: miscopt.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:40:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________
#ifdef  SVL_DLLIMPLEMENTATION
#undef  SVL_DLLIMPLEMENTATION
#endif
#define SVT_DLLIMPLEMENTATION

#include <svtools/miscopt.hxx>

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#include <rtl/logfile.hxx>
#include "itemholder2.hxx"

#include <imgdef.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ASCII_STR(s)                        OUString( RTL_CONSTASCII_USTRINGPARAM(s) )
#define ROOTNODE_MISC                       ASCII_STR("Office.Common/Misc")
#define DEFAULT_PLUGINSENABLED              sal_True;

#define PROPERTYNAME_PLUGINSENABLED         ASCII_STR("PluginsEnabled")
#define PROPERTYHANDLE_PLUGINSENABLED       0
#define PROPERTYNAME_SYMBOLSET              ASCII_STR("SymbolSet")
#define PROPERTYHANDLE_SYMBOLSET            1
#define PROPERTYNAME_TOOLBOXSTYLE           ASCII_STR("ToolboxStyle")
#define PROPERTYHANDLE_TOOLBOXSTYLE         2
#define PROPERTYNAME_USESYSTEMFILEDIALOG    ASCII_STR("UseSystemFileDialog")
#define PROPERTYHANDLE_USESYSTEMFILEDIALOG  3
#define PROPERTYNAME_SYMBOLSTYLE            ASCII_STR("SymbolStyle")
#define PROPERTYHANDLE_SYMBOLSTYLE          4
#define PROPERTYNAME_USESYSTEMPRINTDIALOG   ASCII_STR("UseSystemPrintDialog")
#define PROPERTYHANDLE_USESYSTEMPRINTDIALOG 5

#define PROPERTYCOUNT                       6

#define VCL_TOOLBOX_STYLE_FLAT              ((USHORT)0x0004) // from <vcl/toolbox.hxx>

DECLARE_LIST( LinkList, Link * )

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

class SvtMiscOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:
    LinkList    aList;
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

    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtMiscOptions_Impl();
        ~SvtMiscOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

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

        inline sal_Bool IsPluginsEnabled() const
        { return m_bPluginsEnabled; }

        void SetPluginsEnabled( sal_Bool bEnable );

        inline sal_Bool IsPluginsEnabledReadOnly() const
        { return m_bIsPluginsEnabledRO; }

        inline sal_Int16 GetSymbolsSize()
        { return m_nSymbolsSize; }

        void SetSymbolsSize( sal_Int16 nSet );

        inline sal_Bool IsGetSymbolsSizeReadOnly()
        { return m_bIsSymbolsSizeRO; }

        sal_Int16 GetSymbolsStyle() const;
        ::rtl::OUString GetSymbolsStyleName() const;
        sal_Int16 GetCurrentSymbolsStyle() const;

        inline void SetSymbolsStyle( sal_Int16 nSet )
        { ImplSetSymbolsStyle( true, nSet, ::rtl::OUString() ); }

        inline void SetSymbolsStyleName( ::rtl::OUString &rName )
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

        void AddListener( const Link& rLink );
        void RemoveListener( const Link& rLink );
        void CallListeners();

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of ouer configuration management which represent oue module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        static Sequence< OUString > GetPropertyNames();

    protected:
        void ImplSetSymbolsStyle( bool bValue, sal_Int16 nSet, const ::rtl::OUString &rName );
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

static sal_Int16 implSymbolsStyleFromVCL( ULONG nStyle )
{
    switch ( nStyle )
    {
        case STYLE_SYMBOLS_AUTO:       return SFX_SYMBOLS_STYLE_AUTO;
        case STYLE_SYMBOLS_DEFAULT:    return SFX_SYMBOLS_STYLE_DEFAULT;
        case STYLE_SYMBOLS_HICONTRAST: return SFX_SYMBOLS_STYLE_HICONTRAST;
        case STYLE_SYMBOLS_INDUSTRIAL: return SFX_SYMBOLS_STYLE_INDUSTRIAL;
        case STYLE_SYMBOLS_CRYSTAL:    return SFX_SYMBOLS_STYLE_CRYSTAL;
        case STYLE_SYMBOLS_TANGO:      return SFX_SYMBOLS_STYLE_TANGO;
    }

    return SFX_SYMBOLS_STYLE_AUTO;
}

static ULONG implSymbolsStyleToVCL( sal_Int16 nStyle )
{
    switch ( nStyle )
    {
        case SFX_SYMBOLS_STYLE_AUTO:       return STYLE_SYMBOLS_AUTO;
        case SFX_SYMBOLS_STYLE_DEFAULT:    return STYLE_SYMBOLS_DEFAULT;
        case SFX_SYMBOLS_STYLE_HICONTRAST: return STYLE_SYMBOLS_HICONTRAST;
        case SFX_SYMBOLS_STYLE_INDUSTRIAL: return STYLE_SYMBOLS_INDUSTRIAL;
        case SFX_SYMBOLS_STYLE_CRYSTAL:    return STYLE_SYMBOLS_CRYSTAL;
        case SFX_SYMBOLS_STYLE_TANGO:      return STYLE_SYMBOLS_TANGO;
    }

    return STYLE_SYMBOLS_AUTO;
}

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

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_PLUGINSENABLED :
            {
                if( !(seqValues[nProperty] >>= m_bPluginsEnabled) )
                {
                    DBG_ERROR("Wrong type of \"Misc\\PluginsEnabled\"!" );
                }
                m_bIsPluginsEnabledRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_SYMBOLSET :
            {
                if( !(seqValues[nProperty] >>= m_nSymbolsSize) )
                {
                    DBG_ERROR("Wrong type of \"Misc\\SymbolSet\"!" );
                }
                m_bIsSymbolsSizeRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_TOOLBOXSTYLE :
            {
                if( !(seqValues[nProperty] >>= m_nToolboxStyle) )
                {
                    DBG_ERROR("Wrong type of \"Misc\\ToolboxStyle\"!" );
                }
                m_bIsToolboxStyleRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_USESYSTEMFILEDIALOG :
            {
                if( !(seqValues[nProperty] >>= m_bUseSystemFileDialog) )
                {
                    DBG_ERROR("Wrong type of \"Misc\\UseSystemFileDialog\"!" );
                }
                m_bIsUseSystemFileDialogRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_USESYSTEMPRINTDIALOG :
            {
                if( !(seqValues[nProperty] >>= m_bUseSystemPrintDialog) )
                {
                    DBG_ERROR("Wrong type of \"Misc\\UseSystemPrintDialog\"!" );
                }
                m_bIsUseSystemPrintDialogRO = seqRO[nProperty];
                break;
            }

            case PROPERTYHANDLE_SYMBOLSTYLE :
            {
                ::rtl::OUString aSymbolsStyle;
                if( seqValues[nProperty] >>= aSymbolsStyle )
                    SetSymbolsStyleName( aSymbolsStyle );
                else
                    DBG_ERROR("Wrong type of \"Misc\\SymbolStyle\"!" );
                m_bIsSymbolsStyleRO = seqRO[nProperty];
                break;
            }
        }
    }

    // Enable notification mechanism of ouer baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    EnableNotification( seqNames );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtMiscOptions_Impl::~SvtMiscOptions_Impl()
{
    // We must save our current values .. if user forget it!
    if( IsModified() == sal_True )
    {
        Commit();
    }

    for ( USHORT n=0; n<aList.Count(); )
        delete aList.Remove(n);
}

/*-- 25.02.2005 13:22:04---------------------------------------------------

  -----------------------------------------------------------------------*/
static int lcl_MapPropertyName( const ::rtl::OUString rCompare,
                const uno::Sequence< ::rtl::OUString>& aInternalPropertyNames)
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
    const uno::Sequence< ::rtl::OUString> aInternalPropertyNames( GetPropertyNames());
    Sequence< Any > seqValues = GetProperties( rPropertyNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(rPropertyNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nInvalid property value detected!\n" );
        switch( lcl_MapPropertyName(rPropertyNames[nProperty], aInternalPropertyNames) )
        {
            case PROPERTYHANDLE_PLUGINSENABLED      :   {
                                                            if( !(seqValues[nProperty] >>= m_bPluginsEnabled) )
                                                            {
                                                                DBG_ERROR("Wrong type of \"Misc\\PluginsEnabled\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSET           :   {
                                                            if( !(seqValues[nProperty] >>= m_nSymbolsSize) )
                                                            {
                                                                DBG_ERROR("Wrong type of \"Misc\\SymbolSet\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_TOOLBOXSTYLE        :   {
                                                            if( !(seqValues[nProperty] >>= m_nToolboxStyle) )
                                                            {
                                                                DBG_ERROR("Wrong type of \"Misc\\ToolboxStyle\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_USESYSTEMFILEDIALOG      :   {
                                                            if( !(seqValues[nProperty] >>= m_bUseSystemFileDialog) )
                                                            {
                                                                DBG_ERROR("Wrong type of \"Misc\\UseSystemFileDialog\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_USESYSTEMPRINTDIALOG     :   {
                                                            if( !(seqValues[nProperty] >>= m_bUseSystemPrintDialog) )
                                                            {
                                                                DBG_ERROR("Wrong type of \"Misc\\UseSystemPrintDialog\"!" );
                                                            }
                                                        }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSTYLE         :   {
                                                            ::rtl::OUString aSymbolsStyle;
                                                            if( seqValues[nProperty] >>= aSymbolsStyle )
                                                                SetSymbolsStyleName( aSymbolsStyle );
                                                            else
                                                                DBG_ERROR("Wrong type of \"Misc\\SymbolStyle\"!" );
                                                        }
                                                    break;
        }
    }
}

void SvtMiscOptions_Impl::AddListener( const Link& rLink )
{
    aList.Insert( new Link( rLink ) );
}

void SvtMiscOptions_Impl::RemoveListener( const Link& rLink )
{
    for ( USHORT n=0; n<aList.Count(); n++ )
    {
        if ( (*aList.GetObject(n) ) == rLink )
        {
            delete aList.Remove(n);
            break;
        }
    }
}

void SvtMiscOptions_Impl::CallListeners()
{
    for ( USHORT n = 0; n < aList.Count(); ++n )
        aList.GetObject(n)->Call( this );
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
    return implSymbolsStyleFromVCL( Application::GetSettings().GetStyleSettings().GetSymbolsStyle() );
}

::rtl::OUString SvtMiscOptions_Impl::GetSymbolsStyleName() const
{
    return Application::GetSettings().GetStyleSettings().GetSymbolsStyleName();
}

sal_Int16 SvtMiscOptions_Impl::GetCurrentSymbolsStyle() const
{
    return implSymbolsStyleFromVCL( Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyle() );
}

void SvtMiscOptions_Impl::ImplSetSymbolsStyle( bool bValue, sal_Int16 nSet, const ::rtl::OUString &rName )
{
    if ( ( bValue && ( nSet != GetSymbolsStyle() ) ) ||
         ( !bValue && ( rName != GetSymbolsStyleName() ) ) )
    {
        AllSettings aAllSettings = Application::GetSettings();
        StyleSettings aStyleSettings = aAllSettings.GetStyleSettings();

        if ( bValue )
            aStyleSettings.SetSymbolsStyle( implSymbolsStyleToVCL( nSet ) );
        else
            aStyleSettings.SetSymbolsStyleName( rName );

        aAllSettings.SetStyleSettings(aStyleSettings);
        Application::MergeSystemSettings( aAllSettings );
        Application::SetSettings(aAllSettings);

        SetModified();
        CallListeners();
    }
}

void SvtMiscOptions_Impl::SetPluginsEnabled( sal_Bool bEnable )
{
    m_bPluginsEnabled = bEnable;
    SetModified();
    CallListeners();
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
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_PLUGINSENABLED,
        PROPERTYNAME_SYMBOLSET,
        PROPERTYNAME_TOOLBOXSTYLE,
        PROPERTYNAME_USESYSTEMFILEDIALOG,
        PROPERTYNAME_SYMBOLSTYLE,
        PROPERTYNAME_USESYSTEMPRINTDIALOG
    };

    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
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
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
       RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtMiscOptions_Impl::ctor()");
       m_pDataContainer = new SvtMiscOptions_Impl;
       ItemHolder2::holdConfigItem(E_MISCOPTIONS);
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtMiscOptions::~SvtMiscOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
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

void SvtMiscOptions::SetPluginsEnabled( sal_Bool bEnable )
{
    m_pDataContainer->SetPluginsEnabled( bEnable );
}

sal_Bool SvtMiscOptions::IsPluginsEnabledReadOnly() const
{
    return m_pDataContainer->IsPluginsEnabledReadOnly();
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
        ULONG nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
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

sal_Bool SvtMiscOptions::IsGetSymbolsSizeReadOnly() const
{
    return m_pDataContainer->IsGetSymbolsSizeReadOnly();
}

sal_Int16 SvtMiscOptions::GetSymbolsStyle() const
{
    return m_pDataContainer->GetSymbolsStyle();
}

sal_Int16 SvtMiscOptions::GetCurrentSymbolsStyle() const
{
    return m_pDataContainer->GetCurrentSymbolsStyle();
}

OUString SvtMiscOptions::GetCurrentSymbolsStyleName() const
{
    return Application::GetSettings().GetStyleSettings().GetCurrentSymbolsStyleName();
}

void SvtMiscOptions::SetSymbolsStyle( sal_Int16 nSet )
{
    m_pDataContainer->SetSymbolsStyle( nSet );
}

sal_Bool SvtMiscOptions::IsGetSymbolsStyleReadOnly() const
{
    return m_pDataContainer->IsGetSymbolsStyleReadOnly();
}

sal_Int16 SvtMiscOptions::GetToolboxStyle() const
{
    return m_pDataContainer->GetToolboxStyle();
}

void SvtMiscOptions::SetToolboxStyle( sal_Int16 nStyle )
{
    m_pDataContainer->SetToolboxStyle( nStyle, true );
}

sal_Bool SvtMiscOptions::IsGetToolboxStyleReadOnly() const
{
    return m_pDataContainer->IsGetToolboxStyleReadOnly();
}

sal_Bool SvtMiscOptions::UseSystemPrintDialog() const
{
    return m_pDataContainer->UseSystemPrintDialog();
}

void SvtMiscOptions::SetUseSystemPrintDialog( sal_Bool bEnable )
{
    m_pDataContainer->SetUseSystemPrintDialog( bEnable );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex & SvtMiscOptions::GetInitMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

void SvtMiscOptions::AddListener( const Link& rLink )
{
    m_pDataContainer->AddListener( rLink );
}

void SvtMiscOptions::RemoveListener( const Link& rLink )
{
    m_pDataContainer->RemoveListener( rLink );
}

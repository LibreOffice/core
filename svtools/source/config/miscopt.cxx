/*************************************************************************
 *
 *  $RCSfile: miscopt.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-17 15:23:02 $
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

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include "miscopt.hxx"

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

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

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

#define PROPERTYCOUNT                       4

#define VCL_TOOLBOX_STYLE_FLAT              ((USHORT)0x0004) // from <vcl/toolbox.hxx>

DECLARE_LIST( LinkList, Link * );

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
    sal_Int16   m_nSymbolSet;
    sal_Int16   m_nToolboxStyle;
    sal_Bool    m_bPluginsEnabled;
    sal_Bool    m_bUseSystemFileDialog;
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

        inline sal_Bool IsPluginsEnabled() const
        { return m_bPluginsEnabled; }

        void SetPluginsEnabled( sal_Bool bEnable );

        inline sal_Int16 GetSymbolSet()
        { return m_nSymbolSet; }

        void SetSymbolSet( sal_Int16 nSet );

        // translate to VCL settings ( "0" = 3D, "1" = FLAT )
        inline sal_Int16 GetToolboxStyle()
        { return m_nToolboxStyle ? VCL_TOOLBOX_STYLE_FLAT : 0; }

        // translate from VCL settings
        void SetToolboxStyle( sal_Int16 nStyle, bool _bSetModified );

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
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtMiscOptions_Impl::SvtMiscOptions_Impl()
    // Init baseclasses first
    : ConfigItem( ROOTNODE_MISC )
    , m_nToolboxStyle( 1 )
    , m_nSymbolSet( 0 )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = GetPropertyNames  (           );
    Sequence< Any >         seqValues   = GetProperties     ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_PLUGINSENABLED      :   {
                                                            if( !(seqValues[nProperty] >>= m_bPluginsEnabled) )
                                                                DBG_ERROR("Wrong type of \"Misc\\PluginsEnabled\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSET           :   {
                                                            if( !(seqValues[nProperty] >>= m_nSymbolSet) )
                                                                DBG_ERROR("Wrong type of \"Misc\\SymbolSet\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_TOOLBOXSTYLE        :   {
                                                            if( !(seqValues[nProperty] >>= m_nToolboxStyle) )
                                                                DBG_ERROR("Wrong type of \"Misc\\ToolboxStyle\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_USESYSTEMFILEDIALOG      :   {
                                                            if( !(seqValues[nProperty] >>= m_bUseSystemFileDialog) )
                                                                DBG_ERROR("Wrong type of \"Misc\\PluginsEnabled\"!" );
                                                        }
                                                    break;
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

void SvtMiscOptions_Impl::SetSymbolSet( sal_Int16 nSet )
{
    m_nSymbolSet = nSet;
    SetModified();
    CallListeners();
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
void SvtMiscOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtMiscOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );

    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_PLUGINSENABLED      :   {
                                                            if( !(seqValues[nProperty] >>= m_bPluginsEnabled) )
                                                                DBG_ERROR("Wrong type of \"Misc\\PluginsEnabled\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSET           :   {
                                                            if( !(seqValues[nProperty] >>= m_nSymbolSet) )
                                                                DBG_ERROR("Wrong type of \"Misc\\SymbolSet\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_TOOLBOXSTYLE        :   {
                                                            if( !(seqValues[nProperty] >>= m_nToolboxStyle) )
                                                                DBG_ERROR("Wrong type of \"Misc\\ToolboxStyle\"!" );
                                                        }
                                                    break;
            case PROPERTYHANDLE_USESYSTEMFILEDIALOG      :   {
                                                            if( !(seqValues[nProperty] >>= m_bUseSystemFileDialog) )
                                                                DBG_ERROR("Wrong type of \"Misc\\PluginsEnabled\"!" );
                                                            }
                                                    break;
            default:
                DBG_ERROR( "SvtMiscOptions_Impl::Notify()\nUnkown property detected ... I can't handle these!\n" );
                break;
        }
    }

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
            case PROPERTYHANDLE_PLUGINSENABLED      :   {
                                                        seqValues[nProperty] <<= m_bPluginsEnabled;
                                                    }
                                                    break;
            case PROPERTYHANDLE_SYMBOLSET           :   {
                                                        seqValues[nProperty] <<= m_nSymbolSet;
                                                    }
                                                    break;
            case PROPERTYHANDLE_TOOLBOXSTYLE        :   {
                                                        seqValues[nProperty] <<= m_nToolboxStyle;
                                                    }
                                                    break;
            case PROPERTYHANDLE_USESYSTEMFILEDIALOG      :   {
                                                        seqValues[nProperty] <<= m_bUseSystemFileDialog;
                                                    }
                                                    break;
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
        PROPERTYNAME_USESYSTEMFILEDIALOG
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
        m_pDataContainer = new SvtMiscOptions_Impl;
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

sal_Bool SvtMiscOptions::IsPluginsEnabled() const
{
    return m_pDataContainer->IsPluginsEnabled();
}

void SvtMiscOptions::SetPluginsEnabled( sal_Bool bEnable )
{
    m_pDataContainer->SetPluginsEnabled( bEnable );
}

sal_Int16 SvtMiscOptions::GetSymbolSet() const
{
    return m_pDataContainer->GetSymbolSet();
}

void SvtMiscOptions::SetUseSystemFileDialog( sal_Bool bEnable )
{
    m_pDataContainer->SetUseSystemFileDialog( bEnable );
}

sal_Bool SvtMiscOptions::UseSystemFileDialog() const
{
    return m_pDataContainer->UseSystemFileDialog();
}

void SvtMiscOptions::SetSymbolSet( sal_Int16 nSet )
{
    m_pDataContainer->SetSymbolSet( nSet );
}

sal_Int16 SvtMiscOptions::GetToolboxStyle() const
{
    return m_pDataContainer->GetToolboxStyle();
}

void SvtMiscOptions::SetToolboxStyle( sal_Int16 nStyle )
{
    m_pDataContainer->SetToolboxStyle( nStyle, true );
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

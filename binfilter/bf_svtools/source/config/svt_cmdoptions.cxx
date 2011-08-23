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


//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include <bf_svtools/cmdoptions.hxx>

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

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <itemholder1.hxx>

#include <algorithm>
#include <hash_map>

//_________________________________________________________________________________________________________________
//	namespaces
//_________________________________________________________________________________________________________________

using namespace ::std					;
using namespace ::utl					;
using namespace ::rtl					;
using namespace ::osl					;
using namespace ::com::sun::star::uno	;
using namespace ::com::sun::star::beans	;

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	const
//_________________________________________________________________________________________________________________

#define ROOTNODE_CMDOPTIONS								OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Commands/Execute"	))
#define PATHDELIMITER                                   OUString(RTL_CONSTASCII_USTRINGPARAM("/"						))

#define SETNODE_DISABLED                                OUString(RTL_CONSTASCII_USTRINGPARAM("Disabled"					))

#define PROPERTYNAME_CMD                                OUString(RTL_CONSTASCII_USTRINGPARAM("Command"					))

#define PROPERTYCOUNT                                   1

#define OFFSET_CMD                                      0

//_________________________________________________________________________________________________________________
//	private declarations!
//_________________________________________________________________________________________________________________

// Method to retrieve a hash code from a string. May be we have to change it to decrease collisions in the hash map
struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

/*-****************************************************************************************************************
    @descr  support simple command option structures and operations on it
****************************************************************************************************************-*/
class SvtCmdOptions
{
    public:
        //---------------------------------------------------------------------------------------------------------
        // the only way to free memory!
        void Clear()
        {
            m_aCommandHashMap.clear();
        }

        sal_Bool HasEntries() const
        {
            return ( m_aCommandHashMap.size() > 0 );
        }
        
        void SetContainerSize( sal_Int32 nSize )
        {
            m_aCommandHashMap.resize( nSize );
        }

        sal_Bool Lookup( const OUString& aCmd ) const
        {
            CommandHashMap::const_iterator pEntry = m_aCommandHashMap.find( aCmd );
            return ( pEntry != m_aCommandHashMap.end() );
        }

        void AddCommand( const OUString& aCmd )
        {
            m_aCommandHashMap.insert( CommandHashMap::value_type( aCmd, 0 ) );
        }

        //---------------------------------------------------------------------------------------------------------
        // convert internal list to external format
        // for using it on right menus realy
        // Notice:   We build a property list with 4 entries and set it on result list then.
        //           The while-loop starts with pointer on internal member list lSetupEntries, change to
        //           lUserEntries then and stop after that with NULL!
        //           Separator entries will be packed in another way then normal entries! We define
        //           special strings "sEmpty" and "sSeperator" to perform too ...
        Sequence< OUString > GetList() const
        {
            sal_Int32				nCount = (sal_Int32)m_aCommandHashMap.size();
            sal_Int32				nIndex = 0;
            Sequence< OUString >	aList( nCount );

            CommandHashMap::const_iterator pEntry = m_aCommandHashMap.begin();
            while ( pEntry != m_aCommandHashMap.end() )
                aList[nIndex++] = pEntry->first;

            return aList;
        }

    private:
        class CommandHashMap : public ::std::hash_map< ::rtl::OUString		,
                                                        sal_Int32			,
                                                        OUStringHashCode	,
                                                        ::std::equal_to< ::rtl::OUString >	>
        {
            public:
                inline void free()
                {
                    CommandHashMap().swap( *this );
                }
        };

        CommandHashMap m_aCommandHashMap;
};

typedef ::std::vector< ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame > > SvtFrameVector;

class SvtCommandOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtCommandOptions_Impl();
        ~SvtCommandOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //	overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		called for notify of configmanager
            @descr		These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso	baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& lPropertyNames );

        /*-****************************************************************************************************//**
            @short		write changes to configuration
            @descr		These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso	baseclass ConfigItem

            @param		-
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

        virtual void Commit();

    //-------------------------------------------------------------------------------------------------------------
    //	private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of our configuration management which represent oue module tree
            @descr		These methods return the current list of key names! We need it to get needed values from our
                        configuration management and support dynamical menu item lists!

            @seealso	-

            @param      "nDisabledCount"	,   returns count of menu entries for "new"
            @return		A list of configuration key names is returned.

            @onerror	-
        *//*-*****************************************************************************************************/

        Sequence< OUString > impl_GetPropertyNames();

    //-------------------------------------------------------------------------------------------------------------
    //	private member
    //-------------------------------------------------------------------------------------------------------------

    private:
        SvtCmdOptions  m_aDisabledCommands;
        SvtFrameVector m_lFrames;
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtCommandOptions_Impl::SvtCommandOptions_Impl()
    // Init baseclasses first
    :   ConfigItem( ROOTNODE_CMDOPTIONS )
    // Init member then...
{
    // Get names and values of all accessable menu entries and fill internal structures.
    // See impl_GetPropertyNames() for further informations.
    Sequence< OUString >    lNames              = impl_GetPropertyNames ();
    Sequence< Any >         lValues             = GetProperties         ( lNames         );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtCommandOptions_Impl::SvtCommandOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    // Attention: List for names and values have an internal construction pattern!
    sal_Int32	nItem     = 0 ;
    OUString    sCmd		  ;

    // Set size of hash_map reach a used size of approx. 60%
    m_aDisabledCommands.SetContainerSize( lNames.getLength() * 10 / 6 );

    // Get names/values for disabled commands.
    for( nItem=0; nItem < lNames.getLength(); ++nItem )
    {
        // Currently only one value
        lValues[nItem] >>= sCmd;
        m_aDisabledCommands.AddCommand( sCmd );
    }

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of ouer baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys! */
    Sequence< OUString > aNotifySeq( 1 );
    aNotifySeq[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( "Disabled" ));
    EnableNotification( aNotifySeq, sal_True );
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtCommandOptions_Impl::~SvtCommandOptions_Impl()
{
    // We must save our current values .. if user forget it!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
void SvtCommandOptions_Impl::Notify( const Sequence< OUString >& )
{
    MutexGuard aGuard( SvtCommandOptions::GetOwnStaticMutex() );

    Sequence< OUString >    lNames   = impl_GetPropertyNames ();
    Sequence< Any >         lValues  = GetProperties         ( lNames         );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtCommandOptions_Impl::SvtCommandOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    // Attention: List for names and values have an internal construction pattern!
    sal_Int32	nItem     = 0 ;
    OUString    sCmd		  ;

    // Set size of hash_map reach a used size of approx. 60%
    m_aDisabledCommands.Clear();
    m_aDisabledCommands.SetContainerSize( lNames.getLength() * 10 / 6 );

    // Get names/values for disabled commands.
    for( nItem=0; nItem < lNames.getLength(); ++nItem )
    {
        // Currently only one value
        lValues[nItem] >>= sCmd;
        m_aDisabledCommands.AddCommand( sCmd );
    }

    // dont forget to update all existing frames and her might cached dispatch objects!
    // But look for already killed frames. We hold weak references instead of hard ones ...
    for (SvtFrameVector::const_iterator pIt  = m_lFrames.begin();
                                        pIt != m_lFrames.end()  ;
                                      ++pIt                     )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame(pIt->get(), ::com::sun::star::uno::UNO_QUERY);
        if (xFrame.is())
            xFrame->contextChanged();
    }
}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
void SvtCommandOptions_Impl::Commit()
{
    DBG_ERROR( "SvtCommandOptions_Impl::Commit()\nNot implemented yet!\n" );
}

//*****************************************************************************************************************
//	private method
//*****************************************************************************************************************
Sequence< OUString > SvtCommandOptions_Impl::impl_GetPropertyNames()
{
    // First get ALL names of current existing list items in configuration!
    Sequence< OUString > lDisabledItems      = GetNodeNames( SETNODE_DISABLED, utl::CONFIG_NAME_LOCAL_PATH );

    OUString aSetNode( SETNODE_DISABLED );
    aSetNode += PATHDELIMITER;

    OUString aCommandKey( PATHDELIMITER );
    aCommandKey += PROPERTYNAME_CMD;

    // Expand all keys
    for (sal_Int32 i=0; i<lDisabledItems.getLength(); ++i )
    {
        OUStringBuffer aBuffer( 32 );
        aBuffer.append( aSetNode );
        aBuffer.append( lDisabledItems[i] );
        aBuffer.append( aCommandKey );
        lDisabledItems[i] = aBuffer.makeStringAndClear();
    }

    // Return result.
    return lDisabledItems;
}

//*****************************************************************************************************************
//	initialize static member
//	DON'T DO IT IN YOUR HEADER!
//	see definition for further informations
//*****************************************************************************************************************
SvtCommandOptions_Impl*     SvtCommandOptions::m_pDataContainer = NULL  ;
sal_Int32                   SvtCommandOptions::m_nRefCount      = 0     ;

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtCommandOptions::SvtCommandOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtCommandOptions_Impl;
        ItemHolder1::holdConfigItem(E_CMDOPTIONS);
    }
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtCommandOptions::~SvtCommandOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
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

//*****************************************************************************************************************
//	private method
//*****************************************************************************************************************
Mutex& SvtCommandOptions::GetOwnStaticMutex()
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
}

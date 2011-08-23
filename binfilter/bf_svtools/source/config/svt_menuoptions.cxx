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

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include <bf_svtools/menuoptions.hxx>

#include <unotools/configmgr.hxx>

#include <unotools/configitem.hxx>

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

#include <tools/link.hxx>
#include <tools/list.hxx>

//_________________________________________________________________________________________________________________
//	namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl					;
using namespace ::rtl					;
using namespace ::osl					;
using namespace ::com::sun::star::uno	;

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	const
//_________________________________________________________________________________________________________________

#define	ROOTNODE_MENU							OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/View/Menu"	))
#define	DEFAULT_DONTHIDEDISABLEDENTRIES			sal_False
#define	DEFAULT_FOLLOWMOUSE						sal_True
#define	DEFAULT_MENUICONS						sal_True

#define	PROPERTYNAME_DONTHIDEDISABLEDENTRIES	OUString(RTL_CONSTASCII_USTRINGPARAM("DontHideDisabledEntry"	))
#define	PROPERTYNAME_FOLLOWMOUSE				OUString(RTL_CONSTASCII_USTRINGPARAM("FollowMouse"				))
#define PROPERTYNAME_SHOWICONSINMENUES          OUString(RTL_CONSTASCII_USTRINGPARAM("ShowIconsInMenues"        ))

#define	PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES	0
#define	PROPERTYHANDLE_FOLLOWMOUSE				1
#define PROPERTYHANDLE_SHOWICONSINMENUES        2

#define PROPERTYCOUNT                           3

DECLARE_LIST( LinkList, Link * )

//_________________________________________________________________________________________________________________
//	private declarations!
//_________________________________________________________________________________________________________________

class SvtMenuOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //	private member
    //-------------------------------------------------------------------------------------------------------------

    private:
        LinkList    aList;
        sal_Bool	m_bDontHideDisabledEntries			;	/// cache "DontHideDisabledEntries" of Menu section
        sal_Bool	m_bFollowMouse						;	/// cache "FollowMouse" of Menu section
        sal_Bool	m_bMenuIcons						;	/// cache "MenuIcons" of Menu section

    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtMenuOptions_Impl();
        ~SvtMenuOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //	overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		called for notify of configmanager
            @descr		These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso	baseclass ConfigItem

            @param		"seqPropertyNames" is the list of properties which should be updated.
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );

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

        //---------------------------------------------------------------------------------------------------------
        //	public interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		access method to get internal values
            @descr		These method give us a chance to regulate acces to ouer internal values.
                        It's not used in the moment - but it's possible for the feature!

            @seealso	-

            @param		-
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

        sal_Bool	IsEntryHidingEnabled() const
                    { return m_bDontHideDisabledEntries; }

        sal_Bool	IsFollowMouseEnabled() const
                    { return m_bFollowMouse; }

        sal_Bool	IsMenuIconsEnabled() const
                    { return m_bMenuIcons; }

        void		SetEntryHidingState	( sal_Bool bState )
                    {
                        m_bDontHideDisabledEntries = bState;
                        SetModified();
                        for ( USHORT n=0; n<aList.Count(); n++ )
                            aList.GetObject(n)->Call( this );
                        Commit();
                    }

        void		SetFollowMouseState	( sal_Bool bState )
                    {
                        m_bFollowMouse = bState;
                        SetModified();
                        for ( USHORT n=0; n<aList.Count(); n++ )
                            aList.GetObject(n)->Call( this );
                        Commit();
                    }

        void		SetMenuIconsState ( sal_Bool bState	)
                    {
                        m_bMenuIcons = bState;
                        SetModified();
                        for ( USHORT n=0; n<aList.Count(); n++ )
                            aList.GetObject(n)->Call( this );
                        Commit();
                    }

    //-------------------------------------------------------------------------------------------------------------
    //	private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short		return list of fix key names of ouer configuration management which represent oue module tree
            @descr		These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso	-

            @param		-
            @return		A list of needed configuration keys is returned.

            @onerror	-
        *//*-*****************************************************************************************************/

        static Sequence< OUString > impl_GetPropertyNames();
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtMenuOptions_Impl::SvtMenuOptions_Impl()
    // Init baseclasses first
    :	ConfigItem					( ROOTNODE_MENU						)
    // Init member then.
    ,	m_bDontHideDisabledEntries	( DEFAULT_DONTHIDEDISABLEDENTRIES	)
    ,	m_bFollowMouse				( DEFAULT_FOLLOWMOUSE				)
    ,   m_bMenuIcons				( DEFAULT_MENUICONS 				)
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >	seqNames	= impl_GetPropertyNames();
    Sequence< Any >			seqValues	= GetProperties( seqNames )	;

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount	=	seqValues.getLength()	;
    sal_Int32 nProperty			=	0						;
    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nInvalid property value for property detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES	:	{
                                                                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\DontHideDisabledEntry\"?" );
                                                                seqValues[nProperty] >>= m_bDontHideDisabledEntries;
                                                            }
                                                            break;

            case PROPERTYHANDLE_FOLLOWMOUSE				:	{
                                                                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\FollowMouse\"?" );
                                                                seqValues[nProperty] >>= m_bFollowMouse;
                                                            }
                                                            break;
            case PROPERTYHANDLE_SHOWICONSINMENUES       :   {
                                                                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\ShowIconsInMenues\"?" );
                                                                seqValues[nProperty] >>= m_bMenuIcons;
                                                            }
                                                            break;
        }
    }

    EnableNotification( seqNames );
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtMenuOptions_Impl::~SvtMenuOptions_Impl()
{
    // Flush data to configuration!
    // User has no chance to do that.
    if( IsModified() == sal_True )
    {
        Commit();
    }

    for ( USHORT n=0; n<aList.Count(); )
        delete aList.Remove(n);
}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
void SvtMenuOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtMenuOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        if( seqPropertyNames[nProperty] == PROPERTYNAME_DONTHIDEDISABLEDENTRIES )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\View\\Menu\\DontHideDisabledEntry\"?" );
            seqValues[nProperty] >>= m_bDontHideDisabledEntries;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_FOLLOWMOUSE )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\View\\Menu\\FollowMouse\"?" );
            seqValues[nProperty] >>= m_bFollowMouse;
        }
        if( seqPropertyNames[nProperty] == PROPERTYNAME_SHOWICONSINMENUES )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\ShowIconsInMenues\"?" );
            seqValues[nProperty] >>= m_bMenuIcons;
        }
        #if OSL_DEBUG_LEVEL > 1
        else DBG_ASSERT( sal_False, "SvtMenuOptions_Impl::Notify()\nUnkown property detected ... I can't handle these!\n" );
        #endif
    }

    for ( USHORT n=0; n<aList.Count(); n++ )
        aList.GetObject(n)->Call( this );
}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
void SvtMenuOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >	seqNames	= impl_GetPropertyNames();
    sal_Int32				nCount		= seqNames.getLength();
    Sequence< Any >			seqValues	( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES	:	{
                                                                seqValues[nProperty] <<= m_bDontHideDisabledEntries;
                                                            }
                                                            break;

            case PROPERTYHANDLE_FOLLOWMOUSE				:	{
                                                                seqValues[nProperty] <<= m_bFollowMouse;
                                                            }
                                                            break;
            case PROPERTYHANDLE_SHOWICONSINMENUES       :   {
                                                                seqValues[nProperty] <<= m_bMenuIcons;
                                                            }
                                                            break;
        }
    }
    // Set properties in configuration.
    PutProperties( seqNames, seqValues );
}

//*****************************************************************************************************************
//	private method
//*****************************************************************************************************************
Sequence< OUString > SvtMenuOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_DONTHIDEDISABLEDENTRIES	,
        PROPERTYNAME_FOLLOWMOUSE				,
        PROPERTYNAME_SHOWICONSINMENUES
    };
    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//	initialize static member
//	DON'T DO IT IN YOUR HEADER!
//	see definition for further informations
//*****************************************************************************************************************
SvtMenuOptions_Impl*	SvtMenuOptions::m_pDataContainer	= NULL	;
sal_Int32				SvtMenuOptions::m_nRefCount			= 0		;

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtMenuOptions::SvtMenuOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtMenuOptions_Impl::ctor()");
        m_pDataContainer = new SvtMenuOptions_Impl();
        
        ItemHolder1::holdConfigItem(E_MENUOPTIONS);
    }
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtMenuOptions::~SvtMenuOptions()
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
Mutex& SvtMenuOptions::GetOwnStaticMutex()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

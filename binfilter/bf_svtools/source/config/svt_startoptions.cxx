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

#include <bf_svtools/startoptions.hxx>

#include <unotools/configmgr.hxx>

#include <unotools/configitem.hxx>

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

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

#define	DEFAULT_SHOWINTRO				sal_True
#define	DEFAULT_CONNECTIONURL			OUString()

#define	ROOTNODE_START			   		OUString(RTL_CONSTASCII_USTRINGPARAM("Setup/Office"	))
#define	PROPERTYNAME_SHOWINTRO			OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupShowIntro"	))
#define	PROPERTYNAME_CONNECTIONURL		OUString(RTL_CONSTASCII_USTRINGPARAM("ooSetupConnectionURL"	))

#define	PROPERTYHANDLE_SHOWINTRO		0
#define	PROPERTYHANDLE_CONNECTIONURL	1

#define	PROPERTYCOUNT					2

//_________________________________________________________________________________________________________________
//	private declarations!
//_________________________________________________________________________________________________________________

class SvtStartOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtStartOptions_Impl();
        ~SvtStartOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //	overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		called for notify of configmanager
            @descr		These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @ATTENTION	We don't implement these method - because we support readonly values at runtime only!

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

            @ATTENTION	We don't implement these method - because we support readonly values at runtime only!

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
            @short		return list of fix key names of ouer configuration management which represent oue module tree
            @descr		These methods return a static const list of key names. We need it to get needed values from our
                        configuration management. We return well known key names only - because the "UserData" node
                        is handled in a special way!

            @seealso	-

            @param		-
            @return		A list of needed configuration keys is returned.

            @onerror	-
        *//*-*****************************************************************************************************/

        static Sequence< OUString > impl_GetPropertyNames();

    //-------------------------------------------------------------------------------------------------------------
    //	private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        sal_Bool	m_bShowIntro		;	/// cache "ShowIntro" of Start section
        OUString	m_sConnectionURL	;	/// cache "Connection" of Start section
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtStartOptions_Impl::SvtStartOptions_Impl()
    // Init baseclasses first
    :	ConfigItem			( ROOTNODE_START	)
    // Init member then.
    ,	m_bShowIntro		( DEFAULT_SHOWINTRO	)
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >	seqNames	= impl_GetPropertyNames();
    Sequence< Any >			seqValues	= GetProperties( seqNames )	;

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtStartOptions_Impl::SvtStartOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount	=	seqValues.getLength()	;
    sal_Int32 nProperty			=	0						;
    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtStartOptions_Impl::SvtStartOptions_Impl()\nInvalid property value for property detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_SHOWINTRO		:	{
                                                            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtStartOptions_Impl::SvtStartOptions_Impl()\nWho has changed the value type of \"Office.Common\\Start\\ShowIntro\"?" );
                                                        seqValues[nProperty] >>= m_bShowIntro;
                                                    }
                                                    break;

            case PROPERTYHANDLE_CONNECTIONURL	:	{
                                                         DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_STRING), "SvtStartOptions_Impl::SvtStartOptions_Impl()\nWho has changed the value type of \"Office.Common\\Start\\Connection\"?" );
                                                        seqValues[nProperty] >>= m_sConnectionURL;
                                                    }
                                                    break;
        }
    }

    // Don't enable notification mechanism of ouer baseclass!
    // We support readonly variables in the moment.
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtStartOptions_Impl::~SvtStartOptions_Impl()
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
void SvtStartOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtStartOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        if( seqPropertyNames[nProperty] == PROPERTYNAME_SHOWINTRO )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtStartOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Start\\ShowIntro\"?" );
            seqValues[nProperty] >>= m_bShowIntro;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_CONNECTIONURL )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_STRING), "SvtStartOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Start\\Connection\"?" );
            seqValues[nProperty] >>= m_sConnectionURL;
        }
        #if OSL_DEBUG_LEVEL > 1
        else DBG_ASSERT( sal_False, "SvtStartOptions_Impl::Notify()\nUnkown property detected ... I can't handle these!\n" );
        #endif
    }
}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
void SvtStartOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >	seqNames	= impl_GetPropertyNames();
    sal_Int32				nCount		= seqNames.getLength();
    Sequence< Any >			seqValues	( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_SHOWINTRO		:	{
                                                         seqValues[nProperty] <<= m_bShowIntro;
                                                     }
                                                     break;
            case PROPERTYHANDLE_CONNECTIONURL	:	{
                                                         seqValues[nProperty] <<= m_sConnectionURL;
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
Sequence< OUString > SvtStartOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_SHOWINTRO		,
        PROPERTYNAME_CONNECTIONURL	,
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
SvtStartOptions_Impl*	SvtStartOptions::m_pDataContainer	= NULL	;
sal_Int32				SvtStartOptions::m_nRefCount		= 0		;

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtStartOptions::SvtStartOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtStartOptions_Impl::ctor()");
        m_pDataContainer = new SvtStartOptions_Impl();
         
        ItemHolder1::holdConfigItem(E_STARTOPTIONS);
    }
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtStartOptions::~SvtStartOptions()
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
Mutex& SvtStartOptions::GetOwnStaticMutex()
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

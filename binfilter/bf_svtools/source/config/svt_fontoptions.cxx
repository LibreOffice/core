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

#ifndef GCC
#endif

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include <bf_svtools/fontoptions.hxx>

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

#define	ROOTNODE_FONT						OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Font"			))

#define	PROPERTYNAME_REPLACEMENTTABLE		OUString(RTL_CONSTASCII_USTRINGPARAM("Substitution/Replacement"		))
#define	PROPERTYNAME_FONTHISTORY			OUString(RTL_CONSTASCII_USTRINGPARAM("View/History"					))
#define	PROPERTYNAME_FONTWYSIWYG			OUString(RTL_CONSTASCII_USTRINGPARAM("View/ShowFontBoxWYSIWYG"		))

#define	PROPERTYHANDLE_REPLACEMENTTABLE		0
#define	PROPERTYHANDLE_FONTHISTORY			1
#define	PROPERTYHANDLE_FONTWYSIWYG			2

#define	PROPERTYCOUNT						3

//_________________________________________________________________________________________________________________
//	private declarations!
//_________________________________________________________________________________________________________________

class SvtFontOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtFontOptions_Impl();
        ~SvtFontOptions_Impl();

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

    //-------------------------------------------------------------------------------------------------------------
    //	private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short		return list of key names of ouer configuration management which represent oue module tree
            @descr		These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

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

        sal_Bool		m_bReplacementTable		;
        sal_Bool		m_bFontHistory			;
        sal_Bool		m_bFontWYSIWYG			;
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtFontOptions_Impl::SvtFontOptions_Impl()
    // Init baseclasses first
    :	ConfigItem			( ROOTNODE_FONT	)
    // Init member then.
    ,	m_bReplacementTable	( sal_False		)
    ,	m_bFontHistory		( sal_False		)
    ,	m_bFontWYSIWYG		( sal_False		)
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >	seqNames	= impl_GetPropertyNames	(			);
    Sequence< Any >			seqValues	= GetProperties			( seqNames	);

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_REPLACEMENTTABLE	:	{
                                                            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nWho has changed the value type of \"Office.Common\\Font\\Substitution\\Replacement\"?" );
                                                            seqValues[nProperty] >>= m_bReplacementTable;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTHISTORY			:	{
                                                            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nWho has changed the value type of \"Office.Common\\Font\\View\\History\"?" );
                                                            seqValues[nProperty] >>= m_bFontHistory;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTWYSIWYG			:	{
                                                            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nWho has changed the value type of \"Office.Common\\Font\\View\\ShowFontBoxWYSIWYG\"?" );
                                                            seqValues[nProperty] >>= m_bFontWYSIWYG;
                                                        }
                                                        break;
        }
    }

    // Enable notification mechanism of ouer baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    EnableNotification( seqNames );
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtFontOptions_Impl::~SvtFontOptions_Impl()
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
void SvtFontOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtFontOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        if( seqPropertyNames[nProperty] == PROPERTYNAME_REPLACEMENTTABLE )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Font\\Substitution\\Replacement\"?" );
            seqValues[nProperty] >>= m_bReplacementTable;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_FONTHISTORY )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Font\\View\\History\"?" );
            seqValues[nProperty] >>= m_bFontHistory;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_FONTWYSIWYG )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Font\\View\\ShowFontBoxWYSIWYG\"?" );
            seqValues[nProperty] >>= m_bFontWYSIWYG;
        }
        #if OSL_DEBUG_LEVEL > 1
        else DBG_ASSERT( sal_False, "SvtFontOptions_Impl::Notify()\nUnkown property detected ... I can't handle these!\n" );
        #endif
    }
}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
void SvtFontOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >	seqNames	= impl_GetPropertyNames();
    sal_Int32				nCount		= seqNames.getLength();
    Sequence< Any >			seqValues	( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_REPLACEMENTTABLE	:	{
                                                            seqValues[nProperty] <<= m_bReplacementTable;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTHISTORY			:	{
                                                            seqValues[nProperty] <<= m_bFontHistory;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTWYSIWYG			:	{
                                                            seqValues[nProperty] <<= m_bFontWYSIWYG;
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
Sequence< OUString > SvtFontOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_REPLACEMENTTABLE	,
        PROPERTYNAME_FONTHISTORY		,
        PROPERTYNAME_FONTWYSIWYG		,
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
SvtFontOptions_Impl*	SvtFontOptions::m_pDataContainer	= NULL	;
sal_Int32				SvtFontOptions::m_nRefCount			= 0		;

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtFontOptions::SvtFontOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtFontOptions_Impl::ctor()");
        m_pDataContainer = new SvtFontOptions_Impl;
        
        ItemHolder1::holdConfigItem(E_FONTOPTIONS);
    }
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtFontOptions::~SvtFontOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
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
Mutex& SvtFontOptions::impl_GetOwnStaticMutex()
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

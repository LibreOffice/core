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

#include <bf_svtools/extendedsecurityoptions.hxx>

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

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <bf_svtools/pathoptions.hxx>

#include <hash_map>

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

#define	ROOTNODE_SECURITY				OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Security"))

#define SECURE_EXTENSIONS_SET			OUString(RTL_CONSTASCII_USTRINGPARAM("SecureExtensions"))
#define EXTENSION_PROPNAME				OUString(RTL_CONSTASCII_USTRINGPARAM("/Extension"))

#define PROPERTYNAME_HYPERLINKS_OPEN	OUString(RTL_CONSTASCII_USTRINGPARAM("Hyperlinks/Open"))

#define PROPERTYHANDLE_HYPERLINKS_OPEN	0

#define PROPERTYCOUNT                   1

//_________________________________________________________________________________________________________________
//	private declarations!
//_________________________________________________________________________________________________________________

struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

class ExtensionHashMap : public ::std::hash_map< ::rtl::OUString,
                                                 sal_Int32,
                                                 OUStringHashCode,
                                                 ::std::equal_to< ::rtl::OUString > >
{
    public:
        inline void free()
        {
            ExtensionHashMap().swap( *this );
        }
};

class SvtExtendedSecurityOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtExtendedSecurityOptions_Impl();
        ~SvtExtendedSecurityOptions_Impl();

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

        static Sequence< OUString > GetPropertyNames();

        /*-****************************************************************************************************//**
            @short		Fills the hash map with all extensions known to be secure
            @descr		These methods fills the given hash map object with all extensions known to be secure.

            @seealso	-

            @param		aHashMap
                        A hash map to be filled with secure extension strings.
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/
        void FillExtensionHashMap( ExtensionHashMap& aHashMap );

    //-------------------------------------------------------------------------------------------------------------
    //	private member
    //-------------------------------------------------------------------------------------------------------------

    private:
        OUString										m_aSecureExtensionsSetName;
        OUString										m_aExtensionPropName;

        SvtExtendedSecurityOptions::OpenHyperlinkMode	m_eOpenHyperlinkMode;
        sal_Bool                                        m_bROOpenHyperlinkMode;
        ExtensionHashMap								m_aExtensionHashMap;
};

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()
    // Init baseclasses first
    :	ConfigItem			( ROOTNODE_SECURITY			),
    m_aSecureExtensionsSetName( SECURE_EXTENSIONS_SET ),
    m_aExtensionPropName( EXTENSION_PROPNAME ),
    m_bROOpenHyperlinkMode(sal_False)
    // Init member then.
{
    // Fill the extension hash map with all secure extension strings
    FillExtensionHashMap( m_aExtensionHashMap );

    Sequence< OUString >	seqNames	= GetPropertyNames();
    Sequence< Any >			seqValues	= GetProperties( seqNames );
    Sequence< sal_Bool >    seqRO       = GetReadOnlyStates ( seqNames  );

    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_HYPERLINKS_OPEN:
            {
                DBG_ASSERT( ( seqValues[nProperty].getValueTypeClass() == TypeClass_LONG ), "SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()\nWho has changed the value type of 'Hyperlink/Open'?" );

                sal_Int32 nMode = SvtExtendedSecurityOptions::OPEN_WITHSECURITYCHECK;
                if ( seqValues[nProperty] >>= nMode )
                    m_eOpenHyperlinkMode = (SvtExtendedSecurityOptions::OpenHyperlinkMode)nMode;
                else
                {
                    DBG_ERROR("Wrong type for Open mode!");
                }
                m_bROOpenHyperlinkMode = seqRO[nProperty];
            }
            break;
        }
    }

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    Sequence< OUString > seqNotifyNames( 1 );
    seqNotifyNames[0] = m_aSecureExtensionsSetName;
    EnableNotification( seqNotifyNames );
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions_Impl::~SvtExtendedSecurityOptions_Impl()
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
void SvtExtendedSecurityOptions_Impl::Notify( const Sequence< OUString >& )
{
    // Not implemented
}

//*****************************************************************************************************************
//	public method
//*****************************************************************************************************************
void SvtExtendedSecurityOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >	seqNames	= GetPropertyNames	();
    sal_Int32				nCount		= seqNames.getLength();
    Sequence< Any >			seqValues	( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_HYPERLINKS_OPEN:	{
                                                        seqValues[nProperty] <<= (sal_Int32)m_eOpenHyperlinkMode;
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
void SvtExtendedSecurityOptions_Impl::FillExtensionHashMap( ExtensionHashMap& aHashMap )
{
    // Get sequence with secure extensions from configuration
    Sequence< OUString >	seqNodes = GetNodeNames( m_aSecureExtensionsSetName );

    OUString				aValue;
    Sequence< Any >			aValues;
    Sequence< OUString >	aPropSeq( 1 );
    for ( int i = 0; i < seqNodes.getLength(); i++ )
    {
        // Create access name for property
        OUStringBuffer aExtEntryProp( m_aSecureExtensionsSetName );
        aExtEntryProp.appendAscii( "/" );
        aExtEntryProp.append( seqNodes[i] );
        aExtEntryProp.append( m_aExtensionPropName );

        aPropSeq[0] = aExtEntryProp.makeStringAndClear();
        aValues = GetProperties( aPropSeq );
        if ( aValues.getLength() == 1 )
        {
            // Don't use value if sequence has not the correct length
            if ( aValues[0] >>= aValue )
                // Add extension into secure extensions hash map
                aHashMap.insert( ExtensionHashMap::value_type( aValue.toAsciiLowerCase(), 1 ) );
            else
            {
                DBG_ERRORFILE( "SvtExtendedSecurityOptions_Impl::FillExtensionHashMap(): not string value?" );
            }
        }
    }
}

//*****************************************************************************************************************
//	private method (currently not used)
//*****************************************************************************************************************
Sequence< OUString > SvtExtendedSecurityOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_HYPERLINKS_OPEN
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
SvtExtendedSecurityOptions_Impl*	SvtExtendedSecurityOptions::m_pDataContainer	= NULL	;
sal_Int32							SvtExtendedSecurityOptions::m_nRefCount			= 0		;

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions::SvtExtendedSecurityOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
       RTL_LOGFILE_CONTEXT(aLog, "bf_svtools ( ??? ) ::SvtExtendedSecurityOptions_Impl::ctor()");
       m_pDataContainer = new SvtExtendedSecurityOptions_Impl;
        
        ItemHolder1::holdConfigItem(E_EXTENDEDSECURITYOPTIONS);
    }
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions::~SvtExtendedSecurityOptions()
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

//*****************************************************************************************************************
//	private method
//*****************************************************************************************************************
Mutex& SvtExtendedSecurityOptions::GetInitMutex()
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

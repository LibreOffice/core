/*************************************************************************
 *
 *  $RCSfile: securityoptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: as $ $Date: 2000-10-31 11:38:09 $
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

#include "securityoptions.hxx"

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

#define ROOTNODE_SECURITY               OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/Security/Scripting"))
#define DEFAULT_SECUREURL               Sequence< OUString >()
#define DEFAULT_STAROFFICEBASIC         eALWAYS_EXECUTE

#define PROPERTYNAME_SECUREURL          OUString(RTL_CONSTASCII_USTRINGPARAM("SecureURL"        ))
#define PROPERTYNAME_STAROFFICEBASIC    OUString(RTL_CONSTASCII_USTRINGPARAM("StarOfficeBasic"  ))

#define PROPERTYHANDLE_SECUREURL        0
#define PROPERTYHANDLE_STAROFFICEBASIC  1

#define PROPERTYCOUNT                   2

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

class SvtSecurityOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtSecurityOptions_Impl();
        ~SvtSecurityOptions_Impl();

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

        /*-****************************************************************************************************//**
            @short      access method to get internal values
            @descr      These method give us a chance to regulate acces to ouer internal values.
                        It's not used in the moment - but it's possible for the feature!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        Sequence< OUString >    GetSecureURLs   (                                           ) const ;
        void                    SetSecureURLs   ( const Sequence< OUString >&   seqURLList  )       ;
        EBasicSecurityMode      GetBasicMode    (                                           ) const ;
        void                    SetBasicMode    ( EBasicSecurityMode            eMode       )       ;

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

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        Sequence< OUString >    m_seqSecureURL      ;
        sal_Int32               m_nStarOfficeBasic  ;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_SECURITY         )
    // Init member then.
    ,   m_seqSecureURL      ( DEFAULT_SECUREURL         )
    ,   m_nStarOfficeBasic  ( DEFAULT_STAROFFICEBASIC   )
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
            case PROPERTYHANDLE_SECUREURL       :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SEQUENCE), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nWho has changed the value type of \"Security\\SecureURL\"?" );
                                                        seqValues[nProperty] >>= m_seqSecureURL;
                                                    }
                                                    break;

            case PROPERTYHANDLE_STAROFFICEBASIC :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nWho has changed the value type of \"Security\\StarOfficeBasic\"?" );
                                                        seqValues[nProperty] >>= m_nStarOfficeBasic;
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
SvtSecurityOptions_Impl::~SvtSecurityOptions_Impl()
{
    // We must save our current values .. if user forget it!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtSecurityOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        if( seqPropertyNames[nProperty] == PROPERTYNAME_SECUREURL )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SEQUENCE), "SvtSecurityOptions_Impl::Notify()\nWho has changed the value type of \"Security\\SecureURL\"?" );
            seqValues[nProperty] >>= m_seqSecureURL;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_STAROFFICEBASIC )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtSecurityOptions_Impl::Notify()\nWho has changed the value type of \"Security\\StarOfficeBasic\"?" );
            seqValues[nProperty] >>= m_nStarOfficeBasic;
        }
        #ifdef DEBUG
        else DBG_ASSERT( sal_False, "SvtSecurityOptions_Impl::Notify()\nUnkown property detected ... I can't handle these!\n" );
        #endif
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = GetPropertyNames  ();
    sal_Int32               nCount      = seqNames.getLength();
    Sequence< Any >         seqValues   ( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_SECUREURL       :   {
                                                        seqValues[nProperty] <<= m_seqSecureURL;
                                                    }
                                                    break;

            case PROPERTYHANDLE_STAROFFICEBASIC :   {
                                                        seqValues[nProperty] <<= m_nStarOfficeBasic;
                                                    }
                                                    break;
        }
    }
    // Set properties in configuration.
    PutProperties( seqNames, seqValues );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< OUString > SvtSecurityOptions_Impl::GetSecureURLs() const
{
    return m_seqSecureURL;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions_Impl::SetSecureURLs( const Sequence< OUString >& seqURLList )
{
    m_seqSecureURL = seqURLList;
    SetModified();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
EBasicSecurityMode SvtSecurityOptions_Impl::GetBasicMode() const
{
    return ((EBasicSecurityMode)m_nStarOfficeBasic);
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions_Impl::SetBasicMode( EBasicSecurityMode eMode )
{
    m_nStarOfficeBasic = ((sal_Int32)eMode);
    SetModified();
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtSecurityOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_SECUREURL          ,
        PROPERTYNAME_STAROFFICEBASIC    ,
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
SvtSecurityOptions_Impl*    SvtSecurityOptions::m_pDataContainer    = NULL  ;
sal_Int32                   SvtSecurityOptions::m_nRefCount         = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtSecurityOptions::SvtSecurityOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtSecurityOptions_Impl;
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtSecurityOptions::~SvtSecurityOptions()
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
//  public method
//*****************************************************************************************************************
Sequence< OUString > SvtSecurityOptions::GetSecureURLs() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetSecureURLs();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions::SetSecureURLs( const Sequence< OUString >& seqURLList )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetSecureURLs( seqURLList );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
EBasicSecurityMode SvtSecurityOptions::GetBasicMode() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetBasicMode();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions::SetBasicMode( EBasicSecurityMode eMode )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetBasicMode( eMode );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtSecurityOptions::GetInitMutex()
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

/*************************************************************************
 *
 *  $RCSfile: securityoptions.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: as $ $Date: 2000-11-01 12:01:31 $
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

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _WLDCRD_HXX
#include <tools/wldcrd.hxx>
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

        Sequence< OUString >    GetSecureURLs   (                                               ) const ;
        void                    SetSecureURLs   (   const   Sequence< OUString >&   seqURLList  )       ;
        EBasicSecurityMode      GetBasicMode    (                                               ) const ;
        void                    SetBasicMode    (           EBasicSecurityMode      eMode       )       ;
        sal_Bool                IsSecureURL     (   const   OUString&               sURL        ,
                                                    const   OUString&               sReferer    ) const ;

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

        Sequence< OUString >    m_seqSecureURLs     ;
        EBasicSecurityMode      m_eBasicMode    ;
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
    ,   m_seqSecureURLs     ( DEFAULT_SECUREURL         )
    ,   m_eBasicMode    ( DEFAULT_STAROFFICEBASIC   )
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
                                                        seqValues[nProperty] >>= m_seqSecureURLs;
                                                    }
                                                    break;

            case PROPERTYHANDLE_STAROFFICEBASIC :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtSecurityOptions_Impl::SvtSecurityOptions_Impl()\nWho has changed the value type of \"Security\\StarOfficeBasic\"?" );
                                                        sal_Int32 nMode;
                                                        seqValues[nProperty] >>= nMode;
                                                        m_eBasicMode = (EBasicSecurityMode)nMode;
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
            seqValues[nProperty] >>= m_seqSecureURLs;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_STAROFFICEBASIC )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtSecurityOptions_Impl::Notify()\nWho has changed the value type of \"Security\\StarOfficeBasic\"?" );
            sal_Int32 nMode;
            seqValues[nProperty] >>= nMode;
            m_eBasicMode = (EBasicSecurityMode)nMode;
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
                                                        seqValues[nProperty] <<= m_seqSecureURLs;
                                                    }
                                                    break;

            case PROPERTYHANDLE_STAROFFICEBASIC :   {
                                                        seqValues[nProperty] <<= (sal_Int32)m_eBasicMode;
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
    return m_seqSecureURLs;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions_Impl::SetSecureURLs( const Sequence< OUString >& seqURLList )
{
    m_seqSecureURLs = seqURLList;
    SetModified();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
EBasicSecurityMode SvtSecurityOptions_Impl::GetBasicMode() const
{
    return m_eBasicMode;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtSecurityOptions_Impl::SetBasicMode( EBasicSecurityMode eMode )
{
    m_eBasicMode = eMode;
    SetModified();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtSecurityOptions_Impl::IsSecureURL(  const   OUString&   sURL    ,
                                                const   OUString&   sReferer) const
{
    // Set defult return value to "NO" ...
    // If anything goes wrong at follow operations - it's better to say "THIS URL ISN'T SECURE!"! - I think so.
    // Implement follow code to change these state to true only ...
    // All cases which will return false too are "ignored" ...
    sal_Bool bState = sal_False;

    //  1)  Scripting completly impossible?
    //      Don't check these state ... because we have set ouer return value to FALSE ...
    //      See begin of method for further informations.

    //  2)  Scripting always allowed!
    //      Set return value to TRUE.
    if( m_eBasicMode == eALWAYS_EXECUTE )
    {
        bState = sal_True;
    }
    else
    //  3)  Scripting allowed if URL in list!
    if( m_eBasicMode == eFROM_LIST )
    {
        // Check for uncritical protocols first!
        // All protocols different from "macro..." and "slot..." are secure per definition and must not be checked.
        // There exist two exceptions - "macro://#..." and "slot:5500"!
        INetURLObject   aURL        ( sURL );
        INetProtocol    aProtocol   = aURL.GetProtocol();
        if  (
                (
                    ( aProtocol !=  INET_PROT_MACRO )   &&
                    ( aProtocol !=  INET_PROT_SLOT  )
                )   ||
                ( aURL.GetMainURL().CompareIgnoreCaseToAscii( "macro://#", 9 )  ==  COMPARE_EQUAL   )   ||
                ( aURL.GetMainURL().CompareIgnoreCaseToAscii( "slot:5500"    )  ==  COMPARE_EQUAL   )
            )
        {
            bState = sal_True;
        }
        // All other URLs must checked in combination with referer and internal informations about security!
        else
        {
            // Trusted referer given?
            // NO  => bState will be false per default!
            // YES => Search it in ouer internal url list.
            if( sReferer.getLength() > 0 )
            {
                // Search in internal list ...
                sal_uInt32 nCount = m_seqSecureURLs.getLength();
                for( sal_uInt32 nItem=0; nItem<nCount; ++nItem )
                {
                    OUString sCheckURL = m_seqSecureURLs[nItem]+'*';
                    if( WildCard( sCheckURL ).Matches( sReferer ) == sal_True )
                    {
                        bState = sal_True;
                        break;
                    }
                }
            }
        }
    }
    // Return result of operation.
    return bState;
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
//  public method
//*****************************************************************************************************************
sal_Bool SvtSecurityOptions::IsSecureURL(   const   OUString&   sURL        ,
                                            const   OUString&   sReferer    ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->IsSecureURL( sURL, sReferer );
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

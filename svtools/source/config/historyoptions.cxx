/*************************************************************************
 *
 *  $RCSfile: historyoptions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2000-10-31 14:39:08 $
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

#include "historyoptions.hxx"

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

#ifndef __SGI_STL_DEQUE
#include <stl/deque>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::std                   ;
using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ROOTNODE_HISTORY                OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/History/"   ))
#define DEFAULT_PICKLISTSIZE            0
#define DEFAULT_PICKLIST                Sequence< OUString >()
#define DEFAULT_HISTORYSIZE             0
#define DEFAULT_HISTORY                 Sequence< OUString >()

#define PROPERTYNAME_PICKLISTSIZE       OUString(RTL_CONSTASCII_USTRINGPARAM("Picklist/Size"            ))
#define PROPERTYNAME_PICKLIST           OUString(RTL_CONSTASCII_USTRINGPARAM("Picklist/List"            ))
#define PROPERTYNAME_HISTORYSIZE        OUString(RTL_CONSTASCII_USTRINGPARAM("History/Size"             ))
#define PROPERTYNAME_HISTORY            OUString(RTL_CONSTASCII_USTRINGPARAM("History/List"             ))

#define PROPERTYHANDLE_PICKLISTSIZE     0
#define PROPERTYHANDLE_PICKLIST         1
#define PROPERTYHANDLE_HISTORYSIZE      2
#define PROPERTYHANDLE_HISTORY          3

#define PROPERTYCOUNT                   4

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

class SvtHistoryOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtHistoryOptions_Impl();
        ~SvtHistoryOptions_Impl();

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
            @short      base implementation of public interface for "SvtHistoryOptions"!
            @descr      These class is used as static member of "SvtHistoryOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_uInt32              GetSize     ( EHistoryType eHistory                     ) const ;
        void                    SetSize     ( EHistoryType eHistory, sal_uInt32 nSize       )       ;
        void                    Clear       ( EHistoryType eHistory                     )       ;
        Sequence< OUString >    GetList     ( EHistoryType eHistory                     ) const ;
        void                    AppendItem  ( EHistoryType eHistory, const OUSTRING& sItem  )       ;

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

        static Sequence< OUString > impl_GetPropertyNames();

        /*-****************************************************************************************************//**
            @short      convert routines from/to deque/sequences
            @descr      Intern we hold ouer values in a deque. Sometimes we need his content as a return sequence.
                        Or the user will set a sequence ... then we must convert it to ouer internal format.
                        That is the reason for these methods!

            @seealso    -

            @param      "aList" list in deque format.
            @param      "seqList" list in sequence format.
            @return     A list which right format is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        Sequence< OUString >    impl_GetSequenceFromList(   const   deque< OUString >&      aList       ) const ;
        void                    impl_SetSequenceOnList  (   const   Sequence< OUString >&   seqSource   ,
                                                                    deque< OUString >&      aList       ,
                                                                    sal_uInt32              nMaxCount   )       ;

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        deque< OUString >   m_aPicklist     ;
        sal_uInt32          m_nPicklistSize ;
        deque< OUString >   m_aHistory      ;
        sal_uInt32          m_nHistorySize  ;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_HISTORY  )
    // Init member then...
    ,   m_nPicklistSize     ( 0                 )
    ,   m_nHistorySize      ( 0                 )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = impl_GetPropertyNames (           );
    Sequence< Any >         seqValues   = GetProperties         ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_PICKLISTSIZE    :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_UNSIGNED_LONG), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nWho has changed the value type of \"Office.Common\\History\\Picklist\\Size\"?" );
                                                        seqValues[nProperty] >>= m_nPicklistSize;
                                                        DBG_ASSERT(!(m_nPicklistSize<1), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nA picklist size of 0 isn't realy possible ...!\n");
                                                    }
                                                    break;

            case PROPERTYHANDLE_PICKLIST        :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SEQUENCE), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nWho has changed the value type of \"Office.Common\\History\\Picklist\\List\"?" );
                                                        Sequence< OUString > seqPicklist;
                                                        seqValues[nProperty] >>= seqPicklist;
                                                        DBG_ASSERT(!(seqPicklist.getLength()>m_nPicklistSize), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nInconsitency in configuration detected! Picklist is greater then his max size.\n" )
                                                        impl_SetSequenceOnList( seqPicklist, m_aPicklist, m_nPicklistSize );
                                                    }
                                                    break;

            case PROPERTYHANDLE_HISTORYSIZE     :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_UNSIGNED_LONG), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nWho has changed the value type of \"Office.Common\\History\\History\\Size\"?" );
                                                        seqValues[nProperty] >>= m_nHistorySize;
                                                        DBG_ASSERT(!(m_nHistorySize<1), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nA history size of 0 isn't realy possible ...!\n");
                                                    }
                                                    break;

            case PROPERTYHANDLE_HISTORY         :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SEQUENCE), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nWho has changed the value type of \"Office.Common\\History\\History\\List\"?" );
                                                        Sequence< OUString > seqHistory;
                                                        seqValues[nProperty] >>= seqHistory;
                                                        DBG_ASSERT(!(seqHistory.getLength()>m_nHistorySize), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nInconsitency in configuration detected! History is greater then his max size.\n" )
                                                        impl_SetSequenceOnList( seqHistory, m_aHistory, m_nHistorySize );
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
SvtHistoryOptions_Impl::~SvtHistoryOptions_Impl()
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
void SvtHistoryOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of configuration keys to get his values.
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtHistoryOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    // Attention: Use order of names to get right value in value list!
    sal_Int32 nPropertyCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtHistoryOptions_Impl::Notify()\nInvalid property value detected!\n" );
        if( seqPropertyNames[nProperty] == PROPERTYNAME_PICKLISTSIZE )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_UNSIGNED_LONG), "SvtHistoryOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\History\\Picklist\\Size\"?" );
            seqValues[nProperty] >>= m_nPicklistSize;
            DBG_ASSERT(!(m_nPicklistSize<1), "SvtHistoryOptions_Impl::Notify()\nA picklist size of 0 isn't realy possible ...!\n");
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_PICKLIST )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SEQUENCE), "SvtHistoryOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\History\\Picklist\\List\"?" );
            Sequence< OUString > seqPicklist;
            seqValues[nProperty] >>= seqPicklist;
            DBG_ASSERT(!(seqPicklist.getLength()>m_nPicklistSize), "SvtHistoryOptions_Impl::Notify()\nInconsitency in configuration detected! Picklist is greater then his max size.\n" )
            impl_SetSequenceOnList( seqPicklist, m_aPicklist, m_nPicklistSize );
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_HISTORYSIZE )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_UNSIGNED_LONG), "SvtHistoryOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\History\\History\\Size\"?" );
            seqValues[nProperty] >>= m_nHistorySize;
            DBG_ASSERT(!(m_nHistorySize<1), "SvtHistoryOptions_Impl::Notify()\nA history size of 0 isn't realy possible ...!\n");
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_HISTORY )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_SEQUENCE), "SvtHistoryOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\History\\History\\List\"?" );
            Sequence< OUString > seqHistory;
            seqValues[nProperty] >>= seqHistory;
            DBG_ASSERT(!(seqHistory.getLength()>m_nHistorySize), "SvtHistoryOptions_Impl::Notify()\nInconsitency in configuration detected! History is greater then his max size.\n" )
            impl_SetSequenceOnList( seqHistory, m_aHistory, m_nHistorySize );
        }
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = impl_GetPropertyNames ();
    sal_Int32               nCount      = seqNames.getLength    ();
    Sequence< Any >         seqValues   ( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_PICKLISTSIZE    :   {
                                                        seqValues[nProperty] <<= m_nPicklistSize;
                                                    }
                                                    break;

            case PROPERTYHANDLE_PICKLIST        :   {
                                                        seqValues[nProperty] <<= impl_GetSequenceFromList( m_aPicklist );
                                                    }
                                                    break;

            case PROPERTYHANDLE_HISTORYSIZE     :   {
                                                        seqValues[nProperty] <<= m_nHistorySize;
                                                    }
                                                    break;

            case PROPERTYHANDLE_HISTORY         :   {
                                                        seqValues[nProperty] <<= impl_GetSequenceFromList( m_aHistory );
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
sal_uInt32 SvtHistoryOptions_Impl::GetSize( EHistoryType eHistory ) const
{
    // Attention: We return the max. size of our internal lists - That is the capacity not the size!

    // Set default return value if method failed!
    sal_uInt32 nSize = 0;
    // Get size of searched history list.
    switch( eHistory )
    {
        case ePICKLIST  :   {
                                nSize = m_nPicklistSize;
                            }
                            break;

        case eHISTORY   :   {
                                nSize = m_nHistorySize;
                            }
                            break;
    }
    // Return result of operation.
    return nSize;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::SetSize( EHistoryType eHistory, sal_uInt32 nSize )
{
    // Attention: We set the max. size of our internal lists - That is the capacity not the size!
    // Set size of searched history list.
    switch( eHistory )
    {
        case ePICKLIST  :   {
                                // If to much items in current list ...
                                // truncate the oldest items BEFORE you set the new one.
                                if( nSize < m_aPicklist.size() )
                                {
                                    sal_uInt32 nOldItemCount = m_aPicklist.size()-nSize;
                                    while( nOldItemCount>0 )
                                    {
                                        m_aPicklist.pop_back();
                                        --nOldItemCount;
                                    }
                                }
                                m_nPicklistSize = nSize;
                            }
                            break;

        case eHISTORY   :   {
                                if( nSize < m_aHistory.size() )
                                {
                                    sal_uInt32 nOldItemCount = m_aHistory.size()-nSize;
                                    while( nOldItemCount>0 )
                                    {
                                        m_aHistory.pop_back();
                                        --nOldItemCount;
                                    }
                                }
                                m_nHistorySize = nSize;
                            }
                            break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::Clear( EHistoryType eHistory )
{
    // Clear specified history list.
    switch( eHistory )
    {
        case ePICKLIST  :   {
                                m_aPicklist.clear();
                            }
                            break;

        case eHISTORY   :   {
                                m_aHistory.clear();
                            }
                            break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< OUString > SvtHistoryOptions_Impl::GetList( EHistoryType eHistory ) const
{
    // Set default return value.
    Sequence< OUString > seqReturn;
    // Set right list for return.
    switch( eHistory )
    {
        case ePICKLIST  :   {
                                seqReturn = impl_GetSequenceFromList( m_aPicklist );
                            }
                            break;

        case eHISTORY   :   {
                                seqReturn = impl_GetSequenceFromList( m_aHistory );
                            }
                            break;
    }
    return seqReturn;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::AppendItem( EHistoryType eHistory, const OUSTRING& sItem )
{
    switch( eHistory )
    {
        case ePICKLIST  :   {
                                // If current list full ... delete the oldest item.
                                if( m_aPicklist.size() >= m_nPicklistSize )
                                {
                                    m_aPicklist.pop_back();
                                }
                                // Append new item to list.
                                m_aPicklist.push_front( sItem );
                            }
                            break;

        case eHISTORY   :   {
                                // If current list full ... delete the oldest item.
                                if( m_aHistory.size() >= m_nHistorySize )
                                {
                                    m_aHistory.pop_back();
                                }
                                // Append new item to list.
                                m_aHistory.push_front( sItem );
                            }
                            break;
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtHistoryOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_PICKLISTSIZE   ,
        PROPERTYNAME_PICKLIST       ,
        PROPERTYNAME_HISTORYSIZE    ,
        PROPERTYNAME_HISTORY        ,
    };
    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtHistoryOptions_Impl::impl_GetSequenceFromList( const deque< OUString >& aList ) const
{
    // Initialize return sequence with right size.
    sal_Int32 nCount = aList.size();
    Sequence< OUString > seqResult( nCount );
    // Copy items from given to return list.
    for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        seqResult[nItem] = aList[nItem];
    }
    return seqResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::impl_SetSequenceOnList( const Sequence< OUString >& seqSource, deque< OUString >& aList, sal_uInt32 nMaxCount )
{
    // Attention: We could not enlarge ouer internal lists!
    // We must truncate to much items.
    // The capacity of a deque is ouer maxsize ... see SetSize() for further informations.

    // Get right count of source items to copy in return list!
    aList.clear();
    sal_Int32 nCount = seqSource.getLength();
    if( nCount > nMaxCount )
    {
        nCount = nMaxCount;
    }
    // Copy items from given to return list.
    for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        aList.push_front( seqSource[nItem] );
    }
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtHistoryOptions_Impl*     SvtHistoryOptions::m_pDataContainer = NULL  ;
sal_Int32                   SvtHistoryOptions::m_nRefCount      = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtHistoryOptions::SvtHistoryOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtHistoryOptions_Impl;
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtHistoryOptions::~SvtHistoryOptions()
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
sal_uInt32 SvtHistoryOptions::GetSize( EHistoryType eHistory ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetSize( eHistory );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions::SetSize( EHistoryType eHistory, sal_uInt32 nSize )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->SetSize( eHistory, nSize );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions::Clear( EHistoryType eHistory )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->Clear( eHistory );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< OUString > SvtHistoryOptions::GetList( EHistoryType eHistory ) const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetList( eHistory );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions::AppendItem( EHistoryType eHistory, const OUSTRING& sItem )
{
    MutexGuard aGuard( GetInitMutex() );
    m_pDataContainer->AppendItem( eHistory, sItem );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtHistoryOptions::GetInitMutex()
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

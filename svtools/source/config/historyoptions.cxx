/*************************************************************************
 *
 *  $RCSfile: historyoptions.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: as $ $Date: 2000-11-17 11:04:22 $
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
using namespace ::com::sun::star::beans ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ROOTNODE_HISTORY                        OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/History/"   ))
#define DEFAULT_PICKLISTSIZE                    4
#define DEFAULT_HISTORYSIZE                     10

#define PATHDELIMITER                           OUString(RTL_CONSTASCII_USTRINGPARAM("/"                        ))
#define PROPERTYNAME_PICKLISTSIZE               OUString(RTL_CONSTASCII_USTRINGPARAM("PickListSize"             ))
#define PROPERTYNAME_HISTORYSIZE                OUString(RTL_CONSTASCII_USTRINGPARAM("Size"                     ))
#define PROPERTYNAME_PICKLIST                   OUString(RTL_CONSTASCII_USTRINGPARAM("PickList/"                ))
#define PROPERTYNAME_HISTORY                    OUString(RTL_CONSTASCII_USTRINGPARAM("List/"                    ))
#define PROPERTYNAME_HISTORYITEM_URL            HISTORY_PROPERTYNAME_URL+PATHDELIMITER
#define PROPERTYNAME_HISTORYITEM_FILTER         HISTORY_PROPERTYNAME_FILTER+PATHDELIMITER
#define PROPERTYNAME_HISTORYITEM_TITLE          HISTORY_PROPERTYNAME_TITLE+PATHDELIMITER
#define PROPERTYNAME_HISTORYITEM_PASSWORD       HISTORY_PROPERTYNAME_PASSWORD+PATHDELIMITER
#define OFFSET_URL                              0
#define OFFSET_FILTER                           1
#define OFFSET_TITLE                            2
#define OFFSET_PASSWORD                         3
#define PROPERTYHANDLE_PICKLISTSIZE             0
#define PROPERTYHANDLE_HISTORYSIZE              PROPERTYHANDLE_PICKLISTSIZE+1   //!!!

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

struct IMPL_THistoryItem
{
    IMPL_THistoryItem()
    {
    }

    IMPL_THistoryItem(  const   OUString&   sNewURL         ,
                        const   OUString&   sNewFilter      ,
                        const   OUString&   sNewTitle       ,
                        const   OUString&   sNewPassword    )
    {
        sURL        = sNewURL       ;
        sFilter     = sNewFilter    ;
        sTitle      = sNewTitle     ;
        sPassword   = sNewPassword  ;
    }

    OUString    sURL        ;
    OUString    sFilter     ;
    OUString    sTitle      ;
    OUString    sPassword   ;
};

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

        sal_uInt32                              GetSize     (           EHistoryType    eHistory    ) const ;
        void                                    SetSize     (           EHistoryType    eHistory    ,
                                                                        sal_uInt32      nSize       )       ;
        void                                    Clear       (           EHistoryType    eHistory    )       ;
        Sequence< Sequence< PropertyValue > >   GetList     (           EHistoryType    eHistory    ) const ;
        void                                    AppendItem  (           EHistoryType    eHistory    ,
                                                                const   OUString&       sURL        ,
                                                                const   OUString&       sFilter     ,
                                                                const   OUString&       sTitle      ,
                                                                const   OUString&       sPassword   )       ;

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of ouer configuration management which represent oue module tree
            @descr      These methods return the current list of key names! We need it to get needed values from our
                        configuration management and support dynamical history lists!

            @seealso    -

            @param      -
            @return     A list of configuration key names is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        Sequence< OUString > impl_GetPropertyNames( sal_uInt32& nPicklistCount, sal_uInt32& nHistoryCount );

        /*-****************************************************************************************************//**
            @short      convert routine
            @descr      Intern we hold ouer values in a deque. Sometimes we need his content as a return sequence.
                        Then we must convert ouer internal format to extern.
                        That is the reason for these method!

            @seealso    -

            @param      "aList" list in deque format.
            @return     A list which right format is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        Sequence< Sequence< PropertyValue > > impl_GetSequenceFromList( const deque< IMPL_THistoryItem >& aList ) const ;

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        deque< IMPL_THistoryItem >  m_aPicklist     ;
        sal_uInt32                  m_nPicklistSize ;
        deque< IMPL_THistoryItem >  m_aHistory      ;
        sal_uInt32                  m_nHistorySize  ;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_HISTORY      )
    // Init member then...
{
    // Use our list snapshot of configuration keys to get his values.
    // See impl_GetPropertyNames() for further informations.
    sal_uInt32 nPicklistCount   = 0;
    sal_uInt32 nHistoryCount    = 0;
    Sequence< OUString >    seqNames    = impl_GetPropertyNames ( nPicklistCount, nHistoryCount );
    Sequence< Any >         seqValues   = GetProperties         ( seqNames                      );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    // Attention: List for names and values have an internal construction pattern!
    // zB:
    //      Name                        Value
    //      /Picklist/Size              2
    //      /History/Size               3
    //      /Picklist/List/1/URL        "file://a"
    //      /Picklist/List/1/Filter     "writer-..."
    //      /Picklist/List/1/Title      "Test1"
    //      /Picklist/List/1/Password   "lysemyf1"
    //      /Picklist/List/2/URL        "file://b"
    //      /Picklist/List/2/Filter     "calc-..."
    //      /Picklist/List/2/Title      "Test2"
    //      /Picklist/List/2/Password   "lysemyf2"
    //      /History/List/2/URL         "http://blub"
    //      /History/List/2/Filter      "html-..."
    //      /History/List/2/Title       "blub"
    //      /History/List/2/Password    "xxx"
    //      ... and so on ...

    // First we must read sizes of ouer history lists => the first to values.
    // We need these informations to work correctly with follow keys!
    seqValues[PROPERTYHANDLE_PICKLISTSIZE] >>= m_nPicklistSize;
    seqValues[PROPERTYHANDLE_HISTORYSIZE ] >>= m_nHistorySize ;

    // Safe impossible cases.
    // I think a size of 0 isn't relay meaningful.
    if( m_nPicklistSize < 1 )
    {
        m_nPicklistSize = DEFAULT_PICKLISTSIZE;
        DBG_ASSERT( sal_False, "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nI think a picklist size of 0 isn't relay meaningful! Set new value to 4 entries.\n" );
    }
    if( m_nHistorySize < 1 )
    {
        m_nHistorySize = DEFAULT_HISTORYSIZE;
        DBG_ASSERT( sal_False, "SvtHistoryOptions_Impl::SvtHistoryOptions_Impl()\nI think a history size of 0 isn't relay meaningful! Set new value to 10 entries.\n" );
    }

    IMPL_THistoryItem aItem;
    sal_uInt32 nPosition = PROPERTYHANDLE_HISTORYSIZE+1;
    // Get names/values for picklist.
    // 4 subkeys for every item!
    for( sal_Int32 nItem=0; nItem<nPicklistCount; ++nItem )
    {
        seqValues[nPosition+OFFSET_URL      ] >>= aItem.sURL        ;
        seqValues[nPosition+OFFSET_FILTER   ] >>= aItem.sFilter     ;
        seqValues[nPosition+OFFSET_TITLE    ] >>= aItem.sTitle      ;
        seqValues[nPosition+OFFSET_PASSWORD ] >>= aItem.sPassword   ;
        m_aPicklist.push_front( aItem );
        ++nPosition;
    }

    // Attention: Don't reset nPosition here!

    // Get names/values for picklist.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nHistoryCount; ++nItem )
    {
        seqValues[nPosition+OFFSET_URL      ] >>= aItem.sURL        ;
        seqValues[nPosition+OFFSET_FILTER   ] >>= aItem.sFilter     ;
        seqValues[nPosition+OFFSET_TITLE    ] >>= aItem.sTitle      ;
        seqValues[nPosition+OFFSET_PASSWORD ] >>= aItem.sPassword   ;
        m_aHistory.push_front( aItem );
        ++nPosition;
    }

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of ouer baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    Sequence< OUString > seqNotifications( seqNames );
    sal_Int32 nNotifyCount = seqNames.getLength();
    seqNotifications.realloc( nNotifyCount+PROPERTYCOUNT_LISTNODES );
    seqNotification[nNotifyCount  ] = PROPERTYNAME_PICKLIST;
    seqNotification[nNotifyCount+1] = PROPERTYNAME_HISTORY ;
    EnableNotification( seqNotification );
*/
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
    DBG_ASSERT( sal_False, "SvtHistoryOptions_Impl::Notify()\nNot implemented yet! I don't know how I can handle a dynamical list of unknown properties ...\n" );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions_Impl::Commit()
{
    // First write fix properties.
    Sequence< OUString >    seqFixPropertyNames ( 2 );
    Sequence< Any >         seqFixPropertyValues( 2 );
    seqFixPropertyNames [PROPERTYHANDLE_PICKLISTSIZE] =   PROPERTYNAME_PICKLISTSIZE ;
    seqFixPropertyNames [PROPERTYHANDLE_HISTORYSIZE ] =   PROPERTYNAME_HISTORYSIZE  ;
    seqFixPropertyValues[PROPERTYHANDLE_PICKLISTSIZE] <<= m_nPicklistSize           ;
    seqFixPropertyValues[PROPERTYHANDLE_HISTORYSIZE ] <<= m_nHistorySize            ;
    PutProperties( seqFixPropertyNames, seqFixPropertyValues );

    // Write set of dynamic properties then.
    ClearNodeSet( PROPERTYNAME_PICKLIST );
    ClearNodeSet( PROPERTYNAME_HISTORY  );

    IMPL_THistoryItem           aItem                   ;
    OUString                    sNode                   ;
    Sequence< PropertyValue >   seqPropertyValues( 4 )  ;

    // Copy picklist entries to save-list!
    sal_uInt32 nPicklistCount = m_aPicklist.size();
    for( sal_uInt32 nItem=0; nItem<nPicklistCount; ++nItem )
    {
        aItem = m_aPicklist[nItem];
        seqPropertyValues[OFFSET_URL        ].Name  =   PROPERTYNAME_HISTORYITEM_URL        ;
        seqPropertyValues[OFFSET_FILTER     ].Name  =   PROPERTYNAME_HISTORYITEM_FILTER     ;
        seqPropertyValues[OFFSET_TITLE      ].Name  =   PROPERTYNAME_HISTORYITEM_TITLE      ;
        seqPropertyValues[OFFSET_PASSWORD   ].Name  =   PROPERTYNAME_HISTORYITEM_PASSWORD   ;
        seqPropertyValues[OFFSET_URL        ].Value <<= aItem.sURL                          ;
        seqPropertyValues[OFFSET_FILTER     ].Value <<= aItem.sFilter                       ;
        seqPropertyValues[OFFSET_TITLE      ].Value <<= aItem.sTitle                        ;
        seqPropertyValues[OFFSET_PASSWORD   ].Value <<= aItem.sPassword                     ;

        sNode = PROPERTYNAME_PICKLIST + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;
        SetSetProperties( sNode, seqPropertyValues );
    }

    // Copy URL-list entries to save-list!
    sal_uInt32 nHistoryCount = m_aHistory.size();
    for( nItem=0; nItem<nHistoryCount; ++nItem )
    {
        aItem = m_aHistory[nItem];
        seqPropertyValues[OFFSET_URL        ].Name  =   PROPERTYNAME_HISTORYITEM_URL        ;
        seqPropertyValues[OFFSET_FILTER     ].Name  =   PROPERTYNAME_HISTORYITEM_FILTER     ;
        seqPropertyValues[OFFSET_TITLE      ].Name  =   PROPERTYNAME_HISTORYITEM_TITLE      ;
        seqPropertyValues[OFFSET_PASSWORD   ].Name  =   PROPERTYNAME_HISTORYITEM_PASSWORD   ;
        seqPropertyValues[OFFSET_URL        ].Value <<= aItem.sURL                          ;
        seqPropertyValues[OFFSET_FILTER     ].Value <<= aItem.sFilter                       ;
        seqPropertyValues[OFFSET_TITLE      ].Value <<= aItem.sTitle                        ;
        seqPropertyValues[OFFSET_PASSWORD   ].Value <<= aItem.sPassword                     ;

        sNode = PROPERTYNAME_HISTORY + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;
        SetSetProperties( sNode, seqPropertyValues );
    }
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
                                SetModified();
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
                                SetModified();
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
                                SetModified();
                            }
                            break;

        case eHISTORY   :   {
                                m_aHistory.clear();
                                SetModified();
                            }
                            break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtHistoryOptions_Impl::GetList( EHistoryType eHistory ) const
{
    // Set default return value.
    Sequence< Sequence< PropertyValue > > seqReturn;
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
void SvtHistoryOptions_Impl::AppendItem(            EHistoryType    eHistory    ,
                                            const   OUString&       sURL        ,
                                            const   OUString&       sFilter     ,
                                            const   OUString&       sTitle      ,
                                            const   OUString&       sPassword   )
{
    IMPL_THistoryItem aItem( sURL, sFilter, sTitle, sPassword );

    switch( eHistory )
    {
        case ePICKLIST  :   {
                                // If current list full ... delete the oldest item.
                                if( m_aPicklist.size() >= m_nPicklistSize )
                                {
                                    m_aPicklist.pop_back();
                                }
                                // Append new item to list.
                                m_aPicklist.push_front( aItem );
                                SetModified();
                            }
                            break;

        case eHISTORY   :   {
                                // If current list full ... delete the oldest item.
                                if( m_aHistory.size() >= m_nHistorySize )
                                {
                                    m_aHistory.pop_back();
                                }
                                // Append new item to list.
                                m_aHistory.push_front( aItem );
                                SetModified();
                            }
                            break;
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtHistoryOptions_Impl::impl_GetPropertyNames( sal_uInt32& nPicklistCount, sal_uInt32& nHistoryCount )
{
    // First get ALL names of current existing list items in configuration!
    Sequence< OUString > seqPicklistItems = GetNodeNames( PROPERTYNAME_PICKLIST );
    Sequence< OUString > seqHistoryItems  = GetNodeNames( PROPERTYNAME_HISTORY  );
    // Get information about list counts ...
    nPicklistCount = seqPicklistItems.getLength();
    nHistoryCount  = seqHistoryItems.getLength ();
    // ... and create a property list with right size! (+2...see fix properties below!)
    Sequence< OUString > seqProperties( 2 + (nPicklistCount*4) + (nHistoryCount*4) );

    // Add names of fix properties to list.
    seqProperties[PROPERTYHANDLE_PICKLISTSIZE]  =   PROPERTYNAME_PICKLISTSIZE   ;
    seqProperties[PROPERTYHANDLE_HISTORYSIZE ]  =   PROPERTYNAME_HISTORYSIZE    ;

    sal_uInt32 nPosition = PROPERTYHANDLE_HISTORYSIZE+1;
    // Add names for picklist to list.
    // 4 subkeys for every item!
    // nPosition is the start point of an history item, nItem an index into right list of node names!
    for( sal_Int32 nItem=0; nItem<nPicklistCount; ++nItem )
    {
        seqProperties[nPosition+OFFSET_URL      ] = PROPERTYNAME_PICKLIST + seqPicklistItems[nItem] + PROPERTYNAME_HISTORYITEM_URL      ;
        seqProperties[nPosition+OFFSET_FILTER   ] = PROPERTYNAME_PICKLIST + seqPicklistItems[nItem] + PROPERTYNAME_HISTORYITEM_FILTER   ;
        seqProperties[nPosition+OFFSET_TITLE    ] = PROPERTYNAME_PICKLIST + seqPicklistItems[nItem] + PROPERTYNAME_HISTORYITEM_TITLE    ;
        seqProperties[nPosition+OFFSET_PASSWORD ] = PROPERTYNAME_PICKLIST + seqPicklistItems[nItem] + PROPERTYNAME_HISTORYITEM_PASSWORD ;
        ++nPosition;
    }

    // Attention: Don't reset nPosition here!

    // Add names for URL-list to list.
    // 4 subkeys for every item!
    // nPosition is the start point of an history item, nItem an index into right list of node names!
    for( nItem=0; nItem<nHistoryCount; ++nItem )
    {
        seqProperties[nPosition+OFFSET_URL      ] = PROPERTYNAME_HISTORY + seqHistoryItems[nItem] + PROPERTYNAME_HISTORYITEM_URL        ;
        seqProperties[nPosition+OFFSET_FILTER   ] = PROPERTYNAME_HISTORY + seqHistoryItems[nItem] + PROPERTYNAME_HISTORYITEM_FILTER     ;
        seqProperties[nPosition+OFFSET_TITLE    ] = PROPERTYNAME_HISTORY + seqHistoryItems[nItem] + PROPERTYNAME_HISTORYITEM_TITLE      ;
        seqProperties[nPosition+OFFSET_PASSWORD ] = PROPERTYNAME_HISTORY + seqHistoryItems[nItem] + PROPERTYNAME_HISTORYITEM_PASSWORD   ;
        ++nPosition;
    }

    // Return result.
    return seqProperties;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtHistoryOptions_Impl::impl_GetSequenceFromList( const deque< IMPL_THistoryItem >& aList ) const
{
    // Initialize return sequence with right size.
    sal_Int32 nCount = aList.size();
    Sequence< Sequence< PropertyValue > >   seqResult( nCount );
    Sequence< PropertyValue >               seqProperties( 4 );
    // Copy items from given to return list.
    for( sal_Int32 nItem=0; nItem<nCount; ++nItem )
    {
        seqProperties[OFFSET_URL        ].Name  =   HISTORY_PROPERTYNAME_URL        ;
        seqProperties[OFFSET_FILTER     ].Name  =   HISTORY_PROPERTYNAME_FILTER     ;
        seqProperties[OFFSET_TITLE      ].Name  =   HISTORY_PROPERTYNAME_TITLE      ;
        seqProperties[OFFSET_PASSWORD   ].Name  =   HISTORY_PROPERTYNAME_PASSWORD   ;
        seqProperties[OFFSET_URL        ].Value <<= aList[nItem].sURL               ;
        seqProperties[OFFSET_FILTER     ].Value <<= aList[nItem].sFilter            ;
        seqProperties[OFFSET_TITLE      ].Value <<= aList[nItem].sTitle             ;
        seqProperties[OFFSET_PASSWORD   ].Value <<= aList[nItem].sPassword          ;
        seqResult[nItem] = seqProperties;
    }
    return seqResult;
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
    MutexGuard aGuard( GetOwnStaticMutex() );
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
//  public method
//*****************************************************************************************************************
sal_uInt32 SvtHistoryOptions::GetSize( EHistoryType eHistory ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetSize( eHistory );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions::SetSize( EHistoryType eHistory, sal_uInt32 nSize )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetSize( eHistory, nSize );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions::Clear( EHistoryType eHistory )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->Clear( eHistory );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< Sequence< PropertyValue > > SvtHistoryOptions::GetList( EHistoryType eHistory ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetList( eHistory );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtHistoryOptions::AppendItem(         EHistoryType    eHistory    ,
                                    const   OUString&       sURL        ,
                                    const   OUString&       sFilter     ,
                                    const   OUString&       sTitle      ,
                                    const   OUString&       sPassword   )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->AppendItem( eHistory, sURL, sFilter, sTitle, sPassword );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtHistoryOptions::GetOwnStaticMutex()
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

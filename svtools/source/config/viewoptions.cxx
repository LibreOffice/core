/*************************************************************************
 *
 *  $RCSfile: viewoptions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2000-11-10 09:32:42 $
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

#include "viewoptions.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <stl/hash_map>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::std                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::beans ;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#define ROOTNODE_DIALOGS                    OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Views/Dialogs"     ))
#define ROOTNODE_TABDIALOGS                 OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Views/TabDialogs"  ))
#define ROOTNODE_TABPAGES                   OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Views/TabPages"    ))
#define ROOTNODE_WINDOWS                    OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Views/Windows"     ))

#define PROPERTYNAME_POSITION_X             OUString(RTL_CONSTASCII_USTRINGPARAM("Position/X"               ))
#define PROPERTYNAME_POSITION_Y             OUString(RTL_CONSTASCII_USTRINGPARAM("Position/Y"               ))
#define PROPERTYNAME_SIZE_WIDTH             OUString(RTL_CONSTASCII_USTRINGPARAM("Size/Width"               ))
#define PROPERTYNAME_SIZE_HEIGHT            OUString(RTL_CONSTASCII_USTRINGPARAM("Size/Height"              ))
#define PROPERTYNAME_PAGEID                 OUString(RTL_CONSTASCII_USTRINGPARAM("PageID"                   ))
#define PROPERTYNAME_VISIBLE                OUString(RTL_CONSTASCII_USTRINGPARAM("Visible"                  ))
#define PROPERTYNAME_USERDATA               OUString(RTL_CONSTASCII_USTRINGPARAM("UserData"                 ))

#define SEPERATOR_NOT_FOUND                 -1
#define PATHSEPERATOR                       sal_Unicode('/')
#define ROOTNODE                            OUString(RTL_CONSTASCII_USTRINGPARAM("/"                        ))

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr  We define different data structures which present the format of a list item of right view type.
*//*-*************************************************************************************************************/

struct IMPL_TDialogData
{
    sal_Int32   nX          ;
    sal_Int32   nY          ;
    sal_Int32   nWidth      ;
    sal_Int32   nHeight     ;
    OUString    sUserData   ;
};

struct IMPL_TTabDialogData
{
    sal_Int32   nX          ;
    sal_Int32   nY          ;
    sal_Int32   nPageID     ;
    OUString    sUserData   ;
};

struct IMPL_TTabPageData
{
    OUString    sUserData   ;
};

struct IMPL_TWindowData
{
    sal_Int32   nX          ;
    sal_Int32   nY          ;
    sal_Int32   nWidth      ;
    sal_Int32   nHeight     ;
    sal_Bool    bVisible    ;
    OUString    sUserData   ;
};

/*-************************************************************************************************************//**
    @descr  We define different hash list to hold the view type data.
*//*-*************************************************************************************************************/

struct IMPL_TStringHashCode
{
    size_t operator()(const OUString& sString) const
    {
        return sString.hashCode();
    }
};

typedef hash_map<   OUString                    ,
                    IMPL_TDialogData            ,
                    IMPL_TStringHashCode        ,
                    ::std::equal_to< OUString > > IMPL_TDialogHash;

typedef hash_map<   OUString                    ,
                    IMPL_TTabDialogData         ,
                    IMPL_TStringHashCode        ,
                    ::std::equal_to< OUString > > IMPL_TTabDialogHash;

typedef hash_map<   OUString                    ,
                    IMPL_TTabPageData           ,
                    IMPL_TStringHashCode        ,
                    ::std::equal_to< OUString > > IMPL_TTabPageHash;

typedef hash_map<   OUString                    ,
                    IMPL_TWindowData            ,
                    IMPL_TStringHashCode        ,
                    ::std::equal_to< OUString > > IMPL_TWindowHash;

/*-************************************************************************************************************//**
    @descr  Implement the data container for dialogs.
*//*-*************************************************************************************************************/

class SvtViewDialogOptions_Impl : public ConfigItem
{
    public:

         SvtViewDialogOptions_Impl();
        ~SvtViewDialogOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        void        GetPosition (   const   OUString&   sName   ,           sal_Int32&  nX      ,   sal_Int32&  nY      );
        void        SetPosition (   const   OUString&   sName   ,           sal_Int32   nX      ,   sal_Int32   nY      );
        void        GetSize     (   const   OUString&   sName   ,           sal_Int32&  nWidth  ,   sal_Int32&  nHeight );
        void        SetSize     (   const   OUString&   sName   ,           sal_Int32   nWidth  ,   sal_Int32   nHeight );
        OUString    GetUserData (   const   OUString&   sName                                                           );
        void        SetUserData (   const   OUString&   sName   ,   const   OUString&   sData                           );

    private:

        IMPL_TDialogHash    m_aList     ;
};

//*****************************************************************************************************************
SvtViewDialogOptions_Impl::SvtViewDialogOptions_Impl()
        :   ConfigItem  ( ROOTNODE_DIALOGS )
{
    // Build a list of current existing coniguration keys in right dialog set.
    // We use these list and our Notify() to get his values!!!
    Sequence< OUString > seqNames = GetNodeNames( ROOTNODE );
    Notify( seqNames );

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
    Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE;
    EnableNotification( seqNotifyList );
}

//*****************************************************************************************************************
SvtViewDialogOptions_Impl::~SvtViewDialogOptions_Impl()
{
    // Save cached data - if config item manager forget to call our virtual Commit() method!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given name list to get his values from our configuration management.
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // Name and value list must have the same size - otherwise some keys are deleted during notification and GetProperties ...
    // but this is a bug - I think!
    DBG_ASSERT( sal_False, "SvtViewDialogOptions_Impl::Notify()\nMiss some values for notified properties!\n" );
    sal_uInt32  nCount          = seqPropertyNames.getLength();
    sal_Int32   nFirstSeperator ;
    OUString    sDialogName     ;
    OUString    sPropertyName   ;
    for( sal_uInt32 nProperty=0; nProperty<nCount; ++nCount )
    {
        // zB   seqPropertyNames[nProperty] =   "FileOpen/Position/X"
        //  =>  sDialogName                 =   "FileOpen"
        //  =>  sPropertyName               =   "Position/X"    => MUST one of predefined property names!!!
        //                                      Otherwise our set tree is inconsistent - because somewhere has add other keys ...
        // Attention:   We don't accept empty property names! Our baseclass never send notifications for trees yet ...
        //              or somewhere has changed the implementation!

        nFirstSeperator = seqPropertyNames[nProperty].indexOf( PATHSEPERATOR );

        DBG_ASSERT( !(nFirstSeperator==SEPERATOR_NOT_FOUND), "SvtViewDialogOptions_Impl::Notify()\nBaselcass has changed his implementation - I don't accept notifications for a complete tree at one time!\n" );

        sDialogName     = seqPropertyNames[nProperty].copy( 0, nFirstSeperator );
        sPropertyName   = seqPropertyNames[nProperty].copy( nFirstSeperator, seqPropertyNames[nProperty].getLength()-nFirstSeperator );

        // We use the index operator to set the new value.
        // If the item already exist - all is ok ...
        // otherwise these operator create a new hash item automaticly!
        if( sPropertyName == PROPERTYNAME_POSITION_X )
        {
            seqValues[nProperty] >>= m_aList[ sDialogName ].nX;
        }
        else
        if( sPropertyName == PROPERTYNAME_POSITION_Y )
        {
            seqValues[nProperty] >>= m_aList[ sDialogName ].nY;
        }
        else
        if( sPropertyName == PROPERTYNAME_SIZE_WIDTH )
        {
            seqValues[nProperty] >>= m_aList[ sDialogName ].nWidth;
        }
        else
        if( sPropertyName == PROPERTYNAME_SIZE_HEIGHT )
        {
            seqValues[nProperty] >>= m_aList[ sDialogName ].nHeight;
        }
        else
        if( sPropertyName == PROPERTYNAME_USERDATA )
        {
            seqValues[nProperty] >>= m_aList[ sDialogName ].sUserData;
        }
        else
        {
            DBG_ASSERT( sal_False, "SvtViewDialogOptions_Impl::Notify()\nUnknown property detected!\n" );
        }
    }
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 5 properties for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue > seqProperties( 5 );
    seqProperties[0].Name   = PROPERTYNAME_POSITION_X   ;
    seqProperties[1].Name   = PROPERTYNAME_POSITION_Y   ;
    seqProperties[2].Name   = PROPERTYNAME_SIZE_WIDTH   ;
    seqProperties[3].Name   = PROPERTYNAME_SIZE_HEIGHT  ;
    seqProperties[4].Name   = PROPERTYNAME_USERDATA     ;

    for( IMPL_TDialogHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        seqProperties[0].Value  <<= pIterator->second.nX        ;
        seqProperties[1].Value  <<= pIterator->second.nY        ;
        seqProperties[2].Value  <<= pIterator->second.nWidth    ;
        seqProperties[3].Value  <<= pIterator->second.nHeight   ;
        seqProperties[4].Value  <<= pIterator->second.sUserData ;

        OUString sName = pIterator->first;
        SetSetProperties( sName, seqProperties ); // The keyname of our hash is the kename of our set!
    }
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::GetPosition( const OUString& sName, sal_Int32& nX, sal_Int32& nY )
{
    nX = m_aList[ sName ].nX;
    nY = m_aList[ sName ].nY;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::SetPosition( const OUString& sName, sal_Int32 nX, sal_Int32 nY )
{
    m_aList[ sName ].nX = nX;
    m_aList[ sName ].nY = nY;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::GetSize( const OUString& sName, sal_Int32& nWidth, sal_Int32& nHeight )
{
    nWidth  = m_aList[ sName ].nWidth   ;
    nHeight = m_aList[ sName ].nHeight  ;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::SetSize( const OUString& sName, sal_Int32 nWidth, sal_Int32 nHeight )
{
    m_aList[ sName ].nWidth  = nWidth  ;
    m_aList[ sName ].nHeight = nHeight ;
    SetModified();
}

//*****************************************************************************************************************
OUString SvtViewDialogOptions_Impl::GetUserData( const OUString& sName )
{
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    m_aList[ sName ].sUserData = sData;
}

/*-************************************************************************************************************//**
    @descr  Implement the data container for tab-dialogs.
*//*-*************************************************************************************************************/

class SvtViewTabDialogOptions_Impl : public ConfigItem
{
    public:

         SvtViewTabDialogOptions_Impl();
        ~SvtViewTabDialogOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        void        GetPosition (   const   OUString&   sName   ,           sal_Int32&  nX      ,   sal_Int32&  nY      );
        void        SetPosition (   const   OUString&   sName   ,           sal_Int32   nX      ,   sal_Int32   nY      );
        sal_Int32   GetPageID   (   const   OUString&   sName                                                           );
        void        SetPageID   (   const   OUString&   sName   ,           sal_Int32   nID                             );
        OUString    GetUserData (   const   OUString&   sName                                                           );
        void        SetUserData (   const   OUString&   sName   ,   const   OUString&   sData                           );

    private:

        IMPL_TTabDialogHash     m_aList     ;
};

//*****************************************************************************************************************
SvtViewTabDialogOptions_Impl::SvtViewTabDialogOptions_Impl()
        :   ConfigItem  ( ROOTNODE_TABDIALOGS )
{
    // Build a list of current existing coniguration keys in right tab dialog set.
    // We use these list and our Notify() to get his values!!!
    Sequence< OUString > seqNames = GetNodeNames( ROOTNODE );
    Notify( seqNames );

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
    Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE;
    EnableNotification( seqNotifyList );
}

//*****************************************************************************************************************
SvtViewTabDialogOptions_Impl::~SvtViewTabDialogOptions_Impl()
{
    // Save cached data - if config item manager forget to call our virtual Commit() method!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given name list to get his values from our configuration management.
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // Name and value list must have the same size - otherwise some keys are deleted during notification and GetProperties ...
    // but this is a bug - I think!
    DBG_ASSERT( sal_False, "SvtViewTabDialogOptions_Impl::Notify()\nMiss some values for notified properties!\n" );
    sal_uInt32  nCount          = seqPropertyNames.getLength();
    sal_Int32   nFirstSeperator ;
    OUString    sTabDialogName  ;
    OUString    sPropertyName   ;
    for( sal_uInt32 nProperty=0; nProperty<nCount; ++nCount )
    {
        // zB   seqPropertyNames[nProperty] =   "TabDialog_001/Position/X"
        //  =>  sTabDialogName              =   "TabDialog_001"
        //  =>  sPropertyName               =   "Position/X"    => MUST one of predefined property names!!!
        //                                      Otherwise our set tree is inconsistent - because somewhere has add other keys ...
        // Attention:   We don't accept empty property names! Our baseclass never send notifications for trees yet ...
        //              or somewhere has changed the implementation!

        nFirstSeperator = seqPropertyNames[nProperty].indexOf( PATHSEPERATOR );

        DBG_ASSERT( !(nFirstSeperator==SEPERATOR_NOT_FOUND), "SvtViewTabDialogOptions_Impl::Notify()\nBaselcass has changed his implementation - I don't accept notifications for a complete tree at one time!\n" );

        sTabDialogName  = seqPropertyNames[nProperty].copy( 0, nFirstSeperator );
        sPropertyName   = seqPropertyNames[nProperty].copy( nFirstSeperator, seqPropertyNames[nProperty].getLength()-nFirstSeperator );

        // We use the index operator to set the new value.
        // If the item already exist - all is ok ...
        // otherwise these operator create a new hash item automaticly!
        if( sPropertyName == PROPERTYNAME_POSITION_X )
        {
            seqValues[nProperty] >>= m_aList[ sTabDialogName ].nX;
        }
        else
        if( sPropertyName == PROPERTYNAME_POSITION_Y )
        {
            seqValues[nProperty] >>= m_aList[ sTabDialogName ].nY;
        }
        else
        if( sPropertyName == PROPERTYNAME_PAGEID )
        {
            seqValues[nProperty] >>= m_aList[ sTabDialogName ].nPageID;
        }
        else
        if( sPropertyName == PROPERTYNAME_USERDATA )
        {
            seqValues[nProperty] >>= m_aList[ sTabDialogName ].sUserData;
        }
        else
        {
            DBG_ASSERT( sal_False, "SvtViewTabDialogOptions_Impl::Notify()\nUnknown property detected!\n" );
        }
    }
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 4 properties for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue > seqProperties( 4 );
    seqProperties[0].Name   = PROPERTYNAME_POSITION_X   ;
    seqProperties[1].Name   = PROPERTYNAME_POSITION_Y   ;
    seqProperties[2].Name   = PROPERTYNAME_PAGEID       ;
    seqProperties[3].Name   = PROPERTYNAME_USERDATA     ;

    for( IMPL_TTabDialogHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        seqProperties[0].Value  <<= pIterator->second.nX        ;
        seqProperties[1].Value  <<= pIterator->second.nY        ;
        seqProperties[2].Value  <<= pIterator->second.nPageID   ;
        seqProperties[3].Value  <<= pIterator->second.sUserData ;

        OUString sName = pIterator->first;
        SetSetProperties( sName, seqProperties ); // The keyname of our hash is the kename of our set!
    }
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::GetPosition( const OUString& sName, sal_Int32& nX, sal_Int32& nY )
{
    nX = m_aList[ sName ].nX;
    nY = m_aList[ sName ].nY;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::SetPosition( const OUString& sName, sal_Int32 nX, sal_Int32 nY )
{
    m_aList[ sName ].nX = nX;
    m_aList[ sName ].nY = nY;
    SetModified();
}

//*****************************************************************************************************************
sal_Int32 SvtViewTabDialogOptions_Impl::GetPageID( const OUString& sName )
{
    return m_aList[ sName ].nPageID;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::SetPageID( const OUString& sName, sal_Int32 nID )
{
    m_aList[ sName ].nPageID = nID;
    SetModified();
}

//*****************************************************************************************************************
OUString SvtViewTabDialogOptions_Impl::GetUserData( const OUString& sName )
{
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    m_aList[ sName ].sUserData = sData;
}

/*-************************************************************************************************************//**
    @descr  Implement the data container for tab-pages.
*//*-*************************************************************************************************************/

class SvtViewTabPageOptions_Impl : public ConfigItem
{
    public:

         SvtViewTabPageOptions_Impl();
        ~SvtViewTabPageOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        OUString    GetUserData (   const   OUString&   sName                               );
        void        SetUserData (   const   OUString&   sName,  const   OUString&   sData   );

    private:

        IMPL_TTabPageHash       m_aList     ;
};

//*****************************************************************************************************************
SvtViewTabPageOptions_Impl::SvtViewTabPageOptions_Impl()
        :   ConfigItem  ( ROOTNODE_TABPAGES )
{
    // Build a list of current existing coniguration keys in right pages set.
    // We use these list and our Notify() to get his values!!!
    Sequence< OUString > seqNames = GetNodeNames( ROOTNODE );
    Notify( seqNames );

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
    Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE;
    EnableNotification( seqNotifyList );
}

//*****************************************************************************************************************
SvtViewTabPageOptions_Impl::~SvtViewTabPageOptions_Impl()
{
    // Save cached data - if config item manager forget to call our virtual Commit() method!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given name list to get his values from our configuration management.
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // Name and value list must have the same size - otherwise some keys are deleted during notification and GetProperties ...
    // but this is a bug - I think!
    DBG_ASSERT( sal_False, "SvtViewTabPageOptions_Impl::Notify()\nMiss some values for notified properties!\n" );
    sal_uInt32  nCount          = seqPropertyNames.getLength();
    sal_Int32   nFirstSeperator ;
    OUString    sTabPageName    ;
    OUString    sPropertyName   ;
    for( sal_uInt32 nProperty=0; nProperty<nCount; ++nCount )
    {
        // zB   seqPropertyNames[nProperty] =   "TabPage_001/Position/X"
        //  =>  sTabPageName                =   "TabPage_001"
        //  =>  sPropertyName               =   "UserData"  => MUST one of predefined property names!!!
        //                                      Otherwise our set tree is inconsistent - because somewhere has add other keys ...
        // Attention:   We don't accept empty property names! Our baseclass never send notifications for trees yet ...
        //              or somewhere has changed the implementation!

        nFirstSeperator = seqPropertyNames[nProperty].indexOf( PATHSEPERATOR );

        DBG_ASSERT( !(nFirstSeperator==SEPERATOR_NOT_FOUND), "SvtViewTabPageOptions_Impl::Notify()\nBaselcass has changed his implementation - I don't accept notifications for a complete tree at one time!\n" );

        sTabPageName    = seqPropertyNames[nProperty].copy( 0, nFirstSeperator );
        sPropertyName   = seqPropertyNames[nProperty].copy( nFirstSeperator, seqPropertyNames[nProperty].getLength()-nFirstSeperator );

        // We use the index operator to set the new value.
        // If the item already exist - all is ok ...
        // otherwise these operator create a new hash item automaticly!
        if( sPropertyName == PROPERTYNAME_USERDATA )
        {
            seqValues[nProperty] >>= m_aList[ sTabPageName ].sUserData;
        }
        else
        {
            DBG_ASSERT( sal_False, "SvtViewTabPageOptions_Impl::Notify()\nUnknown property detected!\n" );
        }
    }
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 1 property for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue > seqProperties( 1 );
    seqProperties[0].Name = PROPERTYNAME_USERDATA;

    for( IMPL_TTabPageHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        seqProperties[0].Value <<= pIterator->second.sUserData;

        OUString sName = pIterator->first;
        SetSetProperties( sName, seqProperties ); // The keyname of our hash is the kename of our set!
    }
}

//*****************************************************************************************************************
OUString SvtViewTabPageOptions_Impl::GetUserData( const OUString& sName )
{
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    m_aList[ sName ].sUserData = sData;
}

/*-************************************************************************************************************//**
    @descr  Implement the data container for windows.
*//*-*************************************************************************************************************/

class SvtViewWindowOptions_Impl : public ConfigItem
{
    public:

         SvtViewWindowOptions_Impl();
        ~SvtViewWindowOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        void        GetPosition (   const   OUString&   sName   ,           sal_Int32&  nX      ,   sal_Int32&  nY      );
        void        SetPosition (   const   OUString&   sName   ,           sal_Int32   nX      ,   sal_Int32   nY      );
        void        GetSize     (   const   OUString&   sName   ,           sal_Int32&  nWidth  ,   sal_Int32&  nHeight );
        void        SetSize     (   const   OUString&   sName   ,           sal_Int32   nWidth  ,   sal_Int32   nHeight );
        sal_Bool    IsVisible   (   const   OUString&   sName                                                           );
        void        SetVisible  (   const   OUString&   sName   ,           sal_Bool    bState                          );
        OUString    GetUserData (   const   OUString&   sName                                                           );
        void        SetUserData (   const   OUString&   sName   ,   const   OUString&   sData                           );

    private:

        IMPL_TWindowHash    m_aList     ;
};

//*****************************************************************************************************************
SvtViewWindowOptions_Impl::SvtViewWindowOptions_Impl()
        :   ConfigItem  ( ROOTNODE_WINDOWS )
{
    // Build a list of current existing coniguration keys in right window set.
    // We use these list and our Notify() to get his values!!!
    Sequence< OUString > seqNames = GetNodeNames( ROOTNODE );
    Notify( seqNames );

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
    Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE;
    EnableNotification( seqNotifyList );
}

//*****************************************************************************************************************
SvtViewWindowOptions_Impl::~SvtViewWindowOptions_Impl()
{
    // Save cached data - if config item manager forget to call our virtual Commit() method!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given name list to get his values from our configuration management.
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // Name and value list must have the same size - otherwise some keys are deleted during notification and GetProperties ...
    // but this is a bug - I think!
    DBG_ASSERT( sal_False, "SvtViewWindowOptions_Impl::Notify()\nMiss some values for notified properties!\n" );
    sal_uInt32  nCount          = seqPropertyNames.getLength();
    sal_Int32   nFirstSeperator ;
    OUString    sWindowName     ;
    OUString    sPropertyName   ;
    for( sal_uInt32 nProperty=0; nProperty<nCount; ++nCount )
    {
        // zB   seqPropertyNames[nProperty] =   "Window_001/Position/X"
        //  =>  sWindowName                 =   "Window_001"
        //  =>  sPropertyName               =   "Position/X"    => MUST one of predefined property names!!!
        //                                      Otherwise our set tree is inconsistent - because somewhere has add other keys ...
        // Attention:   We don't accept empty property names! Our baseclass never send notifications for trees yet ...
        //              or somewhere has changed the implementation!

        nFirstSeperator = seqPropertyNames[nProperty].indexOf( PATHSEPERATOR );

        DBG_ASSERT( !(nFirstSeperator==SEPERATOR_NOT_FOUND), "SvtViewWindowOptions_Impl::Notify()\nBaselcass has changed his implementation - I don't accept notifications for a complete tree at one time!\n" );

        sWindowName     = seqPropertyNames[nProperty].copy( 0, nFirstSeperator );
        sPropertyName   = seqPropertyNames[nProperty].copy( nFirstSeperator, seqPropertyNames[nProperty].getLength()-nFirstSeperator );

        // We use the index operator to set the new value.
        // If the item already exist - all is ok ...
        // otherwise these operator create a new hash item automaticly!
        if( sPropertyName == PROPERTYNAME_POSITION_X )
        {
            seqValues[nProperty] >>= m_aList[ sWindowName ].nX;
        }
        else
        if( sPropertyName == PROPERTYNAME_POSITION_Y )
        {
            seqValues[nProperty] >>= m_aList[ sWindowName ].nY;
        }
        else
        if( sPropertyName == PROPERTYNAME_SIZE_WIDTH )
        {
            seqValues[nProperty] >>= m_aList[ sWindowName ].nWidth;
        }
        else
        if( sPropertyName == PROPERTYNAME_SIZE_HEIGHT )
        {
            seqValues[nProperty] >>= m_aList[ sWindowName ].nHeight;
        }
        else
        if( sPropertyName == PROPERTYNAME_USERDATA )
        {
            seqValues[nProperty] >>= m_aList[ sWindowName ].sUserData;
        }
        else
        {
            DBG_ASSERT( sal_False, "SvtViewWindowOptions_Impl::Notify()\nUnknown property detected!\n" );
        }
    }
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 5 properties for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue > seqProperties( 5 );
    seqProperties[0].Name   = PROPERTYNAME_POSITION_X   ;
    seqProperties[1].Name   = PROPERTYNAME_POSITION_Y   ;
    seqProperties[2].Name   = PROPERTYNAME_SIZE_WIDTH   ;
    seqProperties[3].Name   = PROPERTYNAME_SIZE_HEIGHT  ;
    seqProperties[4].Name   = PROPERTYNAME_USERDATA     ;

    for( IMPL_TWindowHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        seqProperties[0].Value  <<= pIterator->second.nX        ;
        seqProperties[1].Value  <<= pIterator->second.nY        ;
        seqProperties[2].Value  <<= pIterator->second.nWidth    ;
        seqProperties[3].Value  <<= pIterator->second.nHeight   ;
        seqProperties[4].Value  <<= pIterator->second.sUserData ;

        OUString sName = pIterator->first;
        SetSetProperties( sName, seqProperties ); // The keyname of our hash is the kename of our set!
    }
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::GetPosition( const OUString& sName, sal_Int32& nX, sal_Int32& nY )
{
    nX = m_aList[ sName ].nX;
    nY = m_aList[ sName ].nY;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetPosition( const OUString& sName, sal_Int32 nX, sal_Int32 nY )
{
    m_aList[ sName ].nX = nX;
    m_aList[ sName ].nY = nY;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::GetSize( const OUString& sName, sal_Int32& nWidth, sal_Int32& nHeight )
{
    nWidth  = m_aList[ sName ].nWidth ;
    nHeight = m_aList[ sName ].nHeight;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetSize( const OUString& sName, sal_Int32 nWidth, sal_Int32 nHeight )
{
    m_aList[ sName ].nWidth  = nWidth  ;
    m_aList[ sName ].nHeight = nHeight ;
    SetModified();
}

//*****************************************************************************************************************
sal_Bool SvtViewWindowOptions_Impl::IsVisible( const OUString& sName )
{
    return m_aList[ sName ].bVisible;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetVisible( const OUString& sName, sal_Bool bState )
{
    m_aList[ sName ].bVisible = bState;
}

//*****************************************************************************************************************
OUString SvtViewWindowOptions_Impl::GetUserData( const OUString& sName )
{
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    m_aList[ sName ].sUserData = sData;
}

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtViewOptions::SvtViewOptions( EViewType eType, const OUString& sViewName )
    :   m_eViewType ( eType     )
    ,   m_sViewName ( sViewName )
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and initialize right data container or set right ref count!
    switch( eType )
    {
        case E_DIALOG       :   {
                                    // Increase ref count for dialog data container first.
                                    ++m_nRefCount_Dialogs;
                                    // If these instance the first user of the dialog data container - create these impl static container!
                                    if( m_nRefCount_Dialogs <= 0 )
                                    {
                                        m_pDataContainer_Dialogs = new SvtViewDialogOptions_Impl;
                                    }
                                }
                                break;
        case E_TABDIALOG    :   {
                                    // Increase ref count for tab-dialog data container first.
                                    ++m_nRefCount_TabDialogs;
                                    // If these instance the first user of the tab-dialog data container - create these impl static container!
                                    if( m_nRefCount_TabDialogs <= 0 )
                                    {
                                        m_pDataContainer_TabDialogs = new SvtViewTabDialogOptions_Impl;
                                    }
                                }
                                break;
        case E_TABPAGE      :   {
                                    // Increase ref count for tab-page data container first.
                                    ++m_nRefCount_TabPages;
                                    // If these instance the first user of the tab-page data container - create these impl static container!
                                    if( m_nRefCount_TabPages <= 0 )
                                    {
                                        m_pDataContainer_TabPages = new SvtViewTabPageOptions_Impl;
                                    }
                                }
                                break;
        case E_WINDOW       :   {
                                    // Increase ref count for window data container first.
                                    ++m_nRefCount_Windows;
                                    // If these instance the first user of the window data container - create these impl static container!
                                    if( m_nRefCount_Windows <= 0 )
                                    {
                                        m_pDataContainer_Windows = new SvtViewWindowOptions_Impl;
                                    }
                                }
                                break;
        default             :   DBG_ASSERT( sal_False, "SvtViewOptions::SvtViewOptions()\nThese view type is unknown! All following calls at these instance will do nothing!\n" );
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtViewOptions::~SvtViewOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and deinitialize right data container or set right ref count!
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    // Decrease ref count for dialog data container first.
                                    --m_nRefCount_Dialogs;
                                    // If these instance the last user of the dialog data container - delete these impl static container!
                                    if( m_nRefCount_Dialogs <= 0 )
                                    {
                                        delete m_pDataContainer_Dialogs;
                                        m_pDataContainer_Dialogs = NULL;
                                    }
                                }
                                break;
        case E_TABDIALOG    :   {
                                    // Decrease ref count for tab-dialog data container first.
                                    --m_nRefCount_TabDialogs;
                                    // If these instance the last user of the tab-dialog data container - delete these impl static container!
                                    if( m_nRefCount_TabDialogs <= 0 )
                                    {
                                        delete m_pDataContainer_TabDialogs;
                                        m_pDataContainer_TabDialogs = NULL;
                                    }
                                }
                                break;
        case E_TABPAGE      :   {
                                    // Decrease ref count for tab-page data container first.
                                    --m_nRefCount_TabPages;
                                    // If these instance the last user of the tab-page data container - delete these impl static container!
                                    if( m_nRefCount_TabPages <= 0 )
                                    {
                                        delete m_pDataContainer_TabPages;
                                        m_pDataContainer_TabPages = NULL;
                                    }
                                }
                                break;
        case E_WINDOW       :   {
                                    // Decrease ref count for window data container first.
                                    --m_nRefCount_Windows;
                                    // If these instance the last user of the window data container - delete these impl static container!
                                    if( m_nRefCount_Windows <= 0 )
                                    {
                                        delete m_pDataContainer_Windows;
                                        m_pDataContainer_Windows = NULL;
                                    }
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::GetPosition( sal_Int32& nX, sal_Int32& nY ) const
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for tab-pages!
    DBG_ASSERT( !(m_eViewType==E_TABPAGE), "SvtViewOptions::GetPosition()\nCall not allowed for TabPages! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->GetPosition( m_sViewName, nX, nY );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->GetPosition( m_sViewName, nX, nY );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->GetPosition( m_sViewName, nX, nY );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetPosition( sal_Int32  nX, sal_Int32  nY )
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for tab-pages!
    DBG_ASSERT( !(m_eViewType==E_TABPAGE), "SvtViewOptions::SetPosition()\nCall not allowed for TabPages! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->SetPosition( m_sViewName, nX, nY );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetPosition( m_sViewName, nX, nY );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetPosition( m_sViewName, nX, nY );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::GetSize( sal_Int32& nWidth, sal_Int32& nHeight ) const
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for tab-pages or tab-dialogs!
    DBG_ASSERT( !(m_eViewType==E_TABPAGE||m_eViewType==E_TABDIALOG), "SvtViewOptions::GetSize()\nCall not allowed for TabPages or TabDialogs! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->GetSize( m_sViewName, nWidth, nHeight );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->GetSize( m_sViewName, nWidth, nHeight );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetSize( sal_Int32 nWidth, sal_Int32 nHeight )
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for tab-pages or tab-dialogs!
    DBG_ASSERT( !(m_eViewType==E_TABPAGE||m_eViewType==E_TABDIALOG), "SvtViewOptions::SetSize()\nCall not allowed for TabPages or TabDialogs! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->SetSize( m_sViewName, nWidth, nHeight );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetSize( m_sViewName, nWidth, nHeight );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtViewOptions::GetPageID() const
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    DBG_ASSERT( !(m_eViewType==E_DIALOG||m_eViewType==E_TABPAGE||m_eViewType==E_WINDOW), "SvtViewOptions::GetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!\n" );

    sal_Int32 nID = 0;
    switch( m_eViewType )
    {
        case E_TABDIALOG    :   {
                                    nID = m_pDataContainer_TabDialogs->GetPageID( m_sViewName );
                                }
                                break;
    }
    return nID;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetPageID( sal_Int32 nID )
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    DBG_ASSERT( !(m_eViewType==E_DIALOG||m_eViewType==E_TABPAGE||m_eViewType==E_WINDOW), "SvtViewOptions::SetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetPageID( m_sViewName, nID );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtViewOptions::IsVisible() const
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    DBG_ASSERT( !(m_eViewType==E_DIALOG||m_eViewType==E_TABDIALOG||m_eViewType==E_TABPAGE), "SvtViewOptions::IsVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!\n" );

    sal_Bool bState = sal_False;
    switch( m_eViewType )
    {
        case E_WINDOW       :   {
                                    bState = m_pDataContainer_Windows->IsVisible( m_sViewName );
                                }
                                break;
    }
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetVisible( sal_Bool bState )
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    DBG_ASSERT( !(m_eViewType==E_DIALOG||m_eViewType==E_TABDIALOG||m_eViewType==E_TABPAGE), "SvtViewOptions::SetVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetVisible( m_sViewName, bState );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
OUString SvtViewOptions::GetUserData() const
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    OUString sData;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    sData = m_pDataContainer_Dialogs->GetUserData( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    sData = m_pDataContainer_TabDialogs->GetUserData( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    sData = m_pDataContainer_TabPages->GetUserData( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    sData = m_pDataContainer_Windows->GetUserData( m_sViewName );
                                }
                                break;
    }
    return sData;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetUserData( const OUString& sData )
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->SetUserData( m_sViewName, sData );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetUserData( m_sViewName, sData );
                                }
                                break;
        case E_TABPAGE      :   {
                                    m_pDataContainer_TabPages->SetUserData( m_sViewName, sData );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetUserData( m_sViewName, sData );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtViewOptions::GetOwnStaticMutex()
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

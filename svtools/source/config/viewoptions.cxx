/*************************************************************************
 *
 *  $RCSfile: viewoptions.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-10 09:21:35 $
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

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
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

#define VIEWPACKAGE                         OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Views"             ))

#define PROPERTYNAME_DIALOGS                OUString(RTL_CONSTASCII_USTRINGPARAM("Dialogs"                  ))
#define PROPERTYNAME_TABDIALOGS             OUString(RTL_CONSTASCII_USTRINGPARAM("TabDialogs"               ))
#define PROPERTYNAME_TABPAGES               OUString(RTL_CONSTASCII_USTRINGPARAM("TabPages"                 ))
#define PROPERTYNAME_WINDOWS                OUString(RTL_CONSTASCII_USTRINGPARAM("Windows"                  ))
#define PROPERTYNAME_X                      OUString(RTL_CONSTASCII_USTRINGPARAM("X"                        ))
#define PROPERTYNAME_Y                      OUString(RTL_CONSTASCII_USTRINGPARAM("Y"                        ))
#define PROPERTYNAME_WIDTH                  OUString(RTL_CONSTASCII_USTRINGPARAM("Width"                    ))
#define PROPERTYNAME_HEIGHT                 OUString(RTL_CONSTASCII_USTRINGPARAM("Height"                   ))
#define PROPERTYNAME_PAGEID                 OUString(RTL_CONSTASCII_USTRINGPARAM("PageID"                   ))
#define PROPERTYNAME_VISIBLE                OUString(RTL_CONSTASCII_USTRINGPARAM("Visible"                  ))
#define PROPERTYNAME_USERDATA               OUString(RTL_CONSTASCII_USTRINGPARAM("UserData"                 ))
#define PROPERTYNAME_ANYDATA                OUString(RTL_CONSTASCII_USTRINGPARAM("AnyData"                  ))

#define SEPERATOR_NOT_FOUND                 -1
#define PATHSEPERATOR                       OUString(RTL_CONSTASCII_USTRINGPARAM("/"))

#define ROOTNODE_DIALOGS                    OUString(RTL_CONSTASCII_USTRINGPARAM("Dialogs"                  ))
#define ROOTNODE_TABDIALOGS                 OUString(RTL_CONSTASCII_USTRINGPARAM("TabDialogs"               ))
#define ROOTNODE_TABPAGES                   OUString(RTL_CONSTASCII_USTRINGPARAM("TabPages"                 ))
#define ROOTNODE_WINDOWS                    OUString(RTL_CONSTASCII_USTRINGPARAM("Windows"                  ))

#define SHORTNAME_DIALOGS                   OUString(RTL_CONSTASCII_USTRINGPARAM("dl_"                      ))
#define SHORTNAME_TABDIALOGS                OUString(RTL_CONSTASCII_USTRINGPARAM("td_"                      ))
#define SHORTNAME_TABPAGES                  OUString(RTL_CONSTASCII_USTRINGPARAM("tp_"                      ))
#define SHORTNAME_WINDOWS                   OUString(RTL_CONSTASCII_USTRINGPARAM("wi_"                      ))

#define SHORTNAME_ENDPOSITION               3

#define DEFAULT_X                           0
#define DEFAULT_Y                           0
#define DEFAULT_WIDTH                       100
#define DEFAULT_HEIGHT                      100
#define DEFAULT_PAGEID                      0
#define DEFAULT_VISIBLE                     sal_True
#define DEFAULT_USERDATA                    OUString()

//_________________________________________________________________________________________________________________
//  initialization!
//_________________________________________________________________________________________________________________

SvtViewDialogOptions_Impl*      SvtViewOptions::m_pDataContainer_Dialogs    =   NULL    ;
sal_Int32                       SvtViewOptions::m_nRefCount_Dialogs         =   0       ;
SvtViewTabDialogOptions_Impl*   SvtViewOptions::m_pDataContainer_TabDialogs =   NULL    ;
sal_Int32                       SvtViewOptions::m_nRefCount_TabDialogs      =   0       ;
SvtViewTabPageOptions_Impl*     SvtViewOptions::m_pDataContainer_TabPages   =   NULL    ;
sal_Int32                       SvtViewOptions::m_nRefCount_TabPages        =   0       ;
SvtViewWindowOptions_Impl*      SvtViewOptions::m_pDataContainer_Windows    =   NULL    ;
sal_Int32                       SvtViewOptions::m_nRefCount_Windows         =   0       ;

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr  We define different data structures which present the format of a list item of right view type.
*//*-*************************************************************************************************************/

struct IMPL_TDialogData
{
    sal_Int32                   nX          ;
    sal_Int32                   nY          ;
    sal_Int32                   nWidth      ;
    sal_Int32                   nHeight     ;
    OUString                    sUserData   ;
    Sequence< PropertyValue >   aAnyData    ;
};

struct IMPL_TTabDialogData
{
    sal_Int32                   nX          ;
    sal_Int32                   nY          ;
    sal_Int32                   nPageID     ;
    OUString                    sUserData   ;
    Sequence< PropertyValue >   aAnyData    ;
};

struct IMPL_TTabPageData
{
    OUString                    sUserData   ;
    Sequence< PropertyValue >   aAnyData    ;
};

struct IMPL_TWindowData
{
    sal_Int32                   nX          ;
    sal_Int32                   nY          ;
    sal_Int32                   nWidth      ;
    sal_Int32                   nHeight     ;
    sal_Bool                    bVisible    ;
    OUString                    sUserData   ;
    Sequence< PropertyValue >   aAnyData    ;
};

// TODO:
// why don't these classes share a common base class? It could handle the UserData a´nd the AnyData,
// and a second base class could handle X, Y (and perhaps an additional one for the Width/Height)
//
// This way we would avoid the extensive code duplication we do currently !!!
//

/*-************************************************************************************************************//**
    @descr  helper for reading AnyValue nodes
*//*-*************************************************************************************************************/

namespace reading
{
    //------------------------------------------------------------------------------------------------------------/
    class IPublicConfigReadAccess
    {
    public:
        virtual void getNodeNames( const OUString& _rNode, Sequence< OUString >& _rNames ) = 0;
    };

    //------------------------------------------------------------------------------------------------------------/
    static void lcl_implInitReadAnyValues(  ::std::vector< OUString >& _rNames, ::std::vector< sal_Int32 >& _rCounts,
                                            ::std::vector< sal_Int32 >& _rNamePrefixLen, const sal_Int32 _nNodeCount )
    {
        // not much to do here
        _rNames.reserve         ( _nNodeCount * 2 );    // just a guess
        _rCounts.reserve        ( _nNodeCount * 2 );    // just a guess
        _rNamePrefixLen.reserve ( _nNodeCount * 2 );    // just a guess
    }

    //------------------------------------------------------------------------------------------------------------/
    static void lcl_implReadOneNodeAnyValues(
                            OUString& _rNameBase,                           // the name above the AnyData node
                            ::std::vector< OUString >& _rNames,             // the names of the nodes containing AnyData
                            ::std::vector< sal_Int32 >& _rCounts,           // the vector if node counts
                            ::std::vector< sal_Int32 >& _rNamePrefixLen,    // the vector of name prefix lengths
                            IPublicConfigReadAccess* _pReader               // the instance implementing the read
                        )
    {
        _rNameBase += PROPERTYNAME_ANYDATA;
        _rNameBase += PATHSEPERATOR;

        _rNamePrefixLen.push_back( _rNameBase.getLength() );

        // node names
        Sequence< OUString > aAnyNodeNames;
        _pReader->getNodeNames( _rNameBase, aAnyNodeNames );
        // node count
        _rCounts.push_back( aAnyNodeNames.getLength() );

        // collect the assembled names
        const OUString* pAnyNodeNames       =                   aAnyNodeNames.getConstArray();
        const OUString* pAnyNodeNamesEnd    =   pAnyNodeNames + aAnyNodeNames.getLength();
        for (;pAnyNodeNames < pAnyNodeNamesEnd; ++pAnyNodeNames)
            _rNames.push_back( _rNameBase + *pAnyNodeNames );
    }

    //------------------------------------------------------------------------------------------------------------/
    static void lcl_implFillReadAnyValues( Sequence< PropertyValue >& _rValues, sal_Int32 const* _pCount,
        OUString const* & _rpAnyName, Any const* & _rpAnyValue, const sal_Int32 _nNamePrefixLen )
    {
        if ( *_pCount )
        {
            _rValues.realloc( *_pCount );
            PropertyValue* pAnyData     =                   _rValues.getArray();
            PropertyValue* pAnyDataEnd  =   pAnyData    +   _rValues.getLength();
            for (;pAnyData != pAnyDataEnd; ++pAnyData, ++_rpAnyName, ++_rpAnyValue)
            {
                pAnyData->Name  =   _rpAnyName->copy( _nNamePrefixLen );
                pAnyData->Value =   *_rpAnyValue;
            }
        }
    }
}

/*-************************************************************************************************************//**
    @descr  helper for writing AnyValue nodes
*//*-*************************************************************************************************************/

namespace writing
{
    //------------------------------------------------------------------------------------------------------------/
    static void lcl_implPrefixAnyValues( OUString& _rNameBase, Sequence< PropertyValue >& _rAnyValues )
    {
        // the path to the AnyData node
        _rNameBase += PROPERTYNAME_ANYDATA;

        OUString sPrefix = _rNameBase;
        sPrefix += PATHSEPERATOR;

        // prefix the AnyData names with the path to the node for the window
                PropertyValue* pAnyData     =               _rAnyValues.getArray();
        const   PropertyValue* pAnyDataEnd  = pAnyData  +   _rAnyValues.getLength();
        for (;pAnyData != pAnyDataEnd; ++pAnyData)
            pAnyData->Name = sPrefix + pAnyData->Name;

    }
}

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

class SvtViewDialogOptions_Impl : public ConfigItem, public reading::IPublicConfigReadAccess
{
    public:

         SvtViewDialogOptions_Impl();
        ~SvtViewDialogOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        sal_Bool    Exists      (   const   OUString&   sName                                                           );
        sal_Bool    Delete      (   const   OUString&   sName                                                           );
        void        GetPosition (   const   OUString&   sName   ,           sal_Int32&  nX      ,   sal_Int32&  nY      );
        void        SetPosition (   const   OUString&   sName   ,           sal_Int32   nX      ,   sal_Int32   nY      );
        void        GetSize     (   const   OUString&   sName   ,           sal_Int32&  nWidth  ,   sal_Int32&  nHeight );
        void        SetSize     (   const   OUString&   sName   ,           sal_Int32   nWidth  ,   sal_Int32   nHeight );
        OUString    GetUserData (   const   OUString&   sName                                                           );
        void        SetUserData (   const   OUString&   sName   ,   const   OUString&   sData                           );

        Sequence< PropertyValue >   GetAnyData( const   OUString&   sName                                                   );
        void                        SetAnyData( const   OUString&   sName   ,   const   Sequence< PropertyValue >&  aData   );

        virtual void getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames );

    private:

        void    impl_ReadWholeList      (                       );
        void    impl_CreateIfNotExist   ( const OUString& sName );

    private:

        IMPL_TDialogHash    m_aList     ;
};

//*****************************************************************************************************************
SvtViewDialogOptions_Impl::SvtViewDialogOptions_Impl()
        :   ConfigItem  ( VIEWPACKAGE )
{
    // Read complete list from configuration.
    impl_ReadWholeList();

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
/*  Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE_DIALOGS;
    EnableNotification( seqNotifyList );*/
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
    // We ignore given name sequence.
    // It's to complex to handle a dynamic set with notifys for add/delete/change elements!
    // I think to read the whole list is the best way.
    impl_ReadWholeList();
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 5 properties for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue >   seqProperties( 5 )  ;
    OUString                    sName               ;

    for( IMPL_TDialogHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        sName  = ROOTNODE_DIALOGS   ;   //  "Dialogs"
        sName += PATHSEPERATOR      ;   //  "Dialogs/"
        sName += SHORTNAME_DIALOGS  ;   //  "Dialogs/dl_"
        sName += pIterator->first   ;   //  "Dialogs/dl_<...>"
        sName += PATHSEPERATOR      ;   //  "Dialogs/dl_<...>/"

        seqProperties[0].Name   = sName + PROPERTYNAME_X        ;
        seqProperties[1].Name   = sName + PROPERTYNAME_Y        ;
        seqProperties[2].Name   = sName + PROPERTYNAME_WIDTH    ;
        seqProperties[3].Name   = sName + PROPERTYNAME_HEIGHT   ;
        seqProperties[4].Name   = sName + PROPERTYNAME_USERDATA ;

        seqProperties[0].Value  <<= pIterator->second.nX        ;
        seqProperties[1].Value  <<= pIterator->second.nY        ;
        seqProperties[2].Value  <<= pIterator->second.nWidth    ;
        seqProperties[3].Value  <<= pIterator->second.nHeight   ;
        seqProperties[4].Value  <<= pIterator->second.sUserData ;

        SetSetProperties( ROOTNODE_DIALOGS, seqProperties ); // The keyname of our hash is the kename of our set!

        // the AnyData nodes
        Sequence< PropertyValue > aAnyValues( pIterator->second.aAnyData ); // copy, 'cause we want to modify it
        writing::lcl_implPrefixAnyValues( sName, aAnyValues );
        ClearNodeSet( sName );
        SetSetProperties( sName, aAnyValues );
    }
}

//*****************************************************************************************************************
sal_Bool SvtViewDialogOptions_Impl::Exists( const OUString& sName )
{
    return ( m_aList.find( sName ) != m_aList.end() );
}

//*****************************************************************************************************************
sal_Bool SvtViewDialogOptions_Impl::Delete( const OUString& sName )
{
    // Set default return value to "element no longer exist"!
    // It doesnt matter for user if element not exist or was deleted!
    // Not exist is not exist is ...
    sal_Bool bDeleteState = sal_True;

    if( m_aList.find( sName ) != m_aList.end() )
    {
        bDeleteState = ClearNodeSet( sName );
        if( bDeleteState == sal_True )
        {
            m_aList.erase( sName );
            SetModified();
        }
    }
    return bDeleteState;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::GetPosition( const OUString& sName, sal_Int32& nX, sal_Int32& nY )
{
    // If entry not exist before our index call in stl vector will create it automaticly!
    // But we must change the default values. An we must call "SetModifed()" ... because
    // we have created a new cache entry.
    impl_CreateIfNotExist( sName );
    nX = m_aList[ sName ].nX;
    nY = m_aList[ sName ].nY;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::SetPosition( const OUString& sName, sal_Int32 nX, sal_Int32 nY )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].nX = nX;
    m_aList[ sName ].nY = nY;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::GetSize( const OUString& sName, sal_Int32& nWidth, sal_Int32& nHeight )
{
    // If entry not exist before our index call in stl vector will create it automaticly!
    // But we must change the default values. An we must call "SetModifed()" ... because
    // we have created a new cache entry.
    impl_CreateIfNotExist( sName );
    nWidth  = m_aList[ sName ].nWidth ;
    nHeight = m_aList[ sName ].nHeight;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::SetSize( const OUString& sName, sal_Int32 nWidth, sal_Int32 nHeight )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].nWidth  = nWidth  ;
    m_aList[ sName ].nHeight = nHeight ;
    SetModified();
}

//*****************************************************************************************************************
OUString SvtViewDialogOptions_Impl::GetUserData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].sUserData = sData;
    SetModified();
}

//*****************************************************************************************************************
Sequence< PropertyValue > SvtViewDialogOptions_Impl::GetAnyData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].aAnyData;
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::SetAnyData( const OUString& sName, const Sequence< PropertyValue >& seqData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].aAnyData = seqData;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames )
{
    _rNames = GetNodeNames( _rNode );
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::impl_ReadWholeList()
{
    // Clear cache, get current name list of existing dialogs in configuration.
    // Insert 5 subkeys for every entry and use these list as SNAPSHOT to read ALL values of our subtree!
    // At least add these values in our hash map.
    m_aList.clear();

    Sequence< OUString >    seqNodeNames    = GetNodeNames( ROOTNODE_DIALOGS )  ;
    sal_uInt32              nNodeCount      = seqNodeNames.getLength()          ;
    Sequence< OUString >    seqAllNames     ( nNodeCount*5 )                    ;
    sal_uInt32              nNodeName       = 0                                 ;
    sal_uInt32              nProperty       = 0                                 ;
    OUString                sName                                               ;

    ::std::vector< OUString >           aAnyValuesNames;
    ::std::vector< sal_Int32 >          aAnyValuesCount;
    ::std::vector< sal_Int32 >          aAnyValuesNodeNamePrefixLen;
    reading::lcl_implInitReadAnyValues( aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, nNodeCount );

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName )
    {
        sName  = ROOTNODE_DIALOGS       ;
        sName += PATHSEPERATOR          ;
        sName += seqNodeNames[nNodeName];
        sName += PATHSEPERATOR          ;
        seqAllNames[nProperty] = sName + PROPERTYNAME_X         ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_Y         ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_WIDTH     ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_HEIGHT    ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_USERDATA  ;
        ++nProperty;

        // under the AnyData node, there may be 0 to n sub nodes ....
        reading::lcl_implReadOneNodeAnyValues( sName, aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, this );
    }

    // the fixed values
    Sequence< Any > seqAllValues = GetProperties( seqAllNames );
    // the AnyValue's
    Sequence< Any > seqAnyValues;
    if ( aAnyValuesNames.size() )
        seqAnyValues = GetProperties( Sequence< OUString >( aAnyValuesNames.begin(), aAnyValuesNames.size() ) );

    // Safe impossible cases.
    // We have asked for ALL our subtree keys and we would get all his values.
    // It's important for next loop and our index using!
    DBG_ASSERT( !(seqAllNames.getLength()!=seqAllValues.getLength()), "SvtViewDialogOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for dialog set!\n" );
    DBG_ASSERT( !(aAnyValuesNames.size()!=(sal_uInt32)seqAnyValues.getLength()), "SvtViewDialogOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for dialog set (any data)!\n" );

    nProperty = 0;
    const sal_Int32*    pAnyCount   = aAnyValuesCount.begin();
    const OUString*     pAnyName    = aAnyValuesNames.begin();
    const Any*          pAnyValue   = seqAnyValues.getConstArray();
    const sal_Int32*    pAnyNameLen = aAnyValuesNodeNamePrefixLen.begin();

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName, ++pAnyCount, ++pAnyNameLen )
    {
        sName = seqNodeNames[nNodeName].copy( SHORTNAME_ENDPOSITION, seqNodeNames[nNodeName].getLength()-SHORTNAME_ENDPOSITION );
        seqAllValues[nProperty] >>= m_aList[sName].nX           ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nY           ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nWidth       ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nHeight      ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].sUserData    ;
        ++nProperty;

        reading::lcl_implFillReadAnyValues( m_aList[sName].aAnyData, pAnyCount, pAnyName, pAnyValue, *pAnyNameLen );
    }
}

//*****************************************************************************************************************
void SvtViewDialogOptions_Impl::impl_CreateIfNotExist( const OUString& sName )
{
    if( m_aList.find( sName ) == m_aList.end() )
    {
        m_aList[sName].nX           = DEFAULT_X         ;
        m_aList[sName].nY           = DEFAULT_Y         ;
        m_aList[sName].nWidth       = DEFAULT_WIDTH     ;
        m_aList[sName].nHeight      = DEFAULT_HEIGHT    ;
        m_aList[sName].sUserData    = DEFAULT_USERDATA  ;
        SetModified();
    }
}

/*-************************************************************************************************************//**
    @descr  Implement the data container for tab-dialogs.
*//*-*************************************************************************************************************/

class SvtViewTabDialogOptions_Impl : public ConfigItem, public reading::IPublicConfigReadAccess
{
    public:

         SvtViewTabDialogOptions_Impl();
        ~SvtViewTabDialogOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        sal_Bool    Exists      (   const   OUString&   sName                                                           );
        sal_Bool    Delete      (   const   OUString&   sName                                                           );
        void        GetPosition (   const   OUString&   sName   ,           sal_Int32&  nX      ,   sal_Int32&  nY      );
        void        SetPosition (   const   OUString&   sName   ,           sal_Int32   nX      ,   sal_Int32   nY      );
        sal_Int32   GetPageID   (   const   OUString&   sName                                                           );
        void        SetPageID   (   const   OUString&   sName   ,           sal_Int32   nID                             );
        OUString    GetUserData (   const   OUString&   sName                                                           );
        void        SetUserData (   const   OUString&   sName   ,   const   OUString&   sData                           );

        Sequence< PropertyValue >   GetAnyData( const   OUString&   sName                                                   );
        void                        SetAnyData( const   OUString&   sName   ,   const   Sequence< PropertyValue >&  aData   );

        virtual void getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames );

    private:

        void    impl_ReadWholeList      (                       );
        void    impl_CreateIfNotExist   ( const OUString& sName );

    private:

        IMPL_TTabDialogHash     m_aList     ;
};

//*****************************************************************************************************************
SvtViewTabDialogOptions_Impl::SvtViewTabDialogOptions_Impl()
        :   ConfigItem  ( VIEWPACKAGE )
{
    // Read complete list from configuration.
    impl_ReadWholeList();

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
/*  Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE_TABDIALOGS;
    EnableNotification( seqNotifyList );*/
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
    // We ignore given name sequence.
    // It's to complex to handle a dynamic set with notifys for add/delete/change elements!
    // I think to read the whole list is the best way.
    impl_ReadWholeList();
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 4 properties for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue >   seqProperties( 4 )  ;
    OUString                    sName               ;

    for( IMPL_TTabDialogHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        sName =  ROOTNODE_TABDIALOGS    ;   //  "TabDialogs"
        sName += PATHSEPERATOR          ;   //  "TabDialogs/"
        sName += SHORTNAME_TABDIALOGS   ;   //  "TabDialogs/td_"
        sName += pIterator->first       ;   //  "TabDialogs/td_<...>"
        sName += PATHSEPERATOR          ;   //  "TabDialogs/td_<...>/"

        seqProperties[0].Name   = sName + PROPERTYNAME_X        ;
        seqProperties[1].Name   = sName + PROPERTYNAME_Y        ;
        seqProperties[2].Name   = sName + PROPERTYNAME_PAGEID   ;
        seqProperties[3].Name   = sName + PROPERTYNAME_USERDATA ;

        seqProperties[0].Value  <<= pIterator->second.nX        ;
        seqProperties[1].Value  <<= pIterator->second.nY        ;
        seqProperties[2].Value  <<= pIterator->second.nPageID   ;
        seqProperties[3].Value  <<= pIterator->second.sUserData ;

        SetSetProperties( ROOTNODE_TABDIALOGS, seqProperties ); // The keyname of our hash is the kename of our set!

        // the AnyData nodes
        Sequence< PropertyValue > aAnyValues( pIterator->second.aAnyData ); // copy, 'cause we want to modify it
        writing::lcl_implPrefixAnyValues( sName, aAnyValues );
        ClearNodeSet( sName );
        SetSetProperties( sName, aAnyValues );
    }
}

//*****************************************************************************************************************
sal_Bool SvtViewTabDialogOptions_Impl::Exists( const OUString& sName )
{
    return ( m_aList.find( sName ) != m_aList.end() );
}

//*****************************************************************************************************************
sal_Bool SvtViewTabDialogOptions_Impl::Delete( const OUString& sName )
{
    // Set default return value to "element no longer exist"!
    // It doesnt matter for user if element not exist or was deleted!
    // Not exist is not exist is ...
    sal_Bool bDeleteState = sal_True;

    if( m_aList.find( sName ) != m_aList.end() )
    {
        bDeleteState = ClearNodeSet( sName );
        if( bDeleteState == sal_True )
        {
            m_aList.erase( sName );
            SetModified();
        }
    }
    return bDeleteState;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::GetPosition( const OUString& sName, sal_Int32& nX, sal_Int32& nY )
{
    // If entry not exist before our index call in stl vector will create it automaticly!
    // But we must change the default values. An we must call "SetModifed()" ... because
    // we have created a new cache entry.
    impl_CreateIfNotExist( sName );
    nX = m_aList[ sName ].nX;
    nY = m_aList[ sName ].nY;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::SetPosition( const OUString& sName, sal_Int32 nX, sal_Int32 nY )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].nX = nX;
    m_aList[ sName ].nY = nY;
    SetModified();
}

//*****************************************************************************************************************
sal_Int32 SvtViewTabDialogOptions_Impl::GetPageID( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].nPageID;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::SetPageID( const OUString& sName, sal_Int32 nID )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].nPageID = nID;
    SetModified();
}

//*****************************************************************************************************************
OUString SvtViewTabDialogOptions_Impl::GetUserData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].sUserData = sData;
    SetModified();
}

//*****************************************************************************************************************
Sequence< PropertyValue > SvtViewTabDialogOptions_Impl::GetAnyData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].aAnyData;
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::SetAnyData( const OUString& sName, const Sequence< PropertyValue >& seqData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].aAnyData = seqData;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames )
{
    _rNames = GetNodeNames( _rNode );
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::impl_ReadWholeList()
{
    // Clear cache, get current name list of existing dialogs in configuration.
    // Insert 4 subkeys for every entry and use these list as SNAPSHOT to read ALL values of our subtree!
    // At least add these values in our hash map.
    m_aList.clear();

    Sequence< OUString >    seqNodeNames    = GetNodeNames( ROOTNODE_TABDIALOGS )   ;
    sal_uInt32              nNodeCount      = seqNodeNames.getLength()              ;
    Sequence< OUString >    seqAllNames     ( nNodeCount*4 )                        ;
    sal_uInt32              nNodeName       = 0                                     ;
    sal_uInt32              nProperty       = 0                                     ;
    OUString                sName                                                   ;

    ::std::vector< OUString >           aAnyValuesNames;
    ::std::vector< sal_Int32 >          aAnyValuesCount;
    ::std::vector< sal_Int32 >          aAnyValuesNodeNamePrefixLen;
    reading::lcl_implInitReadAnyValues( aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, nNodeCount );

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName )
    {
        sName  = ROOTNODE_TABDIALOGS    ;
        sName += PATHSEPERATOR          ;
        sName += seqNodeNames[nNodeName];
        sName += PATHSEPERATOR          ;
        seqAllNames[nProperty] = sName + PROPERTYNAME_X         ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_Y         ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_PAGEID    ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_USERDATA  ;
        ++nProperty;

        // under the AnyData node, there may be 0 to n sub nodes ....
        reading::lcl_implReadOneNodeAnyValues( sName, aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, this );
    }

    // the fixed values
    Sequence< Any > seqAllValues = GetProperties( seqAllNames );
    // the AnyValue's
    Sequence< Any > seqAnyValues;
    if ( aAnyValuesNames.size() )
        seqAnyValues = GetProperties( Sequence< OUString >( aAnyValuesNames.begin(), aAnyValuesNames.size() ) );

    // Safe impossible cases.
    // We have asked for ALL our subtree keys and we would get all his values.
    // It's neccessary for next loop and our index using!
    DBG_ASSERT( !(seqAllNames.getLength()!=seqAllValues.getLength()), "SvtViewTabDialogOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for tab-dialog set!\n" );
    DBG_ASSERT( !(aAnyValuesNames.size()!=(sal_uInt32)seqAnyValues.getLength()), "SvtViewTabDialogOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for dialog set (any data)!\n" );

    nProperty = 0;
    const sal_Int32*    pAnyCount   = aAnyValuesCount.begin();
    const OUString*     pAnyName    = aAnyValuesNames.begin();
    const Any*          pAnyValue   = seqAnyValues.getConstArray();
    const sal_Int32*    pAnyNameLen = aAnyValuesNodeNamePrefixLen.begin();

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName, ++pAnyCount, ++pAnyNameLen )
    {
        sName = seqNodeNames[nNodeName].copy( SHORTNAME_ENDPOSITION, seqNodeNames[nNodeName].getLength()-SHORTNAME_ENDPOSITION );
        seqAllValues[nProperty] >>= m_aList[sName].nX           ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nY           ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nPageID      ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].sUserData    ;
        ++nProperty;

        reading::lcl_implFillReadAnyValues( m_aList[sName].aAnyData, pAnyCount, pAnyName, pAnyValue, *pAnyNameLen );
    }
}

//*****************************************************************************************************************
void SvtViewTabDialogOptions_Impl::impl_CreateIfNotExist( const OUString& sName )
{
    if( m_aList.find( sName ) == m_aList.end() )
    {
        m_aList[sName].nX           = DEFAULT_X         ;
        m_aList[sName].nY           = DEFAULT_Y         ;
        m_aList[sName].nPageID      = DEFAULT_PAGEID    ;
        m_aList[sName].sUserData    = DEFAULT_USERDATA  ;
        SetModified();
    }
}

/*-************************************************************************************************************//**
    @descr  Implement the data container for tab-pages.
*//*-*************************************************************************************************************/

class SvtViewTabPageOptions_Impl : public ConfigItem, public reading::IPublicConfigReadAccess
{
    public:

         SvtViewTabPageOptions_Impl();
        ~SvtViewTabPageOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        sal_Bool    Exists      (   const   OUString&   sName                               );
        sal_Bool    Delete      (   const   OUString&   sName                               );
        OUString    GetUserData (   const   OUString&   sName                               );
        void        SetUserData (   const   OUString&   sName,  const   OUString&   sData   );

        Sequence< PropertyValue >   GetAnyData( const   OUString&   sName                                                   );
        void                        SetAnyData( const   OUString&   sName   ,   const   Sequence< PropertyValue >&  aData   );

        virtual void getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames );

    private:

        void    impl_ReadWholeList      (                       );
        void    impl_CreateIfNotExist   ( const OUString& sName );

    private:

        IMPL_TTabPageHash       m_aList     ;
};

//*****************************************************************************************************************
SvtViewTabPageOptions_Impl::SvtViewTabPageOptions_Impl()
        :   ConfigItem  ( VIEWPACKAGE )
{
    // Read complete list from configuration.
    impl_ReadWholeList();

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
/*  Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE_TABPAGES;
    EnableNotification( seqNotifyList );*/
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
    // We ignore given name sequence.
    // It's to complex to handle a dynamic set with notifys for add/delete/change elements!
    // I think to read the whole list is the best way.
    impl_ReadWholeList();
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 1 property for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue >   seqProperties( 1 )  ;
    OUString                    sName               ;

    for( IMPL_TTabPageHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        sName  = ROOTNODE_TABPAGES  ;   //  "TabPages"
        sName += PATHSEPERATOR      ;   //  "TabPages/"
        sName += SHORTNAME_TABPAGES ;   //  "TabPages/tp_"
        sName += pIterator->first   ;   //  "TabPages/tp_<...>"
        sName += PATHSEPERATOR      ;   //  "TabPages/tp_<...>/"

        seqProperties[0].Name   =   sName + PROPERTYNAME_USERDATA;
        seqProperties[0].Value  <<= pIterator->second.sUserData;

        SetSetProperties( ROOTNODE_TABPAGES, seqProperties ); // The keyname of our hash is the kename of our set!

        // the AnyData nodes
        Sequence< PropertyValue > aAnyValues( pIterator->second.aAnyData ); // copy, 'cause we want to modify it
        writing::lcl_implPrefixAnyValues( sName, aAnyValues );
        ClearNodeSet( sName );
        SetSetProperties( sName, aAnyValues );
    }
}

//*****************************************************************************************************************
sal_Bool SvtViewTabPageOptions_Impl::Exists( const OUString& sName )
{
    return ( m_aList.find( sName ) != m_aList.end() );
}

//*****************************************************************************************************************
sal_Bool SvtViewTabPageOptions_Impl::Delete( const OUString& sName )
{
    // Set default return value to "element no longer exist"!
    // It doesnt matter for user if element not exist or was deleted!
    // Not exist is not exist is ...
    sal_Bool bDeleteState = sal_True;

    if( m_aList.find( sName ) != m_aList.end() )
    {
        bDeleteState = ClearNodeSet( sName );
        if( bDeleteState == sal_True )
        {
            m_aList.erase( sName );
            SetModified();
        }
    }
    return bDeleteState;
}

//*****************************************************************************************************************
OUString SvtViewTabPageOptions_Impl::GetUserData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].sUserData = sData;
    SetModified();
}

//*****************************************************************************************************************
Sequence< PropertyValue > SvtViewTabPageOptions_Impl::GetAnyData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].aAnyData;
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::SetAnyData( const OUString& sName, const Sequence< PropertyValue >& seqData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].aAnyData = seqData;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames )
{
    _rNames = GetNodeNames( _rNode );
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::impl_ReadWholeList()
{
    // Clear cache, get current name list of existing dialogs in configuration.
    // Use these list as SNAPSHOT to read ALL values of our subtree!
    // At least add these values in our hash map.

    // Attention: Method is prepared for more then one properties for one node entry!
    // That's the reason for two counters "nNodeName" and "nProperty"!

    m_aList.clear();

    Sequence< OUString >    seqNodeNames    = GetNodeNames( ROOTNODE_TABPAGES ) ;
    sal_uInt32              nNodeCount      = seqNodeNames.getLength()          ;
    Sequence< OUString >    seqAllNames     ( nNodeCount )                      ;
    sal_uInt32              nNodeName       = 0                                 ;
    sal_uInt32              nProperty       = 0                                 ;
    OUString                sName                                               ;

    ::std::vector< OUString >           aAnyValuesNames;
    ::std::vector< sal_Int32 >          aAnyValuesCount;
    ::std::vector< sal_Int32 >          aAnyValuesNodeNamePrefixLen;
    reading::lcl_implInitReadAnyValues( aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, nNodeCount );

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName )
    {
        sName  = ROOTNODE_TABPAGES      ;
        sName += PATHSEPERATOR          ;
        sName += seqNodeNames[nNodeName];
        sName += PATHSEPERATOR          ;
        seqAllNames[nProperty] = sName + PROPERTYNAME_USERDATA;
        ++nProperty;

        // under the AnyData node, there may be 0 to n sub nodes ....
        reading::lcl_implReadOneNodeAnyValues( sName, aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, this );
    }

    // the fixed values
    Sequence< Any > seqAllValues = GetProperties( seqAllNames );
    // the AnyValue's
    Sequence< Any > seqAnyValues;
    if ( aAnyValuesNames.size() )
        seqAnyValues = GetProperties( Sequence< OUString >( aAnyValuesNames.begin(), aAnyValuesNames.size() ) );

    // Safe impossible cases.
    // We have asked for ALL our subtree keys and we would get all his values.
    // It's neccessary for next loop and our index using!
    DBG_ASSERT( !(seqAllNames.getLength()!=seqAllValues.getLength()), "SvtViewTabPageOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for tab-page set!\n" );
    DBG_ASSERT( !(aAnyValuesNames.size()!=(sal_uInt32)seqAnyValues.getLength()), "SvtViewTabPageOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for dialog set (any data)!\n" );

    nProperty = 0;
    const sal_Int32*    pAnyCount   = aAnyValuesCount.begin();
    const OUString*     pAnyName    = aAnyValuesNames.begin();
    const Any*          pAnyValue   = seqAnyValues.getConstArray();
    const sal_Int32*    pAnyNameLen = aAnyValuesNodeNamePrefixLen.begin();

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName )
    {
        sName = seqNodeNames[nNodeName].copy( SHORTNAME_ENDPOSITION, seqNodeNames[nNodeName].getLength()-SHORTNAME_ENDPOSITION );
        seqAllValues[nProperty] >>= m_aList[sName].sUserData;
        ++nProperty;

        reading::lcl_implFillReadAnyValues( m_aList[sName].aAnyData, pAnyCount, pAnyName, pAnyValue, *pAnyNameLen );
    }
}

//*****************************************************************************************************************
void SvtViewTabPageOptions_Impl::impl_CreateIfNotExist( const OUString& sName )
{
    if( m_aList.find( sName ) == m_aList.end() )
    {
        m_aList[sName].sUserData = DEFAULT_USERDATA;
        SetModified();
    }
}

/*-************************************************************************************************************//**
    @descr  Implement the data container for windows.
*//*-*************************************************************************************************************/

class SvtViewWindowOptions_Impl : public ConfigItem, public reading::IPublicConfigReadAccess
{
    public:

         SvtViewWindowOptions_Impl();
        ~SvtViewWindowOptions_Impl();

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );
        virtual void Commit();

        sal_Bool    Exists      (   const   OUString&   sName                                                           );
        sal_Bool    Delete      (   const   OUString&   sName                                                           );
        void        GetPosition (   const   OUString&   sName   ,           sal_Int32&  nX      ,   sal_Int32&  nY      );
        void        SetPosition (   const   OUString&   sName   ,           sal_Int32   nX      ,   sal_Int32   nY      );
        void        GetSize     (   const   OUString&   sName   ,           sal_Int32&  nWidth  ,   sal_Int32&  nHeight );
        void        SetSize     (   const   OUString&   sName   ,           sal_Int32   nWidth  ,   sal_Int32   nHeight );
        sal_Bool    IsVisible   (   const   OUString&   sName                                                           );
        void        SetVisible  (   const   OUString&   sName   ,           sal_Bool    bState                          );
        OUString    GetUserData (   const   OUString&   sName                                                           );
        void        SetUserData (   const   OUString&   sName   ,   const   OUString&   sData                           );

        Sequence< PropertyValue >   GetAnyData( const   OUString&   sName                                                   );
        void                        SetAnyData( const   OUString&   sName   ,   const   Sequence< PropertyValue >&  aData   );

        virtual void getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames );

    private:

        void    impl_ReadWholeList      (                       );
        void    impl_CreateIfNotExist   ( const OUString& sName );

    private:

        IMPL_TWindowHash    m_aList     ;
};

//*****************************************************************************************************************
SvtViewWindowOptions_Impl::SvtViewWindowOptions_Impl()
        :   ConfigItem  ( VIEWPACKAGE )
{
    // Read complete list from configuration.
    impl_ReadWholeList();

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
/*  Sequence< OUString > seqNotifyList(1);
    seqNotifyList[0] = ROOTNODE_WINDOWS;
    EnableNotification( seqNotifyList );*/
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
    // We ignore given name sequence.
    // It's to complex to handle a dynamic set with notifys for add/delete/change elements!
    // I think to read the whole list is the best way.
    impl_ReadWholeList();
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::Commit()
{
    // Calculate size of dynamic set, copy names and values to it and set it in configuration.
    // For structure informations see class description of "SvtViewOptions" in header!

    // We save 6 properties for every hash item. But his names are fix ...
    // Prepare sequence!
    Sequence< PropertyValue >   seqProperties( 6 )  ;
    OUString                    sName               ;

    for( IMPL_TWindowHash::iterator pIterator=m_aList.begin(); pIterator!=m_aList.end(); ++pIterator )
    {
        sName  = ROOTNODE_WINDOWS   ;   //  "Windows"
        sName += PATHSEPERATOR      ;   //  "Windows/"
        sName += SHORTNAME_WINDOWS  ;   //  "Windows/wi_"
        sName += pIterator->first   ;   //  "Windows/wi_<...>"
        sName += PATHSEPERATOR      ;   //  "Windows/wi_<...>/"

        seqProperties[0].Name   = sName + PROPERTYNAME_X        ;
        seqProperties[1].Name   = sName + PROPERTYNAME_Y        ;
        seqProperties[2].Name   = sName + PROPERTYNAME_WIDTH    ;
        seqProperties[3].Name   = sName + PROPERTYNAME_HEIGHT   ;
        seqProperties[4].Name   = sName + PROPERTYNAME_VISIBLE  ;
        seqProperties[5].Name   = sName + PROPERTYNAME_USERDATA ;

        seqProperties[0].Value  <<= pIterator->second.nX        ;
        seqProperties[1].Value  <<= pIterator->second.nY        ;
        seqProperties[2].Value  <<= pIterator->second.nWidth    ;
        seqProperties[3].Value  <<= pIterator->second.nHeight   ;
        seqProperties[4].Value  <<= pIterator->second.bVisible  ;
        seqProperties[5].Value  <<= pIterator->second.sUserData ;

        SetSetProperties( ROOTNODE_WINDOWS, seqProperties ); // The keyname of our hash is the kename of our set!

        // the AnyData nodes
        Sequence< PropertyValue > aAnyValues( pIterator->second.aAnyData ); // copy, 'cause we want to modify it
        writing::lcl_implPrefixAnyValues( sName, aAnyValues );
        ClearNodeSet( sName );
        SetSetProperties( sName, aAnyValues );
    }
}

//*****************************************************************************************************************
sal_Bool SvtViewWindowOptions_Impl::Exists( const OUString& sName )
{
    return ( m_aList.find( sName ) != m_aList.end() );
}

//*****************************************************************************************************************
sal_Bool SvtViewWindowOptions_Impl::Delete( const OUString& sName )
{
    // Set default return value to "element no longer exist"!
    // It doesnt matter for user if element not exist or was deleted!
    // Not exist is not exist is ...
    sal_Bool bDeleteState = sal_True;

    if( m_aList.find( sName ) != m_aList.end() )
    {
        bDeleteState = ClearNodeSet( sName );
        if( bDeleteState == sal_True )
        {
            m_aList.erase( sName );
            SetModified();
        }
    }
    return bDeleteState;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::GetPosition( const OUString& sName, sal_Int32& nX, sal_Int32& nY )
{
    // If entry not exist before our index call in stl vector will create it automaticly!
    // But we must change the default values. An we must call "SetModifed()" ... because
    // we have created a new cache entry.
    impl_CreateIfNotExist( sName );
    nX = m_aList[ sName ].nX;
    nY = m_aList[ sName ].nY;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetPosition( const OUString& sName, sal_Int32 nX, sal_Int32 nY )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].nX = nX;
    m_aList[ sName ].nY = nY;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::GetSize( const OUString& sName, sal_Int32& nWidth, sal_Int32& nHeight )
{
    // If entry not exist before our index call in stl vector will create it automaticly!
    // But we must change the default values. An we must call "SetModifed()" ... because
    // we have created a new cache entry.
    impl_CreateIfNotExist( sName );
    nWidth  = m_aList[ sName ].nWidth ;
    nHeight = m_aList[ sName ].nHeight;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetSize( const OUString& sName, sal_Int32 nWidth, sal_Int32 nHeight )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].nWidth  = nWidth  ;
    m_aList[ sName ].nHeight = nHeight ;
    SetModified();
}

//*****************************************************************************************************************
sal_Bool SvtViewWindowOptions_Impl::IsVisible( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].bVisible;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetVisible( const OUString& sName, sal_Bool bState )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].bVisible = bState;
    SetModified();
}

//*****************************************************************************************************************
OUString SvtViewWindowOptions_Impl::GetUserData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].sUserData;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetUserData( const OUString& sName, const OUString& sData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].sUserData = sData;
    SetModified();
}

//*****************************************************************************************************************
Sequence< PropertyValue > SvtViewWindowOptions_Impl::GetAnyData( const OUString& sName )
{
    impl_CreateIfNotExist( sName );
    return m_aList[ sName ].aAnyData;
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::SetAnyData( const OUString& sName, const Sequence< PropertyValue >& seqData )
{
    impl_CreateIfNotExist( sName );
    m_aList[ sName ].aAnyData = seqData;
    SetModified();
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::getNodeNames( const rtl::OUString& _rNode, Sequence< OUString >& _rNames )
{
    _rNames = GetNodeNames( _rNode );
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::impl_ReadWholeList()
{
    // Clear cache, get current name list of existing dialogs in configuration.
    // Insert 6 subkeys for every entry and use these list as SNAPSHOT to read ALL values of our subtree!
    // At least add these values in our hash map.
    m_aList.clear();

    Sequence< OUString >    seqNodeNames    = GetNodeNames( ROOTNODE_WINDOWS )  ;
    sal_uInt32              nNodeCount      = seqNodeNames.getLength()          ;
    Sequence< OUString >    seqAllNames     ( nNodeCount*6 )                    ;
    sal_uInt32              nNodeName       = 0                                 ;
    sal_uInt32              nProperty       = 0                                 ;
    OUString                sName                                               ;

    ::std::vector< OUString >           aAnyValuesNames;
    ::std::vector< sal_Int32 >          aAnyValuesCount;
    ::std::vector< sal_Int32 >          aAnyValuesNodeNamePrefixLen;
    reading::lcl_implInitReadAnyValues( aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, nNodeCount );

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName )
    {
        sName  = ROOTNODE_WINDOWS       ;
        sName += PATHSEPERATOR          ;
        sName += seqNodeNames[nNodeName];
        sName += PATHSEPERATOR          ;
        seqAllNames[nProperty] = sName + PROPERTYNAME_X         ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_Y         ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_WIDTH     ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_HEIGHT    ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_VISIBLE   ;
        ++nProperty;
        seqAllNames[nProperty] = sName + PROPERTYNAME_USERDATA  ;
        ++nProperty;

        // under the AnyData node, there may be 0 to n sub nodes ....
        reading::lcl_implReadOneNodeAnyValues( sName, aAnyValuesNames, aAnyValuesCount, aAnyValuesNodeNamePrefixLen, this );
    }

    // the fixed values
    Sequence< Any > seqAllValues = GetProperties( seqAllNames );
    // the AnyValue's
    Sequence< Any > seqAnyValues;
    if ( aAnyValuesNames.size() )
        seqAnyValues = GetProperties( Sequence< OUString >( aAnyValuesNames.begin(), aAnyValuesNames.size() ) );

    // Safe impossible cases.
    // We have asked for ALL our subtree keys and we would get all his values.
    // It's neccessary for next loop and our index using!
    DBG_ASSERT( !(seqAllNames.getLength()!=seqAllValues.getLength()), "SvtViewWindowOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for window set!\n" );
    DBG_ASSERT( !(aAnyValuesNames.size()!=(sal_uInt32)seqAnyValues.getLength()), "SvtViewWindowOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for dialog set (any data)!\n" );

    nProperty = 0;
    const sal_Int32*    pAnyCount   = aAnyValuesCount.begin();
    const OUString*     pAnyName    = aAnyValuesNames.begin();
    const Any*          pAnyValue   = seqAnyValues.getConstArray();
    const sal_Int32*    pAnyNameLen = aAnyValuesNodeNamePrefixLen.begin();

    for( nNodeName=0; nNodeName<nNodeCount; ++nNodeName )
    {
        sName = seqNodeNames[nNodeName].copy( SHORTNAME_ENDPOSITION, seqNodeNames[nNodeName].getLength()-SHORTNAME_ENDPOSITION );
        seqAllValues[nProperty] >>= m_aList[sName].nX           ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nY           ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nWidth       ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].nHeight      ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].bVisible     ;
        ++nProperty;
        seqAllValues[nProperty] >>= m_aList[sName].sUserData    ;
        ++nProperty;

        reading::lcl_implFillReadAnyValues( m_aList[sName].aAnyData, pAnyCount, pAnyName, pAnyValue, *pAnyNameLen );
    }
}

//*****************************************************************************************************************
void SvtViewWindowOptions_Impl::impl_CreateIfNotExist( const OUString& sName )
{
    if( m_aList.find( sName ) == m_aList.end() )
    {
        m_aList[sName].nX           = DEFAULT_X         ;
        m_aList[sName].nY           = DEFAULT_Y         ;
        m_aList[sName].nWidth       = DEFAULT_WIDTH     ;
        m_aList[sName].nHeight      = DEFAULT_HEIGHT    ;
        m_aList[sName].bVisible     = DEFAULT_VISIBLE   ;
        m_aList[sName].sUserData    = DEFAULT_USERDATA  ;
        SetModified();
    }
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
                                    if( m_nRefCount_Dialogs == 1 )
                                    {
                                        m_pDataContainer_Dialogs = new SvtViewDialogOptions_Impl;
                                    }
                                }
                                break;
        case E_TABDIALOG    :   {
                                    // Increase ref count for tab-dialog data container first.
                                    ++m_nRefCount_TabDialogs;
                                    // If these instance the first user of the tab-dialog data container - create these impl static container!
                                    if( m_nRefCount_TabDialogs == 1 )
                                    {
                                        m_pDataContainer_TabDialogs = new SvtViewTabDialogOptions_Impl;
                                    }
                                }
                                break;
        case E_TABPAGE      :   {
                                    // Increase ref count for tab-page data container first.
                                    ++m_nRefCount_TabPages;
                                    // If these instance the first user of the tab-page data container - create these impl static container!
                                    if( m_nRefCount_TabPages == 1 )
                                    {
                                        m_pDataContainer_TabPages = new SvtViewTabPageOptions_Impl;
                                    }
                                }
                                break;
        case E_WINDOW       :   {
                                    // Increase ref count for window data container first.
                                    ++m_nRefCount_Windows;
                                    // If these instance the first user of the window data container - create these impl static container!
                                    if( m_nRefCount_Windows == 1 )
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
                                    if( m_nRefCount_Dialogs == 0 )
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
                                    if( m_nRefCount_TabDialogs == 0 )
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
                                    if( m_nRefCount_TabPages == 0 )
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
                                    if( m_nRefCount_Windows == 0 )
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
sal_Bool SvtViewOptions::Exists() const
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    sal_Bool bExists = sal_False;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    bExists = m_pDataContainer_Dialogs->Exists( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    bExists = m_pDataContainer_TabDialogs->Exists( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    bExists = m_pDataContainer_TabPages->Exists( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    bExists = m_pDataContainer_Windows->Exists( m_sViewName );
                                }
                                break;
    }
    return bExists;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtViewOptions::Delete()
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    sal_Bool bState = sal_False;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    bState = m_pDataContainer_Dialogs->Delete( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    bState = m_pDataContainer_TabDialogs->Delete( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    bState = m_pDataContainer_TabPages->Delete( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    bState = m_pDataContainer_Windows->Delete( m_sViewName );
                                }
                                break;
    }
    return bState;
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
Sequence< PropertyValue > SvtViewOptions::GetAnyData( )
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    Sequence< PropertyValue > aData;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    aData = m_pDataContainer_Dialogs->GetAnyData( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    aData = m_pDataContainer_TabDialogs->GetAnyData( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    aData = m_pDataContainer_TabPages->GetAnyData( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    aData = m_pDataContainer_Windows->GetAnyData( m_sViewName );
                                }
                                break;
    }
    return aData;
}

//*****************************************************************************************************************
void SvtViewOptions::SetAnyData( const Sequence< PropertyValue >& seqData )
{
    // Ready for multithreading
    MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->SetAnyData( m_sViewName, seqData );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetAnyData( m_sViewName, seqData );
                                }
                                break;
        case E_TABPAGE      :   {
                                    m_pDataContainer_TabPages->SetAnyData( m_sViewName, seqData );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetAnyData( m_sViewName, seqData );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  static public method
//*****************************************************************************************************************
Sequence< OUString > SvtViewOptions::SeperateUserData( const OUString& sData, sal_Unicode cSeperator )
{
    Sequence< OUString > seqToken;

    sal_Int32 nLength   = sData.getLength() ;
    sal_Int32 nStart    = 0                 ;
    sal_Int32 nEnd      = 0                 ;
    sal_Int32 nToken    = 0                 ;

    while   (
                nStart < nLength
            )
    {
        nEnd = sData.indexOf( cSeperator, nStart );
        if( nEnd != -1 )
        {
            seqToken[nToken] = sData.copy( nStart, nEnd-nStart );
            nStart = nEnd;
        }
        else
        if( nStart < nLength )
        {
            seqToken[nToken] = sData.copy( nStart, nLength-nStart );
            nStart = nLength;
        }
        ++nToken;
    }

    return seqToken;
}

//*****************************************************************************************************************
//  static public method
//*****************************************************************************************************************
OUString SvtViewOptions::GenerateUserData( const Sequence< OUString >& seqData, sal_Unicode cSeperator )
{
    OUStringBuffer sUserData( 1000 );
    sal_Int32 nCount = seqData.getLength();
    for( sal_Int32 nToken=0; nToken<nCount; ++nToken )
    {
        sUserData.append( seqData[nToken] );
        if( nToken < nCount )
        {
            sUserData.append( cSeperator );
        }
    }
    return sUserData.makeStringAndClear();
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

void SvtViewOptions::AcquireOptions()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    if( ++m_nRefCount_Dialogs == 1 )
        m_pDataContainer_Dialogs = new SvtViewDialogOptions_Impl;
    if( ++m_nRefCount_TabDialogs == 1 )
        m_pDataContainer_TabDialogs = new SvtViewTabDialogOptions_Impl;
    if( ++m_nRefCount_TabPages == 1 )
        m_pDataContainer_TabPages = new SvtViewTabPageOptions_Impl;
    if( ++m_nRefCount_Windows == 1 )
        m_pDataContainer_Windows = new SvtViewWindowOptions_Impl;
}

void SvtViewOptions::ReleaseOptions()
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    if( --m_nRefCount_Dialogs == 0 )
    {
        delete m_pDataContainer_Dialogs;
        m_pDataContainer_Dialogs = NULL;
    }
    if( --m_nRefCount_TabDialogs == 0 )
    {
        delete m_pDataContainer_TabDialogs;
        m_pDataContainer_TabDialogs = NULL;
    }
    if( --m_nRefCount_TabPages == 0 )
    {
        delete m_pDataContainer_TabPages;
        m_pDataContainer_TabPages = NULL;
    }
    if( --m_nRefCount_Windows == 0 )
    {
        delete m_pDataContainer_Windows;
        m_pDataContainer_Windows = NULL;
    }
}

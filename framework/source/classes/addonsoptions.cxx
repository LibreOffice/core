/*************************************************************************
 *
 *  $RCSfile: addonsoptions.cxx,v $
 *
 *  $Revision: 1.2 $
 *  last change: $Author: hr $ $Date: 2003-03-25 18:21:26 $
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

#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include <classes/addonsoptions.hxx>
#endif

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <hash_map>
#include <algorithm>

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

#define ROOTNODE_ADDONMENU                              OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Addons"    ))
#define PATHDELIMITER                                   OUString(RTL_CONSTASCII_USTRINGPARAM("/"                ))
#define SEPARATOR_URL                                   OUString(RTL_CONSTASCII_USTRINGPARAM("private:separator"))

#define PROPERTYNAME_URL                                ADDONSMENUITEM_PROPERTYNAME_URL
#define PROPERTYNAME_TITLE                              ADDONSMENUITEM_PROPERTYNAME_TITLE
#define PROPERTYNAME_TARGET                             ADDONSMENUITEM_PROPERTYNAME_TARGET
#define PROPERTYNAME_IMAGEIDENTIFIER                    ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER

#define PROPERTYNAME_SUBMENU                            ADDONSMENUITEM_PROPERTYNAME_SUBMENU
#define PROPERTYNAME_COMPONENTID                        ADDONSMENUITEM_PROPERTYNAME_COMPONENTID
#define PROPERTYNAME_CONTEXT                            ADDONSMENUITEM_PROPERTYNAME_CONTEXT

#define PROPERTYNAME_IMAGESMALL                         OUString(RTL_CONSTASCII_USTRINGPARAM("ImageSmall" ))
#define PROPERTYNAME_IMAGEBIG                           OUString(RTL_CONSTASCII_USTRINGPARAM("ImageBig" ))
#define PROPERTYNAME_IMAGESMALLHC                       OUString(RTL_CONSTASCII_USTRINGPARAM("ImageSmallHC" ))
#define PROPERTYNAME_IMAGEBIGHC                         OUString(RTL_CONSTASCII_USTRINGPARAM("ImageBigHC" ))

#define IMAGES_NODENAME                                 OUString(RTL_CONSTASCII_USTRINGPARAM("Images" ))

#define PROPERTYCOUNT_ROOT                              6   // Used for C++ interface
#define PROPERTYCOUNT_UI_ROOT                           7
#define PROPERTYCOUNT                                   5
#define CFG_PROPERTYCOUNT                               4
#define PROPERTYIMAGES_COUNT                            4

#define OFFSET_URL                                      0
#define OFFSET_TITLE                                    1
#define OFFSET_TARGET                                   2
#define OFFSET_IMAGEIDENTIFIER                          3
#define OFFSET_SUBMENU                                  4
#define OFFSET_COMPONENTID                              5

#define CFG_UITYPE_PROPERTYCOUNT                        5
#define OFFSET_UITYPE_TITLE                             0
#define OFFSET_UITYPE_TARGET                            1
#define OFFSET_UITYPE_IMAGEIDENTIFIER                   2
#define OFFSET_UITYPE_CONTEXT                           3
#define OFFSET_UITYPE_SUBMENU                           4
#define OFFSET_UITYPE_COMPONENTID                       5
#define OFFSET_UITYPE_URL                               6

#define OFFSET_IMAGESMALL                               0
#define OFFSET_IMAGEBIG                                 1
#define OFFSET_IMAGESMALLHC                             2
#define OFFSET_IMAGEBIGHC                               3

#define MENUNODENAME_PREFIX                             OUString(RTL_CONSTASCII_USTRINGPARAM( "m" ))

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

/*-****************************************************************************************************************
    @descr  struct to hold information about one menu entry.
****************************************************************************************************************-*/

namespace framework
{

struct AddonMenuEntry;
typedef vector< AddonMenuEntry > AddonSubMenu;

struct AddonMenuEntry
{
    public:
        AddonMenuEntry() {};

        AddonMenuEntry(  const OUString&        sNewURL             ,
                         const OUString&        sNewTitle           ,
                         const OUString&        sNewImageIdentifier ,
                         const OUString&        sNewTarget          ,
                         const AddonSubMenu&    aNewSubMenu             ) :
            sURL( sNewURL ),
            sTitle( sNewTitle ),
            sImageIdentifier( sNewImageIdentifier ),
            sTarget( sNewTarget ),
            aSubMenu( aNewSubMenu )
        {
        }

        void AppendMenuEntry( const AddonMenuEntry& rEntry )
        {
            aSubMenu.push_back( rEntry );
        }

    public:
        OUString        sName               ;
        OUString        sURL                ;
        OUString        sTitle              ;
        OUString        sTarget             ;
        OUString        sImageIdentifier    ;
        AddonSubMenu    aSubMenu            ;
};

struct AddonMenuRootEntry
{
    public:
        AddonMenuRootEntry() {};

        AddonMenuRootEntry(  const OUString&        sNewComponentID     ,
                              const OUString&       sNewURL             ,
                             const OUString&        sNewTitle           ,
                             const OUString&        sNewTarget          ,
                             const OUString&        sNewImageIdentifier ,
                             const OUString&        sNewContext         ,
                             const AddonSubMenu&    aNewSubMenu             ) :
            sName( sNewComponentID ),
            sURL( sNewURL ),
            sTitle( sNewTitle ),
            sImageIdentifier( sNewImageIdentifier ),
            sTarget( sNewTarget ),
            sContext( sNewContext ),
            aSubMenu( aNewSubMenu )
        {
        }

    public:
        OUString        sName               ;
        OUString        sURL                ;
        OUString        sTitle              ;
        OUString        sTarget             ;
        OUString        sImageIdentifier    ;
        OUString        sContext            ;
        AddonSubMenu    aSubMenu            ;
};

/*-****************************************************************************************************************
    @descr  support simple menu structures and operations on it
****************************************************************************************************************-*/
class SvAddonMenu
{
    public:
        SvAddonMenu() {}

        sal_Int32 Size() const { return aAddonsMenuList.size(); }

        //---------------------------------------------------------------------------------------------------------
        // append sub menu entry
        // Assign a unique node name to fulfill to the configuration requirements.
        sal_Bool AppendMenuEntry( const rtl::OUString& aComponentID, const AddonMenuEntry& rEntry )
        {
            for ( sal_uInt32 nIndex = 0; nIndex < aAddonsMenuList.size(); nIndex++ )
            {
                AddonMenuRootEntry& rAddonMenuRootEntry = aAddonsMenuList[nIndex];
                if ( rAddonMenuRootEntry.sName.equals( aComponentID ))
                {
                    rAddonMenuRootEntry.aSubMenu.push_back( rEntry );
                    return sal_True;
                }
            }

            return sal_False;
        }

        //---------------------------------------------------------------------------------------------------------
        // append root menu entry
        void AppendRootMenuEntry( const AddonMenuRootEntry& rEntry )
        {
            aAddonsMenuList.push_back( rEntry );
        }

        //---------------------------------------------------------------------------------------------------------
        // retrieve a root menu entry from an index
        const AddonMenuRootEntry* GetRootMenuEntry( sal_uInt32 nIndex ) const
        {
            if ( nIndex < aAddonsMenuList.size() )
                return &(aAddonsMenuList[nIndex]);
            else
                return NULL;
        }

        //---------------------------------------------------------------------------------------------------------
        // the only way to free memory!
        void Clear()
        {
            aAddonsMenuList.clear();
        }

        //---------------------------------------------------------------------------------------------------------
        // convert internal list to external format
        // for using it on right menus realy
        // Notice:   We build a property list with 4 entries and set it on result list then.
        //           The while-loop starts with pointer on internal member list lSetupEntries, change to
        //           lUserEntries then and stop after that with NULL!
        //           Separator entries will be packed in another way then normal entries! We define
        //           special strings "sEmpty" and "sSeperator" to perform too ...
        Sequence< Sequence< PropertyValue > > GetList() const;

        //---------------------------------------------------------------------------------------------------------
        // convert internal submenu to external format
        // for using it on right menus realy
        // Notice:   We build a property list with 4 entries and set it on result list then.
        //           The while-loop starts with pointer on internal member list lSetupEntries, change to
        //           lUserEntries then and stop after that with NULL!
        //           Separator entries will be packed in another way then normal entries! We define
        //           special strings "sEmpty" and "sSeperator" to perform too ...
        Sequence< Sequence< PropertyValue > > GetSubMenuList( const AddonSubMenu& rSubMenu ) const;

        //---------------------------------------------------------------------------------------------------------
        // convert internal menu bar list to external format
        // for using it on right menus realy
        // Notice:   We build a property list with 4 entries and set it on result list then.
        //           The while-loop starts with pointer on internal member list lSetupEntries, change to
        //           lUserEntries then and stop after that with NULL!
        //           Separator entries will be packed in another way then normal entries! We define
        //           special strings "sEmpty" and "sSeperator" to perform too ...
        Sequence< Sequence< PropertyValue > > SvAddonMenu::GetMenuBarList() const;

    private:
        //---------------------------------------------------------------------------------------------------------
        // search for an entry named "ux" with x=[0..i] inside our menu
        // which has set highest number x. So we can add another user entry.
        sal_Int32 impl_getNextSubMenuEntryNr( const AddonSubMenu& rSubMenu ) const
        {
            sal_Int32 nNr = 0;
            for( vector< AddonMenuEntry >::const_iterator pItem = rSubMenu.begin()  ;
                                                        pItem!=rSubMenu.end()           ;
                                                        ++pItem                         )
            {
                sal_Int32 nCurrNr = pItem->sName.copy( 1 ).toInt32();
                if ( nCurrNr > nNr )
                    nNr = nCurrNr;
            }
            return nNr;
        }

    private:
        const OUString                  sEmpty          ;
        vector< AddonMenuRootEntry > aAddonsMenuList    ;
};


Sequence< Sequence< PropertyValue > > SvAddonMenu::GetList() const
{
    sal_Int32                               nCount      = (sal_Int32)aAddonsMenuList.size();
    sal_Int32                               nStep       = 0;
    Sequence< PropertyValue >               lRootProperties ( PROPERTYCOUNT_ROOT );
    Sequence< Sequence< PropertyValue > >   lResult     ( nCount );
    const vector< AddonMenuRootEntry >*     pList       = &aAddonsMenuList;

    // Root menu item properites
    lRootProperties[OFFSET_URL              ].Name = PROPERTYNAME_URL               ;
    lRootProperties[OFFSET_TITLE            ].Name = PROPERTYNAME_TITLE             ;
    lRootProperties[OFFSET_IMAGEIDENTIFIER  ].Name = PROPERTYNAME_IMAGEIDENTIFIER   ;
    lRootProperties[OFFSET_TARGET           ].Name = PROPERTYNAME_TARGET            ;
    lRootProperties[OFFSET_SUBMENU          ].Name = PROPERTYNAME_SUBMENU           ;
    lRootProperties[OFFSET_COMPONENTID      ].Name = PROPERTYNAME_COMPONENTID       ;

    for( vector< AddonMenuRootEntry >::const_iterator pItem =pList->begin();
                                                pItem!=pList->end()  ;
                                                ++pItem              )
    {
        lRootProperties[OFFSET_URL              ].Value <<= pItem->sURL             ;
        lRootProperties[OFFSET_TITLE            ].Value <<= pItem->sTitle           ;
        lRootProperties[OFFSET_TARGET           ].Value <<= pItem->sTarget          ;
        lRootProperties[OFFSET_IMAGEIDENTIFIER  ].Value <<= pItem->sImageIdentifier ;

        sal_Int32 nSubMenuCount = pItem->aSubMenu.size();
        if ( nSubMenuCount > 0 )
            lRootProperties[OFFSET_SUBMENU].Value <<= GetSubMenuList( pItem->aSubMenu );
        else
            lRootProperties[OFFSET_SUBMENU].Value <<= Sequence< Sequence< PropertyValue > >();
        lRootProperties[OFFSET_COMPONENTID      ].Value <<= pItem->sName            ;

        lResult[nStep++] = lRootProperties;
    }

    return lResult;
}

Sequence< Sequence< PropertyValue > > SvAddonMenu::GetMenuBarList() const
{
    sal_Int32                               nCount      = (sal_Int32)aAddonsMenuList.size();
    sal_Int32                               nStep       = 0;
    Sequence< PropertyValue >               lRootProperties ( PROPERTYCOUNT_UI_ROOT );
    Sequence< Sequence< PropertyValue > >   lResult     ( nCount );
    const vector< AddonMenuRootEntry >*     pList       = &aAddonsMenuList;

    // Root menu bar item properites
    lRootProperties[OFFSET_UITYPE_URL               ].Name = PROPERTYNAME_URL               ;
    lRootProperties[OFFSET_UITYPE_TITLE             ].Name = PROPERTYNAME_TITLE             ;
    lRootProperties[OFFSET_UITYPE_IMAGEIDENTIFIER   ].Name = PROPERTYNAME_IMAGEIDENTIFIER   ;
    lRootProperties[OFFSET_UITYPE_IMAGEIDENTIFIER   ].Name = PROPERTYNAME_TARGET            ;
    lRootProperties[OFFSET_UITYPE_SUBMENU           ].Name = PROPERTYNAME_SUBMENU           ;
    lRootProperties[OFFSET_UITYPE_COMPONENTID       ].Name = PROPERTYNAME_COMPONENTID       ;
    lRootProperties[OFFSET_UITYPE_CONTEXT           ].Name = PROPERTYNAME_CONTEXT           ;

    for( vector< AddonMenuRootEntry >::const_iterator pItem =pList->begin();
                                                pItem!=pList->end()  ;
                                                ++pItem              )
    {
        lRootProperties[OFFSET_UITYPE_URL               ].Value <<= pItem->sURL             ;
        lRootProperties[OFFSET_UITYPE_TITLE             ].Value <<= pItem->sTitle           ;
        lRootProperties[OFFSET_UITYPE_TARGET            ].Value <<= pItem->sTarget          ;
        lRootProperties[OFFSET_UITYPE_IMAGEIDENTIFIER   ].Value <<= pItem->sImageIdentifier ;
        lRootProperties[OFFSET_UITYPE_CONTEXT           ].Value <<= pItem->sContext         ;

        sal_Int32 nSubMenuCount = pItem->aSubMenu.size();
        if ( nSubMenuCount > 0 )
            lRootProperties[OFFSET_UITYPE_SUBMENU].Value <<= GetSubMenuList( pItem->aSubMenu );
        else
            lRootProperties[OFFSET_SUBMENU].Value <<= Sequence< Sequence< PropertyValue > >();
        lRootProperties[OFFSET_UITYPE_COMPONENTID       ].Value <<= pItem->sName            ;

        lResult[nStep++] = lRootProperties;
    }

    return lResult;
}


Sequence< Sequence< PropertyValue > > SvAddonMenu::GetSubMenuList( const AddonSubMenu& rSubMenu ) const
{
    const OUString sSeparator( RTL_CONSTASCII_USTRINGPARAM("private:separator") );

    // Sub menu item properites
    sal_Int32                               nStep       = 0;
    Sequence< PropertyValue >               lProperties ( PROPERTYCOUNT );
    Sequence< Sequence< PropertyValue > >   lResult     ( rSubMenu.size() );

    lProperties[OFFSET_URL              ].Name = PROPERTYNAME_URL               ;
    lProperties[OFFSET_TITLE            ].Name = PROPERTYNAME_TITLE             ;
    lProperties[OFFSET_TARGET           ].Name = PROPERTYNAME_TARGET            ;
    lProperties[OFFSET_IMAGEIDENTIFIER  ].Name = PROPERTYNAME_IMAGEIDENTIFIER   ;
    lProperties[OFFSET_SUBMENU          ].Name = PROPERTYNAME_SUBMENU           ;

    for( vector< AddonMenuEntry >::const_iterator pItem = rSubMenu.begin()  ;
                                                pItem!=rSubMenu.end()           ;
                                                ++pItem                         )
    {
        if ( pItem->sURL == sSeparator )
        {
            lProperties[OFFSET_URL              ].Value <<= sSeparator                              ;
            lProperties[OFFSET_TITLE            ].Value <<= sEmpty                                  ;
            lProperties[OFFSET_TARGET           ].Value <<= sEmpty                                  ;
            lProperties[OFFSET_IMAGEIDENTIFIER  ].Value <<= sEmpty                                  ;
            lProperties[OFFSET_SUBMENU          ].Value <<= Sequence< Sequence< PropertyValue > >() ;
        }
        else
        {
            lProperties[OFFSET_URL              ].Value <<= pItem->sURL             ;
            lProperties[OFFSET_TITLE            ].Value <<= pItem->sTitle           ;
            lProperties[OFFSET_TARGET           ].Value <<= pItem->sTarget          ;
            lProperties[OFFSET_IMAGEIDENTIFIER  ].Value <<= pItem->sImageIdentifier ;

            sal_Int32 nSubMenuCount = pItem->aSubMenu.size();
            if ( nSubMenuCount > 0 )
                lProperties[OFFSET_SUBMENU].Value <<= GetSubMenuList( pItem->aSubMenu );
            else
                lProperties[OFFSET_SUBMENU].Value <<= Sequence< Sequence< PropertyValue > >();
        }

        lResult[nStep++] = lProperties;
    }

    return lResult;
}


class AddonsOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:
        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         AddonsOptions_Impl();
        ~AddonsOptions_Impl();

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& lPropertyNames );

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
            @short      base implementation of public interface for "SvtDynamicMenuOptions"!
            @descr      These class is used as static member of "SvtDynamicMenuOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool                                        HasAddonsMenu   () const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenu   () const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenuBarPart() const;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsToolBarPart() const;
        Sequence< PropertyValue >                       GetAddonsComponentMenu( const ::rtl::OUString aComponentID ) const;
        Image                                           GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of our configuration management which represent oue module tree
            @descr      These methods return the current list of key names! We need it to get needed values from our
                        configuration management!

            @seealso    -

            @param      "nCount"     ,   returns count of menu entries for "new"
            @return     A list of configuration key names is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool             ReadRootMenuEntry( const OUString& aRootMenuNodeName, AddonMenuRootEntry& rRootMenuEntry );
        sal_Bool             ReadRootMenuBarEntry( const OUString& aRootMenuBarNodeName, AddonMenuRootEntry& rRootMenuBarEntry );

        sal_Bool             ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, AddonSubMenu& rSubMenuContainer );
        sal_Bool             ReadSubMenuEntry( const OUString& aMenuEntryNodeName, AddonSubMenu& rSubMenuContainer );
        Sequence< OUString > GetPropertyNames( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetUITypePropertyNames( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetImagesPropertyNames( const OUString& aPropertyRootNode ) const;
        sal_Bool             CreateImageFromSequence( Image& rImage, sal_Bool bBig, Sequence< sal_Int8 >& rBitmapDataSeq ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:
        struct OUStringHashCode
        {
            size_t operator()( const ::rtl::OUString& sString ) const
            {
                return sString.hashCode();
            }
        };

        struct ImageEntry
        {
            Image   aImageSmall;
            Image   aImageBig;
            Image   aImageSmallHC;
            Image   aImageBigHC;
        };

        ImageEntry* ReadOptionalImageData( const OUString& aMenuNodeName );
        void        AddImageEntryToImageManager( const OUString& aURL, ImageEntry* pImageEntry );

        typedef std::hash_map< OUString, ImageEntry, OUStringHashCode, ::std::equal_to< OUString > > ImageManager;

        sal_Int32                               m_nRootAddonPopupMenuId;
        OUString                                m_aPropNames[PROPERTYCOUNT_ROOT];
        OUString                                m_aPropUINames[PROPERTYCOUNT_UI_ROOT];
        OUString                                m_aPropImagesNames[PROPERTYIMAGES_COUNT];
        OUString                                m_aEmpty;
        OUString                                m_aPathDelimiter;
        OUString                                m_aSeparator;
        OUString                                m_aRootAddonPopupMenuURLPrexfix;
        SvAddonMenu                             m_aAddonMenu;
        SvAddonMenu                             m_aAddonMenuBarPart;
        SvAddonMenu                             m_aAddonToolBarPart;
        Sequence< Sequence< PropertyValue > >   m_aCachedMenuProperties;
        Sequence< Sequence< PropertyValue > >   m_aCachedMenuBarPartProperties;
        Sequence< Sequence< PropertyValue > >   m_aCachedToolBarPartProperties;
        ImageManager                            m_aImageManager;
};

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
AddonsOptions_Impl::AddonsOptions_Impl()
    // Init baseclasses first
    :   ConfigItem( ROOTNODE_ADDONMENU ),
    m_aPathDelimiter( PATHDELIMITER ),
    m_aSeparator( SEPARATOR_URL ),
    m_nRootAddonPopupMenuId( 0 ),
    m_aRootAddonPopupMenuURLPrexfix( ADDONSPOPUPMENU_URL_PREFIX )
{
    // initialize array with fixed property names
    m_aPropNames[ OFFSET_URL             ] = PROPERTYNAME_URL;
    m_aPropNames[ OFFSET_TITLE           ] = PROPERTYNAME_TITLE;
    m_aPropNames[ OFFSET_TARGET          ] = PROPERTYNAME_TARGET;
    m_aPropNames[ OFFSET_IMAGEIDENTIFIER ] = PROPERTYNAME_IMAGEIDENTIFIER;
    m_aPropNames[ OFFSET_SUBMENU         ] = PROPERTYNAME_SUBMENU;      // Submenu set!
    m_aPropNames[ OFFSET_COMPONENTID     ] = PROPERTYNAME_COMPONENTID;  // External property

    // initialize array with fixed property names
    m_aPropUINames[ OFFSET_UITYPE_URL               ] = PROPERTYNAME_URL;
    m_aPropUINames[ OFFSET_UITYPE_TITLE             ] = PROPERTYNAME_TITLE;
    m_aPropUINames[ OFFSET_UITYPE_TARGET            ] = PROPERTYNAME_TARGET;
    m_aPropUINames[ OFFSET_UITYPE_IMAGEIDENTIFIER   ] = PROPERTYNAME_IMAGEIDENTIFIER;
    m_aPropUINames[ OFFSET_UITYPE_CONTEXT           ] = PROPERTYNAME_CONTEXT;       // Context
    m_aPropUINames[ OFFSET_UITYPE_SUBMENU           ] = PROPERTYNAME_SUBMENU;       // Submenu set!
    m_aPropUINames[ OFFSET_UITYPE_COMPONENTID       ] = PROPERTYNAME_COMPONENTID;   // External property

    // initialize array with fixed images property names
    m_aPropImagesNames[ OFFSET_IMAGESMALL   ] = PROPERTYNAME_IMAGESMALL;
    m_aPropImagesNames[ OFFSET_IMAGEBIG     ] = PROPERTYNAME_IMAGEBIG;
    m_aPropImagesNames[ OFFSET_IMAGESMALLHC ] = PROPERTYNAME_IMAGESMALLHC;
    m_aPropImagesNames[ OFFSET_IMAGEBIGHC   ] = PROPERTYNAME_IMAGEBIGHC;

    // Get names and values of all accessable menu entries and fill internal structures.
    // See impl_GetPropertyNames() for further informations.
    OUString                aAddonMenuNodeName( RTL_CONSTASCII_USTRINGPARAM( "AddonUI/AddonMenu" ));
    Sequence< OUString >    aAddonRootMenuNodeSeq = GetNodeNames( aAddonMenuNodeName );
    OUString                aAddonMenuTreeNode( aAddonMenuNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonRootMenuNodeSeq.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        AddonMenuRootEntry   aItem;
        OUString aRootMenuNode( aAddonMenuTreeNode + aAddonRootMenuNodeSeq[n] );
        if ( ReadRootMenuEntry( aRootMenuNode, aItem ) )
            m_aAddonMenu.AppendRootMenuEntry( aItem );
    }

    // Save values to the external representation structure to save memory and time.
    // Currently this is possible as there is no way to register components during Office runtime!
    m_aCachedMenuProperties = m_aAddonMenu.GetList();

    // Read the extended OfficeMenuBar definition that allows external component to integrate their
    // popup-menu into the Office menu bar!
    OUString                aAddonMenuBarNodeName( RTL_CONSTASCII_USTRINGPARAM( "AddonUI/OfficeMenuBar" ));
    Sequence< OUString >    aAddonRootMenuBarNodeSeq = GetNodeNames( aAddonMenuBarNodeName );
    OUString                aAddonMenuBarTreeNode( aAddonMenuBarNodeName + m_aPathDelimiter );

    nCount = aAddonRootMenuBarNodeSeq.getLength();
    for ( n = 0; n < nCount; n++ )
    {
        AddonMenuRootEntry   aItem;
        OUString aRootMenuNode( aAddonMenuBarTreeNode + aAddonRootMenuBarNodeSeq[n] );
        if ( ReadRootMenuBarEntry( aRootMenuNode, aItem ) )
            m_aAddonMenuBarPart.AppendRootMenuEntry( aItem );
    }

    // Save values to the external representation structure to save memory and time.
    // Currently this is possible as there is no way to register components during Office runtime!
    m_aCachedMenuBarPartProperties = m_aAddonMenuBarPart.GetMenuBarList();

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of ouer baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    EnableNotification( lNames );
*/
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
AddonsOptions_Impl::~AddonsOptions_Impl()
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
void AddonsOptions_Impl::Notify( const Sequence< OUString >& lPropertyNames )
{
    DBG_ASSERT( sal_False, "AddonsOptions_Impl::Notify()\nNot implemented yet! I don't know how I can handle a dynamical list of unknown properties ...\n" );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void AddonsOptions_Impl::Commit()
{
    DBG_ERROR( "AddonsOptions_Impl::Commit()\nNot implemented yet!\n" );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::HasAddonsMenu() const
{
    return ( m_aAddonMenu.Size() > 0 );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsMenu() const
{
    return m_aCachedMenuProperties;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< PropertyValue > AddonsOptions_Impl::GetAddonsComponentMenu( const ::rtl::OUString aComponentID ) const
{
    Sequence< PropertyValue > lResult( PROPERTYCOUNT_ROOT );

    for ( sal_uInt32 nIndex = 0; nIndex < (sal_uInt32)m_aAddonMenu.Size(); nIndex++ )
    {
        const AddonMenuRootEntry* pRootMenuEntry = m_aAddonMenu.GetRootMenuEntry( nIndex );
        if ( pRootMenuEntry && pRootMenuEntry->sName.equals( aComponentID ))
        {
            lResult[ OFFSET_URL ].Name = m_aPropNames[ OFFSET_URL ];
            lResult[ OFFSET_URL ].Value <<= pRootMenuEntry->sURL;
            lResult[ OFFSET_TITLE ].Name = m_aPropNames[ OFFSET_TITLE ];
            lResult[ OFFSET_TITLE ].Value <<= pRootMenuEntry->sTitle;
            lResult[ OFFSET_TARGET ].Name = m_aPropNames[ OFFSET_TARGET ];
            lResult[ OFFSET_TARGET ].Value <<= pRootMenuEntry->sTarget;
            lResult[ OFFSET_IMAGEIDENTIFIER ].Name = m_aPropNames[ OFFSET_TITLE ];
            lResult[ OFFSET_IMAGEIDENTIFIER ].Value <<= pRootMenuEntry->sImageIdentifier;
            lResult[ OFFSET_COMPONENTID     ].Name = m_aPropNames[ OFFSET_COMPONENTID ];
            lResult[ OFFSET_COMPONENTID     ].Value <<= pRootMenuEntry->sName;
            lResult[ OFFSET_SUBMENU         ].Name = m_aPropNames[ OFFSET_SUBMENU ];

            if ( pRootMenuEntry->aSubMenu.size() > 0 )
                lResult[ OFFSET_SUBMENU ].Value <<= m_aAddonMenu.GetSubMenuList( pRootMenuEntry->aSubMenu );
            else
                lResult[ OFFSET_SUBMENU ].Value <<= Sequence< Sequence< PropertyValue > >();

            return lResult;
        }
    }

    return lResult;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsMenuBarPart() const
{
    return m_aCachedMenuBarPartProperties;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsToolBarPart() const
{
    return m_aCachedToolBarPartProperties;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************

Image AddonsOptions_Impl::GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast ) const
{
    Image aImage;

    ImageManager::const_iterator pIter = m_aImageManager.find( aURL );
    if ( pIter != m_aImageManager.end() )
    {
        if ( !bHiContrast  )
            aImage = ( bBig ? pIter->second.aImageBig : pIter->second.aImageSmall );
        else
            aImage = ( bBig ? pIter->second.aImageBigHC : pIter->second.aImageSmallHC );
    }

    return aImage;
}


//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadRootMenuEntry( const OUString& aRootMenuNodeName, AddonMenuRootEntry& rRootMenuEntry )
{
    sal_Bool            bResult = sal_False;
    OUString            aAddonRootMenuTreeNode( aRootMenuNodeName + m_aPathDelimiter );
    Sequence< Any >     aRootMenuNodePropValues;

    aRootMenuNodePropValues = GetProperties( GetPropertyNames( aAddonRootMenuTreeNode ) );
    if ( aRootMenuNodePropValues[ OFFSET_TITLE ] >>= rRootMenuEntry.sTitle )
    {
        OUString aRootSubMenuName( aAddonRootMenuTreeNode + m_aPropNames[ OFFSET_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.getLength() > 0 )
        {
            // A popup menu only needs a title, ignore other properties and continue to read sub menu nodes
            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aRootSubMenuNodeNames.getLength(); n++ )
                aRootSubMenuNodeNames[n] = OUString( aSubMenuRootNodeName + aRootSubMenuNodeNames[n] );
            ReadSubMenuEntries( aRootSubMenuNodeNames, rRootMenuEntry.aSubMenu );
            rRootMenuEntry.sName = aRootMenuNodeName.copy( aRootMenuNodeName.lastIndexOf( '/' )+1);
            bResult = sal_True;
        }
        else
        {
            // A simple menu item => read the other properties;
            if ( aRootMenuNodePropValues[ OFFSET_URL ] >>= rRootMenuEntry.sURL ) // Menu item must have a command URL
            {
                aRootMenuNodePropValues[ OFFSET_TARGET          ] >>= rRootMenuEntry.sTarget;
                aRootMenuNodePropValues[ OFFSET_IMAGEIDENTIFIER ] >>= rRootMenuEntry.sImageIdentifier;

                // Set the name/component id of the root menu
                rRootMenuEntry.sName = aRootMenuNodeName.copy( aRootMenuNodeName.lastIndexOf( '/' )+1);

                // Store bitmap data defined for this menu item into the internal image manager
                ImageEntry* pImageEntry = ReadOptionalImageData( aRootMenuNodeName );
                if ( pImageEntry )
                    AddImageEntryToImageManager( rRootMenuEntry.sURL, pImageEntry );

                bResult = sal_True;
            }
        }
    }

    return bResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadRootMenuBarEntry( const OUString& aRootMenuBarNodeName, AddonMenuRootEntry& rRootMenuBarEntry )
{
    sal_Bool            bResult = sal_False;
    OUString            aAddonRootMenuBarTreeNode( aRootMenuBarNodeName + m_aPathDelimiter );
    Sequence< Any >     aRootMenuBarNodePropValues;

    aRootMenuBarNodePropValues = GetProperties( GetUITypePropertyNames( aAddonRootMenuBarTreeNode ) );
    if ( aRootMenuBarNodePropValues[ OFFSET_UITYPE_TITLE    ] >>= rRootMenuBarEntry.sTitle  )
    {
        // A top-level popup menu needs a title and prefixed url
        OUStringBuffer aBuf( m_aRootAddonPopupMenuURLPrexfix.getLength() + 3 );
        aBuf.append( m_aRootAddonPopupMenuURLPrexfix );
        aBuf.append( OUString::valueOf( ++m_nRootAddonPopupMenuId ));
        rRootMenuBarEntry.sURL = aBuf.makeStringAndClear();

        OUString aRootSubMenuName( aAddonRootMenuBarTreeNode + m_aPropUINames[ OFFSET_UITYPE_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.getLength() > 0 )
        {
            // Read optional context information and continue to read sub menu nodes
            aRootMenuBarNodePropValues[ OFFSET_UITYPE_CONTEXT ] >>= rRootMenuBarEntry.sContext;

            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aRootSubMenuNodeNames.getLength(); n++ )
                aRootSubMenuNodeNames[n] = OUString( aSubMenuRootNodeName + aRootSubMenuNodeNames[n] );
            ReadSubMenuEntries( aRootSubMenuNodeNames, rRootMenuBarEntry.aSubMenu );
            rRootMenuBarEntry.sName = aRootMenuBarNodeName.copy( aRootMenuBarNodeName.lastIndexOf( '/' )+1);
            bResult = sal_True;
        }
        else
        {
            // Not allowed to have a single menu item on a top-level menu bar!
            return sal_False;
        }
    }

    return bResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, AddonSubMenu& rSubMenuContainer )
{
    sal_uInt32 nCount = aSubMenuNodeNames.getLength();

    for ( sal_uInt32 n = 0; n < nCount; n++ )
        ReadSubMenuEntry( aSubMenuNodeNames[n], rSubMenuContainer );

    return sal_True;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadSubMenuEntry( const OUString& aMenuEntryNodeName, AddonSubMenu& rSubMenuContainer )
{
    sal_Bool            bResult = sal_False;
    OUString            aAddonMenuTreeNode( aMenuEntryNodeName + m_aPathDelimiter );
    Sequence< Any >     aMenuNodePropValues;
    AddonMenuEntry  aMenuEntry;

    aMenuNodePropValues = GetProperties( GetPropertyNames( aAddonMenuTreeNode ) );
    if ( aMenuNodePropValues[ OFFSET_TITLE ] >>= aMenuEntry.sTitle )
    {
        OUString aSubMenuNodeName( aAddonMenuTreeNode + m_aPropNames[ OFFSET_SUBMENU ] );
        Sequence< OUString > aSubMenuNodeNames = GetNodeNames( aSubMenuNodeName );
        if ( aSubMenuNodeNames.getLength() > 0 )
        {
            // A popup menu only needs a title, ignore other properties and continue to read sub menu nodes
            OUString aSubMenuRootNodeName( aSubMenuNodeName + m_aPathDelimiter );
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aSubMenuNodeNames.getLength(); n++ )
                aSubMenuNodeNames[n] = OUString( aSubMenuRootNodeName + aSubMenuNodeNames[n] );
            ReadSubMenuEntries( aSubMenuNodeNames, aMenuEntry.aSubMenu );
            rSubMenuContainer.push_back( aMenuEntry );
        }
        else
        {
            // A simple menu item => read the other properties;
            if ( aMenuNodePropValues[ OFFSET_URL ] >>= aMenuEntry.sURL ) // Menu item must have a command URL
            {
                if ( aMenuEntry.sURL.equals( m_aSeparator ))
                    rSubMenuContainer.push_back( aMenuEntry ); // Separator
                else
                {
                    aMenuNodePropValues[ OFFSET_TARGET          ] >>= aMenuEntry.sTarget;
                    aMenuNodePropValues[ OFFSET_IMAGEIDENTIFIER ] >>= aMenuEntry.sImageIdentifier;
                    rSubMenuContainer.push_back( aMenuEntry );

                    // Store bitmap data defined for this menu item into the internal image manager
                    ImageEntry* pImageEntry = ReadOptionalImageData( aMenuEntryNodeName );
                    if ( pImageEntry )
                        AddImageEntryToImageManager( aMenuEntry.sURL, pImageEntry );
                }

                bResult = sal_True;
            }
        }
    }
    else if ( aMenuNodePropValues[ OFFSET_URL ] >>= aMenuEntry.sURL )
    {
        aMenuEntry.sURL = m_aSeparator;
        rSubMenuContainer.push_back( aMenuEntry );
        bResult = sal_True;
    }

    return bResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
AddonsOptions_Impl::ImageEntry* AddonsOptions_Impl::ReadOptionalImageData( const OUString& aMenuNodeName )
{
    OUStringBuffer aBuffer( aMenuNodeName );
    aBuffer.append( m_aPathDelimiter );
    aBuffer.append( IMAGES_NODENAME );
    aBuffer.append( m_aPathDelimiter );

    OUString aImageNodeName = aBuffer.makeStringAndClear();
    Sequence< OUString > aImageDataNodeNames = GetImagesPropertyNames( aImageNodeName );
    Sequence< Any >      aPropertyData;
    Sequence< sal_Int8 > aImageDataSeq;

    Image       aImage;
    ImageEntry* pEntry = NULL;

    aPropertyData = GetProperties( aImageDataNodeNames );
    for ( int i = 0; i < PROPERTYIMAGES_COUNT; i++ )
    {
        if (( aPropertyData[i] >>= aImageDataSeq ) &&
            ( CreateImageFromSequence( aImage, (( i == OFFSET_IMAGEBIG ) || ( i == OFFSET_IMAGEBIGHC )), aImageDataSeq )) )
        {
            if ( !pEntry )
                pEntry = new ImageEntry;

            if ( i == OFFSET_IMAGESMALL )
                pEntry->aImageSmall = aImage;
            else if ( i == OFFSET_IMAGEBIG )
                pEntry->aImageBig = aImage;
            else if ( i == OFFSET_IMAGESMALLHC )
                pEntry->aImageSmallHC = aImage;
            else
                pEntry->aImageBigHC = aImage;
        }
    }

    return pEntry;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::CreateImageFromSequence( Image& rImage, sal_Bool bBig, Sequence< sal_Int8 >& rBitmapDataSeq ) const
{
    sal_Bool    bResult = sal_False;
    Color       aTransparentColor( COL_LIGHTMAGENTA );
    Size        aSize = bBig ? Size( 26, 26 ) : Size( 16, 16 ); // Sizes used for menu/toolbox images

    if ( rBitmapDataSeq.getLength() > 0 )
    {
        SvMemoryStream aMemStream( rBitmapDataSeq.getArray(), rBitmapDataSeq.getLength(), STREAM_STD_READ );
        Bitmap aBitmap;
        aBitmap.Read( aMemStream );

        // Scale bitmap to fit the correct size for the menu/toolbar. Use best quality
        if ( aBitmap.GetSizePixel() != aSize )
            aBitmap.Scale( aSize, BMP_SCALE_INTERPOLATE );

        rImage = Image( aBitmap, aTransparentColor );
        bResult = sal_True;
    }

    return bResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void AddonsOptions_Impl::AddImageEntryToImageManager( const OUString& aURL, ImageEntry* pImageEntry )
{
    // Attention: ImageEntry pointer belongs now to the implementation!
    m_aImageManager.insert( ImageManager::value_type( aURL, *pImageEntry ));
    delete pImageEntry;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > AddonsOptions_Impl::GetPropertyNames( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( CFG_PROPERTYCOUNT );

    // Create property names dependent from the root node name
    lResult[0] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_URL             ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_TITLE           ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_TARGET          ] );
    lResult[3] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_IMAGEIDENTIFIER ] );

    return lResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > AddonsOptions_Impl::GetUITypePropertyNames( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( CFG_UITYPE_PROPERTYCOUNT );

    // Create property names dependent from the root node name
    lResult[0] = OUString( aPropertyRootNode + m_aPropUINames[ OFFSET_UITYPE_TITLE              ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropUINames[ OFFSET_UITYPE_TARGET             ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropUINames[ OFFSET_UITYPE_IMAGEIDENTIFIER    ] );
    lResult[3] = OUString( aPropertyRootNode + m_aPropUINames[ OFFSET_UITYPE_CONTEXT            ] );
    lResult[4] = OUString( aPropertyRootNode + m_aPropUINames[ OFFSET_UITYPE_SUBMENU            ] );

    return lResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > AddonsOptions_Impl::GetImagesPropertyNames( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYIMAGES_COUNT );

    // Create property names dependent from the root node name
    lResult[0] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGESMALL    ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGEBIG      ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGESMALLHC  ] );
    lResult[3] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGEBIGHC    ] );

    return lResult;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
AddonsOptions_Impl*     AddonsOptions::m_pDataContainer = NULL  ;
sal_Int32               AddonsOptions::m_nRefCount      = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
AddonsOptions::AddonsOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new AddonsOptions_Impl;
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
AddonsOptions::~AddonsOptions()
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
sal_Bool AddonsOptions::HasAddonsMenu   () const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->HasAddonsMenu();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsMenu();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Sequence< PropertyValue > AddonsOptions::GetAddonsComponentMenu( const ::rtl::OUString aComponentID ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsComponentMenu( aComponentID );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsMenuBarPart() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsMenuBarPart();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsToolBarPart() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsToolBarPart();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
Image AddonsOptions::GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetImageFromURL( aURL, bBig, bHiContrast );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& AddonsOptions::GetOwnStaticMutex()
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

/*************************************************************************
 *
 *  $RCSfile: addonsoptions.cxx,v $
 *
 *  $Revision: 1.3 $
 *  last change: $Author: hr $ $Date: 2003-04-04 17:13:34 $
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
#define TOOLBARITEMS                                    OUString(RTL_CONSTASCII_USTRINGPARAM("ToolBarItems"     ))
#define SEPARATOR_URL_STR                               "private:separator"
#define SEPARATOR_URL_LEN                               17
#define SEPARATOR_URL                                   OUString(RTL_CONSTASCII_USTRINGPARAM( SEPARATOR_URL_STR ))

#define PROPERTYNAME_URL                                ADDONSMENUITEM_PROPERTYNAME_URL
#define PROPERTYNAME_TITLE                              ADDONSMENUITEM_PROPERTYNAME_TITLE
#define PROPERTYNAME_TARGET                             ADDONSMENUITEM_PROPERTYNAME_TARGET
#define PROPERTYNAME_IMAGEIDENTIFIER                    ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER
#define PROPERTYNAME_CONTEXT                            ADDONSMENUITEM_PROPERTYNAME_CONTEXT
#define PROPERTYNAME_SUBMENU                            ADDONSMENUITEM_PROPERTYNAME_SUBMENU

#define PROPERTYNAME_IMAGESMALL                         OUString(RTL_CONSTASCII_USTRINGPARAM("ImageSmall" ))
#define PROPERTYNAME_IMAGEBIG                           OUString(RTL_CONSTASCII_USTRINGPARAM("ImageBig" ))
#define PROPERTYNAME_IMAGESMALLHC                       OUString(RTL_CONSTASCII_USTRINGPARAM("ImageSmallHC" ))
#define PROPERTYNAME_IMAGEBIGHC                         OUString(RTL_CONSTASCII_USTRINGPARAM("ImageBigHC" ))

#define IMAGES_NODENAME                                 OUString(RTL_CONSTASCII_USTRINGPARAM("UserDefinedImages" ))

#define PROPERTYCOUNT_MENUITEM                          6
#define OFFSET_MENUITEM_URL                             0
#define OFFSET_MENUITEM_TITLE                           1
#define OFFSET_MENUITEM_IMAGEIDENTIFIER                 2
#define OFFSET_MENUITEM_TARGET                          3
#define OFFSET_MENUITEM_SUBMENU                         4
#define OFFSET_MENUITEM_CONTEXT                         5

#define PROPERTYCOUNT_POPUPMENU                         4
#define OFFSET_POPUPMENU_TITLE                          0
#define OFFSET_POPUPMENU_CONTEXT                        1
#define OFFSET_POPUPMENU_SUBMENU                        2
#define OFFSET_POPUPMENU_URL                            3   // Used for property set

#define PROPERTYCOUNT_TOOLBARITEM                       5
#define OFFSET_TOOLBARITEM_URL                          0
#define OFFSET_TOOLBARITEM_TITLE                        1
#define OFFSET_TOOLBARITEM_IMAGEIDENTIFIER              2
#define OFFSET_TOOLBARITEM_TARGET                       3
#define OFFSET_TOOLBARITEM_CONTEXT                      4

#define PROPERTYCOUNT_IMAGES                            4
#define OFFSET_IMAGES_SMALL                             0
#define OFFSET_IMAGES_BIG                               1
#define OFFSET_IMAGES_SMALLHC                           2
#define OFFSET_IMAGES_BIGHC                             3

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

/*-****************************************************************************************************************
    @descr  struct to hold information about one menu entry.
****************************************************************************************************************-*/

namespace framework
{

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

        sal_Bool                                        HasAddonsMenu       () const ;
        sal_Bool                                        HasAddonsHelpMenu   () const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenu       () const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenuBarPart() const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsToolBarPart() const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsHelpMenu   () const ;
        Image                                           GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig, sal_Bool bHiContrast ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
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

        typedef std::hash_map< OUString, ImageEntry, OUStringHashCode, ::std::equal_to< OUString > > ImageManager;

        /*-****************************************************************************************************//**
            @short      return list of key names of our configuration management which represent oue module tree
            @descr      These methods return the current list of key names! We need it to get needed values from our
                        configuration management!

            @seealso    -

            @param      "nCount"     ,   returns count of menu entries for "new"
            @return     A list of configuration key names is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool             ReadAddonMenuSet( Sequence< Sequence< PropertyValue > >& aAddonMenuSeq );
        sal_Bool             ReadOfficeMenuBarSet( Sequence< Sequence< PropertyValue > >& aAddonOfficeMenuBarSeq );
        sal_Bool             ReadOfficeToolBarSet( Sequence< Sequence< PropertyValue > >& aAddonOfficeToolBarSeq );
        sal_Bool             ReadToolBarItemSet( const rtl::OUString rToolBarItemSetNodeName, Sequence< Sequence< PropertyValue > >& aAddonOfficeToolBarSeq );
        sal_Bool             ReadOfficeHelpSet( Sequence< Sequence< PropertyValue > >& aAddonOfficeHelpMenuSeq );
        sal_Bool             ReadImages( ImageManager& aImageManager );

        sal_Bool             ReadMenuItem( const OUString& aMenuItemNodeName, Sequence< PropertyValue >& aMenuItem, sal_Bool bIgnoreSubMenu = sal_False );
        sal_Bool             ReadPopupMenu( const OUString& aPopupMenuNodeName, Sequence< PropertyValue >& aPopupMenu );
        sal_Bool             ReadToolBarItem( const OUString& aToolBarItemNodeName, Sequence< PropertyValue >& aToolBarItem );
        sal_Bool             ReadImagesItem( const OUString& aImagesItemNodeName, Sequence< PropertyValue >& aImagesItem );
        ImageEntry*          ReadImageData( const OUString& aImagesNodeName );

        sal_Bool             ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, Sequence< Sequence< PropertyValue > >& rSubMenu );
        void                 InsertToolBarSeparator( Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq );
        OUString             GeneratePrefixURL();

        Sequence< OUString > GetPropertyNamesMenuItem( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesPopupMenu( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesToolBarItem( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesImages( const OUString& aPropertyRootNode ) const;
        sal_Bool             CreateImageFromSequence( Image& rImage, sal_Bool bBig, Sequence< sal_Int8 >& rBitmapDataSeq ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:
        ImageEntry* ReadOptionalImageData( const OUString& aMenuNodeName );
        void        AddImageEntryToImageManager( const OUString& aURL, ImageEntry* pImageEntry );

        sal_Int32                               m_nRootAddonPopupMenuId;
        OUString                                m_aPropNames[PROPERTYCOUNT_MENUITEM];
        OUString                                m_aPropImagesNames[PROPERTYCOUNT_IMAGES];
        OUString                                m_aEmpty;
        OUString                                m_aPathDelimiter;
        OUString                                m_aSeparator;
        OUString                                m_aRootAddonPopupMenuURLPrexfix;
        Sequence< Sequence< PropertyValue > >   m_aCachedMenuProperties;
        Sequence< Sequence< PropertyValue > >   m_aCachedMenuBarPartProperties;
        Sequence< Sequence< PropertyValue > >   m_aCachedToolBarPartProperties;
        Sequence< Sequence< PropertyValue > >   m_aCachedHelpMenuProperties;
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
    m_aPropNames[ OFFSET_MENUITEM_URL               ] = PROPERTYNAME_URL;
    m_aPropNames[ OFFSET_MENUITEM_TITLE             ] = PROPERTYNAME_TITLE;
    m_aPropNames[ OFFSET_MENUITEM_TARGET            ] = PROPERTYNAME_TARGET;
    m_aPropNames[ OFFSET_MENUITEM_IMAGEIDENTIFIER   ] = PROPERTYNAME_IMAGEIDENTIFIER;
    m_aPropNames[ OFFSET_MENUITEM_CONTEXT           ] = PROPERTYNAME_CONTEXT;
    m_aPropNames[ OFFSET_MENUITEM_SUBMENU           ] = PROPERTYNAME_SUBMENU;   // Submenu set!

    // initialize array with fixed images property names
    m_aPropImagesNames[ OFFSET_IMAGES_SMALL             ] = PROPERTYNAME_IMAGESMALL;
    m_aPropImagesNames[ OFFSET_IMAGES_BIG               ] = PROPERTYNAME_IMAGEBIG;
    m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC           ] = PROPERTYNAME_IMAGESMALLHC;
    m_aPropImagesNames[ OFFSET_IMAGES_BIGHC             ] = PROPERTYNAME_IMAGEBIGHC;

    ReadAddonMenuSet( m_aCachedMenuProperties );
    ReadOfficeMenuBarSet( m_aCachedMenuBarPartProperties );
    ReadOfficeToolBarSet( m_aCachedToolBarPartProperties );
    ReadOfficeHelpSet( m_aCachedHelpMenuProperties );
    ReadImages( m_aImageManager );

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
    return ( m_aCachedMenuProperties.getLength() > 0 );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::HasAddonsHelpMenu  () const
{
    return ( m_aCachedHelpMenuProperties.getLength() > 0 );
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
const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsHelpMenu  () const
{
    return m_aCachedHelpMenuProperties;
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
sal_Bool AddonsOptions_Impl::ReadAddonMenuSet( Sequence< Sequence< PropertyValue > >& rAddonMenuSeq )
{
    // Read the AddonMenu set and fill property sequences
    OUString                aAddonMenuNodeName( RTL_CONSTASCII_USTRINGPARAM( "AddonUI/AddonMenu" ));
    Sequence< OUString >    aAddonMenuNodeSeq = GetNodeNames( aAddonMenuNodeName );
    OUString                aAddonMenuItemNode( aAddonMenuNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonMenuNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    // Init the property value sequence
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = m_aPropNames[ OFFSET_MENUITEM_URL              ];
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = m_aPropNames[ OFFSET_MENUITEM_TITLE            ];
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = m_aPropNames[ OFFSET_MENUITEM_TARGET           ];
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = m_aPropNames[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ];
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = m_aPropNames[ OFFSET_MENUITEM_CONTEXT          ];
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = m_aPropNames[ OFFSET_MENUITEM_SUBMENU          ];  // Submenu set!

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aRootMenuItemNode( aAddonMenuItemNode + aAddonMenuNodeSeq[n] );

        // Read the MenuItem
        if ( ReadMenuItem( aRootMenuItemNode, aMenuItem ) )
        {
            // Successfully read a menu item, append to our list
            sal_uInt32 nMenuItemCount = rAddonMenuSeq.getLength() + 1;
            rAddonMenuSeq.realloc( nMenuItemCount );
            rAddonMenuSeq[nIndex++] = aMenuItem;
        }
    }

    return ( rAddonMenuSeq.getLength() > 0 );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadOfficeHelpSet( Sequence< Sequence< PropertyValue > >& rAddonOfficeHelpMenuSeq )
{
    // Read the AddonMenu set and fill property sequences
    OUString                aAddonHelpMenuNodeName( RTL_CONSTASCII_USTRINGPARAM( "AddonUI/OfficeHelp" ));
    Sequence< OUString >    aAddonHelpMenuNodeSeq = GetNodeNames( aAddonHelpMenuNodeName );
    OUString                aAddonHelpMenuItemNode( aAddonHelpMenuNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonHelpMenuNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    // Init the property value sequence
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = m_aPropNames[ OFFSET_MENUITEM_URL              ];
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = m_aPropNames[ OFFSET_MENUITEM_TITLE            ];
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = m_aPropNames[ OFFSET_MENUITEM_TARGET           ];
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = m_aPropNames[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ];
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = m_aPropNames[ OFFSET_MENUITEM_CONTEXT          ];
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = m_aPropNames[ OFFSET_MENUITEM_SUBMENU          ];  // Submenu set!

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aRootMenuItemNode( aAddonHelpMenuItemNode + aAddonHelpMenuNodeSeq[n] );

        // Read the MenuItem
        if ( ReadMenuItem( aRootMenuItemNode, aMenuItem, sal_True ) )
        {
            // Successfully read a menu item, append to our list
            sal_uInt32 nMenuItemCount = rAddonOfficeHelpMenuSeq.getLength() + 1;
            rAddonOfficeHelpMenuSeq.realloc( nMenuItemCount );
            rAddonOfficeHelpMenuSeq[nIndex++] = aMenuItem;
        }
    }

    return ( rAddonOfficeHelpMenuSeq.getLength() > 0 );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadOfficeMenuBarSet( Sequence< Sequence< PropertyValue > >& rAddonOfficeMenuBarSeq )
{
    // Read the OfficeMenuBar set and fill property sequences
    OUString                aAddonMenuBarNodeName( RTL_CONSTASCII_USTRINGPARAM( "AddonUI/OfficeMenuBar" ));
    Sequence< OUString >    aAddonMenuBarNodeSeq = GetNodeNames( aAddonMenuBarNodeName );
    OUString                aAddonMenuBarNode( aAddonMenuBarNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonMenuBarNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aPopupMenu( PROPERTYCOUNT_POPUPMENU );

    // Init the property value sequence
    aPopupMenu[ OFFSET_POPUPMENU_TITLE      ].Name = m_aPropNames[ OFFSET_MENUITEM_TITLE    ];
    aPopupMenu[ OFFSET_POPUPMENU_CONTEXT    ].Name = m_aPropNames[ OFFSET_MENUITEM_CONTEXT  ];
    aPopupMenu[ OFFSET_POPUPMENU_SUBMENU    ].Name = m_aPropNames[ OFFSET_MENUITEM_SUBMENU  ];
    aPopupMenu[ OFFSET_POPUPMENU_URL        ].Name = m_aPropNames[ OFFSET_MENUITEM_URL      ];

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aPopupMenuNode( aAddonMenuBarNode + aAddonMenuBarNodeSeq[n] );

        // Read the MenuItem
        if ( ReadPopupMenu( aPopupMenuNode, aPopupMenu ) )
        {
            // Successfully read a popup menu, append to our list
            sal_uInt32 nMenuItemCount = rAddonOfficeMenuBarSeq.getLength() + 1;
            rAddonOfficeMenuBarSeq.realloc( nMenuItemCount );
            rAddonOfficeMenuBarSeq[nIndex++] = aPopupMenu;
        }
    }

    return ( rAddonOfficeMenuBarSeq.getLength() > 0 );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadOfficeToolBarSet( Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq )
{
    // Read the OfficeToolBar set and fill property sequences
    OUString                aAddonToolBarNodeName( RTL_CONSTASCII_USTRINGPARAM( "AddonUI/OfficeToolBar" ));
    Sequence< OUString >    aAddonToolBarNodeSeq = GetNodeNames( aAddonToolBarNodeName );
    OUString                aAddonToolBarNode( aAddonToolBarNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonToolBarNodeSeq.getLength();
    sal_uInt32              nIndex = 0;

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aToolBarItemNode( aAddonToolBarNode + aAddonToolBarNodeSeq[n] );
        ReadToolBarItemSet( aToolBarItemNode, rAddonOfficeToolBarSeq );
    }

    return ( rAddonOfficeToolBarSeq.getLength() > 0 );
}


//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadToolBarItemSet( const rtl::OUString rToolBarItemSetNodeName, Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq )
{
    sal_Bool                    bInsertSeparator        = sal_False;
    sal_uInt32                  nToolBarItemCount       = rAddonOfficeToolBarSeq.getLength();
    OUString                    aAddonToolBarItemSetNode( rToolBarItemSetNodeName + m_aPathDelimiter );
    Sequence< OUString >        aAddonToolBarItemSetNodeSeq = GetNodeNames( rToolBarItemSetNodeName );
    Sequence< PropertyValue >   aToolBarItem( PROPERTYCOUNT_TOOLBARITEM );

    // Separate different toolbar item sets automatically with a separator
    if ( nToolBarItemCount > 0 )
    {
        OUString aURL;
        Sequence< PropertyValue > aPredToolBarItemSeq = rAddonOfficeToolBarSeq[nToolBarItemCount-1];
        if (( aPredToolBarItemSeq[OFFSET_TOOLBARITEM_URL].Value >>= aURL ) && !( aURL.equals( SEPARATOR_URL )))
            bInsertSeparator = sal_True;
    }
    else
        bInsertSeparator = sal_True;

    // Init the property value sequence
    aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Name = m_aPropNames[ OFFSET_MENUITEM_URL              ];
    aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Name = m_aPropNames[ OFFSET_MENUITEM_TITLE            ];
    aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Name = m_aPropNames[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ];
    aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Name = m_aPropNames[ OFFSET_MENUITEM_TARGET           ];
    aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Name = m_aPropNames[ OFFSET_MENUITEM_CONTEXT          ];

    sal_uInt32 nCount = aAddonToolBarItemSetNodeSeq.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aToolBarItemNode( aAddonToolBarItemSetNode + aAddonToolBarItemSetNodeSeq[n] );

        // Read the ToolBarItem
        if ( ReadToolBarItem( aToolBarItemNode, aToolBarItem ) )
        {
            if ( bInsertSeparator )
            {
                bInsertSeparator = sal_False;
                InsertToolBarSeparator( rAddonOfficeToolBarSeq );
            }

            // Successfully read a menu item, append to our list
            sal_uInt32 nCount = rAddonOfficeToolBarSeq.getLength();
            rAddonOfficeToolBarSeq.realloc( nCount+1 );
            rAddonOfficeToolBarSeq[nCount] = aToolBarItem;
        }
    }

    return ( (sal_uInt32)rAddonOfficeToolBarSeq.getLength() > nToolBarItemCount );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void AddonsOptions_Impl::InsertToolBarSeparator( Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq )
{
    Sequence< PropertyValue >   aToolBarItem( PROPERTYCOUNT_TOOLBARITEM );

    aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Name = m_aPropNames[ OFFSET_MENUITEM_URL              ];
    aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Name = m_aPropNames[ OFFSET_MENUITEM_TITLE            ];
    aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Name = m_aPropNames[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ];
    aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Name = m_aPropNames[ OFFSET_MENUITEM_TARGET           ];
    aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Name = m_aPropNames[ OFFSET_MENUITEM_CONTEXT          ];

    aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= SEPARATOR_URL;
    aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= m_aEmpty;
    aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value <<= m_aEmpty;
    aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= m_aEmpty;
    aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value <<= m_aEmpty;

    sal_uInt32 nToolBarItemCount = rAddonOfficeToolBarSeq.getLength();
    rAddonOfficeToolBarSeq.realloc( nToolBarItemCount+1 );
    rAddonOfficeToolBarSeq[nToolBarItemCount] = aToolBarItem;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadImages( ImageManager& aImageManager )
{
    // Read the user-defined Images set and fill image manager
    OUString                aAddonImagesNodeName( RTL_CONSTASCII_USTRINGPARAM( "AddonUI/Images" ));
    Sequence< OUString >    aAddonImagesNodeSeq = GetNodeNames( aAddonImagesNodeName );
    OUString                aAddonImagesNode( aAddonImagesNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonImagesNodeSeq.getLength();
    sal_uInt32              nIndex = 0;

    // Init the property value sequence
    Sequence< OUString >    aAddonImageItemNodePropNames( 1 );
    OUString                aURL;

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aImagesItemNode( aAddonImagesNode + aAddonImagesNodeSeq[n] );

        // Create sequence for data access
        OUStringBuffer aBuf( aImagesItemNode );
        aBuf.append( m_aPathDelimiter );
        aBuf.append( m_aPropNames[ OFFSET_MENUITEM_URL ] );
        aAddonImageItemNodePropNames[0] = aBuf.makeStringAndClear();

        Sequence< Any > aAddonImageItemNodeValues = GetProperties( aAddonImageItemNodePropNames );

        // An user-defined image entry must have an URL.
        if (( aAddonImageItemNodeValues[0] >>= aURL ) && aURL.getLength() > 0 )
        {
            OUStringBuffer aBuf( aImagesItemNode );
            aBuf.append( m_aPathDelimiter );
            aBuf.append( IMAGES_NODENAME );
            aBuf.append( m_aPathDelimiter );
            OUString aImagesUserDefinedItemNode = aBuf.makeStringAndClear();

            // Read a user-defined images data
            ImageEntry* pImageEntry = ReadImageData( aImagesUserDefinedItemNode );
            if ( pImageEntry )
            {
                // Successfully read a user-defined images item, put it into our image manager
                aImageManager.insert( ImageManager::value_type( aURL, *pImageEntry ));
                delete pImageEntry; // We have to ownership of the pointer
            }
        }
    }

    return sal_True;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************

OUString AddonsOptions_Impl::GeneratePrefixURL()
{
    // Create an unique prefixed Add-On popup menu URL so it can be identified later as a runtime popup menu.
    // They use a different image manager, so they must be identified by the sfx2/framework code.
    OUString aPopupMenuURL;
    OUStringBuffer aBuf( m_aRootAddonPopupMenuURLPrexfix.getLength() + 3 );
    aBuf.append( m_aRootAddonPopupMenuURLPrexfix );
    aBuf.append( OUString::valueOf( ++m_nRootAddonPopupMenuId ));
    aPopupMenuURL = aBuf.makeStringAndClear();
    return aPopupMenuURL;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadMenuItem( const OUString& aMenuNodeName, Sequence< PropertyValue >& aMenuItem, sal_Bool bIgnoreSubMenu )
{
    sal_Bool            bResult = sal_False;
    OUString            aStrValue;
    OUString            aAddonMenuItemTreeNode( aMenuNodeName + m_aPathDelimiter );
    Sequence< Any >     aMenuItemNodePropValues;

    aMenuItemNodePropValues = GetProperties( GetPropertyNamesMenuItem( aAddonMenuItemTreeNode ) );
    if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_TITLE ] >>= aStrValue ) && aStrValue.getLength() > 0 )
    {
        aMenuItem[ OFFSET_MENUITEM_TITLE ].Value <<= aStrValue;

        OUString aRootSubMenuName( aAddonMenuItemTreeNode + m_aPropNames[ OFFSET_MENUITEM_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.getLength() > 0 && !bIgnoreSubMenu )
        {
            // Set a unique prefixed Add-On popup menu URL so it can be identified later
            OUString aPopupMenuURL = GeneratePrefixURL();

            // A popup menu must have a title and can have a URL and ImageIdentifier
            // Set the other property values to empty
            aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aPopupMenuURL;
            aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= m_aEmpty;
            aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_IMAGEIDENTIFIER ];
            aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_CONTEXT ];

            // Continue to read the sub menu nodes
            Sequence< Sequence< PropertyValue > > aSubMenuSeq;
            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aRootSubMenuNodeNames.getLength(); n++ )
                aRootSubMenuNodeNames[n] = OUString( aSubMenuRootNodeName + aRootSubMenuNodeNames[n] );
            ReadSubMenuEntries( aRootSubMenuNodeNames, aSubMenuSeq );
            aMenuItem[ OFFSET_MENUITEM_SUBMENU ].Value <<= aSubMenuSeq;
            bResult = sal_True;
        }
        else if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_URL ] >>= aStrValue ) && aStrValue.getLength() > 0 )
        {
            // A simple menu item => read the other properties;
            OUString aString;

            aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aStrValue;
            aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_TARGET         ];
            aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_IMAGEIDENTIFIER];
            aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_CONTEXT        ];
            aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Value <<= Sequence< Sequence< PropertyValue > >(); // Submenu set!

            bResult = sal_True;
        }
    }
    else if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_URL ] >>= aStrValue ) &&
              aStrValue.equalsAsciiL( SEPARATOR_URL_STR, SEPARATOR_URL_LEN ))
    {
        // Separator
        aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aStrValue;
        aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= m_aEmpty;
        aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= m_aEmpty;
        aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value <<= m_aEmpty;
        aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Value <<= Sequence< Sequence< PropertyValue > >(); // Submenu set!
        bResult = sal_True;
    }

    return bResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadPopupMenu( const OUString& aPopupMenuNodeName, Sequence< PropertyValue >& aPopupMenu )
{
    sal_Bool            bResult = sal_False;
    OUString            aStrValue;
    OUString            aAddonPopupMenuTreeNode( aPopupMenuNodeName + m_aPathDelimiter );
    Sequence< Any >     aPopupMenuNodePropValues;

    aPopupMenuNodePropValues = GetProperties( GetPropertyNamesPopupMenu( aAddonPopupMenuTreeNode ) );
    if (( aPopupMenuNodePropValues[ OFFSET_POPUPMENU_TITLE ] >>= aStrValue ) &&
         aStrValue.getLength() > 0 )
    {
        aPopupMenu[ OFFSET_POPUPMENU_TITLE ].Value <<= aStrValue;

        OUString aRootSubMenuName( aAddonPopupMenuTreeNode + m_aPropNames[ OFFSET_MENUITEM_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.getLength() > 0 )
        {
            // A top-level popup menu needs a title
            // Set a unique prefixed Add-On popup menu URL so it can be identified later
            OUString aPopupMenuURL = GeneratePrefixURL();

            aPopupMenu[ OFFSET_POPUPMENU_URL        ].Value <<= aPopupMenuURL;
            aPopupMenu[ OFFSET_POPUPMENU_CONTEXT    ].Value <<= aPopupMenuNodePropValues[ OFFSET_POPUPMENU_CONTEXT ];

            // Continue to read the sub menu nodes
            Sequence< Sequence< PropertyValue > > aSubMenuSeq;
            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aRootSubMenuNodeNames.getLength(); n++ )
                aRootSubMenuNodeNames[n] = OUString( aSubMenuRootNodeName + aRootSubMenuNodeNames[n] );
            ReadSubMenuEntries( aRootSubMenuNodeNames, aSubMenuSeq );
            aPopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value <<= aSubMenuSeq;
            bResult = sal_True;
        }
    }

    return bResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadToolBarItem( const OUString& aToolBarItemNodeName, Sequence< PropertyValue >& aToolBarItem )
{
    sal_Bool            bResult = sal_False;
    OUString            aTitle;
    OUString            aURL;
    OUString            aAddonToolBarItemTreeNode( aToolBarItemNodeName + m_aPathDelimiter );
    Sequence< Any >     aToolBarItemNodePropValues;

    aToolBarItemNodePropValues = GetProperties( GetPropertyNamesToolBarItem( aAddonToolBarItemTreeNode ) );

    // A toolbar item must have a command URL
    if (( aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_URL ] >>= aURL ) && aURL.getLength() > 0 )
    {
        if ( aURL.equals( SEPARATOR_URL ))
        {
            // A speparator toolbar item only needs a URL
            aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= aURL;
            aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value <<= m_aEmpty;

            bResult = sal_True;
        }
        else if (( aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_TITLE ] >>= aTitle ) && aTitle.getLength() > 0 )
        {
            // A normal toolbar item must also have title => read the other properties;
            aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= aURL;
            aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= aTitle;
            aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value <<= aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_TARGET          ];
            aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER ];
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value <<= aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_CONTEXT         ];

            bResult = sal_True;
        }
    }

    return bResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
sal_Bool AddonsOptions_Impl::ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, Sequence< Sequence< PropertyValue > >& rSubMenuSeq )
{
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    // Init the property value sequence
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = PROPERTYNAME_URL;
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = PROPERTYNAME_TITLE;
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = PROPERTYNAME_TARGET;
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = PROPERTYNAME_IMAGEIDENTIFIER;
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = PROPERTYNAME_CONTEXT;
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = PROPERTYNAME_SUBMENU;  // Submenu set!

    sal_uInt32 nIndex = 0;
    sal_uInt32 nCount = aSubMenuNodeNames.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        if ( ReadMenuItem( aSubMenuNodeNames[n], aMenuItem ))
        {
            sal_uInt32 nSubMenuCount = rSubMenuSeq.getLength() + 1;
            rSubMenuSeq.realloc( nSubMenuCount );
            rSubMenuSeq[nIndex++] = aMenuItem;
        }
    }

    return sal_True;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
AddonsOptions_Impl::ImageEntry* AddonsOptions_Impl::ReadImageData( const OUString& aImagesNodeName )
{
    Sequence< OUString > aImageDataNodeNames = GetPropertyNamesImages( aImagesNodeName );
    Sequence< Any >      aPropertyData;
    Sequence< sal_Int8 > aImageDataSeq;

    Image       aImage;
    ImageEntry* pEntry = NULL;

    aPropertyData = GetProperties( aImageDataNodeNames );
    for ( int i = 0; i < PROPERTYCOUNT_IMAGES; i++ )
    {
        if (( aPropertyData[i] >>= aImageDataSeq ) &&
            ( CreateImageFromSequence( aImage,
                                      (( i == OFFSET_IMAGES_BIG ) ||
                                       ( i == OFFSET_IMAGES_BIGHC )),
                                      aImageDataSeq )) )
        {
            if ( !pEntry )
                pEntry = new ImageEntry;

            if ( i == OFFSET_IMAGES_SMALL )
                pEntry->aImageSmall = aImage;
            else if ( i == OFFSET_IMAGES_BIG )
                pEntry->aImageBig = aImage;
            else if ( i == OFFSET_IMAGES_SMALLHC )
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
Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesMenuItem( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_MENUITEM );

    // Create property names dependent from the root node name
    lResult[0] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_URL            ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_TITLE          ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_IMAGEIDENTIFIER] );
    lResult[3] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_TARGET         ] );
    lResult[4] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_SUBMENU        ] );
    lResult[5] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_CONTEXT        ] );

    return lResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesPopupMenu( const OUString& aPropertyRootNode ) const
{
    // The URL is automatically set and not read from the configuration.
    Sequence< OUString > lResult( PROPERTYCOUNT_POPUPMENU-1 );

    // Create property names dependent from the root node name
    lResult[0] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_TITLE      ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_CONTEXT    ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_SUBMENU    ] );

    return lResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesToolBarItem( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_TOOLBARITEM );

    // Create property names dependent from the root node name
    lResult[0] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_URL            ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_TITLE          ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_IMAGEIDENTIFIER] );
    lResult[3] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_TARGET         ] );
    lResult[4] = OUString( aPropertyRootNode + m_aPropNames[ OFFSET_MENUITEM_CONTEXT        ] );

    return lResult;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesImages( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_IMAGES );

    // Create property names dependent from the root node name
    lResult[0] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_SMALL      ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_BIG        ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC    ] );
    lResult[3] = OUString( aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_BIGHC      ] );

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
sal_Bool AddonsOptions::HasAddonsMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->HasAddonsMenu();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************

sal_Bool AddonsOptions::HasAddonsHelpMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->HasAddonsHelpMenu();
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
const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsHelpMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsHelpMenu();
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

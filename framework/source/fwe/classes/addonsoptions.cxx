/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <framework/addonsoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include "com/sun/star/util/theMacroExpander.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/toolbox.hxx>

#include <boost/unordered_map.hpp>
#include <algorithm>
#include <vector>





using namespace ::std                   ;
using namespace ::utl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;
using namespace ::com::sun::star::beans ;
using namespace ::com::sun::star::lang  ;
using namespace ::com::sun::star;

#define ROOTNODE_ADDONMENU                              OUString("Office.Addons" )
#define PATHDELIMITER                                   OUString("/"             )
#define SEPARATOR_URL_STR                               "private:separator"
#define SEPARATOR_URL                                   OUString( SEPARATOR_URL_STR )

#define PROPERTYNAME_URL                                ADDONSMENUITEM_PROPERTYNAME_URL
#define PROPERTYNAME_TITLE                              ADDONSMENUITEM_PROPERTYNAME_TITLE
#define PROPERTYNAME_TARGET                             ADDONSMENUITEM_PROPERTYNAME_TARGET
#define PROPERTYNAME_IMAGEIDENTIFIER                    ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER
#define PROPERTYNAME_CONTEXT                            ADDONSMENUITEM_PROPERTYNAME_CONTEXT
#define PROPERTYNAME_SUBMENU                            ADDONSMENUITEM_PROPERTYNAME_SUBMENU
#define PROPERTYNAME_CONTROLTYPE                        ADDONSMENUITEM_PROPERTYNAME_CONTROLTYPE
#define PROPERTYNAME_WIDTH                              ADDONSMENUITEM_PROPERTYNAME_WIDTH

#define PROPERTYNAME_ALIGN                              STATUSBARITEM_PROPERTYNAME_ALIGN
#define PROPERTYNAME_AUTOSIZE                           STATUSBARITEM_PROPERTYNAME_AUTOSIZE
#define PROPERTYNAME_OWNERDRAW                          STATUSBARITEM_PROPERTYNAME_OWNERDRAW

#define PROPERTYNAME_IMAGESMALL                         OUString("ImageSmall" )
#define PROPERTYNAME_IMAGEBIG                           OUString("ImageBig" )
#define PROPERTYNAME_IMAGESMALLHC                       OUString("ImageSmallHC" )
#define PROPERTYNAME_IMAGEBIGHC                         OUString("ImageBigHC" )
#define PROPERTYNAME_IMAGESMALL_URL                     OUString("ImageSmallURL" )
#define PROPERTYNAME_IMAGEBIG_URL                       OUString("ImageBigURL" )
#define PROPERTYNAME_IMAGESMALLHC_URL                   OUString("ImageSmallHCURL" )
#define PROPERTYNAME_IMAGEBIGHC_URL                     OUString("ImageBigHCURL" )

#define IMAGES_NODENAME                                 OUString("UserDefinedImages" )
#define PRIVATE_IMAGE_URL                               OUString("private:image/" )

#define PROPERTYNAME_MERGEMENU_MERGEPOINT               OUString("MergePoint" )
#define PROPERTYNAME_MERGEMENU_MERGECOMMAND             OUString("MergeCommand" )
#define PROPERTYNAME_MERGEMENU_MERGECOMMANDPARAMETER    OUString("MergeCommandParameter" )
#define PROPERTYNAME_MERGEMENU_MERGEFALLBACK            OUString("MergeFallback" )
#define PROPERTYNAME_MERGEMENU_MERGECONTEXT             OUString("MergeContext" )
#define PROPERTYNAME_MERGEMENU_MENUITEMS                OUString("MenuItems" )

#define PROPERTYNAME_MERGETOOLBAR_TOOLBAR               OUString("MergeToolBar" )
#define PROPERTYNAME_MERGETOOLBAR_MERGEPOINT            OUString("MergePoint" )
#define PROPERTYNAME_MERGETOOLBAR_MERGECOMMAND          OUString("MergeCommand" )
#define PROPERTYNAME_MERGETOOLBAR_MERGECOMMANDPARAMETER OUString("MergeCommandParameter" )
#define PROPERTYNAME_MERGETOOLBAR_MERGEFALLBACK         OUString("MergeFallback" )
#define PROPERTYNAME_MERGETOOLBAR_MERGECONTEXT          OUString("MergeContext" )
#define PROPERTYNAME_MERGETOOLBAR_TOOLBARITEMS          OUString("ToolBarItems" )

#define PROPERTYNAME_MERGESTATUSBAR_MERGEPOINT               OUString("MergePoint")
#define PROPERTYNAME_MERGESTATUSBAR_MERGECOMMAND             OUString("MergeCommand")
#define PROPERTYNAME_MERGESTATUSBAR_MERGECOMMANDPARAMETER    OUString("MergeCommandParameter")
#define PROPERTYNAME_MERGESTATUSBAR_MERGEFALLBACK            OUString("MergeFallback")
#define PROPERTYNAME_MERGESTATUSBAR_MERGECONTEXT             OUString("MergeContext")
#define PROPERTYNAME_MERGESTATUSBAR_STATUSBARITEMS           OUString("StatusBarItems")


#define INDEX_URL               0
#define INDEX_TITLE             1
#define INDEX_IMAGEIDENTIFIER   2
#define INDEX_TARGET            3
#define INDEX_CONTEXT           4
#define INDEX_SUBMENU           5
#define INDEX_CONTROLTYPE       6
#define INDEX_WIDTH             7
#define INDEX_ALIGN             8
#define INDEX_AUTOSIZE          9
#define INDEX_OWNERDRAW         10
#define PROPERTYCOUNT_INDEX     11


#define PROPERTYCOUNT_MENUITEM                          6
#define OFFSET_MENUITEM_URL                             0
#define OFFSET_MENUITEM_TITLE                           1
#define OFFSET_MENUITEM_IMAGEIDENTIFIER                 2
#define OFFSET_MENUITEM_TARGET                          3
#define OFFSET_MENUITEM_CONTEXT                         4
#define OFFSET_MENUITEM_SUBMENU                         5


#define PROPERTYCOUNT_POPUPMENU                         4
#define OFFSET_POPUPMENU_TITLE                          0
#define OFFSET_POPUPMENU_CONTEXT                        1
#define OFFSET_POPUPMENU_SUBMENU                        2
#define OFFSET_POPUPMENU_URL                            3   


#define PROPERTYCOUNT_TOOLBARITEM                       7
#define OFFSET_TOOLBARITEM_URL                          0
#define OFFSET_TOOLBARITEM_TITLE                        1
#define OFFSET_TOOLBARITEM_IMAGEIDENTIFIER              2
#define OFFSET_TOOLBARITEM_TARGET                       3
#define OFFSET_TOOLBARITEM_CONTEXT                      4
#define OFFSET_TOOLBARITEM_CONTROLTYPE                  5
#define OFFSET_TOOLBARITEM_WIDTH                        6


#define PROPERTYCOUNT_STATUSBARITEM                     7
#define OFFSET_STATUSBARITEM_URL                        0
#define OFFSET_STATUSBARITEM_TITLE                      1
#define OFFSET_STATUSBARITEM_CONTEXT                    2
#define OFFSET_STATUSBARITEM_ALIGN                      3
#define OFFSET_STATUSBARITEM_AUTOSIZE                   4
#define OFFSET_STATUSBARITEM_OWNERDRAW                  5
#define OFFSET_STATUSBARITEM_WIDTH                      6



#define PROPERTYCOUNT_IMAGES                            8
#define PROPERTYCOUNT_EMBEDDED_IMAGES                   4
#define OFFSET_IMAGES_SMALL                             0
#define OFFSET_IMAGES_BIG                               1
#define OFFSET_IMAGES_SMALLHC                           2
#define OFFSET_IMAGES_BIGHC                             3
#define OFFSET_IMAGES_SMALL_URL                         4
#define OFFSET_IMAGES_BIG_URL                           5
#define OFFSET_IMAGES_SMALLHC_URL                       6
#define OFFSET_IMAGES_BIGHC_URL                         7

#define PROPERTYCOUNT_MERGE_MENUBAR                     6
#define OFFSET_MERGEMENU_MERGEPOINT                     0
#define OFFSET_MERGEMENU_MERGECOMMAND                   1
#define OFFSET_MERGEMENU_MERGECOMMANDPARAMETER          2
#define OFFSET_MERGEMENU_MERGEFALLBACK                  3
#define OFFSET_MERGEMENU_MERGECONTEXT                   4
#define OFFSET_MERGEMENU_MENUITEMS                      5

#define PROPERTYCOUNT_MERGE_TOOLBAR                     7
#define OFFSET_MERGETOOLBAR_TOOLBAR                     0
#define OFFSET_MERGETOOLBAR_MERGEPOINT                  1
#define OFFSET_MERGETOOLBAR_MERGECOMMAND                2
#define OFFSET_MERGETOOLBAR_MERGECOMMANDPARAMETER       3
#define OFFSET_MERGETOOLBAR_MERGEFALLBACK               4
#define OFFSET_MERGETOOLBAR_MERGECONTEXT                5
#define OFFSET_MERGETOOLBAR_TOOLBARITEMS                6

#define PROPERTYCOUNT_MERGE_STATUSBAR                   6
#define OFFSET_MERGESTATUSBAR_MERGEPOINT                0
#define OFFSET_MERGESTATUSBAR_MERGECOMMAND              1
#define OFFSET_MERGESTATUSBAR_MERGECOMMANDPARAMETER     2
#define OFFSET_MERGESTATUSBAR_MERGEFALLBACK             3
#define OFFSET_MERGESTATUSBAR_MERGECONTEXT              4
#define OFFSET_MERGESTATUSBAR_STATUSBARITEMS            5

#define EXPAND_PROTOCOL                                 "vnd.sun.star.expand:"





/*-****************************************************************************************************************
    @descr  struct to hold information about one menu entry.
****************************************************************************************************************-*/

namespace framework
{

class AddonsOptions_Impl : public ConfigItem
{
    
    
    

    public:
        
        
        

         AddonsOptions_Impl();
        ~AddonsOptions_Impl();

        
        
        

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *

        virtual void Notify( const Sequence< OUString >& lPropertyNames );

        /*-****************************************************************************************************
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *

        virtual void Commit();

        
        
        

        /*-****************************************************************************************************
            @short      base implementation of public interface for "SvtDynamicMenuOptions"!
            @descr      These class is used as static member of "SvtDynamicMenuOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *

        sal_Bool                                        HasAddonsMenu        () const ;
        sal_Int32                                       GetAddonsToolBarCount() const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenu        () const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenuBarPart () const ;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsToolBarPart ( sal_uInt32 nIndex ) const ;
        const OUString                           GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsHelpMenu    () const ;
        Image                                           GetImageFromURL( const OUString& aURL, sal_Bool bBig, sal_Bool bNoScale );
        const MergeMenuInstructionContainer&            GetMergeMenuInstructions() const;
        bool                                            GetMergeToolbarInstructions( const OUString& rToolbarName, MergeToolbarInstructionContainer& rToolbarInstructions ) const;
        const MergeStatusbarInstructionContainer&       GetMergeStatusbarInstructions() const;
        void                                            ReadConfigurationData();

    
    
    

    private:
        enum ImageSize
        {
            IMGSIZE_SMALL,
            IMGSIZE_BIG
        };

        struct ImageEntry
        {
            
            

            
            Image    aScaled[2];       
            Image    aImage[2];        
            OUString aURL[2];         
            ImageEntry() {}
            void addImage(ImageSize eSize, const Image &rImage, const OUString &rURL);
        };

        typedef boost::unordered_map< OUString, ImageEntry, OUStringHash, ::std::equal_to< OUString > > ImageManager;
        typedef boost::unordered_map< OUString, sal_uInt32, OUStringHash, ::std::equal_to< OUString > > StringToIndexMap;
        typedef std::vector< Sequence< Sequence< PropertyValue > > > AddonToolBars;
        typedef ::boost::unordered_map< OUString, MergeToolbarInstructionContainer, OUStringHash, ::std::equal_to< OUString > > ToolbarMergingInstructions;


        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent oue module tree
            @descr      These methods return the current list of key names! We need it to get needed values from our
                        configuration management!

            @seealso    -

            @param      "nCount"     ,   returns count of menu entries for "new"
            @return     A list of configuration key names is returned.

            @onerror    -
        *

        sal_Bool             ReadAddonMenuSet( Sequence< Sequence< PropertyValue > >& aAddonMenuSeq );
        sal_Bool             ReadOfficeMenuBarSet( Sequence< Sequence< PropertyValue > >& aAddonOfficeMenuBarSeq );
        sal_Bool             ReadOfficeToolBarSet( AddonToolBars& rAddonOfficeToolBars, std::vector< OUString >& rAddonOfficeToolBarResNames );
        sal_Bool             ReadToolBarItemSet( const OUString rToolBarItemSetNodeName, Sequence< Sequence< PropertyValue > >& aAddonOfficeToolBarSeq );
        sal_Bool             ReadOfficeHelpSet( Sequence< Sequence< PropertyValue > >& aAddonOfficeHelpMenuSeq );
        void                 ReadImages( ImageManager& aImageManager );
        sal_Bool             ReadMenuMergeInstructions( MergeMenuInstructionContainer& rContainer );
        sal_Bool             ReadToolbarMergeInstructions( ToolbarMergingInstructions& rToolbarMergeMap );
        sal_Bool             ReadStatusbarMergeInstructions( MergeStatusbarInstructionContainer& rContainer );

        sal_Bool             ReadMergeMenuData( const OUString& aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeMenu );
        sal_Bool             ReadMergeToolbarData( const OUString& aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeToolbarItems );
        sal_Bool             ReadMergeStatusbarData( const OUString& aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeStatusbar );
        sal_Bool             ReadMenuItem( const OUString& aMenuItemNodeName, Sequence< PropertyValue >& aMenuItem, sal_Bool bIgnoreSubMenu = sal_False );
        sal_Bool             ReadPopupMenu( const OUString& aPopupMenuNodeName, Sequence< PropertyValue >& aPopupMenu );
        sal_Bool             AppendPopupMenu( Sequence< PropertyValue >& aTargetPopupMenu, const Sequence< PropertyValue >& rSourcePopupMenu );
        sal_Bool             ReadToolBarItem( const OUString& aToolBarItemNodeName, Sequence< PropertyValue >& aToolBarItem );
        sal_Bool             ReadStatusBarItem( const OUString& aStatusbarItemNodeName, Sequence< PropertyValue >& aStatusbarItem );
        sal_Bool             ReadImagesItem( const OUString& aImagesItemNodeName, Sequence< PropertyValue >& aImagesItem );
        ImageEntry*          ReadImageData( const OUString& aImagesNodeName );
        void                 ReadAndAssociateImages( const OUString& aURL, const OUString& aImageId );
        Image                ReadImageFromURL( const OUString& aURL );
        sal_Bool             HasAssociatedImages( const OUString& aURL );
        void                 SubstituteVariables( OUString& aURL );

        sal_Bool             ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, Sequence< Sequence< PropertyValue > >& rSubMenu );
        void                 InsertToolBarSeparator( Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq );
        OUString          GeneratePrefixURL();

        Sequence< OUString > GetPropertyNamesMenuItem( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesPopupMenu( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesToolBarItem( const OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesStatusbarItem( const ::rtl::OUString& aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesImages( const OUString& aPropertyRootNode ) const;
        sal_Bool             CreateImageFromSequence( Image& rImage, Sequence< sal_Int8 >& rBitmapDataSeq ) const;

    
    
    

    private:
        ImageEntry* ReadOptionalImageData( const OUString& aMenuNodeName );

        sal_Int32                                         m_nRootAddonPopupMenuId;
        OUString                                   m_aPropNames[PROPERTYCOUNT_INDEX];
        OUString                                   m_aPropImagesNames[PROPERTYCOUNT_IMAGES];
        OUString                                   m_aPropMergeMenuNames[PROPERTYCOUNT_MERGE_MENUBAR];
        OUString                                   m_aPropMergeToolbarNames[PROPERTYCOUNT_MERGE_TOOLBAR];
        OUString                                   m_aPropMergeStatusbarNames[PROPERTYCOUNT_MERGE_STATUSBAR];
        OUString                                   m_aEmpty;
        OUString                                   m_aPathDelimiter;
        OUString                                   m_aSeparator;
        OUString                                   m_aRootAddonPopupMenuURLPrexfix;
        OUString                                   m_aPrivateImageURL;
        Sequence< Sequence< PropertyValue > >             m_aCachedMenuProperties;
        Sequence< Sequence< PropertyValue > >             m_aCachedMenuBarPartProperties;
        AddonToolBars                                     m_aCachedToolBarPartProperties;
        std::vector< OUString >                      m_aCachedToolBarPartResourceNames;
        Sequence< Sequence< PropertyValue > >             m_aCachedHelpMenuProperties;
        Reference< util::XMacroExpander >                 m_xMacroExpander;
        ImageManager                                      m_aImageManager;
        Sequence< Sequence< PropertyValue > >             m_aEmptyAddonToolBar;
        MergeMenuInstructionContainer                     m_aCachedMergeMenuInsContainer;
        ToolbarMergingInstructions                        m_aCachedToolbarMergingInstructions;
        MergeStatusbarInstructionContainer                m_aCachedStatusbarMergingInstructions;
};

void AddonsOptions_Impl::ImageEntry::addImage(ImageSize eSize,
                                              const Image &rImage,
                                              const OUString &rURL)
{
    aImage[(int)eSize] = rImage;
    aURL[(int)eSize] = rURL;
}




AddonsOptions_Impl::AddonsOptions_Impl()
    
    : ConfigItem( ROOTNODE_ADDONMENU ),
    m_nRootAddonPopupMenuId( 0 ),
    m_aPathDelimiter( PATHDELIMITER ),
    m_aSeparator( SEPARATOR_URL ),
    m_aRootAddonPopupMenuURLPrexfix( ADDONSPOPUPMENU_URL_PREFIX ),
    m_aPrivateImageURL( PRIVATE_IMAGE_URL )
{
    
    m_aPropNames[ INDEX_URL             ] = PROPERTYNAME_URL;
    m_aPropNames[ INDEX_TITLE           ] = PROPERTYNAME_TITLE;
    m_aPropNames[ INDEX_TARGET          ] = PROPERTYNAME_TARGET;
    m_aPropNames[ INDEX_IMAGEIDENTIFIER ] = PROPERTYNAME_IMAGEIDENTIFIER;
    m_aPropNames[ INDEX_CONTEXT         ] = PROPERTYNAME_CONTEXT;
    m_aPropNames[ INDEX_SUBMENU         ] = PROPERTYNAME_SUBMENU; 
    m_aPropNames[ INDEX_CONTROLTYPE     ] = PROPERTYNAME_CONTROLTYPE;
    m_aPropNames[ INDEX_WIDTH           ] = PROPERTYNAME_WIDTH;
    m_aPropNames[ INDEX_ALIGN           ] = PROPERTYNAME_ALIGN;
    m_aPropNames[ INDEX_AUTOSIZE        ] = PROPERTYNAME_AUTOSIZE;
    m_aPropNames[ INDEX_OWNERDRAW       ] = PROPERTYNAME_OWNERDRAW;

    
    m_aPropImagesNames[ OFFSET_IMAGES_SMALL         ] = PROPERTYNAME_IMAGESMALL;
    m_aPropImagesNames[ OFFSET_IMAGES_BIG           ] = PROPERTYNAME_IMAGEBIG;
    m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC       ] = PROPERTYNAME_IMAGESMALLHC;
    m_aPropImagesNames[ OFFSET_IMAGES_BIGHC         ] = PROPERTYNAME_IMAGEBIGHC;
    m_aPropImagesNames[ OFFSET_IMAGES_SMALL_URL     ] = PROPERTYNAME_IMAGESMALL_URL;
    m_aPropImagesNames[ OFFSET_IMAGES_BIG_URL       ] = PROPERTYNAME_IMAGEBIG_URL;
    m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC_URL   ] = PROPERTYNAME_IMAGESMALLHC_URL;
    m_aPropImagesNames[ OFFSET_IMAGES_BIGHC_URL     ] = PROPERTYNAME_IMAGEBIGHC_URL;

    
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEPOINT    ] = PROPERTYNAME_MERGEMENU_MERGEPOINT;
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMAND  ] = PROPERTYNAME_MERGEMENU_MERGECOMMAND;
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMANDPARAMETER ] = PROPERTYNAME_MERGEMENU_MERGECOMMANDPARAMETER;
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEFALLBACK ] = PROPERTYNAME_MERGEMENU_MERGEFALLBACK;
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECONTEXT  ] = PROPERTYNAME_MERGEMENU_MERGECONTEXT;
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MENUITEMS     ] = PROPERTYNAME_MERGEMENU_MENUITEMS;

    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBAR               ] = PROPERTYNAME_MERGETOOLBAR_TOOLBAR;
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEPOINT            ] = PROPERTYNAME_MERGETOOLBAR_MERGEPOINT;
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMAND          ] = PROPERTYNAME_MERGETOOLBAR_MERGECOMMAND;
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMANDPARAMETER ] = PROPERTYNAME_MERGETOOLBAR_MERGECOMMANDPARAMETER;
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEFALLBACK         ] = PROPERTYNAME_MERGETOOLBAR_MERGEFALLBACK;
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECONTEXT          ] = PROPERTYNAME_MERGETOOLBAR_MERGECONTEXT;
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBARITEMS          ] = PROPERTYNAME_MERGETOOLBAR_TOOLBARITEMS;

    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGEPOINT            ] = PROPERTYNAME_MERGESTATUSBAR_MERGEPOINT;
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGECOMMAND          ] = PROPERTYNAME_MERGESTATUSBAR_MERGECOMMAND;
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGECOMMANDPARAMETER ] = PROPERTYNAME_MERGESTATUSBAR_MERGECOMMANDPARAMETER;
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGEFALLBACK         ] = PROPERTYNAME_MERGESTATUSBAR_MERGEFALLBACK;
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGECONTEXT          ] = PROPERTYNAME_MERGESTATUSBAR_MERGECONTEXT;
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_STATUSBARITEMS        ] = PROPERTYNAME_MERGESTATUSBAR_STATUSBARITEMS;

    Reference< XComponentContext > xContext(
        comphelper::getProcessComponentContext() );
    m_xMacroExpander =  util::theMacroExpander::get(xContext);

    ReadConfigurationData();

    
    
    Sequence< OUString > aNotifySeq( 1 );
    aNotifySeq[0] = "AddonUI";
    EnableNotification( aNotifySeq );
}




AddonsOptions_Impl::~AddonsOptions_Impl()
{
    
    if( IsModified() )
    {
        Commit();
    }
}

void AddonsOptions_Impl::ReadConfigurationData()
{
    
    m_aCachedMenuProperties = Sequence< Sequence< PropertyValue > >();
    m_aCachedMenuBarPartProperties = Sequence< Sequence< PropertyValue > >();
    m_aCachedToolBarPartProperties = AddonToolBars();
    m_aCachedHelpMenuProperties = Sequence< Sequence< PropertyValue > >();
    m_aCachedToolBarPartResourceNames.clear();
    m_aImageManager = ImageManager();

    ReadAddonMenuSet( m_aCachedMenuProperties );
    ReadOfficeMenuBarSet( m_aCachedMenuBarPartProperties );
    ReadOfficeToolBarSet( m_aCachedToolBarPartProperties, m_aCachedToolBarPartResourceNames );

    ReadOfficeHelpSet( m_aCachedHelpMenuProperties );
    ReadImages( m_aImageManager );

    m_aCachedMergeMenuInsContainer.clear();
    m_aCachedToolbarMergingInstructions.clear();
    m_aCachedStatusbarMergingInstructions.clear();

    ReadMenuMergeInstructions( m_aCachedMergeMenuInsContainer );
    ReadToolbarMergeInstructions( m_aCachedToolbarMergingInstructions );
    ReadStatusbarMergeInstructions( m_aCachedStatusbarMergingInstructions );
}




void AddonsOptions_Impl::Notify( const Sequence< OUString >& /*lPropertyNames*/ )
{
    Application::PostUserEvent( STATIC_LINK( 0, AddonsOptions, Notify ) );
}




void AddonsOptions_Impl::Commit()
{
    OSL_FAIL( "AddonsOptions_Impl::Commit()\nNot implemented yet!\n" );
}




sal_Bool AddonsOptions_Impl::HasAddonsMenu() const
{
    return ( m_aCachedMenuProperties.getLength() > 0 );
}




sal_Int32 AddonsOptions_Impl::GetAddonsToolBarCount() const
{
    return m_aCachedToolBarPartProperties.size();
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsMenu() const
{
    return m_aCachedMenuProperties;
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsMenuBarPart() const
{
    return m_aCachedMenuBarPartProperties;
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsToolBarPart( sal_uInt32 nIndex ) const
{
    if ( /*nIndex >= 0 &&*/ nIndex < m_aCachedToolBarPartProperties.size() )
        return m_aCachedToolBarPartProperties[nIndex];
    else
        return m_aEmptyAddonToolBar;
}




const OUString AddonsOptions_Impl::GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const
{
    if ( nIndex < m_aCachedToolBarPartResourceNames.size() )
        return m_aCachedToolBarPartResourceNames[nIndex];
    else
        return OUString();
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsHelpMenu  () const
{
    return m_aCachedHelpMenuProperties;
}




const MergeMenuInstructionContainer& AddonsOptions_Impl::GetMergeMenuInstructions() const
{
    return m_aCachedMergeMenuInsContainer;
}




bool AddonsOptions_Impl::GetMergeToolbarInstructions(
    const OUString& rToolbarName,
    MergeToolbarInstructionContainer& rToolbarInstructions ) const
{
    ToolbarMergingInstructions::const_iterator pIter = m_aCachedToolbarMergingInstructions.find( rToolbarName );
    if ( pIter != m_aCachedToolbarMergingInstructions.end() )
    {
        rToolbarInstructions = pIter->second;
        return true;
    }
    else
        return false;
}

const MergeStatusbarInstructionContainer& AddonsOptions_Impl::GetMergeStatusbarInstructions() const
{
    return m_aCachedStatusbarMergingInstructions;
}




static Image ScaleImage( const Image &rImage, bool bBig )
{
    Size aSize = ToolBox::GetDefaultImageSize(bBig);
    BitmapEx aScaleBmp(rImage.GetBitmapEx());
    SAL_INFO("fwk", "Addons: expensive scale image from "
             << aScaleBmp.GetSizePixel() << " to " << aSize);
    aScaleBmp.Scale(aSize, BMP_SCALE_BESTQUALITY);
    return Image(aScaleBmp);
}

Image AddonsOptions_Impl::GetImageFromURL( const OUString& aURL, sal_Bool bBig, sal_Bool bNoScale )
{
    Image aImage;
    ImageSize eSize = bBig ? IMGSIZE_BIG : IMGSIZE_SMALL;
    int nIdx = (int)eSize;
    int nOtherIdx = nIdx ? 0 : 1;

    SAL_INFO("fwk", "Expensive: Addons GetImageFromURL " << aURL <<
             " big " << (bBig?"big":"litte") <<
             " scale " << (bNoScale ? "noscale" : "scale"));

    ImageManager::iterator pIter = m_aImageManager.find(aURL);
    if ( pIter != m_aImageManager.end() )
    {
        ImageEntry &rEntry = pIter->second;
        
        if (!rEntry.aImage[nIdx])
            rEntry.aImage[nIdx] = ReadImageFromURL(rEntry.aURL[nIdx]);

        if (!rEntry.aImage[nIdx])
        { 
            aImage = ScaleImage(ReadImageFromURL(rEntry.aURL[nOtherIdx]), bBig);
            rEntry.aImage[nIdx] = aImage;
            if (!rEntry.aImage[nIdx])
                SAL_WARN("fwk", "failed to load addons image " << aURL);
        }

        

        if (!aImage && bNoScale)
            aImage = rEntry.aImage[nIdx];

        if (!aImage && !!rEntry.aScaled[nIdx])
            aImage = rEntry.aScaled[nIdx];

        else 
        {
            aImage = rEntry.aImage[nIdx];
            if (!aImage) 
                aImage = rEntry.aImage[nOtherIdx];

            aImage = ScaleImage(aImage, bBig);
            rEntry.aScaled[nIdx] = aImage; 
        }
    }

    return aImage;
}




sal_Bool AddonsOptions_Impl::ReadAddonMenuSet( Sequence< Sequence< PropertyValue > >& rAddonMenuSeq )
{
    
    OUString             aAddonMenuNodeName( "AddonUI/AddonMenu" );
    Sequence< OUString > aAddonMenuNodeSeq = GetNodeNames( aAddonMenuNodeName );
    OUString             aAddonMenuItemNode( aAddonMenuNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonMenuNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = m_aPropNames[ INDEX_URL            ];
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = m_aPropNames[ INDEX_TITLE          ];
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = m_aPropNames[ INDEX_TARGET         ];
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = m_aPropNames[ INDEX_CONTEXT        ];
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = m_aPropNames[ INDEX_SUBMENU        ];  

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aRootMenuItemNode( aAddonMenuItemNode + aAddonMenuNodeSeq[n] );

        
        if ( ReadMenuItem( aRootMenuItemNode, aMenuItem ) )
        {
            
            sal_uInt32 nMenuItemCount = rAddonMenuSeq.getLength() + 1;
            rAddonMenuSeq.realloc( nMenuItemCount );
            rAddonMenuSeq[nIndex++] = aMenuItem;
        }
    }

    return ( rAddonMenuSeq.getLength() > 0 );
}




sal_Bool AddonsOptions_Impl::ReadOfficeHelpSet( Sequence< Sequence< PropertyValue > >& rAddonOfficeHelpMenuSeq )
{
    
    OUString             aAddonHelpMenuNodeName( "AddonUI/OfficeHelp" );
    Sequence< OUString > aAddonHelpMenuNodeSeq = GetNodeNames( aAddonHelpMenuNodeName );
    OUString             aAddonHelpMenuItemNode( aAddonHelpMenuNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonHelpMenuNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = m_aPropNames[ INDEX_URL            ];
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = m_aPropNames[ INDEX_TITLE          ];
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = m_aPropNames[ INDEX_TARGET         ];
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = m_aPropNames[ INDEX_CONTEXT        ];
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = m_aPropNames[ INDEX_SUBMENU        ];  

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aRootMenuItemNode( aAddonHelpMenuItemNode + aAddonHelpMenuNodeSeq[n] );

        
        if ( ReadMenuItem( aRootMenuItemNode, aMenuItem, sal_True ) )
        {
            
            sal_uInt32 nMenuItemCount = rAddonOfficeHelpMenuSeq.getLength() + 1;
            rAddonOfficeHelpMenuSeq.realloc( nMenuItemCount );
            rAddonOfficeHelpMenuSeq[nIndex++] = aMenuItem;
        }
    }

    return ( rAddonOfficeHelpMenuSeq.getLength() > 0 );
}




sal_Bool AddonsOptions_Impl::ReadOfficeMenuBarSet( Sequence< Sequence< PropertyValue > >& rAddonOfficeMenuBarSeq )
{
    
    OUString             aAddonMenuBarNodeName( "AddonUI/OfficeMenuBar" );
    Sequence< OUString > aAddonMenuBarNodeSeq = GetNodeNames( aAddonMenuBarNodeName );
    OUString             aAddonMenuBarNode( aAddonMenuBarNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonMenuBarNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aPopupMenu( PROPERTYCOUNT_POPUPMENU );

    
    aPopupMenu[ OFFSET_POPUPMENU_TITLE      ].Name = m_aPropNames[ INDEX_TITLE  ];
    aPopupMenu[ OFFSET_POPUPMENU_CONTEXT    ].Name = m_aPropNames[ INDEX_CONTEXT];
    aPopupMenu[ OFFSET_POPUPMENU_SUBMENU    ].Name = m_aPropNames[ INDEX_SUBMENU];
    aPopupMenu[ OFFSET_POPUPMENU_URL        ].Name = m_aPropNames[ INDEX_URL    ];

    StringToIndexMap aTitleToIndexMap;

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aPopupMenuNode( aAddonMenuBarNode + aAddonMenuBarNodeSeq[n] );

        
        if ( ReadPopupMenu( aPopupMenuNode, aPopupMenu ) )
        {
            
            OUString aPopupTitle;
            if ( aPopupMenu[OFFSET_POPUPMENU_TITLE].Value >>= aPopupTitle )
            {
                StringToIndexMap::const_iterator pIter = aTitleToIndexMap.find( aPopupTitle );
                if ( pIter != aTitleToIndexMap.end() )
                {
                    
                    Sequence< PropertyValue >& rOldPopupMenu = rAddonOfficeMenuBarSeq[pIter->second];
                    AppendPopupMenu( rOldPopupMenu, aPopupMenu );
                }
                else
                {
                    
                    sal_uInt32 nMenuItemCount = rAddonOfficeMenuBarSeq.getLength() + 1;
                    rAddonOfficeMenuBarSeq.realloc( nMenuItemCount );
                    rAddonOfficeMenuBarSeq[nIndex] = aPopupMenu;
                    aTitleToIndexMap.insert( StringToIndexMap::value_type( aPopupTitle, nIndex ));
                    ++nIndex;
                }
            }
        }
    }

    return ( rAddonOfficeMenuBarSeq.getLength() > 0 );
}




sal_Bool AddonsOptions_Impl::ReadOfficeToolBarSet( AddonToolBars& rAddonOfficeToolBars, std::vector< OUString >& rAddonOfficeToolBarResNames )
{
    
    OUString             aAddonToolBarNodeName( "AddonUI/OfficeToolBar" );
    Sequence< OUString > aAddonToolBarNodeSeq = GetNodeNames( aAddonToolBarNodeName );
    OUString             aAddonToolBarNode( aAddonToolBarNodeName + m_aPathDelimiter );

    sal_uInt32           nCount = aAddonToolBarNodeSeq.getLength();

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aToolBarItemNode( aAddonToolBarNode + aAddonToolBarNodeSeq[n] );
        rAddonOfficeToolBarResNames.push_back( aAddonToolBarNodeSeq[n] );
        rAddonOfficeToolBars.push_back( m_aEmptyAddonToolBar );
        ReadToolBarItemSet( aToolBarItemNode, rAddonOfficeToolBars[n] );
    }

    return ( !rAddonOfficeToolBars.empty() );
}





sal_Bool AddonsOptions_Impl::ReadToolBarItemSet( const OUString rToolBarItemSetNodeName, Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq )
{
    sal_Bool                    bInsertSeparator        = sal_False;
    sal_uInt32                  nToolBarItemCount       = rAddonOfficeToolBarSeq.getLength();
    OUString                 aAddonToolBarItemSetNode( rToolBarItemSetNodeName + m_aPathDelimiter );
    Sequence< OUString >     aAddonToolBarItemSetNodeSeq = GetNodeNames( rToolBarItemSetNodeName );
    Sequence< PropertyValue >   aToolBarItem( PROPERTYCOUNT_TOOLBARITEM );

    
    aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Name = m_aPropNames[ INDEX_URL            ];
    aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Name = m_aPropNames[ INDEX_TITLE          ];
    aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Name = m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Name = m_aPropNames[ INDEX_TARGET         ];
    aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Name = m_aPropNames[ INDEX_CONTEXT        ];
    aToolBarItem[ OFFSET_TOOLBARITEM_CONTROLTYPE        ].Name = m_aPropNames[ INDEX_CONTROLTYPE    ];
    aToolBarItem[ OFFSET_TOOLBARITEM_WIDTH              ].Name = m_aPropNames[ INDEX_WIDTH          ];

    sal_uInt32 nCount = aAddonToolBarItemSetNodeSeq.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aToolBarItemNode( aAddonToolBarItemSetNode + aAddonToolBarItemSetNodeSeq[n] );

        
        if ( ReadToolBarItem( aToolBarItemNode, aToolBarItem ) )
        {
            if ( bInsertSeparator )
            {
                bInsertSeparator = sal_False;
                InsertToolBarSeparator( rAddonOfficeToolBarSeq );
            }

            
            sal_uInt32 nAddonCount = rAddonOfficeToolBarSeq.getLength();
            rAddonOfficeToolBarSeq.realloc( nAddonCount+1 );
            rAddonOfficeToolBarSeq[nAddonCount] = aToolBarItem;
        }
    }

    return ( (sal_uInt32)rAddonOfficeToolBarSeq.getLength() > nToolBarItemCount );
}




void AddonsOptions_Impl::InsertToolBarSeparator( Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq )
{
    Sequence< PropertyValue >   aToolBarItem( PROPERTYCOUNT_TOOLBARITEM );

    aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Name = m_aPropNames[ INDEX_URL            ];
    aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Name = m_aPropNames[ INDEX_TITLE          ];
    aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Name = m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Name = m_aPropNames[ INDEX_TARGET         ];
    aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Name = m_aPropNames[ INDEX_CONTEXT        ];

    aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= SEPARATOR_URL;
    aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= m_aEmpty;
    aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value <<= m_aEmpty;
    aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= m_aEmpty;
    aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value <<= m_aEmpty;

    sal_uInt32 nToolBarItemCount = rAddonOfficeToolBarSeq.getLength();
    rAddonOfficeToolBarSeq.realloc( nToolBarItemCount+1 );
    rAddonOfficeToolBarSeq[nToolBarItemCount] = aToolBarItem;
}





void AddonsOptions_Impl::ReadImages( ImageManager& aImageManager )
{
    
    OUString                aAddonImagesNodeName( "AddonUI/Images" );
    Sequence< OUString > aAddonImagesNodeSeq = GetNodeNames( aAddonImagesNodeName );
    OUString                aAddonImagesNode( aAddonImagesNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonImagesNodeSeq.getLength();

    
    Sequence< OUString > aAddonImageItemNodePropNames( 1 );
    OUString                aURL;

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aImagesItemNode( aAddonImagesNode + aAddonImagesNodeSeq[n] );

        
        OUStringBuffer aBuffer( aImagesItemNode );
        aBuffer.append( m_aPathDelimiter );
        aBuffer.append( m_aPropNames[ OFFSET_MENUITEM_URL ] );
        aAddonImageItemNodePropNames[0] = aBuffer.makeStringAndClear();

        Sequence< Any > aAddonImageItemNodeValues = GetProperties( aAddonImageItemNodePropNames );

        
        
        if (( aAddonImageItemNodeValues[0] >>= aURL ) &&
            !aURL.isEmpty() &&
            !HasAssociatedImages( aURL ))
        {
            OUStringBuffer aBuf( aImagesItemNode );
            aBuf.append( m_aPathDelimiter );
            aBuf.append( IMAGES_NODENAME );
            aBuf.append( m_aPathDelimiter );
            OUString aImagesUserDefinedItemNode = aBuf.makeStringAndClear();

            
            ImageEntry* pImageEntry = ReadImageData( aImagesUserDefinedItemNode );
            if ( pImageEntry )
            {
                
                aImageManager.insert( ImageManager::value_type( aURL, *pImageEntry ));
                delete pImageEntry; 
            }
        }
    }
}





OUString AddonsOptions_Impl::GeneratePrefixURL()
{
    
    
    OUString aPopupMenuURL;
    OUStringBuffer aBuf( m_aRootAddonPopupMenuURLPrexfix.getLength() + 3 );
    aBuf.append( m_aRootAddonPopupMenuURLPrexfix );
    aBuf.append( OUString::number( ++m_nRootAddonPopupMenuId ));
    aPopupMenuURL = aBuf.makeStringAndClear();
    return aPopupMenuURL;
}





sal_Bool AddonsOptions_Impl::ReadMenuMergeInstructions( MergeMenuInstructionContainer& aContainer )
{
    const OUString aMenuMergeRootName( "AddonUI/OfficeMenuBarMerging/" );

    Sequence< OUString > aAddonMergeNodesSeq = GetNodeNames( aMenuMergeRootName );
    OUString                aAddonMergeNode( aMenuMergeRootName );

    sal_uInt32              nCount = aAddonMergeNodesSeq.getLength();

    
    Sequence< OUString > aNodePropNames( 5 );

    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        OUString aMergeAddonInstructions( aAddonMergeNode + aAddonMergeNodesSeq[i] );

        Sequence< OUString > aAddonInstMergeNodesSeq = GetNodeNames( aMergeAddonInstructions );
        sal_uInt32           nCountAddons = aAddonInstMergeNodesSeq.getLength();

        for ( sal_uInt32 j = 0; j < nCountAddons; j++ )
        {
            OUStringBuffer aMergeAddonInstructionBase( aMergeAddonInstructions );
            aMergeAddonInstructionBase.append( m_aPathDelimiter );
            aMergeAddonInstructionBase.append( aAddonInstMergeNodesSeq[j] );
            aMergeAddonInstructionBase.append( m_aPathDelimiter );

            
            OUStringBuffer aBuffer( aMergeAddonInstructionBase );
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEPOINT ] );
            aNodePropNames[0] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMAND ] );
            aNodePropNames[1] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMANDPARAMETER ] );
            aNodePropNames[2] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEFALLBACK ] );
            aNodePropNames[3] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECONTEXT ] );
            aNodePropNames[4] = aBuffer.makeStringAndClear();

            Sequence< Any > aNodePropValues = GetProperties( aNodePropNames );

            MergeMenuInstruction aMergeMenuInstruction;
            aNodePropValues[0] >>= aMergeMenuInstruction.aMergePoint;
            aNodePropValues[1] >>= aMergeMenuInstruction.aMergeCommand;
            aNodePropValues[2] >>= aMergeMenuInstruction.aMergeCommandParameter;
            aNodePropValues[3] >>= aMergeMenuInstruction.aMergeFallback;
            aNodePropValues[4] >>= aMergeMenuInstruction.aMergeContext;

            OUString aMergeMenuBase = aMergeAddonInstructionBase.makeStringAndClear();
            ReadMergeMenuData( aMergeMenuBase, aMergeMenuInstruction.aMergeMenu );

            aContainer.push_back( aMergeMenuInstruction );
        }
    }

    return sal_True;
}




sal_Bool AddonsOptions_Impl::ReadMergeMenuData( const OUString& aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeMenu )
{
    OUString aMergeMenuBaseNode( aMergeAddonInstructionBase+m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MENUITEMS ] );

    Sequence< OUString > aSubMenuNodeNames = GetNodeNames( aMergeMenuBaseNode );
    aMergeMenuBaseNode += m_aPathDelimiter;

    
    for ( sal_uInt32 i = 0; i < (sal_uInt32)aSubMenuNodeNames.getLength(); i++ )
        aSubMenuNodeNames[i] = aMergeMenuBaseNode + aSubMenuNodeNames[i];

    return ReadSubMenuEntries( aSubMenuNodeNames, rMergeMenu );
}




sal_Bool AddonsOptions_Impl::ReadToolbarMergeInstructions( ToolbarMergingInstructions& rCachedToolbarMergingInstructions )
{
    const OUString aToolbarMergeRootName( "AddonUI/OfficeToolbarMerging/" );

    Sequence< OUString > aAddonMergeNodesSeq = GetNodeNames( aToolbarMergeRootName );
    OUString                aAddonMergeNode( aToolbarMergeRootName );

    sal_uInt32              nCount = aAddonMergeNodesSeq.getLength();

    
    Sequence< OUString > aNodePropNames( 6 );

    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        OUString aMergeAddonInstructions( aAddonMergeNode + aAddonMergeNodesSeq[i] );

        Sequence< OUString > aAddonInstMergeNodesSeq = GetNodeNames( aMergeAddonInstructions );
        sal_uInt32           nCountAddons = aAddonInstMergeNodesSeq.getLength();

        for ( sal_uInt32 j = 0; j < nCountAddons; j++ )
        {
            OUStringBuffer aMergeAddonInstructionBase( aMergeAddonInstructions );
            aMergeAddonInstructionBase.append( m_aPathDelimiter );
            aMergeAddonInstructionBase.append( aAddonInstMergeNodesSeq[j] );
            aMergeAddonInstructionBase.append( m_aPathDelimiter );

            
            OUStringBuffer aBuffer( aMergeAddonInstructionBase );
            aBuffer.append( m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBAR ] );
            aNodePropNames[0] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEPOINT ] );
            aNodePropNames[1] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMAND ] );
            aNodePropNames[2] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMANDPARAMETER ] );
            aNodePropNames[3] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEFALLBACK ] );
            aNodePropNames[4] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECONTEXT ] );
            aNodePropNames[5] = aBuffer.makeStringAndClear();

            Sequence< Any > aNodePropValues = GetProperties( aNodePropNames );

            MergeToolbarInstruction aMergeToolbarInstruction;
            aNodePropValues[0] >>= aMergeToolbarInstruction.aMergeToolbar;
            aNodePropValues[1] >>= aMergeToolbarInstruction.aMergePoint;
            aNodePropValues[2] >>= aMergeToolbarInstruction.aMergeCommand;
            aNodePropValues[3] >>= aMergeToolbarInstruction.aMergeCommandParameter;
            aNodePropValues[4] >>= aMergeToolbarInstruction.aMergeFallback;
            aNodePropValues[5] >>= aMergeToolbarInstruction.aMergeContext;

            ReadMergeToolbarData( aMergeAddonInstructionBase.makeStringAndClear(),
                                  aMergeToolbarInstruction.aMergeToolbarItems );

            MergeToolbarInstructionContainer& rVector = rCachedToolbarMergingInstructions[ aMergeToolbarInstruction.aMergeToolbar ];
            rVector.push_back( aMergeToolbarInstruction );
        }
    }

    return sal_True;
}




sal_Bool AddonsOptions_Impl::ReadMergeToolbarData( const OUString& aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeToolbarItems )
{
    OUStringBuffer aBuffer( aMergeAddonInstructionBase );
    aBuffer.append( m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBARITEMS ] );

    OUString aMergeToolbarBaseNode = aBuffer.makeStringAndClear();

    return ReadToolBarItemSet( aMergeToolbarBaseNode, rMergeToolbarItems );
}


sal_Bool AddonsOptions_Impl::ReadStatusbarMergeInstructions( MergeStatusbarInstructionContainer& aContainer )
{
    const ::rtl::OUString aStatusbarMergeRootName( "AddonUI/OfficeStatusbarMerging/" );

    Sequence< ::rtl::OUString > aAddonMergeNodesSeq = GetNodeNames( aStatusbarMergeRootName );
    ::rtl::OUString aAddonMergeNode( aStatusbarMergeRootName );
    sal_uInt32  nCount = aAddonMergeNodesSeq.getLength();

    Sequence< ::rtl::OUString > aNodePropNames( 5 );

    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        ::rtl::OUString aMergeAddonInstructions( aAddonMergeNode + aAddonMergeNodesSeq[i] );

        Sequence< ::rtl::OUString > aAddonInstMergeNodesSeq = GetNodeNames( aMergeAddonInstructions );
        sal_uInt32 nCountAddons = aAddonInstMergeNodesSeq.getLength();

        for ( sal_uInt32 j = 0; j < nCountAddons; j++ )
        {
            ::rtl::OUStringBuffer aMergeAddonInstructionBase( aMergeAddonInstructions );
            aMergeAddonInstructionBase.append( m_aPathDelimiter );
            aMergeAddonInstructionBase.append( aAddonInstMergeNodesSeq[j] );
            aMergeAddonInstructionBase.append( m_aPathDelimiter );

            
            ::rtl::OUStringBuffer aBuffer( aMergeAddonInstructionBase );
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGEPOINT ] );
            aNodePropNames[0] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGECOMMAND ] );
            aNodePropNames[1] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGECOMMANDPARAMETER ] );
            aNodePropNames[2] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGEFALLBACK ] );
            aNodePropNames[3] = aBuffer.makeStringAndClear();

            aBuffer = aMergeAddonInstructionBase;
            aBuffer.append( m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGECONTEXT ] );
            aNodePropNames[4] = aBuffer.makeStringAndClear();

            Sequence< Any > aNodePropValues = GetProperties( aNodePropNames );

            MergeStatusbarInstruction aMergeStatusbarInstruction;
            aNodePropValues[0] >>= aMergeStatusbarInstruction.aMergePoint;
            aNodePropValues[1] >>= aMergeStatusbarInstruction.aMergeCommand;
            aNodePropValues[2] >>= aMergeStatusbarInstruction.aMergeCommandParameter;
            aNodePropValues[3] >>= aMergeStatusbarInstruction.aMergeFallback;
            aNodePropValues[4] >>= aMergeStatusbarInstruction.aMergeContext;

            ReadMergeStatusbarData( aMergeAddonInstructionBase.makeStringAndClear(),
                                    aMergeStatusbarInstruction.aMergeStatusbarItems );

            aContainer.push_back( aMergeStatusbarInstruction );
        }
    }

    return sal_True;
}

sal_Bool AddonsOptions_Impl::ReadMergeStatusbarData(
    const ::rtl::OUString& aMergeAddonInstructionBase,
    Sequence< Sequence< PropertyValue > >& rMergeStatusbarItems )
{
    sal_uInt32 nStatusbarItemCount = rMergeStatusbarItems.getLength();

    ::rtl::OUStringBuffer aBuffer( aMergeAddonInstructionBase );
    aBuffer.append( m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_STATUSBARITEMS ] );
    ::rtl::OUString aMergeStatusbarBaseNode = aBuffer.makeStringAndClear();

    ::rtl::OUString aAddonStatusbarItemSetNode( aMergeStatusbarBaseNode + m_aPathDelimiter );
    Sequence< ::rtl::OUString > aAddonStatusbarItemSetNodeSeq = GetNodeNames( aMergeStatusbarBaseNode );

    Sequence< PropertyValue > aStatusbarItem( PROPERTYCOUNT_STATUSBARITEM );
    aStatusbarItem[ OFFSET_STATUSBARITEM_URL       ].Name = m_aPropNames[ INDEX_URL       ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_TITLE     ].Name = m_aPropNames[ INDEX_TITLE     ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_CONTEXT   ].Name = m_aPropNames[ INDEX_CONTEXT   ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_ALIGN     ].Name = m_aPropNames[ INDEX_ALIGN     ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_AUTOSIZE  ].Name = m_aPropNames[ INDEX_AUTOSIZE  ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_OWNERDRAW ].Name = m_aPropNames[ INDEX_OWNERDRAW ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_WIDTH     ].Name = m_aPropNames[ INDEX_WIDTH     ];

    sal_uInt32 nCount = aAddonStatusbarItemSetNodeSeq.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        ::rtl::OUString aStatusbarItemNode( aAddonStatusbarItemSetNode + aAddonStatusbarItemSetNodeSeq[n] );

        if ( ReadStatusBarItem( aStatusbarItemNode, aStatusbarItem ) )
        {
            sal_uInt32 nAddonCount = rMergeStatusbarItems.getLength();
            rMergeStatusbarItems.realloc( nAddonCount+1 );
            rMergeStatusbarItems[nAddonCount] = aStatusbarItem;
        }
    }

    return ( (sal_uInt32)rMergeStatusbarItems.getLength() > nStatusbarItemCount );
}

sal_Bool AddonsOptions_Impl::ReadStatusBarItem(
    const ::rtl::OUString& aStatusarItemNodeName,
    Sequence< PropertyValue >& aStatusbarItem )
{
    sal_Bool bResult( sal_False );
    ::rtl::OUString aURL;
    ::rtl::OUString aAddonStatusbarItemTreeNode( aStatusarItemNodeName + m_aPathDelimiter );
    Sequence< Any > aStatusbarItemNodePropValues;

    aStatusbarItemNodePropValues = GetProperties( GetPropertyNamesStatusbarItem( aAddonStatusbarItemTreeNode ) );

    
    if (( aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_URL ] >>= aURL ) && aURL.getLength() > 0 )
    {
        aStatusbarItem[ OFFSET_STATUSBARITEM_URL        ].Value <<= aURL;
        aStatusbarItem[ OFFSET_STATUSBARITEM_TITLE      ].Value <<= aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_TITLE     ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_CONTEXT    ].Value <<= aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_CONTEXT   ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_ALIGN      ].Value <<= aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_ALIGN     ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_AUTOSIZE   ].Value <<= aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_AUTOSIZE  ];;
        aStatusbarItem[ OFFSET_STATUSBARITEM_OWNERDRAW  ].Value <<= aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_OWNERDRAW ];

        
        sal_Int64 nValue( 0 );
        aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_WIDTH ] >>= nValue;
        aStatusbarItem[ OFFSET_STATUSBARITEM_WIDTH ].Value <<= sal_Int32( nValue );

        bResult = sal_True;
    }

    return bResult;
}




sal_Bool AddonsOptions_Impl::ReadMenuItem( const OUString& aMenuNodeName, Sequence< PropertyValue >& aMenuItem, sal_Bool bIgnoreSubMenu )
{
    sal_Bool            bResult = sal_False;
    OUString         aStrValue;
    OUString         aAddonMenuItemTreeNode( aMenuNodeName + m_aPathDelimiter );
    Sequence< Any >     aMenuItemNodePropValues;

    aMenuItemNodePropValues = GetProperties( GetPropertyNamesMenuItem( aAddonMenuItemTreeNode ) );
    if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_TITLE ] >>= aStrValue ) && !aStrValue.isEmpty() )
    {
        aMenuItem[ OFFSET_MENUITEM_TITLE ].Value <<= aStrValue;

        OUString aRootSubMenuName( aAddonMenuItemTreeNode + m_aPropNames[ INDEX_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.getLength() > 0 && !bIgnoreSubMenu )
        {
            
            OUString aPopupMenuURL     = GeneratePrefixURL();
            OUString aPopupMenuImageId;

            aMenuItemNodePropValues[ OFFSET_MENUITEM_IMAGEIDENTIFIER ] >>= aPopupMenuImageId;
            ReadAndAssociateImages( aPopupMenuURL, aPopupMenuImageId );

            
            
            aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aPopupMenuURL;
            aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= m_aEmpty;
            aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= aPopupMenuImageId;
            aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_CONTEXT ];

            
            Sequence< Sequence< PropertyValue > > aSubMenuSeq;
            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aRootSubMenuNodeNames.getLength(); n++ )
                aRootSubMenuNodeNames[n] = aSubMenuRootNodeName + aRootSubMenuNodeNames[n];
            ReadSubMenuEntries( aRootSubMenuNodeNames, aSubMenuSeq );
            aMenuItem[ OFFSET_MENUITEM_SUBMENU ].Value <<= aSubMenuSeq;
            bResult = sal_True;
        }
        else if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_URL ] >>= aStrValue ) && !aStrValue.isEmpty() )
        {
            
            OUString aMenuImageId;

            aMenuItemNodePropValues[ OFFSET_MENUITEM_IMAGEIDENTIFIER ] >>= aMenuImageId;
             ReadAndAssociateImages( aStrValue, aMenuImageId );

            aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aStrValue;
            aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_TARGET         ];
            aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= aMenuImageId;
            aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value <<= aMenuItemNodePropValues[ OFFSET_MENUITEM_CONTEXT        ];
            aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Value <<= Sequence< Sequence< PropertyValue > >(); 

            bResult = sal_True;
        }
    }
    else if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_URL ] >>= aStrValue ) &&
              aStrValue == SEPARATOR_URL_STR )
    {
        
        aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aStrValue;
        aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= m_aEmpty;
        aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= m_aEmpty;
        aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value <<= m_aEmpty;
        aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Value <<= Sequence< Sequence< PropertyValue > >(); 
        bResult = sal_True;
    }

    return bResult;
}




sal_Bool AddonsOptions_Impl::ReadPopupMenu( const OUString& aPopupMenuNodeName, Sequence< PropertyValue >& aPopupMenu )
{
    sal_Bool            bResult = sal_False;
    OUString         aStrValue;
    OUString         aAddonPopupMenuTreeNode( aPopupMenuNodeName + m_aPathDelimiter );
    Sequence< Any >     aPopupMenuNodePropValues;

    aPopupMenuNodePropValues = GetProperties( GetPropertyNamesPopupMenu( aAddonPopupMenuTreeNode ) );
    if (( aPopupMenuNodePropValues[ OFFSET_POPUPMENU_TITLE ] >>= aStrValue ) &&
         !aStrValue.isEmpty() )
    {
        aPopupMenu[ OFFSET_POPUPMENU_TITLE ].Value <<= aStrValue;

        OUString aRootSubMenuName( aAddonPopupMenuTreeNode + m_aPropNames[ INDEX_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.getLength() > 0 )
        {
            
            
            OUString aPopupMenuURL = GeneratePrefixURL();

            aPopupMenu[ OFFSET_POPUPMENU_URL        ].Value <<= aPopupMenuURL;
            aPopupMenu[ OFFSET_POPUPMENU_CONTEXT    ].Value <<= aPopupMenuNodePropValues[ OFFSET_POPUPMENU_CONTEXT ];

            
            Sequence< Sequence< PropertyValue > > aSubMenuSeq;
            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( sal_uInt32 n = 0; n < (sal_uInt32)aRootSubMenuNodeNames.getLength(); n++ )
                aRootSubMenuNodeNames[n] = aSubMenuRootNodeName + aRootSubMenuNodeNames[n];
            ReadSubMenuEntries( aRootSubMenuNodeNames, aSubMenuSeq );
            aPopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value <<= aSubMenuSeq;
            bResult = sal_True;
        }
    }

    return bResult;
}




sal_Bool AddonsOptions_Impl::AppendPopupMenu( Sequence< PropertyValue >& rTargetPopupMenu, const Sequence< PropertyValue >& rSourcePopupMenu )
{
    Sequence< Sequence< PropertyValue > > aTargetSubMenuSeq;
    Sequence< Sequence< PropertyValue > > aSourceSubMenuSeq;

    if (( rTargetPopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value >>= aTargetSubMenuSeq ) &&
        ( rSourcePopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value >>= aSourceSubMenuSeq ))
    {
        sal_uInt32 nIndex = aTargetSubMenuSeq.getLength();
        aTargetSubMenuSeq.realloc( nIndex + aSourceSubMenuSeq.getLength() );
        for ( sal_uInt32 i = 0; i < sal_uInt32( aSourceSubMenuSeq.getLength() ); i++ )
            aTargetSubMenuSeq[nIndex++] = aSourceSubMenuSeq[i];
        rTargetPopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value <<= aTargetSubMenuSeq;
    }

    return sal_True;
}




sal_Bool AddonsOptions_Impl::ReadToolBarItem( const OUString& aToolBarItemNodeName, Sequence< PropertyValue >& aToolBarItem )
{
    sal_Bool            bResult = sal_False;
    OUString         aTitle;
    OUString         aURL;
    OUString         aAddonToolBarItemTreeNode( aToolBarItemNodeName + m_aPathDelimiter );
    Sequence< Any >     aToolBarItemNodePropValues;

    aToolBarItemNodePropValues = GetProperties( GetPropertyNamesToolBarItem( aAddonToolBarItemTreeNode ) );

    
    if (( aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_URL ] >>= aURL ) && !aURL.isEmpty() )
    {
        if ( aURL.equals( SEPARATOR_URL ))
        {
            
            aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= aURL;
            aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTROLTYPE        ].Value <<= m_aEmpty;
            aToolBarItem[ OFFSET_TOOLBARITEM_WIDTH              ].Value <<= sal_Int32( 0 );

            bResult = sal_True;
        }
        else if (( aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_TITLE ] >>= aTitle ) && !aTitle.isEmpty() )
        {
            
            OUString aImageId;

            
            aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER ] >>= aImageId;
            ReadAndAssociateImages( aURL, aImageId );

            aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= aURL;
            aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= aTitle;
            aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value <<= aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_TARGET      ];
            aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= aImageId;
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value <<= aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_CONTEXT     ];
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTROLTYPE        ].Value <<= aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_CONTROLTYPE ];

            
            sal_Int64 nValue( 0 );
            aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_WIDTH ] >>= nValue;
            aToolBarItem[ OFFSET_TOOLBARITEM_WIDTH              ].Value <<= sal_Int32( nValue );

            bResult = sal_True;
        }
    }

    return bResult;
}




sal_Bool AddonsOptions_Impl::ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, Sequence< Sequence< PropertyValue > >& rSubMenuSeq )
{
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = PROPERTYNAME_URL;
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = PROPERTYNAME_TITLE;
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = PROPERTYNAME_TARGET;
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = PROPERTYNAME_IMAGEIDENTIFIER;
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = PROPERTYNAME_CONTEXT;
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = PROPERTYNAME_SUBMENU;  

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




sal_Bool AddonsOptions_Impl::HasAssociatedImages( const OUString& aURL )
{
    
    ImageManager::const_iterator pIter = m_aImageManager.find( aURL );
    return ( pIter != m_aImageManager.end() );
}




void AddonsOptions_Impl::SubstituteVariables( OUString& aURL )
{
    if ( aURL.startsWith( EXPAND_PROTOCOL ) )
    {
        
        OUString macro( aURL.copy( sizeof ( EXPAND_PROTOCOL ) -1 ) );
        
        macro = ::rtl::Uri::decode(
            macro, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8 );
        
        aURL = m_xMacroExpander->expandMacros( macro );
    }
}




Image AddonsOptions_Impl::ReadImageFromURL(const OUString& aImageURL)
{
    Image aImage;

    SvStream* pStream = UcbStreamHelper::CreateStream( aImageURL, STREAM_STD_READ );
    if ( pStream && ( pStream->GetErrorCode() == 0 ))
    {
        
        Graphic aGraphic;

        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        rGF.ImportGraphic( aGraphic, OUString(), *pStream, GRFILTER_FORMAT_DONTKNOW );

        BitmapEx aBitmapEx = aGraphic.GetBitmapEx();

        Size aBmpSize = aBitmapEx.GetSizePixel();
        if ( aBmpSize.Width() > 0 && aBmpSize.Height() > 0 )
        {
            
            if( !aBitmapEx.IsTransparent() )
                aBitmapEx = BitmapEx( aBitmapEx.GetBitmap(), COL_LIGHTMAGENTA );

            aImage = Image(aBitmapEx);
        }
    }

    delete pStream;

    return aImage;
}




void AddonsOptions_Impl::ReadAndAssociateImages( const OUString& aURL, const OUString& aImageId )
{
    if ( aImageId.isEmpty() )
        return;

    ImageEntry  aImageEntry;
    OUString    aImageURL( aImageId );

    SubstituteVariables( aImageURL );

    
    static const char* aExtArray[] = { "_16", "_26" };
    for ( size_t i = 0; i < SAL_N_ELEMENTS(aExtArray); i++ )
    {
        OUStringBuffer aFileURL( aImageURL );
        aFileURL.appendAscii( aExtArray[i] );
        aFileURL.appendAscii( ".bmp" );

        aImageEntry.addImage( !i ? IMGSIZE_SMALL : IMGSIZE_BIG,
                              Image(), aFileURL.makeStringAndClear() );
    }

    m_aImageManager.insert( ImageManager::value_type( aURL, aImageEntry ));
}




AddonsOptions_Impl::ImageEntry* AddonsOptions_Impl::ReadImageData( const OUString& aImagesNodeName )
{
    Sequence< OUString > aImageDataNodeNames = GetPropertyNamesImages( aImagesNodeName );
    Sequence< Any >      aPropertyData;
    Sequence< sal_Int8 > aImageDataSeq;
    OUString             aImageURL;

    ImageEntry* pEntry = NULL;

    
    
    aPropertyData = GetProperties( aImageDataNodeNames );
    for ( int i = 0; i < PROPERTYCOUNT_IMAGES; i++ )
    {
        if ( i < PROPERTYCOUNT_EMBEDDED_IMAGES )
        {
            
            Image aImage;
            if (( aPropertyData[i] >>= aImageDataSeq ) &&
                aImageDataSeq.getLength() > 0 &&
                ( CreateImageFromSequence( aImage, aImageDataSeq ) ) )
            {
                if ( !pEntry )
                    pEntry = new ImageEntry;
                pEntry->addImage(i == OFFSET_IMAGES_SMALL ? IMGSIZE_SMALL : IMGSIZE_BIG, aImage, "");
            }
        }
        else
        {
            if(!pEntry)
                pEntry = new ImageEntry();

            
            
            aPropertyData[i] >>= aImageURL;

            SubstituteVariables( aImageURL );

            pEntry->addImage(i == OFFSET_IMAGES_SMALL ? IMGSIZE_SMALL : IMGSIZE_BIG,
                             Image(), aImageURL);
        }
    }

    return pEntry;
}




sal_Bool AddonsOptions_Impl::CreateImageFromSequence( Image& rImage, Sequence< sal_Int8 >& rBitmapDataSeq ) const
{
    sal_Bool bResult = sal_False;

    if ( rBitmapDataSeq.getLength() > 0 )
    {
        SvMemoryStream  aMemStream( rBitmapDataSeq.getArray(), rBitmapDataSeq.getLength(), STREAM_STD_READ );
        BitmapEx        aBitmapEx;

        ReadDIBBitmapEx(aBitmapEx, aMemStream);

        if( !aBitmapEx.IsTransparent() )
        {
            
            aBitmapEx = BitmapEx( aBitmapEx.GetBitmap(), COL_LIGHTMAGENTA );
        }

        rImage = Image( aBitmapEx );
        bResult = sal_True;
    }

    return bResult;
}

Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesMenuItem( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_MENUITEM );

    
    lResult[OFFSET_MENUITEM_URL]             = aPropertyRootNode + m_aPropNames[ INDEX_URL          ] ;
    lResult[OFFSET_MENUITEM_TITLE]           = aPropertyRootNode + m_aPropNames[ INDEX_TITLE            ];
    lResult[OFFSET_MENUITEM_IMAGEIDENTIFIER] = aPropertyRootNode + m_aPropNames[ INDEX_IMAGEIDENTIFIER ];
    lResult[OFFSET_MENUITEM_TARGET]          = aPropertyRootNode + m_aPropNames[ INDEX_TARGET           ];
    lResult[OFFSET_MENUITEM_CONTEXT]         = aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT      ];
    lResult[OFFSET_MENUITEM_SUBMENU]         = aPropertyRootNode + m_aPropNames[ INDEX_SUBMENU      ];

    return lResult;
}




Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesPopupMenu( const OUString& aPropertyRootNode ) const
{
    
    Sequence< OUString > lResult( PROPERTYCOUNT_POPUPMENU-1 );

    
    lResult[OFFSET_POPUPMENU_TITLE]   = aPropertyRootNode + m_aPropNames[ INDEX_TITLE  ];
    lResult[OFFSET_POPUPMENU_CONTEXT] = aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT    ];
    lResult[OFFSET_POPUPMENU_SUBMENU] = aPropertyRootNode + m_aPropNames[ INDEX_SUBMENU    ];

    return lResult;
}




Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesToolBarItem( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_TOOLBARITEM );

    
    lResult[0] = aPropertyRootNode + m_aPropNames[ INDEX_URL             ];
    lResult[1] = aPropertyRootNode + m_aPropNames[ INDEX_TITLE       ];
    lResult[2] = aPropertyRootNode + m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    lResult[3] = aPropertyRootNode + m_aPropNames[ INDEX_TARGET          ];
    lResult[4] = aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT         ];
    lResult[5] = aPropertyRootNode + m_aPropNames[ INDEX_CONTROLTYPE     ];
    lResult[6] = aPropertyRootNode + m_aPropNames[ INDEX_WIDTH       ];

    return lResult;
}

Sequence< ::rtl::OUString > AddonsOptions_Impl::GetPropertyNamesStatusbarItem(
    const ::rtl::OUString& aPropertyRootNode ) const
{
    Sequence< ::rtl::OUString > lResult( PROPERTYCOUNT_STATUSBARITEM );

    lResult[0] = ::rtl::OUString( aPropertyRootNode + m_aPropNames[ INDEX_URL       ] );
    lResult[1] = ::rtl::OUString( aPropertyRootNode + m_aPropNames[ INDEX_TITLE     ] );
    lResult[2] = ::rtl::OUString( aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT   ] );
    lResult[3] = ::rtl::OUString( aPropertyRootNode + m_aPropNames[ INDEX_ALIGN     ] );
    lResult[4] = ::rtl::OUString( aPropertyRootNode + m_aPropNames[ INDEX_AUTOSIZE  ] );
    lResult[5] = ::rtl::OUString( aPropertyRootNode + m_aPropNames[ INDEX_OWNERDRAW ] );
    lResult[6] = ::rtl::OUString( aPropertyRootNode + m_aPropNames[ INDEX_WIDTH     ] );

    return lResult;
}




Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesImages( const OUString& aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_IMAGES );

    
    lResult[0] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_SMALL       ];
    lResult[1] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_BIG     ];
    lResult[2] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC ];
    lResult[3] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_BIGHC       ];
    lResult[4] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_SMALL_URL  ];
    lResult[5] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_BIG_URL ];
    lResult[6] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC_URL];
    lResult[7] = aPropertyRootNode + m_aPropImagesNames[ OFFSET_IMAGES_BIGHC_URL   ];

    return lResult;
}






AddonsOptions_Impl*     AddonsOptions::m_pDataContainer = NULL  ;
sal_Int32               AddonsOptions::m_nRefCount      = 0     ;




AddonsOptions::AddonsOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    ++m_nRefCount;
    
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new AddonsOptions_Impl;
    }
}




AddonsOptions::~AddonsOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    --m_nRefCount;
    
    
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}




sal_Bool AddonsOptions::HasAddonsMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->HasAddonsMenu();
}





sal_Int32 AddonsOptions::GetAddonsToolBarCount() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsToolBarCount();
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsMenu();
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsMenuBarPart() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsMenuBarPart();
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsToolBarPart( sal_uInt32 nIndex ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsToolBarPart( nIndex );
}




const OUString AddonsOptions::GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsToolbarResourceName( nIndex );
}




const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsHelpMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetAddonsHelpMenu();
}




const MergeMenuInstructionContainer& AddonsOptions::GetMergeMenuInstructions() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetMergeMenuInstructions();
}




bool AddonsOptions::GetMergeToolbarInstructions(
    const OUString& rToolbarName,
    MergeToolbarInstructionContainer& rToolbarInstructions ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetMergeToolbarInstructions(
        rToolbarName, rToolbarInstructions );
}

const MergeStatusbarInstructionContainer& AddonsOptions::GetMergeStatusbarInstructions() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetMergeStatusbarInstructions();
}




Image AddonsOptions::GetImageFromURL( const OUString& aURL, sal_Bool bBig, sal_Bool bNoScale ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetImageFromURL( aURL, bBig, bNoScale );
}




Image AddonsOptions::GetImageFromURL( const OUString& aURL, sal_Bool bBig ) const
{
    return GetImageFromURL( aURL, bBig, sal_False );
}




Mutex& AddonsOptions::GetOwnStaticMutex()
{
    
    static Mutex* pMutex = NULL;
    
    if( pMutex == NULL )
    {
        
        
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        
        if( pMutex == NULL )
        {
            
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    
    return *pMutex;
}




IMPL_STATIC_LINK_NOINSTANCE( AddonsOptions, Notify, void*, EMPTYARG )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->ReadConfigurationData();
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

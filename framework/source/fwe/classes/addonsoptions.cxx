/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <framework/addonsoptions.hxx>
#include <o3tl/safeint.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/getexpandeduri.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>

#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <vector>

//  namespaces

using namespace ::std;
using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;

#define ROOTNODE_ADDONMENU                              "Office.Addons"
constexpr OUStringLiteral PATHDELIMITER = u"/";
#define SEPARATOR_URL                                   "private:separator"

#define PROPERTYNAME_URL                                ADDONSMENUITEM_STRING_URL
#define PROPERTYNAME_TITLE                              ADDONSMENUITEM_STRING_TITLE
#define PROPERTYNAME_TARGET                             ADDONSMENUITEM_STRING_TARGET
#define PROPERTYNAME_IMAGEIDENTIFIER                    ADDONSMENUITEM_STRING_IMAGEIDENTIFIER
#define PROPERTYNAME_CONTEXT                            ADDONSMENUITEM_STRING_CONTEXT
#define PROPERTYNAME_SUBMENU                            ADDONSMENUITEM_STRING_SUBMENU

#define IMAGES_NODENAME                                 "UserDefinedImages"

// The following order is mandatory. Please add properties at the end!
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
#define INDEX_MANDATORY         11
#define INDEX_STYLE             12
#define PROPERTYCOUNT_INDEX     13

// The following order is mandatory. Please add properties at the end!
#define PROPERTYCOUNT_MENUITEM                          6
#define OFFSET_MENUITEM_URL                             0
#define OFFSET_MENUITEM_TITLE                           1
#define OFFSET_MENUITEM_IMAGEIDENTIFIER                 2
#define OFFSET_MENUITEM_TARGET                          3
#define OFFSET_MENUITEM_CONTEXT                         4
#define OFFSET_MENUITEM_SUBMENU                         5

// The following order is mandatory. Please add properties at the end!
#define PROPERTYCOUNT_POPUPMENU                         4
#define OFFSET_POPUPMENU_TITLE                          0
#define OFFSET_POPUPMENU_CONTEXT                        1
#define OFFSET_POPUPMENU_SUBMENU                        2
#define OFFSET_POPUPMENU_URL                            3   // Used for property set

// The following order is mandatory. Please add properties at the end!
#define PROPERTYCOUNT_TOOLBARITEM                       7
#define OFFSET_TOOLBARITEM_URL                          0
#define OFFSET_TOOLBARITEM_TITLE                        1
#define OFFSET_TOOLBARITEM_IMAGEIDENTIFIER              2
#define OFFSET_TOOLBARITEM_TARGET                       3
#define OFFSET_TOOLBARITEM_CONTEXT                      4
#define OFFSET_TOOLBARITEM_CONTROLTYPE                  5
#define OFFSET_TOOLBARITEM_WIDTH                        6

// The following order is mandatory. Please add properties at the end!
#define PROPERTYCOUNT_NOTEBOOKBARITEM                   8
#define OFFSET_NOTEBOOKBARITEM_URL                      0
#define OFFSET_NOTEBOOKBARITEM_TITLE                    1
#define OFFSET_NOTEBOOKBARITEM_IMAGEIDENTIFIER          2
#define OFFSET_NOTEBOOKBARITEM_TARGET                   3
#define OFFSET_NOTEBOOKBARITEM_CONTEXT                  4
#define OFFSET_NOTEBOOKBARITEM_CONTROLTYPE              5
#define OFFSET_NOTEBOOKBARITEM_WIDTH                    6
#define OFFSET_NOTEBOOKBARITEM_STYLE                    7

// The following order is mandatory. Please add properties at the end!
#define PROPERTYCOUNT_STATUSBARITEM                     8
#define OFFSET_STATUSBARITEM_URL                        0
#define OFFSET_STATUSBARITEM_TITLE                      1
#define OFFSET_STATUSBARITEM_CONTEXT                    2
#define OFFSET_STATUSBARITEM_ALIGN                      3
#define OFFSET_STATUSBARITEM_AUTOSIZE                   4
#define OFFSET_STATUSBARITEM_OWNERDRAW                  5
#define OFFSET_STATUSBARITEM_MANDATORY                  6
#define OFFSET_STATUSBARITEM_WIDTH                      7

// The following order is mandatory. Please add properties at the end!
#define PROPERTYCOUNT_IMAGES                            8
#define PROPERTYCOUNT_EMBEDDED_IMAGES                   2
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

#define PROPERTYCOUNT_MERGE_NOTEBOOKBAR                 7
#define OFFSET_MERGENOTEBOOKBAR_NOTEBOOKBAR             0
#define OFFSET_MERGENOTEBOOKBAR_MERGEPOINT              1
#define OFFSET_MERGENOTEBOOKBAR_MERGECOMMAND            2
#define OFFSET_MERGENOTEBOOKBAR_MERGECOMMANDPARAMETER   3
#define OFFSET_MERGENOTEBOOKBAR_MERGEFALLBACK           4
#define OFFSET_MERGENOTEBOOKBAR_MERGECONTEXT            5
#define OFFSET_MERGENOTEBOOKBAR_NOTEBOOKBARITEMS        6

#define PROPERTYCOUNT_MERGE_STATUSBAR                   6
#define OFFSET_MERGESTATUSBAR_MERGEPOINT                0
#define OFFSET_MERGESTATUSBAR_MERGECOMMAND              1
#define OFFSET_MERGESTATUSBAR_MERGECOMMANDPARAMETER     2
#define OFFSET_MERGESTATUSBAR_MERGEFALLBACK             3
#define OFFSET_MERGESTATUSBAR_MERGECONTEXT              4
#define OFFSET_MERGESTATUSBAR_STATUSBARITEMS            5

//  private declarations!

/*-****************************************************************************************************************
    @descr  struct to hold information about one menu entry.
****************************************************************************************************************-*/

namespace framework
{

class AddonsOptions_Impl : public ConfigItem
{

    //  public methods

    public:

        //  constructor / destructor

         AddonsOptions_Impl();
        virtual ~AddonsOptions_Impl() override;

        //  overridden methods of baseclass

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      This method is called from the ConfigManager before application ends or from the
                        PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& lPropertyNames ) override;

        //  public interface

        /*-****************************************************************************************************
            @short      base implementation of public interface for "SvtDynamicMenuOptions"!
            @descr      These class is used as static member of "SvtDynamicMenuOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!
        *//*-*****************************************************************************************************/

        bool                                            HasAddonsMenu        () const;
        sal_Int32                                       GetAddonsToolBarCount() const;
        sal_Int32                                       GetAddonsNotebookBarCount() const;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenu        () const { return m_aCachedMenuProperties;}
        const Sequence< Sequence< PropertyValue > >&    GetAddonsMenuBarPart () const { return m_aCachedMenuBarPartProperties;}
        const Sequence< Sequence< PropertyValue > >&    GetAddonsToolBarPart ( sal_uInt32 nIndex ) const;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsNotebookBarPart ( sal_uInt32 nIndex ) const;
        OUString                                        GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const;
        OUString                                        GetAddonsNotebookBarResourceName( sal_uInt32 nIndex ) const;
        const Sequence< Sequence< PropertyValue > >&    GetAddonsHelpMenu    () const { return m_aCachedHelpMenuProperties;}
        BitmapEx                                        GetImageFromURL( const OUString& aURL, bool bBig, bool bNoScale );
        const MergeMenuInstructionContainer&            GetMergeMenuInstructions() const { return m_aCachedMergeMenuInsContainer;}
        bool                                            GetMergeToolbarInstructions( const OUString& rToolbarName, MergeToolbarInstructionContainer& rToolbarInstructions ) const;
        bool                                            GetMergeNotebookBarInstructions( const OUString& rNotebookBarName, MergeNotebookBarInstructionContainer& rNotebookBarInstructions ) const;
        const MergeStatusbarInstructionContainer&       GetMergeStatusbarInstructions() const { return m_aCachedStatusbarMergingInstructions;}
        void                                            ReadConfigurationData();

    private:
        enum ImageSize
        {
            IMGSIZE_SMALL = 0,
            IMGSIZE_BIG
        };

        struct OneImageEntry
        {
            BitmapEx aScaled;   ///< cached scaled image
            BitmapEx aImage;    ///< original un-scaled image
            OUString aURL;      ///< URL in case it is not loaded yet
        };

        struct ImageEntry
        {
            // if the image is set, it was embedded in some way,
            // otherwise we use the associated URL to load on demand

            // accessed in this order
            OneImageEntry aSizeEntry[2];
            ImageEntry() {}
            void addImage(ImageSize eSize, const BitmapEx &rImage);
            void addImage(ImageSize eSize, const OUString &rURL);
        };

        typedef std::unordered_map< OUString, ImageEntry > ImageManager;
        typedef std::unordered_map< OUString, sal_uInt32 > StringToIndexMap;
        typedef std::vector< Sequence< Sequence< PropertyValue > > > AddonToolBars;
        typedef std::vector< Sequence< Sequence< PropertyValue > > > AddonNotebookBars;
        typedef std::unordered_map< OUString, MergeToolbarInstructionContainer > ToolbarMergingInstructions;
        typedef std::unordered_map< OUString, MergeNotebookBarInstructionContainer > NotebookBarMergingInstructions;

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent our module tree
            @descr      These methods return the current list of key names! We need it to get needed values from our
                        configuration management!
            @param      "nCount"     ,   returns count of menu entries for "new"
            @return     A list of configuration key names is returned.
        *//*-*****************************************************************************************************/

        void                 ReadAddonMenuSet( Sequence< Sequence< PropertyValue > >& aAddonMenuSeq );
        void                 ReadOfficeMenuBarSet( Sequence< Sequence< PropertyValue > >& aAddonOfficeMenuBarSeq );
        void                 ReadOfficeToolBarSet( AddonToolBars& rAddonOfficeToolBars, std::vector< OUString >& rAddonOfficeToolBarResNames );
        bool                 ReadToolBarItemSet( const OUString& rToolBarItemSetNodeName, Sequence< Sequence< PropertyValue > >& aAddonOfficeToolBarSeq );
        void                 ReadOfficeNotebookBarSet( AddonNotebookBars& rAddonOfficeNotebookBars, std::vector< OUString >& rAddonOfficeNotebookBarResNames );
        bool                 ReadNotebookBarItemSet( const OUString& rNotebookBarItemSetNodeName, Sequence< Sequence< PropertyValue > >& aAddonOfficeNotebookBarSeq );

        void                 ReadOfficeHelpSet( Sequence< Sequence< PropertyValue > >& aAddonOfficeHelpMenuSeq );
        void                 ReadImages( ImageManager& aImageManager );
        void                 ReadMenuMergeInstructions( MergeMenuInstructionContainer& rContainer );
        void                 ReadToolbarMergeInstructions( ToolbarMergingInstructions& rToolbarMergeMap );
        void                 ReadNotebookBarMergeInstructions( NotebookBarMergingInstructions& rNotebookBarMergeMap );
        void                 ReadStatusbarMergeInstructions( MergeStatusbarInstructionContainer& rContainer );

        void                 ReadMergeMenuData( std::u16string_view aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeMenu );
        bool                 ReadMergeToolbarData( std::u16string_view aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeToolbarItems );
        bool                 ReadMergeNotebookBarData( std::u16string_view aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeNotebookBarItems );
        bool                 ReadMergeStatusbarData( std::u16string_view aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeStatusbar );
        bool                 ReadMenuItem( std::u16string_view aMenuItemNodeName, Sequence< PropertyValue >& aMenuItem, bool bIgnoreSubMenu = false );
        bool                 ReadPopupMenu( std::u16string_view aPopupMenuNodeName, Sequence< PropertyValue >& aPopupMenu );
        void                 AppendPopupMenu( Sequence< PropertyValue >& aTargetPopupMenu, const Sequence< PropertyValue >& rSourcePopupMenu );
        bool                 ReadToolBarItem( std::u16string_view aToolBarItemNodeName, Sequence< PropertyValue >& aToolBarItem );
        bool                 ReadNotebookBarItem( std::u16string_view aNotebookBarItemNodeName, Sequence< PropertyValue >& aNotebookBarItem );

        bool                 ReadStatusBarItem( std::u16string_view aStatusbarItemNodeName, Sequence< PropertyValue >& aStatusbarItem );
        std::unique_ptr<ImageEntry> ReadImageData( std::u16string_view aImagesNodeName );
        void                 ReadAndAssociateImages( const OUString& aURL, const OUString& aImageId );
        BitmapEx             ReadImageFromURL( const OUString& aURL );
        bool                 HasAssociatedImages( const OUString& aURL );
        void                 SubstituteVariables( OUString& aURL );

        void                 ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, Sequence< Sequence< PropertyValue > >& rSubMenu );
        OUString             GeneratePrefixURL();

        Sequence< OUString > GetPropertyNamesMenuItem( std::u16string_view aPropertyRootNode )
            const;
        Sequence< OUString > GetPropertyNamesPopupMenu( std::u16string_view aPropertyRootNode )
            const;
        Sequence< OUString > GetPropertyNamesToolBarItem( std::u16string_view aPropertyRootNode )
            const;
        Sequence< OUString > GetPropertyNamesNotebookBarItem( std::u16string_view aPropertyRootNode ) const;

        Sequence< OUString > GetPropertyNamesStatusbarItem( std::u16string_view aPropertyRootNode ) const;
        Sequence< OUString > GetPropertyNamesImages( std::u16string_view aPropertyRootNode ) const;
        bool                 CreateImageFromSequence( BitmapEx& rImage, Sequence< sal_Int8 >& rBitmapDataSeq ) const;

        DECL_LINK(NotifyEvent, void*, void);

        virtual void ImplCommit() override;

    //  private member

    private:
        sal_Int32                                         m_nRootAddonPopupMenuId;
        OUString                                   m_aPropNames[PROPERTYCOUNT_INDEX];
        OUString                                   m_aPropImagesNames[PROPERTYCOUNT_IMAGES];
        OUString                                   m_aPropMergeMenuNames[PROPERTYCOUNT_MERGE_MENUBAR];
        OUString                                   m_aPropMergeToolbarNames[PROPERTYCOUNT_MERGE_TOOLBAR];
        OUString                                   m_aPropMergeNotebookBarNames[PROPERTYCOUNT_MERGE_NOTEBOOKBAR];
        OUString                                   m_aPropMergeStatusbarNames[PROPERTYCOUNT_MERGE_STATUSBAR];
        OUString                                   m_aPathDelimiter;
        OUString                                   m_aRootAddonPopupMenuURLPrexfix;
        Sequence< Sequence< PropertyValue > >             m_aCachedMenuProperties;
        Sequence< Sequence< PropertyValue > >             m_aCachedMenuBarPartProperties;
        AddonToolBars                                     m_aCachedToolBarPartProperties;
        AddonNotebookBars                                 m_aCachedNotebookBarPartProperties;
        std::vector< OUString >                      m_aCachedToolBarPartResourceNames;
        std::vector< OUString >                      m_aCachedNotebookBarPartResourceNames;
        Sequence< Sequence< PropertyValue > >             m_aCachedHelpMenuProperties;
        ImageManager                                      m_aImageManager;
        Sequence< Sequence< PropertyValue > >             m_aEmptyAddonToolBar;
        Sequence< Sequence< PropertyValue > >             m_aEmptyAddonNotebookBar;
        MergeMenuInstructionContainer                     m_aCachedMergeMenuInsContainer;
        ToolbarMergingInstructions                        m_aCachedToolbarMergingInstructions;
        NotebookBarMergingInstructions                    m_aCachedNotebookBarMergingInstructions;
        MergeStatusbarInstructionContainer                m_aCachedStatusbarMergingInstructions;
};

void AddonsOptions_Impl::ImageEntry::addImage(ImageSize eSize, const BitmapEx& rImage)
{
    aSizeEntry[static_cast<int>(eSize)].aImage = rImage;
}

void AddonsOptions_Impl::ImageEntry::addImage(ImageSize eSize, const OUString &rURL)
{
    aSizeEntry[static_cast<int>(eSize)].aURL = rURL;
}

//  constructor

AddonsOptions_Impl::AddonsOptions_Impl()
    // Init baseclasses first
    : ConfigItem( ROOTNODE_ADDONMENU ),
    m_nRootAddonPopupMenuId( 0 ),
    m_aPathDelimiter( PATHDELIMITER ),
    m_aRootAddonPopupMenuURLPrexfix( ADDONSPOPUPMENU_URL_PREFIX_STR )
{
    // initialize array with fixed property names
    m_aPropNames[ INDEX_URL             ] = PROPERTYNAME_URL;
    m_aPropNames[ INDEX_TITLE           ] = PROPERTYNAME_TITLE;
    m_aPropNames[ INDEX_TARGET          ] = PROPERTYNAME_TARGET;
    m_aPropNames[ INDEX_IMAGEIDENTIFIER ] = PROPERTYNAME_IMAGEIDENTIFIER;
    m_aPropNames[ INDEX_CONTEXT         ] = PROPERTYNAME_CONTEXT;
    m_aPropNames[ INDEX_SUBMENU         ] = PROPERTYNAME_SUBMENU; // Submenu set!
    m_aPropNames[ INDEX_CONTROLTYPE     ] = "ControlType";
    m_aPropNames[ INDEX_WIDTH           ] = "Width";
    m_aPropNames[ INDEX_ALIGN           ] = "Alignment";
    m_aPropNames[ INDEX_AUTOSIZE        ] = "AutoSize";
    m_aPropNames[ INDEX_OWNERDRAW       ] = "OwnerDraw";
    m_aPropNames[ INDEX_MANDATORY       ] = "Mandatory";
    m_aPropNames[ INDEX_STYLE           ] = "Style";

    // initialize array with fixed images property names
    m_aPropImagesNames[ OFFSET_IMAGES_SMALL         ] = "ImageSmall";
    m_aPropImagesNames[ OFFSET_IMAGES_BIG           ] = "ImageBig";
    m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC       ] = "ImageSmallHC";
    m_aPropImagesNames[ OFFSET_IMAGES_BIGHC         ] = "ImageBigHC";
    m_aPropImagesNames[ OFFSET_IMAGES_SMALL_URL     ] = "ImageSmallURL";
    m_aPropImagesNames[ OFFSET_IMAGES_BIG_URL       ] = "ImageBigURL";
    m_aPropImagesNames[ OFFSET_IMAGES_SMALLHC_URL   ] = "ImageSmallHCURL";
    m_aPropImagesNames[ OFFSET_IMAGES_BIGHC_URL     ] = "ImageBigHCURL";

    // initialize array with fixed merge menu property names
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEPOINT    ] = "MergePoint";
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMAND  ] = "MergeCommand";
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMANDPARAMETER ] = "MergeCommandParameter";
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEFALLBACK ] = "MergeFallback";
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECONTEXT  ] = "MergeContext";
    m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MENUITEMS     ] = "MenuItems";

    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBAR               ] = "MergeToolBar";
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEPOINT            ] = "MergePoint";
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMAND          ] = "MergeCommand";
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMANDPARAMETER ] = "MergeCommandParameter";
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEFALLBACK         ] = "MergeFallback";
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECONTEXT          ] = "MergeContext";
    m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBARITEMS          ] = "ToolBarItems";

    m_aPropMergeNotebookBarNames[ OFFSET_MERGENOTEBOOKBAR_NOTEBOOKBAR           ] = "MergeNotebookBar";
    m_aPropMergeNotebookBarNames[ OFFSET_MERGENOTEBOOKBAR_MERGEPOINT            ] = "MergePoint";
    m_aPropMergeNotebookBarNames[ OFFSET_MERGENOTEBOOKBAR_MERGECOMMAND          ] = "MergeCommand";
    m_aPropMergeNotebookBarNames[ OFFSET_MERGENOTEBOOKBAR_MERGECOMMANDPARAMETER ] = "MergeCommandParameter";
    m_aPropMergeNotebookBarNames[ OFFSET_MERGENOTEBOOKBAR_MERGEFALLBACK         ] = "MergeFallback";
    m_aPropMergeNotebookBarNames[ OFFSET_MERGENOTEBOOKBAR_MERGECONTEXT          ] = "MergeContext";
    m_aPropMergeNotebookBarNames[ OFFSET_MERGENOTEBOOKBAR_NOTEBOOKBARITEMS      ] = "NotebookBarItems";

    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGEPOINT            ] = "MergePoint";
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGECOMMAND          ] = "MergeCommand";
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGECOMMANDPARAMETER ] = "MergeCommandParameter";
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGEFALLBACK         ] = "MergeFallback";
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_MERGECONTEXT          ] = "MergeContext";
    m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_STATUSBARITEMS        ] = "StatusBarItems";

    ReadConfigurationData();

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!
    Sequence<OUString> aNotifySeq { "AddonUI" };
    EnableNotification( aNotifySeq );
}

//  destructor

AddonsOptions_Impl::~AddonsOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

void AddonsOptions_Impl::ReadConfigurationData()
{
    // reset members to be read again from configuration
    m_aCachedMenuProperties = Sequence< Sequence< PropertyValue > >();
    m_aCachedMenuBarPartProperties = Sequence< Sequence< PropertyValue > >();
    m_aCachedToolBarPartProperties = AddonToolBars();
    m_aCachedNotebookBarPartProperties = AddonNotebookBars();
    m_aCachedHelpMenuProperties = Sequence< Sequence< PropertyValue > >();
    m_aCachedToolBarPartResourceNames.clear();
    m_aCachedNotebookBarPartResourceNames.clear();
    m_aImageManager = ImageManager();

    ReadAddonMenuSet( m_aCachedMenuProperties );
    ReadOfficeMenuBarSet( m_aCachedMenuBarPartProperties );
    ReadOfficeToolBarSet( m_aCachedToolBarPartProperties, m_aCachedToolBarPartResourceNames );
    ReadOfficeNotebookBarSet( m_aCachedNotebookBarPartProperties, m_aCachedNotebookBarPartResourceNames );

    ReadOfficeHelpSet( m_aCachedHelpMenuProperties );
    ReadImages( m_aImageManager );

    m_aCachedMergeMenuInsContainer.clear();
    m_aCachedToolbarMergingInstructions.clear();
    m_aCachedNotebookBarMergingInstructions.clear();
    m_aCachedStatusbarMergingInstructions.clear();

    ReadMenuMergeInstructions( m_aCachedMergeMenuInsContainer );
    ReadToolbarMergeInstructions( m_aCachedToolbarMergingInstructions );
    ReadNotebookBarMergeInstructions( m_aCachedNotebookBarMergingInstructions );
    ReadStatusbarMergeInstructions( m_aCachedStatusbarMergingInstructions );
}

//  public method

void AddonsOptions_Impl::Notify( const Sequence< OUString >& /*lPropertyNames*/ )
{
    Application::PostUserEvent(LINK(this, AddonsOptions_Impl, NotifyEvent));
}

//  public method

void AddonsOptions_Impl::ImplCommit()
{
    SAL_WARN("fwk", "AddonsOptions_Impl::ImplCommit(): Not implemented yet!");
}

//  public method

bool AddonsOptions_Impl::HasAddonsMenu() const
{
    return m_aCachedMenuProperties.hasElements();
}

//  public method

sal_Int32 AddonsOptions_Impl::GetAddonsToolBarCount() const
{
    return m_aCachedToolBarPartProperties.size();
}

//  public method

sal_Int32 AddonsOptions_Impl::GetAddonsNotebookBarCount() const
{
    return m_aCachedNotebookBarPartProperties.size();
}

//  public method

const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsToolBarPart( sal_uInt32 nIndex ) const
{
    if ( /*nIndex >= 0 &&*/ nIndex < m_aCachedToolBarPartProperties.size() )
        return m_aCachedToolBarPartProperties[nIndex];
    else
        return m_aEmptyAddonToolBar;
}

//  public method

const Sequence< Sequence< PropertyValue > >& AddonsOptions_Impl::GetAddonsNotebookBarPart( sal_uInt32 nIndex ) const
{
    if ( /*nIndex >= 0 &&*/ nIndex < m_aCachedNotebookBarPartProperties.size() )
        return m_aCachedNotebookBarPartProperties[nIndex];
    else
        return m_aEmptyAddonNotebookBar;
}

//  public method

OUString AddonsOptions_Impl::GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const
{
    if ( nIndex < m_aCachedToolBarPartResourceNames.size() )
        return m_aCachedToolBarPartResourceNames[nIndex];
    else
        return OUString();
}

//  public method

OUString AddonsOptions_Impl::GetAddonsNotebookBarResourceName( sal_uInt32 nIndex ) const
{
    if ( nIndex < m_aCachedNotebookBarPartResourceNames.size() )
        return m_aCachedNotebookBarPartResourceNames[nIndex];
    else
        return OUString();
}

//  public method

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

//  public method

bool AddonsOptions_Impl::GetMergeNotebookBarInstructions(
    const OUString& rNotebookBarName,
    MergeNotebookBarInstructionContainer& rNotebookBarInstructions ) const
{
    NotebookBarMergingInstructions::const_iterator pIter = m_aCachedNotebookBarMergingInstructions.find( rNotebookBarName );
    if ( pIter != m_aCachedNotebookBarMergingInstructions.end() )
    {
        rNotebookBarInstructions = pIter->second;
        return true;
    }
    else
        return false;
}

//  public method

static BitmapEx ScaleImage( const BitmapEx &rImage, bool bBig )
{
    Size aSize = ToolBox::GetDefaultImageSize(bBig ? ToolBoxButtonSize::Large : ToolBoxButtonSize::Small);
    BitmapEx aScaleBmp(rImage);
    SAL_INFO("fwk", "Addons: expensive scale image from "
             << aScaleBmp.GetSizePixel() << " to " << aSize);
    aScaleBmp.Scale(aSize, BmpScaleFlag::BestQuality);
    return aScaleBmp;
}

BitmapEx AddonsOptions_Impl::GetImageFromURL( const OUString& aURL, bool bBig, bool bNoScale )
{
    BitmapEx aImage;

    SAL_INFO("fwk", "Expensive: Addons GetImageFromURL " << aURL <<
             " big " << (bBig?"big":"little") <<
             " scale " << (bNoScale ? "noscale" : "scale"));

    ImageManager::iterator pIter = m_aImageManager.find(aURL);
    if ( pIter != m_aImageManager.end() )
    {
        ImageSize eSize = bBig ? IMGSIZE_BIG : IMGSIZE_SMALL;
        int nIdx = static_cast<int>(eSize);
        int nOtherIdx = nIdx ? 0 : 1;

        OneImageEntry& rSizeEntry = pIter->second.aSizeEntry[nIdx];
        OneImageEntry& rOtherEntry = pIter->second.aSizeEntry[nOtherIdx];
        // actually read the image ...
        if (rSizeEntry.aImage.IsEmpty())
            rSizeEntry.aImage = ReadImageFromURL(rSizeEntry.aURL);

        if (rSizeEntry.aImage.IsEmpty())
        { // try the other size and scale it
            aImage = ScaleImage(ReadImageFromURL(rOtherEntry.aURL), bBig);
            rSizeEntry.aImage = aImage;
            if (rSizeEntry.aImage.IsEmpty())
                SAL_WARN("fwk", "failed to load addons image " << aURL);
        }

        // FIXME: bNoScale is not terribly meaningful or useful

        if (aImage.IsEmpty() && bNoScale)
            aImage = rSizeEntry.aImage;

        if (aImage.IsEmpty() && !rSizeEntry.aScaled.IsEmpty())
            aImage = rSizeEntry.aScaled;

        else // scale to the correct size for the theme / toolbox
        {
            aImage = rSizeEntry.aImage;
            if (aImage.IsEmpty()) // use and scale the other if one size is missing
                aImage = rOtherEntry.aImage;

            aImage = ScaleImage(aImage, bBig);
            rSizeEntry.aScaled = aImage; // cache for next time
        }
    }

    return aImage;
}

void AddonsOptions_Impl::ReadAddonMenuSet( Sequence< Sequence< PropertyValue > >& rAddonMenuSeq )
{
    // Read the AddonMenu set and fill property sequences
    OUString             aAddonMenuNodeName( "AddonUI/AddonMenu" );
    Sequence< OUString > aAddonMenuNodeSeq = GetNodeNames( aAddonMenuNodeName );
    OUString             aAddonMenuItemNode( aAddonMenuNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonMenuNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    // Init the property value sequence
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = m_aPropNames[ INDEX_URL            ];
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = m_aPropNames[ INDEX_TITLE          ];
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = m_aPropNames[ INDEX_TARGET         ];
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = m_aPropNames[ INDEX_CONTEXT        ];
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = m_aPropNames[ INDEX_SUBMENU        ];  // Submenu set!

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
}

void AddonsOptions_Impl::ReadOfficeHelpSet( Sequence< Sequence< PropertyValue > >& rAddonOfficeHelpMenuSeq )
{
    // Read the AddonMenu set and fill property sequences
    OUString             aAddonHelpMenuNodeName( "AddonUI/OfficeHelp" );
    Sequence< OUString > aAddonHelpMenuNodeSeq = GetNodeNames( aAddonHelpMenuNodeName );
    OUString             aAddonHelpMenuItemNode( aAddonHelpMenuNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonHelpMenuNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aMenuItem( PROPERTYCOUNT_MENUITEM );

    // Init the property value sequence
    aMenuItem[ OFFSET_MENUITEM_URL              ].Name = m_aPropNames[ INDEX_URL            ];
    aMenuItem[ OFFSET_MENUITEM_TITLE            ].Name = m_aPropNames[ INDEX_TITLE          ];
    aMenuItem[ OFFSET_MENUITEM_TARGET           ].Name = m_aPropNames[ INDEX_TARGET         ];
    aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Name = m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Name = m_aPropNames[ INDEX_CONTEXT        ];
    aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Name = m_aPropNames[ INDEX_SUBMENU        ];  // Submenu set!

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aRootMenuItemNode( aAddonHelpMenuItemNode + aAddonHelpMenuNodeSeq[n] );

        // Read the MenuItem
        if ( ReadMenuItem( aRootMenuItemNode, aMenuItem, true ) )
        {
            // Successfully read a menu item, append to our list
            sal_uInt32 nMenuItemCount = rAddonOfficeHelpMenuSeq.getLength() + 1;
            rAddonOfficeHelpMenuSeq.realloc( nMenuItemCount );
            rAddonOfficeHelpMenuSeq[nIndex++] = aMenuItem;
        }
    }
}

void AddonsOptions_Impl::ReadOfficeMenuBarSet( Sequence< Sequence< PropertyValue > >& rAddonOfficeMenuBarSeq )
{
    // Read the OfficeMenuBar set and fill property sequences
    OUString             aAddonMenuBarNodeName( "AddonUI/OfficeMenuBar" );
    Sequence< OUString > aAddonMenuBarNodeSeq = GetNodeNames( aAddonMenuBarNodeName );
    OUString             aAddonMenuBarNode( aAddonMenuBarNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonMenuBarNodeSeq.getLength();
    sal_uInt32              nIndex = 0;
    Sequence< PropertyValue > aPopupMenu( PROPERTYCOUNT_POPUPMENU );

    // Init the property value sequence
    aPopupMenu[ OFFSET_POPUPMENU_TITLE      ].Name = m_aPropNames[ INDEX_TITLE  ];
    aPopupMenu[ OFFSET_POPUPMENU_CONTEXT    ].Name = m_aPropNames[ INDEX_CONTEXT];
    aPopupMenu[ OFFSET_POPUPMENU_SUBMENU    ].Name = m_aPropNames[ INDEX_SUBMENU];
    aPopupMenu[ OFFSET_POPUPMENU_URL        ].Name = m_aPropNames[ INDEX_URL    ];

    StringToIndexMap aTitleToIndexMap;

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aPopupMenuNode( aAddonMenuBarNode + aAddonMenuBarNodeSeq[n] );

        // Read the MenuItem
        if ( ReadPopupMenu( aPopupMenuNode, aPopupMenu ) )
        {
            // Successfully read a popup menu, append to our list
            OUString aPopupTitle;
            if ( aPopupMenu[OFFSET_POPUPMENU_TITLE].Value >>= aPopupTitle )
            {
                StringToIndexMap::const_iterator pIter = aTitleToIndexMap.find( aPopupTitle );
                if ( pIter != aTitleToIndexMap.end() )
                {
                    // title already there => concat both popup menus
                    Sequence< PropertyValue >& rOldPopupMenu = rAddonOfficeMenuBarSeq[pIter->second];
                    AppendPopupMenu( rOldPopupMenu, aPopupMenu );
                }
                else
                {
                    // not found
                    sal_uInt32 nMenuItemCount = rAddonOfficeMenuBarSeq.getLength() + 1;
                    rAddonOfficeMenuBarSeq.realloc( nMenuItemCount );
                    rAddonOfficeMenuBarSeq[nIndex] = aPopupMenu;
                    aTitleToIndexMap.emplace( aPopupTitle, nIndex );
                    ++nIndex;
                }
            }
        }
    }
}

void AddonsOptions_Impl::ReadOfficeToolBarSet( AddonToolBars& rAddonOfficeToolBars, std::vector< OUString >& rAddonOfficeToolBarResNames )
{
    // Read the OfficeToolBar set and fill property sequences
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
}

bool AddonsOptions_Impl::ReadToolBarItemSet( const OUString& rToolBarItemSetNodeName, Sequence< Sequence< PropertyValue > >& rAddonOfficeToolBarSeq )
{
    sal_uInt32               nToolBarItemCount       = rAddonOfficeToolBarSeq.getLength();
    OUString                 aAddonToolBarItemSetNode( rToolBarItemSetNodeName + m_aPathDelimiter );
    Sequence< OUString >     aAddonToolBarItemSetNodeSeq = GetNodeNames( rToolBarItemSetNodeName );
    Sequence< PropertyValue >   aToolBarItem( PROPERTYCOUNT_TOOLBARITEM );

    // Init the property value sequence
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

        // Read the ToolBarItem
        if ( ReadToolBarItem( aToolBarItemNode, aToolBarItem ) )
        {
            // Successfully read a toolbar item, append to our list
            sal_uInt32 nAddonCount = rAddonOfficeToolBarSeq.getLength();
            rAddonOfficeToolBarSeq.realloc( nAddonCount+1 );
            rAddonOfficeToolBarSeq[nAddonCount] = aToolBarItem;
        }
    }

    return ( o3tl::make_unsigned(rAddonOfficeToolBarSeq.getLength()) > nToolBarItemCount );
}

void AddonsOptions_Impl::ReadOfficeNotebookBarSet(
    AddonNotebookBars& rAddonOfficeNotebookBars,
    std::vector<OUString>& rAddonOfficeNotebookBarResNames)
{
    // Read the OfficeToolBar set and fill property sequences
    OUString aAddonNotebookBarNodeName("AddonUI/OfficeNotebookBar");
    Sequence<OUString> aAddonNotebookBarNodeSeq = GetNodeNames(aAddonNotebookBarNodeName);
    OUString aAddonNotebookBarNode(aAddonNotebookBarNodeName + m_aPathDelimiter);

    sal_uInt32 nCount = aAddonNotebookBarNodeSeq.getLength();

    for (sal_uInt32 n = 0; n < nCount; n++)
    {
        OUString aNotebookBarItemNode(aAddonNotebookBarNode + aAddonNotebookBarNodeSeq[n]);
        rAddonOfficeNotebookBarResNames.push_back(aAddonNotebookBarNodeSeq[n]);
        rAddonOfficeNotebookBars.push_back(m_aEmptyAddonNotebookBar);
        ReadNotebookBarItemSet(aNotebookBarItemNode, rAddonOfficeNotebookBars[n]);
    }
}

bool AddonsOptions_Impl::ReadNotebookBarItemSet(
    const OUString& rNotebookBarItemSetNodeName,
    Sequence<Sequence<PropertyValue>>& rAddonOfficeNotebookBarSeq)
{
    sal_uInt32 nNotebookBarItemCount = rAddonOfficeNotebookBarSeq.getLength();
    OUString aAddonNotebookBarItemSetNode(rNotebookBarItemSetNodeName + m_aPathDelimiter);
    Sequence<OUString> aAddonNotebookBarItemSetNodeSeq = GetNodeNames(rNotebookBarItemSetNodeName);
    Sequence<PropertyValue> aNotebookBarItem(PROPERTYCOUNT_NOTEBOOKBARITEM);

    // Init the property value sequence
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_URL].Name = m_aPropNames[INDEX_URL];
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_TITLE].Name = m_aPropNames[INDEX_TITLE];
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_IMAGEIDENTIFIER].Name
        = m_aPropNames[INDEX_IMAGEIDENTIFIER];
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_TARGET].Name = m_aPropNames[INDEX_TARGET];
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_CONTEXT].Name = m_aPropNames[INDEX_CONTEXT];
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_CONTROLTYPE].Name = m_aPropNames[INDEX_CONTROLTYPE];
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_WIDTH].Name = m_aPropNames[INDEX_WIDTH];
    aNotebookBarItem[OFFSET_NOTEBOOKBARITEM_STYLE].Name = m_aPropNames[INDEX_STYLE];

    sal_uInt32 nCount = aAddonNotebookBarItemSetNodeSeq.getLength();
    for (sal_uInt32 n = 0; n < nCount; n++)
    {
        OUString aNotebookBarItemNode(aAddonNotebookBarItemSetNode
                                      + aAddonNotebookBarItemSetNodeSeq[n]);
        // Read the NotebookBarItem
        if (ReadNotebookBarItem(aNotebookBarItemNode, aNotebookBarItem))
        {
            // Successfully read a toolbar item, append to our list
            sal_uInt32 nAddonCount = rAddonOfficeNotebookBarSeq.getLength();
            rAddonOfficeNotebookBarSeq.realloc(nAddonCount + 1);
            rAddonOfficeNotebookBarSeq[nAddonCount] = aNotebookBarItem;
        }
    }

    return (o3tl::make_unsigned(rAddonOfficeNotebookBarSeq.getLength())
            > nNotebookBarItemCount);
}

void AddonsOptions_Impl::ReadImages( ImageManager& aImageManager )
{
    // Read the user-defined Images set and fill image manager
    OUString                aAddonImagesNodeName( "AddonUI/Images" );
    Sequence< OUString > aAddonImagesNodeSeq = GetNodeNames( aAddonImagesNodeName );
    OUString                aAddonImagesNode( aAddonImagesNodeName + m_aPathDelimiter );

    sal_uInt32              nCount = aAddonImagesNodeSeq.getLength();

    // Init the property value sequence
    Sequence< OUString > aAddonImageItemNodePropNames( 1 );
    OUString                aURL;

    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aImagesItemNode( aAddonImagesNode + aAddonImagesNodeSeq[n] );

        // Create sequence for data access
        aAddonImageItemNodePropNames[0] = aImagesItemNode +
            m_aPathDelimiter +
            m_aPropNames[ OFFSET_MENUITEM_URL ];

        Sequence< Any > aAddonImageItemNodeValues = GetProperties( aAddonImageItemNodePropNames );

        // An user-defined image entry must have a URL. As "ImageIdentifier" has a higher priority
        // we also check if we already have an images association.
        if (( aAddonImageItemNodeValues[0] >>= aURL ) &&
            !aURL.isEmpty() &&
            !HasAssociatedImages( aURL ))
        {
            OUString aImagesUserDefinedItemNode = aImagesItemNode +
                m_aPathDelimiter +
                IMAGES_NODENAME +
                m_aPathDelimiter;

            // Read a user-defined images data
            std::unique_ptr<ImageEntry> pImageEntry = ReadImageData( aImagesUserDefinedItemNode );
            if ( pImageEntry )
            {
                // Successfully read a user-defined images item, put it into our image manager
                aImageManager.emplace( aURL, std::move(*pImageEntry) );
            }
        }
    }
}

OUString AddonsOptions_Impl::GeneratePrefixURL()
{
    // Create a unique prefixed Add-On popup menu URL so it can be identified later as a runtime popup menu.
    OUString aPopupMenuURL;
    OUStringBuffer aBuf( m_aRootAddonPopupMenuURLPrexfix.getLength() + 3 );
    aBuf.append( m_aRootAddonPopupMenuURLPrexfix );
    aBuf.append( ++m_nRootAddonPopupMenuId );
    aPopupMenuURL = aBuf.makeStringAndClear();
    return aPopupMenuURL;
}

void AddonsOptions_Impl::ReadMenuMergeInstructions( MergeMenuInstructionContainer& aContainer )
{
    static const OUStringLiteral aMenuMergeRootName( u"AddonUI/OfficeMenuBarMerging/" );

    Sequence< OUString > aAddonMergeNodesSeq = GetNodeNames( aMenuMergeRootName );

    sal_uInt32           nCount = aAddonMergeNodesSeq.getLength();

    // Init the property value sequence
    Sequence< OUString > aNodePropNames( 5 );

    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        OUString aMergeAddonInstructions( aMenuMergeRootName + aAddonMergeNodesSeq[i] );

        Sequence< OUString > aAddonInstMergeNodesSeq = GetNodeNames( aMergeAddonInstructions );
        sal_uInt32           nCountAddons = aAddonInstMergeNodesSeq.getLength();

        for ( sal_uInt32 j = 0; j < nCountAddons; j++ )
        {
            OUString aMergeAddonInstructionBase =  aMergeAddonInstructions +
                m_aPathDelimiter +
                aAddonInstMergeNodesSeq[j] +
                m_aPathDelimiter;

            // Create sequence for data access
            aNodePropNames[0] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEPOINT ];

            aNodePropNames[1] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMAND ];

            aNodePropNames[2] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECOMMANDPARAMETER ];

            aNodePropNames[3] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGEFALLBACK ];

            aNodePropNames[4] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MERGECONTEXT ];

            Sequence< Any > aNodePropValues = GetProperties( aNodePropNames );

            MergeMenuInstruction aMergeMenuInstruction;
            aNodePropValues[0] >>= aMergeMenuInstruction.aMergePoint;
            aNodePropValues[1] >>= aMergeMenuInstruction.aMergeCommand;
            aNodePropValues[2] >>= aMergeMenuInstruction.aMergeCommandParameter;
            aNodePropValues[3] >>= aMergeMenuInstruction.aMergeFallback;
            aNodePropValues[4] >>= aMergeMenuInstruction.aMergeContext;

            ReadMergeMenuData( aMergeAddonInstructionBase, aMergeMenuInstruction.aMergeMenu );

            aContainer.push_back( aMergeMenuInstruction );
        }
    }
}

void AddonsOptions_Impl::ReadMergeMenuData( std::u16string_view aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeMenu )
{
    OUString aMergeMenuBaseNode( aMergeAddonInstructionBase+m_aPropMergeMenuNames[ OFFSET_MERGEMENU_MENUITEMS ] );

    Sequence< OUString > aSubMenuNodeNames = GetNodeNames( aMergeMenuBaseNode );
    aMergeMenuBaseNode += m_aPathDelimiter;

    // extend the node names to have full path strings
    for ( OUString& rName : aSubMenuNodeNames )
        rName = aMergeMenuBaseNode + rName;

    ReadSubMenuEntries( aSubMenuNodeNames, rMergeMenu );
}

void AddonsOptions_Impl::ReadToolbarMergeInstructions( ToolbarMergingInstructions& rCachedToolbarMergingInstructions )
{
    static const OUStringLiteral aToolbarMergeRootName( u"AddonUI/OfficeToolbarMerging/" );

    Sequence< OUString > aAddonMergeNodesSeq = GetNodeNames( aToolbarMergeRootName );
    sal_uInt32           nCount = aAddonMergeNodesSeq.getLength();

    // Init the property value sequence
    Sequence< OUString > aNodePropNames( 6 );

    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        OUString aMergeAddonInstructions( aToolbarMergeRootName + aAddonMergeNodesSeq[i] );

        Sequence< OUString > aAddonInstMergeNodesSeq = GetNodeNames( aMergeAddonInstructions );
        sal_uInt32           nCountAddons = aAddonInstMergeNodesSeq.getLength();

        for ( sal_uInt32 j = 0; j < nCountAddons; j++ )
        {
            OUString aMergeAddonInstructionBase = aMergeAddonInstructions +
                m_aPathDelimiter +
                aAddonInstMergeNodesSeq[j] +
                m_aPathDelimiter;

            // Create sequence for data access
            aNodePropNames[0] = aMergeAddonInstructionBase +
                m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBAR ];

            aNodePropNames[1] = aMergeAddonInstructionBase +
                m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEPOINT ];

            aNodePropNames[2] = aMergeAddonInstructionBase +
                m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMAND ];

            aNodePropNames[3] = aMergeAddonInstructionBase +
                m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECOMMANDPARAMETER ];

            aNodePropNames[4] = aMergeAddonInstructionBase +
                m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGEFALLBACK ];

            aNodePropNames[5] = aMergeAddonInstructionBase +
                m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_MERGECONTEXT ];

            Sequence< Any > aNodePropValues = GetProperties( aNodePropNames );

            MergeToolbarInstruction aMergeToolbarInstruction;
            aNodePropValues[0] >>= aMergeToolbarInstruction.aMergeToolbar;
            aNodePropValues[1] >>= aMergeToolbarInstruction.aMergePoint;
            aNodePropValues[2] >>= aMergeToolbarInstruction.aMergeCommand;
            aNodePropValues[3] >>= aMergeToolbarInstruction.aMergeCommandParameter;
            aNodePropValues[4] >>= aMergeToolbarInstruction.aMergeFallback;
            aNodePropValues[5] >>= aMergeToolbarInstruction.aMergeContext;

            ReadMergeToolbarData( aMergeAddonInstructionBase,
                                  aMergeToolbarInstruction.aMergeToolbarItems );

            MergeToolbarInstructionContainer& rVector = rCachedToolbarMergingInstructions[ aMergeToolbarInstruction.aMergeToolbar ];
            rVector.push_back( aMergeToolbarInstruction );
        }
    }
}

bool AddonsOptions_Impl::ReadMergeToolbarData( std::u16string_view aMergeAddonInstructionBase, Sequence< Sequence< PropertyValue > >& rMergeToolbarItems )
{
    OUString aMergeToolbarBaseNode = aMergeAddonInstructionBase +
        m_aPropMergeToolbarNames[ OFFSET_MERGETOOLBAR_TOOLBARITEMS ];

    return ReadToolBarItemSet( aMergeToolbarBaseNode, rMergeToolbarItems );
}

void AddonsOptions_Impl::ReadNotebookBarMergeInstructions(
    NotebookBarMergingInstructions& rCachedNotebookBarMergingInstructions)
{
    static const OUStringLiteral aNotebookBarMergeRootName(u"AddonUI/OfficeNotebookBarMerging/");

    Sequence<OUString> aAddonMergeNodesSeq = GetNodeNames(aNotebookBarMergeRootName);
    sal_uInt32 nCount = aAddonMergeNodesSeq.getLength();

    // Init the property value sequence
    Sequence<OUString> aNodePropNames(6);

    for (sal_uInt32 i = 0; i < nCount; i++)
    {
        OUString aMergeAddonInstructions(aNotebookBarMergeRootName + aAddonMergeNodesSeq[i]);

        Sequence<OUString> aAddonInstMergeNodesSeq = GetNodeNames(aMergeAddonInstructions);
        sal_uInt32 nCountAddons = aAddonInstMergeNodesSeq.getLength();

        for (sal_uInt32 j = 0; j < nCountAddons; j++)
        {
            OUString aMergeAddonInstructionBase = aMergeAddonInstructions +
                m_aPathDelimiter +
                aAddonInstMergeNodesSeq[j] +
                m_aPathDelimiter;

            // Create sequence for data access
            aNodePropNames[0] = aMergeAddonInstructionBase +
                m_aPropMergeNotebookBarNames[OFFSET_MERGENOTEBOOKBAR_NOTEBOOKBAR];

            aNodePropNames[1] = aMergeAddonInstructionBase +
                m_aPropMergeNotebookBarNames[OFFSET_MERGENOTEBOOKBAR_MERGEPOINT];

            aNodePropNames[2] = aMergeAddonInstructionBase +
                m_aPropMergeNotebookBarNames[OFFSET_MERGENOTEBOOKBAR_MERGECOMMAND];

            aNodePropNames[3] = aMergeAddonInstructionBase +
                m_aPropMergeNotebookBarNames[OFFSET_MERGENOTEBOOKBAR_MERGECOMMANDPARAMETER];

            aNodePropNames[4] = aMergeAddonInstructionBase +
                m_aPropMergeNotebookBarNames[OFFSET_MERGENOTEBOOKBAR_MERGEFALLBACK];

            aNodePropNames[5] = aMergeAddonInstructionBase +
                m_aPropMergeNotebookBarNames[OFFSET_MERGENOTEBOOKBAR_MERGECONTEXT];

            Sequence<Any> aNodePropValues = GetProperties(aNodePropNames);

            MergeNotebookBarInstruction aMergeNotebookBarInstruction;
            aNodePropValues[0] >>= aMergeNotebookBarInstruction.aMergeNotebookBar;
            aNodePropValues[1] >>= aMergeNotebookBarInstruction.aMergePoint;
            aNodePropValues[2] >>= aMergeNotebookBarInstruction.aMergeCommand;
            aNodePropValues[3] >>= aMergeNotebookBarInstruction.aMergeCommandParameter;
            aNodePropValues[4] >>= aMergeNotebookBarInstruction.aMergeFallback;
            aNodePropValues[5] >>= aMergeNotebookBarInstruction.aMergeContext;

            ReadMergeNotebookBarData(aMergeAddonInstructionBase,
                                     aMergeNotebookBarInstruction.aMergeNotebookBarItems);

            MergeNotebookBarInstructionContainer& rVector
                = rCachedNotebookBarMergingInstructions[aMergeNotebookBarInstruction
                                                            .aMergeNotebookBar];
            rVector.push_back(aMergeNotebookBarInstruction);
        }
    }
}

bool AddonsOptions_Impl::ReadMergeNotebookBarData(
    std::u16string_view aMergeAddonInstructionBase,
    Sequence<Sequence<PropertyValue>>& rMergeNotebookBarItems)
{
    OUString aMergeNotebookBarBaseNode = aMergeAddonInstructionBase +
        m_aPropMergeNotebookBarNames[OFFSET_MERGENOTEBOOKBAR_NOTEBOOKBARITEMS];

    return ReadNotebookBarItemSet(aMergeNotebookBarBaseNode, rMergeNotebookBarItems);
}

void AddonsOptions_Impl::ReadStatusbarMergeInstructions( MergeStatusbarInstructionContainer& aContainer )
{
    static const OUStringLiteral aStatusbarMergeRootName( u"AddonUI/OfficeStatusbarMerging/" );

    Sequence< OUString > aAddonMergeNodesSeq = GetNodeNames( aStatusbarMergeRootName );
    sal_uInt32  nCount = aAddonMergeNodesSeq.getLength();

    Sequence< OUString > aNodePropNames( 5 );

    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        OUString aMergeAddonInstructions( aStatusbarMergeRootName + aAddonMergeNodesSeq[i] );

        Sequence< OUString > aAddonInstMergeNodesSeq = GetNodeNames( aMergeAddonInstructions );
        sal_uInt32 nCountAddons = aAddonInstMergeNodesSeq.getLength();

        for ( sal_uInt32 j = 0; j < nCountAddons; j++ )
        {
            OUString aMergeAddonInstructionBase = aMergeAddonInstructions +
                m_aPathDelimiter +
                aAddonInstMergeNodesSeq[j] +
                m_aPathDelimiter;

            // Create sequence for data access
            aNodePropNames[0] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGEPOINT ];

            aNodePropNames[1] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGECOMMAND ];

            aNodePropNames[2] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGECOMMANDPARAMETER ];

            aNodePropNames[3] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGEFALLBACK ];

            aNodePropNames[4] = aMergeAddonInstructionBase +
                m_aPropMergeMenuNames[ OFFSET_MERGESTATUSBAR_MERGECONTEXT ];

            Sequence< Any > aNodePropValues = GetProperties( aNodePropNames );

            MergeStatusbarInstruction aMergeStatusbarInstruction;
            aNodePropValues[0] >>= aMergeStatusbarInstruction.aMergePoint;
            aNodePropValues[1] >>= aMergeStatusbarInstruction.aMergeCommand;
            aNodePropValues[2] >>= aMergeStatusbarInstruction.aMergeCommandParameter;
            // aNodePropValues[3] >>= aMergeStatusbarInstruction.aMergeFallback;
            aNodePropValues[4] >>= aMergeStatusbarInstruction.aMergeContext;

            ReadMergeStatusbarData( aMergeAddonInstructionBase,
                                    aMergeStatusbarInstruction.aMergeStatusbarItems );

            aContainer.push_back( aMergeStatusbarInstruction );
        }
    }
}

bool AddonsOptions_Impl::ReadMergeStatusbarData(
    std::u16string_view aMergeAddonInstructionBase,
    Sequence< Sequence< PropertyValue > >& rMergeStatusbarItems )
{
    sal_uInt32 nStatusbarItemCount = rMergeStatusbarItems.getLength();

    OUString aMergeStatusbarBaseNode = aMergeAddonInstructionBase +
        m_aPropMergeStatusbarNames[ OFFSET_MERGESTATUSBAR_STATUSBARITEMS ];

    OUString aAddonStatusbarItemSetNode( aMergeStatusbarBaseNode + m_aPathDelimiter );
    Sequence< OUString > aAddonStatusbarItemSetNodeSeq = GetNodeNames( aMergeStatusbarBaseNode );

    Sequence< PropertyValue > aStatusbarItem( PROPERTYCOUNT_STATUSBARITEM );
    aStatusbarItem[ OFFSET_STATUSBARITEM_URL       ].Name = m_aPropNames[ INDEX_URL       ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_TITLE     ].Name = m_aPropNames[ INDEX_TITLE     ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_CONTEXT   ].Name = m_aPropNames[ INDEX_CONTEXT   ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_ALIGN     ].Name = m_aPropNames[ INDEX_ALIGN     ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_AUTOSIZE  ].Name = m_aPropNames[ INDEX_AUTOSIZE  ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_OWNERDRAW ].Name = m_aPropNames[ INDEX_OWNERDRAW ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_MANDATORY ].Name = m_aPropNames[ INDEX_MANDATORY ];
    aStatusbarItem[ OFFSET_STATUSBARITEM_WIDTH     ].Name = m_aPropNames[ INDEX_WIDTH     ];

    sal_uInt32 nCount = aAddonStatusbarItemSetNodeSeq.getLength();
    for ( sal_uInt32 n = 0; n < nCount; n++ )
    {
        OUString aStatusbarItemNode( aAddonStatusbarItemSetNode + aAddonStatusbarItemSetNodeSeq[n] );

        if ( ReadStatusBarItem( aStatusbarItemNode, aStatusbarItem ) )
        {
            sal_uInt32 nAddonCount = rMergeStatusbarItems.getLength();
            rMergeStatusbarItems.realloc( nAddonCount+1 );
            rMergeStatusbarItems[nAddonCount] = aStatusbarItem;
        }
    }

    return ( o3tl::make_unsigned(rMergeStatusbarItems.getLength()) > nStatusbarItemCount );
}

bool AddonsOptions_Impl::ReadStatusBarItem(
    std::u16string_view aStatusarItemNodeName,
    Sequence< PropertyValue >& aStatusbarItem )
{
    bool bResult( false );
    OUString aURL;
    OUString aAddonStatusbarItemTreeNode( aStatusarItemNodeName + m_aPathDelimiter );

    Sequence< Any > aStatusbarItemNodePropValues = GetProperties( GetPropertyNamesStatusbarItem( aAddonStatusbarItemTreeNode ) );

    // Command URL is required
    if (( aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_URL ] >>= aURL ) && aURL.getLength() > 0 )
    {
        aStatusbarItem[ OFFSET_STATUSBARITEM_URL        ].Value <<= aURL;
        aStatusbarItem[ OFFSET_STATUSBARITEM_TITLE      ].Value = aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_TITLE     ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_CONTEXT    ].Value = aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_CONTEXT   ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_ALIGN      ].Value = aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_ALIGN     ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_AUTOSIZE   ].Value = aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_AUTOSIZE  ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_OWNERDRAW  ].Value = aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_OWNERDRAW ];
        aStatusbarItem[ OFFSET_STATUSBARITEM_MANDATORY  ].Value = aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_MANDATORY ];

        // Configuration uses hyper for long. Therefore transform into sal_Int32
        sal_Int64 nValue( 0 );
        aStatusbarItemNodePropValues[ OFFSET_STATUSBARITEM_WIDTH ] >>= nValue;
        aStatusbarItem[ OFFSET_STATUSBARITEM_WIDTH ].Value <<= sal_Int32( nValue );

        bResult = true;
    }

    return bResult;
}

bool AddonsOptions_Impl::ReadMenuItem( std::u16string_view aMenuNodeName, Sequence< PropertyValue >& aMenuItem, bool bIgnoreSubMenu )
{
    bool             bResult = false;
    OUString         aStrValue;
    OUString         aAddonMenuItemTreeNode( aMenuNodeName + m_aPathDelimiter );

    Sequence< Any >  aMenuItemNodePropValues = GetProperties( GetPropertyNamesMenuItem( aAddonMenuItemTreeNode ) );
    if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_TITLE ] >>= aStrValue ) && !aStrValue.isEmpty() )
    {
        aMenuItem[ OFFSET_MENUITEM_TITLE ].Value <<= aStrValue;

        OUString aRootSubMenuName( aAddonMenuItemTreeNode + m_aPropNames[ INDEX_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.hasElements() && !bIgnoreSubMenu )
        {
            // Set a unique prefixed Add-On popup menu URL so it can be identified later
            OUString aPopupMenuURL     = GeneratePrefixURL();
            OUString aPopupMenuImageId;

            aMenuItemNodePropValues[ OFFSET_MENUITEM_IMAGEIDENTIFIER ] >>= aPopupMenuImageId;
            ReadAndAssociateImages( aPopupMenuURL, aPopupMenuImageId );

            // A popup menu must have a title and can have a URL and ImageIdentifier
            // Set the other property values to empty
            aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aPopupMenuURL;
            aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= OUString();
            aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= aPopupMenuImageId;
            aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value = aMenuItemNodePropValues[ OFFSET_MENUITEM_CONTEXT ];

            // Continue to read the sub menu nodes
            Sequence< Sequence< PropertyValue > > aSubMenuSeq;
            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( OUString& rName : aRootSubMenuNodeNames )
                rName = aSubMenuRootNodeName + rName;
            ReadSubMenuEntries( aRootSubMenuNodeNames, aSubMenuSeq );
            aMenuItem[ OFFSET_MENUITEM_SUBMENU ].Value <<= aSubMenuSeq;
            bResult = true;
        }
        else if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_URL ] >>= aStrValue ) && !aStrValue.isEmpty() )
        {
            // A simple menu item => read the other properties;
            OUString aMenuImageId;

            aMenuItemNodePropValues[ OFFSET_MENUITEM_IMAGEIDENTIFIER ] >>= aMenuImageId;
            ReadAndAssociateImages( aStrValue, aMenuImageId );

            aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aStrValue;
            aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value = aMenuItemNodePropValues[ OFFSET_MENUITEM_TARGET         ];
            aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= aMenuImageId;
            aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value = aMenuItemNodePropValues[ OFFSET_MENUITEM_CONTEXT        ];
            aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Value <<= Sequence< Sequence< PropertyValue > >(); // Submenu set!

            bResult = true;
        }
    }
    else if (( aMenuItemNodePropValues[ OFFSET_MENUITEM_URL ] >>= aStrValue ) &&
              aStrValue == SEPARATOR_URL )
    {
        // Separator
        aMenuItem[ OFFSET_MENUITEM_URL              ].Value <<= aStrValue;
        aMenuItem[ OFFSET_MENUITEM_TARGET           ].Value <<= OUString();
        aMenuItem[ OFFSET_MENUITEM_IMAGEIDENTIFIER  ].Value <<= OUString();
        aMenuItem[ OFFSET_MENUITEM_CONTEXT          ].Value <<= OUString();
        aMenuItem[ OFFSET_MENUITEM_SUBMENU          ].Value <<= Sequence< Sequence< PropertyValue > >(); // Submenu set!
        bResult = true;
    }

    return bResult;
}

bool AddonsOptions_Impl::ReadPopupMenu( std::u16string_view aPopupMenuNodeName, Sequence< PropertyValue >& aPopupMenu )
{
    bool             bResult = false;
    OUString         aStrValue;
    OUString         aAddonPopupMenuTreeNode( aPopupMenuNodeName + m_aPathDelimiter );

    Sequence< Any >  aPopupMenuNodePropValues = GetProperties( GetPropertyNamesPopupMenu( aAddonPopupMenuTreeNode ) );
    if (( aPopupMenuNodePropValues[ OFFSET_POPUPMENU_TITLE ] >>= aStrValue ) &&
         !aStrValue.isEmpty() )
    {
        aPopupMenu[ OFFSET_POPUPMENU_TITLE ].Value <<= aStrValue;

        OUString aRootSubMenuName( aAddonPopupMenuTreeNode + m_aPropNames[ INDEX_SUBMENU ] );
        Sequence< OUString > aRootSubMenuNodeNames = GetNodeNames( aRootSubMenuName );
        if ( aRootSubMenuNodeNames.hasElements() )
        {
            // A top-level popup menu needs a title
            // Set a unique prefixed Add-On popup menu URL so it can be identified later
            OUString aPopupMenuURL = GeneratePrefixURL();

            aPopupMenu[ OFFSET_POPUPMENU_URL        ].Value <<= aPopupMenuURL;
            aPopupMenu[ OFFSET_POPUPMENU_CONTEXT    ].Value = aPopupMenuNodePropValues[ OFFSET_POPUPMENU_CONTEXT ];

            // Continue to read the sub menu nodes
            Sequence< Sequence< PropertyValue > > aSubMenuSeq;
            OUString aSubMenuRootNodeName( aRootSubMenuName + m_aPathDelimiter );
            for ( OUString& rName : aRootSubMenuNodeNames )
                rName = aSubMenuRootNodeName + rName;
            ReadSubMenuEntries( aRootSubMenuNodeNames, aSubMenuSeq );
            aPopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value <<= aSubMenuSeq;
            bResult = true;
        }
    }

    return bResult;
}

void AddonsOptions_Impl::AppendPopupMenu( Sequence< PropertyValue >& rTargetPopupMenu, const Sequence< PropertyValue >& rSourcePopupMenu )
{
    Sequence< Sequence< PropertyValue > > aTargetSubMenuSeq;
    Sequence< Sequence< PropertyValue > > aSourceSubMenuSeq;

    if (( rTargetPopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value >>= aTargetSubMenuSeq ) &&
        ( rSourcePopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value >>= aSourceSubMenuSeq ))
    {
        sal_uInt32 nIndex = aTargetSubMenuSeq.getLength();
        aTargetSubMenuSeq.realloc( nIndex + aSourceSubMenuSeq.getLength() );
        for ( Sequence<PropertyValue> const & rSeq : std::as_const(aSourceSubMenuSeq) )
            aTargetSubMenuSeq[nIndex++] = rSeq;
        rTargetPopupMenu[ OFFSET_POPUPMENU_SUBMENU ].Value <<= aTargetSubMenuSeq;
    }
}

bool AddonsOptions_Impl::ReadToolBarItem( std::u16string_view aToolBarItemNodeName, Sequence< PropertyValue >& aToolBarItem )
{
    bool             bResult = false;
    OUString         aURL;
    OUString         aAddonToolBarItemTreeNode( aToolBarItemNodeName + m_aPathDelimiter );

    Sequence< Any >  aToolBarItemNodePropValues = GetProperties( GetPropertyNamesToolBarItem( aAddonToolBarItemTreeNode ) );

    // A toolbar item must have a command URL
    if (( aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_URL ] >>= aURL ) && !aURL.isEmpty() )
    {
        OUString         aTitle;
        if ( aURL == SEPARATOR_URL )
        {
            // A separator toolbar item only needs a URL
            aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= aURL;
            aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= OUString();
            aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value <<= OUString();
            aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= OUString();
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value <<= OUString();
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTROLTYPE        ].Value <<= OUString();
            aToolBarItem[ OFFSET_TOOLBARITEM_WIDTH              ].Value <<= sal_Int32( 0 );

            bResult = true;
        }
        else if (( aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_TITLE ] >>= aTitle ) && !aTitle.isEmpty() )
        {
            // A normal toolbar item must also have title => read the other properties;
            OUString aImageId;

            // Try to map a user-defined image URL to our internal private image URL
            aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER ] >>= aImageId;
            ReadAndAssociateImages( aURL, aImageId );

            aToolBarItem[ OFFSET_TOOLBARITEM_URL                ].Value <<= aURL;
            aToolBarItem[ OFFSET_TOOLBARITEM_TITLE              ].Value <<= aTitle;
            aToolBarItem[ OFFSET_TOOLBARITEM_TARGET             ].Value = aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_TARGET      ];
            aToolBarItem[ OFFSET_TOOLBARITEM_IMAGEIDENTIFIER    ].Value <<= aImageId;
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTEXT            ].Value = aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_CONTEXT     ];
            aToolBarItem[ OFFSET_TOOLBARITEM_CONTROLTYPE        ].Value = aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_CONTROLTYPE ];

            // Configuration uses hyper for long. Therefore transform into sal_Int32
            sal_Int64 nValue( 0 );
            aToolBarItemNodePropValues[ OFFSET_TOOLBARITEM_WIDTH ] >>= nValue;
            aToolBarItem[ OFFSET_TOOLBARITEM_WIDTH              ].Value <<= sal_Int32( nValue );

            bResult = true;
        }
    }

    return bResult;
}

bool AddonsOptions_Impl::ReadNotebookBarItem( std::u16string_view aNotebookBarItemNodeName, Sequence< PropertyValue >& aNotebookBarItem )
{
    bool             bResult = false;
    OUString         aURL;
    OUString         aAddonNotebookBarItemTreeNode( aNotebookBarItemNodeName + m_aPathDelimiter );

    Sequence< Any >  aNotebookBarItemNodePropValues = GetProperties( GetPropertyNamesNotebookBarItem( aAddonNotebookBarItemTreeNode ) );

    // A toolbar item must have a command URL
    if (( aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_URL ] >>= aURL ) && !aURL.isEmpty() )
    {
        OUString         aTitle;
        if ( aURL == SEPARATOR_URL )
        {
            // A separator toolbar item only needs a URL
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_URL                ].Value <<= aURL;
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_TITLE              ].Value <<= OUString();
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_TARGET             ].Value <<= OUString();
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_IMAGEIDENTIFIER    ].Value <<= OUString();
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_CONTEXT            ].Value <<= OUString();
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_CONTROLTYPE        ].Value <<= OUString();
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_WIDTH              ].Value <<= sal_Int32( 0 );
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_STYLE              ].Value <<= OUString();

            bResult = true;
        }
        else if (( aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_TITLE ] >>= aTitle ) && !aTitle.isEmpty() )
        {
            // A normal toolbar item must also have title => read the other properties;
            OUString aImageId;

            // Try to map a user-defined image URL to our internal private image URL
            aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_IMAGEIDENTIFIER ] >>= aImageId;
            ReadAndAssociateImages( aURL, aImageId );

            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_URL                ].Value <<= aURL;
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_TITLE              ].Value <<= aTitle;
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_TARGET             ].Value = aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_TARGET      ];
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_IMAGEIDENTIFIER    ].Value <<= aImageId;
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_CONTEXT            ].Value = aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_CONTEXT     ];
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_CONTROLTYPE        ].Value = aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_CONTROLTYPE ];

            // Configuration uses hyper for long. Therefore transform into sal_Int32
            sal_Int64 nValue( 0 );
            aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_WIDTH ] >>= nValue;
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_WIDTH              ].Value <<= sal_Int32( nValue );
            aNotebookBarItem[ OFFSET_NOTEBOOKBARITEM_STYLE              ].Value = aNotebookBarItemNodePropValues[ OFFSET_NOTEBOOKBARITEM_STYLE ];

            bResult = true;
        }
    }

    return bResult;
}

void AddonsOptions_Impl::ReadSubMenuEntries( const Sequence< OUString >& aSubMenuNodeNames, Sequence< Sequence< PropertyValue > >& rSubMenuSeq )
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
}

bool AddonsOptions_Impl::HasAssociatedImages( const OUString& aURL )
{
    // FIXME: potentially this is not so useful in a world of delayed image loading
    ImageManager::const_iterator pIter = m_aImageManager.find( aURL );
    return ( pIter != m_aImageManager.end() );
}

void AddonsOptions_Impl::SubstituteVariables( OUString& aURL )
{
    aURL = comphelper::getExpandedUri(
        comphelper::getProcessComponentContext(), aURL);
}

BitmapEx AddonsOptions_Impl::ReadImageFromURL(const OUString& aImageURL)
{
    BitmapEx aImage;

    std::unique_ptr<SvStream> pStream = UcbStreamHelper::CreateStream( aImageURL, StreamMode::STD_READ );
    if ( pStream && ( pStream->GetErrorCode() == ERRCODE_NONE ))
    {
        // Use graphic class to also support more graphic formats (bmp,png,...)
        Graphic aGraphic;

        GraphicFilter& rGF = GraphicFilter::GetGraphicFilter();
        rGF.ImportGraphic( aGraphic, OUString(), *pStream );

        BitmapEx aBitmapEx = aGraphic.GetBitmapEx();

        Size aBmpSize = aBitmapEx.GetSizePixel();
        if ( !aBmpSize.IsEmpty() )
        {
            // Support non-transparent bitmaps to be downward compatible with OOo 1.1.x addons
            if( !aBitmapEx.IsAlpha() )
                aBitmapEx = BitmapEx( aBitmapEx.GetBitmap(), COL_LIGHTMAGENTA );

            aImage = aBitmapEx;
        }
    }

    return aImage;
}

void AddonsOptions_Impl::ReadAndAssociateImages( const OUString& aURL, const OUString& aImageId )
{
    if ( aImageId.isEmpty() )
        return;

    ImageEntry  aImageEntry;
    OUString    aImageURL( aImageId );

    SubstituteVariables( aImageURL );

    // Loop to create the two possible image names and try to read the bitmap files
    static const char* aExtArray[] = { "_16", "_26" };
    for ( size_t i = 0; i < SAL_N_ELEMENTS(aExtArray); i++ )
    {
        OUStringBuffer aFileURL( aImageURL );
        aFileURL.appendAscii( aExtArray[i] );
        aFileURL.append( ".bmp" );

        aImageEntry.addImage( !i ? IMGSIZE_SMALL : IMGSIZE_BIG, aFileURL.makeStringAndClear() );
    }

    m_aImageManager.emplace( aURL, aImageEntry );
}

std::unique_ptr<AddonsOptions_Impl::ImageEntry> AddonsOptions_Impl::ReadImageData( std::u16string_view aImagesNodeName )
{
    Sequence< OUString > aImageDataNodeNames = GetPropertyNamesImages( aImagesNodeName );
    Sequence< Any >      aPropertyData;
    Sequence< sal_Int8 > aImageDataSeq;
    OUString             aImageURL;

    std::unique_ptr<ImageEntry> pEntry;

    // It is possible to use both forms (embedded image data and URLs to external bitmap files) at the
    // same time. Embedded image data has a higher priority.
    aPropertyData = GetProperties( aImageDataNodeNames );
    for ( int i = 0; i < PROPERTYCOUNT_IMAGES; i++ )
    {
        if ( i < PROPERTYCOUNT_EMBEDDED_IMAGES )
        {
            // Extract image data from the embedded hex binary sequence
            BitmapEx aImage;
            if (( aPropertyData[i] >>= aImageDataSeq ) &&
                aImageDataSeq.hasElements() &&
                ( CreateImageFromSequence( aImage, aImageDataSeq ) ) )
            {
                if ( !pEntry )
                    pEntry.reset(new ImageEntry);
                pEntry->addImage(i == OFFSET_IMAGES_SMALL ? IMGSIZE_SMALL : IMGSIZE_BIG, aImage);
            }
        }
        else if ( i == OFFSET_IMAGES_SMALL_URL || i == OFFSET_IMAGES_BIG_URL )
        {
            if(!pEntry)
                pEntry.reset(new ImageEntry());

            // Retrieve image data from an external bitmap file. Make sure that embedded image data
            // has a higher priority.
            if (aPropertyData[i] >>= aImageURL)
            {
                SubstituteVariables(aImageURL);
                pEntry->addImage(i == OFFSET_IMAGES_SMALL_URL ? IMGSIZE_SMALL : IMGSIZE_BIG, aImageURL);
            }
        }
    }

    return pEntry;
}

bool AddonsOptions_Impl::CreateImageFromSequence( BitmapEx& rImage, Sequence< sal_Int8 >& rBitmapDataSeq ) const
{
    bool bResult = false;

    if ( rBitmapDataSeq.hasElements() )
    {
        SvMemoryStream  aMemStream( rBitmapDataSeq.getArray(), rBitmapDataSeq.getLength(), StreamMode::STD_READ );

        ReadDIBBitmapEx(rImage, aMemStream);

        if( !rImage.IsAlpha() )
        {
            // Support non-transparent bitmaps to be downward compatible with OOo 1.1.x addons
            rImage = BitmapEx( rImage.GetBitmap(), COL_LIGHTMAGENTA );
        }

        bResult = true;
    }

    return bResult;
}

Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesMenuItem( std::u16string_view aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_MENUITEM );

    // Create property names dependent from the root node name
    lResult[OFFSET_MENUITEM_URL]             = aPropertyRootNode + m_aPropNames[ INDEX_URL          ];
    lResult[OFFSET_MENUITEM_TITLE]           = aPropertyRootNode + m_aPropNames[ INDEX_TITLE            ];
    lResult[OFFSET_MENUITEM_IMAGEIDENTIFIER] = aPropertyRootNode + m_aPropNames[ INDEX_IMAGEIDENTIFIER ];
    lResult[OFFSET_MENUITEM_TARGET]          = aPropertyRootNode + m_aPropNames[ INDEX_TARGET           ];
    lResult[OFFSET_MENUITEM_CONTEXT]         = aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT      ];
    lResult[OFFSET_MENUITEM_SUBMENU]         = aPropertyRootNode + m_aPropNames[ INDEX_SUBMENU      ];

    return lResult;
}

Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesPopupMenu( std::u16string_view aPropertyRootNode ) const
{
    // The URL is automatically set and not read from the configuration.
    Sequence< OUString > lResult( PROPERTYCOUNT_POPUPMENU-1 );

    // Create property names dependent from the root node name
    lResult[OFFSET_POPUPMENU_TITLE]   = aPropertyRootNode + m_aPropNames[ INDEX_TITLE  ];
    lResult[OFFSET_POPUPMENU_CONTEXT] = aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT    ];
    lResult[OFFSET_POPUPMENU_SUBMENU] = aPropertyRootNode + m_aPropNames[ INDEX_SUBMENU    ];

    return lResult;
}

Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesToolBarItem( std::u16string_view aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_TOOLBARITEM );

    // Create property names dependent from the root node name
    lResult[0] = aPropertyRootNode + m_aPropNames[ INDEX_URL             ];
    lResult[1] = aPropertyRootNode + m_aPropNames[ INDEX_TITLE       ];
    lResult[2] = aPropertyRootNode + m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    lResult[3] = aPropertyRootNode + m_aPropNames[ INDEX_TARGET          ];
    lResult[4] = aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT         ];
    lResult[5] = aPropertyRootNode + m_aPropNames[ INDEX_CONTROLTYPE     ];
    lResult[6] = aPropertyRootNode + m_aPropNames[ INDEX_WIDTH       ];

    return lResult;
}

Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesNotebookBarItem( std::u16string_view aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_NOTEBOOKBARITEM );

    // Create property names dependent from the root node name
    lResult[0] = aPropertyRootNode + m_aPropNames[ INDEX_URL             ];
    lResult[1] = aPropertyRootNode + m_aPropNames[ INDEX_TITLE       ];
    lResult[2] = aPropertyRootNode + m_aPropNames[ INDEX_IMAGEIDENTIFIER];
    lResult[3] = aPropertyRootNode + m_aPropNames[ INDEX_TARGET          ];
    lResult[4] = aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT         ];
    lResult[5] = aPropertyRootNode + m_aPropNames[ INDEX_CONTROLTYPE     ];
    lResult[6] = aPropertyRootNode + m_aPropNames[ INDEX_WIDTH       ];
    lResult[7] = aPropertyRootNode + m_aPropNames[ INDEX_STYLE       ];

    return lResult;
}

Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesStatusbarItem(
    std::u16string_view aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_STATUSBARITEM );

    lResult[0] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_URL       ] );
    lResult[1] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_TITLE     ] );
    lResult[2] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_CONTEXT   ] );
    lResult[3] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_ALIGN     ] );
    lResult[4] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_AUTOSIZE  ] );
    lResult[5] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_OWNERDRAW ] );
    lResult[6] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_MANDATORY ] );
    lResult[7] = OUString( aPropertyRootNode + m_aPropNames[ INDEX_WIDTH     ] );

    return lResult;
}

Sequence< OUString > AddonsOptions_Impl::GetPropertyNamesImages( std::u16string_view aPropertyRootNode ) const
{
    Sequence< OUString > lResult( PROPERTYCOUNT_IMAGES );

    // Create property names dependent from the root node name
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

namespace{
    //global
    std::weak_ptr<AddonsOptions_Impl> g_pAddonsOptions;
}

AddonsOptions::AddonsOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl = g_pAddonsOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<AddonsOptions_Impl>();
        g_pAddonsOptions = m_pImpl;
    }
}

AddonsOptions::~AddonsOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl.reset();
}

//  public method

bool AddonsOptions::HasAddonsMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->HasAddonsMenu();
}

//  public method

sal_Int32 AddonsOptions::GetAddonsToolBarCount() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsToolBarCount();
}

//  public method

sal_Int32 AddonsOptions::GetAddonsNotebookBarCount() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsNotebookBarCount();
}

//  public method

const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsMenu();
}

//  public method

const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsMenuBarPart() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsMenuBarPart();
}

//  public method

const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsToolBarPart( sal_uInt32 nIndex ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsToolBarPart( nIndex );
}

//  public method

const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsNotebookBarPart( sal_uInt32 nIndex ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsNotebookBarPart( nIndex );
}

//  public method

OUString AddonsOptions::GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsToolbarResourceName( nIndex );
}

//  public method

OUString AddonsOptions::GetAddonsNotebookBarResourceName( sal_uInt32 nIndex ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsNotebookBarResourceName( nIndex );
}

//  public method

const Sequence< Sequence< PropertyValue > >& AddonsOptions::GetAddonsHelpMenu() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetAddonsHelpMenu();
}

//  public method

const MergeMenuInstructionContainer& AddonsOptions::GetMergeMenuInstructions() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMergeMenuInstructions();
}

//  public method

bool AddonsOptions::GetMergeToolbarInstructions(
    const OUString& rToolbarName,
    MergeToolbarInstructionContainer& rToolbarInstructions ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMergeToolbarInstructions(
        rToolbarName, rToolbarInstructions );
}

//  public method

bool AddonsOptions::GetMergeNotebookBarInstructions(
    const OUString& rNotebookBarName,
    MergeNotebookBarInstructionContainer& rNotebookBarInstructions ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMergeNotebookBarInstructions(
        rNotebookBarName, rNotebookBarInstructions );
}

//public method

const MergeStatusbarInstructionContainer& AddonsOptions::GetMergeStatusbarInstructions() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMergeStatusbarInstructions();
}

//  public method

BitmapEx AddonsOptions::GetImageFromURL( const OUString& aURL, bool bBig, bool bNoScale ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetImageFromURL( aURL, bBig, bNoScale );
}

//  public method

BitmapEx AddonsOptions::GetImageFromURL( const OUString& aURL, bool bBig ) const
{
    return GetImageFromURL( aURL, bBig, false );
}

Mutex& AddonsOptions::GetOwnStaticMutex()
{
    // Create static mutex variable.
    static Mutex ourMutex;

    return ourMutex;
}

IMPL_LINK_NOARG(AddonsOptions_Impl, NotifyEvent, void*, void)
{
    MutexGuard aGuard(AddonsOptions::GetOwnStaticMutex());
    ReadConfigurationData();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

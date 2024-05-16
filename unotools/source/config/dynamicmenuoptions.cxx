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

#include <sal/config.h>

#include <o3tl/string_view.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <tools/debug.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>
#include <algorithm>
#include <string_view>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

constexpr OUStringLiteral DYNAMICMENU_PROPERTYNAME_URL = u"URL";
constexpr OUStringLiteral DYNAMICMENU_PROPERTYNAME_TITLE = u"Title";
constexpr OUStringLiteral DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER = u"ImageIdentifier";
constexpr OUStringLiteral DYNAMICMENU_PROPERTYNAME_TARGETNAME = u"TargetName";

constexpr OUString PATHDELIMITER = u"/"_ustr;

constexpr OUString SETNODE_NEWMENU = u"New"_ustr;
constexpr OUString SETNODE_WIZARDMENU = u"Wizard"_ustr;

#define PROPERTYNAME_URL                                DYNAMICMENU_PROPERTYNAME_URL
#define PROPERTYNAME_TITLE                              DYNAMICMENU_PROPERTYNAME_TITLE
#define PROPERTYNAME_IMAGEIDENTIFIER                    DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER
#define PROPERTYNAME_TARGETNAME                         DYNAMICMENU_PROPERTYNAME_TARGETNAME

#define PROPERTYCOUNT                                   4

constexpr std::u16string_view PATHPREFIX_SETUP = u"m";

namespace
{
/*-****************************************************************************************************************
    @descr  support simple menu structures and operations on it
****************************************************************************************************************-*/
struct SvtDynMenu
{
    // append setup written menu entry
    // Don't touch name of entry. It was defined by setup and must be the same every time!
    // Look for double menu entries here too... may be some separator items are superfluous...
    void AppendSetupEntry( const SvtDynMenuEntry& rEntry )
    {
        if( lSetupEntries.empty() || lSetupEntries.rbegin()->sURL != rEntry.sURL )
            lSetupEntries.push_back( rEntry );
    }

    // convert internal list to external format
    // for using it on right menus really
    // Notice:   We build a property list with 4 entries and set it on result list then.
    //           Separator entries will be packed in another way then normal entries! We define
    //           special string "sSeparator" to perform too ...
    std::vector< SvtDynMenuEntry > GetList() const
    {
        sal_Int32                             nSetupCount = static_cast<sal_Int32>(lSetupEntries.size());
        sal_Int32                             nUserCount  = static_cast<sal_Int32>(lUserEntries.size());
        sal_Int32                             nStep       = 0;
        std::vector< SvtDynMenuEntry >        lResult ( nSetupCount+nUserCount );
        OUString                              sSeparator  ( u"private:separator"_ustr );

        for( const auto& pList : {&lSetupEntries, &lUserEntries} )
        {
            for( const auto& rItem : *pList )
            {
                SvtDynMenuEntry entry;
                if( rItem.sURL == sSeparator )
                {
                    entry.sURL = sSeparator;
                }
                else
                {
                    entry = rItem;
                }
                lResult[nStep] = entry;
                ++nStep;
            }
        }
        return lResult;
    }

private:
    std::vector< SvtDynMenuEntry > lSetupEntries;
    std::vector< SvtDynMenuEntry > lUserEntries;
};

}

namespace SvtDynamicMenuOptions
{

static Sequence< OUString > lcl_GetPropertyNames(
        css::uno::Reference<css::container::XHierarchicalNameAccess> const & xHierarchyAccess,
        sal_uInt32& nNewCount, sal_uInt32& nWizardCount );

std::vector< SvtDynMenuEntry > GetMenu( EDynamicMenuType eMenu )
{
    SvtDynMenu  aNewMenu;
    SvtDynMenu  aWizardMenu;

    Reference<css::container::XHierarchicalNameAccess> xHierarchyAccess = utl::ConfigManager::acquireTree(u"Office.Common/Menus/");

    // Get names and values of all accessible menu entries and fill internal structures.
    // See impl_GetPropertyNames() for further information.
    sal_uInt32              nNewCount           = 0;
    sal_uInt32              nWizardCount        = 0;
    Sequence< OUString >    lNames              = lcl_GetPropertyNames ( xHierarchyAccess, nNewCount           ,
                                                                          nWizardCount        );
    Sequence< Any >         lValues             = utl::ConfigItem::GetProperties( xHierarchyAccess, lNames, /*bAllLocales*/false );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtDynamicMenuOptions_Impl::SvtDynamicMenuOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    // Attention: List for names and values have an internal construction pattern!

    // first "New" menu ...
    //      Name                            Value
    //      /New/1/URL                      "private:factory/swriter"
    //      /New/1/Title                    "New Writer Document"
    //      /New/1/ImageIdentifier          "icon_writer"
    //      /New/1/TargetName               "_blank"

    //      /New/2/URL                      "private:factory/scalc"
    //      /New/2/Title                    "New Calc Document"
    //      /New/2/ImageIdentifier          "icon_calc"
    //      /New/2/TargetName               "_blank"

    // second "Wizard" menu ...
    //      /Wizard/1/URL                   "file://b"
    //      /Wizard/1/Title                 "PaintSomething"
    //      /Wizard/1/ImageIdentifier       "icon_?"
    //      /Wizard/1/TargetName            "_self"

    //      ... and so on ...

    sal_uInt32  nItem     = 0;
    sal_uInt32  nPosition = 0;

    // Get names/values for new menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nNewCount; ++nItem )
    {
        SvtDynMenuEntry   aItem;
        lValues[nPosition] >>= aItem.sURL;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName;
        ++nPosition;
        aNewMenu.AppendSetupEntry( aItem );
    }

    // Attention: Don't reset nPosition here!

    // Get names/values for wizard menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nWizardCount; ++nItem )
    {
        SvtDynMenuEntry   aItem;
        lValues[nPosition] >>= aItem.sURL;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName;
        ++nPosition;
        aWizardMenu.AppendSetupEntry( aItem );
    }

    std::vector< SvtDynMenuEntry > lReturn;
    switch( eMenu )
    {
        case EDynamicMenuType::NewMenu      :
            lReturn = aNewMenu.GetList();
            break;

        case EDynamicMenuType::WizardMenu   :
            lReturn = aWizardMenu.GetList();
            break;
    }
    return lReturn;
}

static void lcl_SortAndExpandPropertyNames( const Sequence< OUString >& lSource,
            Sequence< OUString >& lDestination, std::u16string_view sSetNode );

/*-****************************************************************************************************
    @short      return list of key names of our configuration management which represent our module tree
    @descr      This method returns the current list of key names! We need it to get needed values from our
                configuration management and support dynamical menu item lists!
    @param      "nNewCount"     ,   returns count of menu entries for "new"
    @param      "nWizardCount"  ,   returns count of menu entries for "wizard"
    @return     A list of configuration key names is returned.
*//*-*****************************************************************************************************/
static Sequence< OUString > lcl_GetPropertyNames(
        css::uno::Reference<css::container::XHierarchicalNameAccess> const & xHierarchyAccess,
        sal_uInt32& nNewCount, sal_uInt32& nWizardCount )
{
    // First get ALL names of current existing list items in configuration!
    Sequence< OUString > lNewItems    = utl::ConfigItem::GetNodeNames( xHierarchyAccess, SETNODE_NEWMENU, utl::ConfigNameFormat::LocalPath );
    Sequence< OUString > lWizardItems = utl::ConfigItem::GetNodeNames( xHierarchyAccess, SETNODE_WIZARDMENU, utl::ConfigNameFormat::LocalPath );

    // Get information about list counts ...
    nNewCount           = lNewItems.getLength();
    nWizardCount        = lWizardItems.getLength();

    // Sort and expand all three list to result list ...
    Sequence< OUString > lProperties;
    lcl_SortAndExpandPropertyNames( lNewItems          , lProperties, SETNODE_NEWMENU   );
    lcl_SortAndExpandPropertyNames( lWizardItems       , lProperties, SETNODE_WIZARDMENU );

    // Return result.
    return lProperties;
}

/*-****************************************************************************************************
    @short      sort given source list and expand it for all well known properties to destination
    @descr      We must support sets of entries with count inside the name .. but some of them could be missing!
                e.g. s1-s2-s3-s0-u1-s6-u5-u7
                Then we must sort it by name and expand it to the follow one:
                    sSetNode/s0/URL
                    sSetNode/s0/Title
                    sSetNode/s0/...
                    sSetNode/s1/URL
                    sSetNode/s1/Title
                    sSetNode/s1/...
                    ...
                    sSetNode/s6/URL
                    sSetNode/s6/Title
                    sSetNode/s6/...
                    sSetNode/u1/URL
                    sSetNode/u1/Title
                    sSetNode/u1/...
                    ...
                    sSetNode/u7/URL
                    sSetNode/u7/Title
                    sSetNode/u7/...
                Rules: We start with all setup written entries names "sx" and x=[0..n].
                Then we handle all "ux" items. Inside these blocks we sort it ascending by number.

    @attention  We add these expanded list to the end of given "lDestination" list!
                So we must start on "lDestination.getLength()".
                Reallocation of memory of destination list is done by us!

    @seealso    method impl_GetPropertyNames()

    @param      "lSource"      ,   original list (e.g. [m1-m2-m3-m6-m0] )
    @param      "lDestination" ,   destination of operation
    @param      "sSetNode"     ,   name of configuration set to build complete path
    @return     A list of configuration key names is returned.
*//*-*****************************************************************************************************/

static void lcl_SortAndExpandPropertyNames( const Sequence< OUString >& lSource      ,
                                        Sequence< OUString >& lDestination ,
                                    std::u16string_view         sSetNode     )
{
    struct CountWithPrefixSort
    {
        bool operator() ( std::u16string_view s1, std::u16string_view s2 ) const
        {
            // Get order numbers from entry name without prefix.
            // e.g. "m10" => 10
            //      "m5"  => 5
            sal_Int32 n1 = o3tl::toInt32(s1.substr( 1 ));
            sal_Int32 n2 = o3tl::toInt32(s2.substr( 1 ));
            // MUST be in [0,1] ... because it's a difference between
            // insert-positions of given entries in sorted list!
            return( n1<n2 );
        }
    };
    struct SelectByPrefix
    {
        bool operator() ( std::u16string_view s ) const
        {
            // Prefer setup written entries by check first letter of given string. It must be a "s".
            return o3tl::starts_with( s, PATHPREFIX_SETUP );
        }
    };

    std::vector< OUString >  lTemp;
    sal_Int32           nSourceCount     = lSource.getLength();
    sal_Int32           nDestinationStep = lDestination.getLength(); // start on end of current list ...!

    lDestination.realloc( (nSourceCount*PROPERTYCOUNT)+nDestinationStep ); // get enough memory for copy operations after nDestination ...
    auto plDestination = lDestination.getArray();

    // Copy all items to temp. vector to use fast sort operations :-)
    lTemp.insert( lTemp.end(), lSource.begin(), lSource.end() );

    // Sort all entries by number ...
    std::stable_sort( lTemp.begin(), lTemp.end(), CountWithPrefixSort() );
    // and split into setup & user written entries!
    std::stable_partition( lTemp.begin(), lTemp.end(), SelectByPrefix() );

    // Copy sorted entries to destination and expand every item with
    // 4 supported sub properties.
    for( const auto& rItem : lTemp )
    {
        OUString sFixPath(OUString::Concat(sSetNode) + PATHDELIMITER + rItem + PATHDELIMITER);
        plDestination[nDestinationStep++] = sFixPath + PROPERTYNAME_URL;
        plDestination[nDestinationStep++] = sFixPath + PROPERTYNAME_TITLE;
        plDestination[nDestinationStep++] = sFixPath + PROPERTYNAME_IMAGEIDENTIFIER;
        plDestination[nDestinationStep++] = sFixPath + PROPERTYNAME_TARGETNAME;
    }
}


} // namespace SvtDynamicMenuOptions

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

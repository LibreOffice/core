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

#include <uielement/toolbarmerger.hxx>
#include <uielement/generictoolbarcontroller.hxx>

#include <svtools/miscopt.hxx>
#include <comphelper/processfactory.hxx>

namespace framework
{

static const char MERGE_TOOLBAR_URL[]             = "URL";
static const char MERGE_TOOLBAR_TITLE[]           = "Title";
static const char MERGE_TOOLBAR_IMAGEID[]         = "ImageIdentifier";
static const char MERGE_TOOLBAR_CONTEXT[]         = "Context";
static const char MERGE_TOOLBAR_TARGET[]          = "Target";
static const char MERGE_TOOLBAR_CONTROLTYPE[]     = "ControlType";
static const char MERGE_TOOLBAR_WIDTH[]           = "Width";

static const char MERGECOMMAND_ADDAFTER[]         = "AddAfter";
static const char MERGECOMMAND_ADDBEFORE[]        = "AddBefore";
static const char MERGECOMMAND_REPLACE[]          = "Replace";
static const char MERGECOMMAND_REMOVE[]           = "Remove";

static const char MERGEFALLBACK_ADDLAST[]         = "AddLast";
static const char MERGEFALLBACK_ADDFIRST[]        = "AddFirst";
static const char MERGEFALLBACK_IGNORE[]          = "Ignore";

static const char TOOLBARCONTROLLER_BUTTON[]      = "Button";
static const char TOOLBARCONTROLLER_COMBOBOX[]    = "Combobox";
static const char TOOLBARCONTROLLER_EDIT[]        = "Editfield";
static const char TOOLBARCONTROLLER_SPINFIELD[]   = "Spinfield";
static const char TOOLBARCONTROLLER_IMGBUTTON[]   = "ImageButton";
static const char TOOLBARCONTROLLER_DROPDOWNBOX[] = "Dropdownbox";
static const char TOOLBARCONTROLLER_DROPDOWNBTN[] = "DropdownButton";
static const char TOOLBARCONTROLLER_TOGGLEDDBTN[] = "ToggleDropdownButton";

static const char   TOOLBOXITEM_SEPARATOR_STR[]   = "private:separator";

using namespace ::com::sun::star;

/**
 Check whether a module identifier is part of a context
 defined by a colon separated list of module identifier.

 @param
     rContext

     Describes a context string list where all contexts
     are delimited by a colon. For more information about
     the module identifier used as context strings see the
     IDL description of css::frame::XModuleManager

 @param
     rModuleIdentifier

     A string describing a module identifier. See IDL
     description of css::frame::XModuleManager.

 @result
     The result is true if the rContext is an empty string
     or rModuleIdentifier is part of the context string.

*/
bool ToolBarMerger::IsCorrectContext(
    const OUString& rContext,
    const OUString& rModuleIdentifier )
{
    return ( rContext.isEmpty() || ( rContext.indexOf( rModuleIdentifier ) >= 0 ));
}

/**
 Converts a sequence, sequence of property values to
 a vector of structs.

 @param
     rSequence

     Provides a sequence, sequence of property values.

 @param
     rContainer

     A vector of AddonToolbarItems which will hold the
     conversion from the rSequence argument.

 @result
     The result is true if the sequence, sequence of property
     values could be converted to a vector of structs.

*/
bool ToolBarMerger::ConvertSeqSeqToVector(
    const uno::Sequence< uno::Sequence< beans::PropertyValue > >& rSequence,
    AddonToolbarItemContainer& rContainer )
{
    sal_Int32 nLen( rSequence.getLength() );
    for ( sal_Int32 i = 0; i < nLen; i++ )
    {
        AddonToolbarItem aAddonToolbarItem;
        ConvertSequenceToValues( rSequence[i],
                                 aAddonToolbarItem.aCommandURL,
                                 aAddonToolbarItem.aLabel,
                                 aAddonToolbarItem.aImageIdentifier,
                                 aAddonToolbarItem.aTarget,
                                 aAddonToolbarItem.aContext,
                                 aAddonToolbarItem.aControlType,
                                 aAddonToolbarItem.nWidth );
        rContainer.push_back( aAddonToolbarItem );
    }

    return true;
}

/**
 Converts a sequence of property values to single
 values.

 @param
     rSequence

     Provides a sequence of property values.

 @param
     rCommandURL

     Contains the value of the property with
     Name="CommandURL".

 @param
     rLabel

     Contains the value of the property with
     Name="Title"

 @param
     rImageIdentifier

     Contains the value of the property with
     Name="ImageIdentifier"

 @param
     rTarget

     Contains the value of the property with
     Name="Target"

 @param
     rContext

     Contains the value of the property with
     Name="Context"

 @param
     rControlType

     Contains the value of the property with
     Name="ControlType"

 @result
     All possible mapping between sequence of property
     values and the single values are done.

*/
void ToolBarMerger::ConvertSequenceToValues(
    const uno::Sequence< beans::PropertyValue >& rSequence,
    OUString& rCommandURL,
    OUString& rLabel,
    OUString& rImageIdentifier,
    OUString& rTarget,
    OUString& rContext,
    OUString& rControlType,
    sal_uInt16& rWidth )
{
    for ( sal_Int32 i = 0; i < rSequence.getLength(); i++ )
    {
        if ( rSequence[i].Name == MERGE_TOOLBAR_URL )
            rSequence[i].Value >>= rCommandURL;
        else if ( rSequence[i].Name == MERGE_TOOLBAR_TITLE )
            rSequence[i].Value >>= rLabel;
        else if ( rSequence[i].Name == MERGE_TOOLBAR_IMAGEID )
            rSequence[i].Value >>= rImageIdentifier;
        else if ( rSequence[i].Name == MERGE_TOOLBAR_CONTEXT )
            rSequence[i].Value >>= rContext;
        else if ( rSequence[i].Name == MERGE_TOOLBAR_TARGET )
            rSequence[i].Value >>= rTarget;
        else if ( rSequence[i].Name == MERGE_TOOLBAR_CONTROLTYPE )
            rSequence[i].Value >>= rControlType;
        else if ( rSequence[i].Name == MERGE_TOOLBAR_WIDTH )
        {
            sal_Int32 aValue = 0;
            rSequence[i].Value >>= aValue;
            rWidth = sal_uInt16( aValue );
        }
    }
}

/**
 Tries to find the reference point provided and delivers
 position and result of the search process.

 @param
     pToolbar

     Must be a valid pointer to a toolbar with items which
     should be searched.

 @param
     rReferencePoint

     A command URL which should be the reference point for
     the coming merge operation.

 @result
     Provides information about the search result, the
     position of the reference point and the toolbar used.
*/
ReferenceToolbarPathInfo ToolBarMerger::FindReferencePoint(
    ToolBox*               pToolbar,
    const OUString& rReferencePoint )
{
    ReferenceToolbarPathInfo aResult;
    aResult.bResult  = false;
    aResult.pToolbar = pToolbar;
    aResult.nPos     = TOOLBOX_ITEM_NOTFOUND;

    const sal_uInt16 nSize( pToolbar->GetItemCount() );

    for ( sal_uInt16 i = 0; i < nSize; i++ )
    {
        const sal_uInt16 nItemId = pToolbar->GetItemId( i );
        if ( nItemId > 0 )
        {
            const OUString rCmd = pToolbar->GetItemCommand( nItemId );
            if ( rCmd == rReferencePoint )
            {
                aResult.bResult = true;
                aResult.nPos    = i;
                return aResult;
            }
        }
    }

    return aResult;
}

/**
 Processes a merge operation.

 @param
     pToolbar

     A valid pointer to the toolbar where the merge
     operation is applied to.

 @param
     nPos

     The reference position of the toolbar item for
     the merge operation. Value must be between
     0 and number of toolbar items - 1.

 @param
     rItemId

     A unique item ID.

 @param
     rModuleIdentifier

     The current application module context.

 @param
     rMergeCommand

     A merge command.

 @param
     rMergeCommandParameter.

     An optional argument for the merge command.

 @param
     rItems

     Toolbar items which are associated to the merge
     command.

 @result
     Returns true for a successful operation otherwise
     false.
*/
bool ToolBarMerger::ProcessMergeOperation(
    ToolBox*                               pToolbar,
    sal_uInt16                             nPos,
    sal_uInt16&                            rItemId,
    CommandToInfoMap&                      rCommandMap,
    const OUString&                        rModuleIdentifier,
    const OUString&                        rMergeCommand,
    const OUString&                        rMergeCommandParameter,
    const AddonToolbarItemContainer&       rItems )
{
    if ( rMergeCommand == MERGECOMMAND_ADDAFTER )
        return MergeItems( pToolbar, nPos, 1, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand == MERGECOMMAND_ADDBEFORE )
        return MergeItems( pToolbar, nPos, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand == MERGECOMMAND_REPLACE )
        return ReplaceItem( pToolbar, nPos, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand == MERGECOMMAND_REMOVE )
        return RemoveItems( pToolbar, nPos, rMergeCommandParameter );

    return false;
}

/**
 Processes a merge fallback operation.

 @param
     pToolbar

     A valid pointer to the toolbar where the merge
     fall back operation is applied to.

 @param
     nPos

     The reference position of the toolbar item for
     the merge operation. Value must be between
     0 and number of toolbar items - 1.

 @param
     rItemId

     A unique item ID.

 @param
     rModuleIdentifier

     The current application module context.

 @param
     rMergeCommand

     A merge command.

 @param
     rItems

     Toolbar items which are associated to the merge
     command.

 @result
     Returns true for a successful operation otherwise
     false.
*/
bool ToolBarMerger::ProcessMergeFallback(
    ToolBox*                         pToolbar,
    sal_uInt16                       /*nPos*/,
    sal_uInt16&                      rItemId,
    CommandToInfoMap&                rCommandMap,
    const OUString&           rModuleIdentifier,
    const OUString&           rMergeCommand,
    const OUString&           rMergeFallback,
    const AddonToolbarItemContainer& rItems )
{
    if (( rMergeFallback == MERGEFALLBACK_IGNORE ) ||
        ( rMergeCommand == MERGECOMMAND_REPLACE ) ||
        ( rMergeCommand == MERGECOMMAND_REMOVE ) )
    {
        return true;
    }
    else if (( rMergeCommand == MERGECOMMAND_ADDBEFORE ) ||
             ( rMergeCommand == MERGECOMMAND_ADDAFTER ) )
    {
        if ( rMergeFallback == MERGEFALLBACK_ADDFIRST )
            return MergeItems( pToolbar, 0, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
        else if ( rMergeFallback == MERGEFALLBACK_ADDLAST )
            return MergeItems( pToolbar, TOOLBOX_APPEND, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
    }

    return false;
}

/**
 Merges (adds) toolbar items into an existing toolbar.

 @param
     pToolbar

     A valid pointer to the toolbar where the merge
     fall back operation is applied to.

 @param
     nPos

     The reference position of the toolbar item for
     the merge operation. Value must be between
     0 and number of toolbar items - 1.

 @param
     rItemId

     A unique item ID.

 @param
     rModuleIdentifier

     The current application module context.

 @param
     rItems

     Toolbar items which are associated to the merge
     command.

 @result
     Returns true for a successful operation otherwise
     false.
*/
bool ToolBarMerger::MergeItems(
    ToolBox*                               pToolbar,
    sal_uInt16                             nPos,
    sal_uInt16                             nModIndex,
    sal_uInt16&                            rItemId,
    CommandToInfoMap&                      rCommandMap,
    const OUString&                        rModuleIdentifier,
    const AddonToolbarItemContainer&       rAddonToolbarItems )
{
    const sal_Int32 nSize( rAddonToolbarItems.size() );

    sal_uInt16 nIndex( 0 );
    for ( sal_Int32 i = 0; i < nSize; i++ )
    {
        const AddonToolbarItem& rItem = rAddonToolbarItems[i];
        if ( IsCorrectContext( rItem.aContext, rModuleIdentifier ))
        {
            sal_Int32 nInsPos = nPos+nModIndex+i;
            if ( nInsPos > sal_Int32( pToolbar->GetItemCount() ))
                nInsPos = TOOLBOX_APPEND;

            if ( rItem.aCommandURL == TOOLBOXITEM_SEPARATOR_STR )
                pToolbar->InsertSeparator( sal_uInt16( nInsPos ));
            else
            {
                CommandToInfoMap::iterator pIter = rCommandMap.find( rItem.aCommandURL );
                if ( pIter == rCommandMap.end())
                {
                    CommandInfo aCmdInfo;
                    aCmdInfo.nId = rItemId;
                    const CommandToInfoMap::value_type aValue( rItem.aCommandURL, aCmdInfo );
                    rCommandMap.insert( aValue );
                }
                else
                {
                    pIter->second.aIds.push_back( rItemId );
                }

                ToolBarMerger::CreateToolbarItem( pToolbar, sal_uInt16( nInsPos ), rItemId, rItem );
            }

            ++nIndex;
            ++rItemId;
        }
    }

    return true;
}

/**
 Replaces a toolbar item with new items for an
 existing toolbar.

 @param
     pToolbar

     A valid pointer to the toolbar where the merge
     fall back operation is applied to.

 @param
     nPos

     The reference position of the toolbar item for
     the merge operation. Value must be between
     0 and number of toolbar items - 1.

 @param
     rItemId

     A unique item ID.

 @param
     rModuleIdentifier

     The current application module context.

 @param
     rItems

     Toolbar items which are associated to the merge
     command.

 @result
     Returns true for a successful operation otherwise
     false.
*/
bool ToolBarMerger::ReplaceItem(
    ToolBox*                               pToolbar,
    sal_uInt16                             nPos,
    sal_uInt16&                            rItemId,
    CommandToInfoMap&                      rCommandMap,
    const OUString&                        rModuleIdentifier,
    const AddonToolbarItemContainer&       rAddonToolbarItems )
{
    pToolbar->RemoveItem( nPos );
    return MergeItems( pToolbar, nPos, 0, rItemId, rCommandMap, rModuleIdentifier, rAddonToolbarItems );
}

/**
 Removes toolbar items from an existing toolbar.

 @param
     pToolbar

     A valid pointer to the toolbar where the merge
     fall back operation is applied to.

 @param
     nPos

     The reference position of the toolbar item for
     the merge operation. Value must be between
     0 and number of toolbar items - 1.

 @param
     rMergeCommandParameter.

     An optional argument for the merge command.

 @result
     Returns true for a successful operation otherwise
     false.
*/
bool ToolBarMerger::RemoveItems(
    ToolBox*                  pToolbar,
    sal_uInt16                nPos,
    const OUString&    rMergeCommandParameter )
{
    sal_Int32 nCount = rMergeCommandParameter.toInt32();
    if ( nCount > 0 )
    {
        for ( sal_Int32 i = 0; i < nCount; i++ )
        {
            if ( nPos < pToolbar->GetItemCount() )
                pToolbar->RemoveItem( nPos );
        }
    }
    return true;
}

/**
 Removes toolbar items from an existing toolbar.

 @param
     pToolbar

     A valid pointer to the toolbar where the merge
     fall back operation is applied to.

 @param
     nPos

     The reference position of the toolbar item for
     the merge operation. Value must be between
     0 and number of toolbar items - 1.

 @param
     rMergeCommandParameter.

     An optional argument for the merge command.

 @result
     Returns true for a successful operation otherwise
     false.
*/
::cppu::OWeakObject* ToolBarMerger::CreateController(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Reference< frame::XFrame > & xFrame,
    ToolBox*               pToolbar,
    const OUString& rCommandURL,
    sal_uInt16             nId,
    sal_uInt16             nWidth,
    const OUString& rControlType )
{
    ::cppu::OWeakObject* pResult( nullptr );

    if ( rControlType == TOOLBARCONTROLLER_BUTTON )
        pResult = new ButtonToolbarController( rxContext, pToolbar, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_COMBOBOX )
        pResult = new ComboboxToolbarController( rxContext, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_EDIT )
        pResult = new EditToolbarController( rxContext, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_SPINFIELD )
        pResult = new SpinfieldToolbarController( rxContext, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_IMGBUTTON )
        pResult = new ImageButtonToolbarController( rxContext, xFrame, pToolbar, nId, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_DROPDOWNBOX )
        pResult = new DropdownToolbarController( rxContext, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_DROPDOWNBTN )
        pResult = new ToggleButtonToolbarController( rxContext, xFrame, pToolbar, nId,
                                                     ToggleButtonToolbarController::Style::DropDownButton, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_TOGGLEDDBTN )
        pResult = new ToggleButtonToolbarController( rxContext, xFrame, pToolbar, nId,
                                                     ToggleButtonToolbarController::Style::ToggleDropDownButton, rCommandURL );
    else
        pResult = new GenericToolbarController( rxContext, xFrame, pToolbar, nId, rCommandURL );

    return pResult;
}

void ToolBarMerger::CreateToolbarItem( ToolBox* pToolbar, sal_uInt16 nPos, sal_uInt16 nItemId, const AddonToolbarItem& rItem )
{
    pToolbar->InsertItem( nItemId, rItem.aLabel, ToolBoxItemBits::NONE, nPos );
    pToolbar->SetItemCommand( nItemId, rItem.aCommandURL );
    pToolbar->SetQuickHelpText( nItemId, rItem.aLabel );
    pToolbar->SetItemText( nItemId, rItem.aLabel );
    pToolbar->EnableItem( nItemId );
    pToolbar->SetItemState( nItemId, TRISTATE_FALSE );

    // Use the user data to store add-on specific data with the toolbar item
    AddonsParams* pAddonParams = new AddonsParams;
    pAddonParams->aImageId     = rItem.aImageIdentifier;
    pAddonParams->aTarget      = rItem.aTarget;
    pAddonParams->aControlType = rItem.aControlType;
    pAddonParams->nWidth       = rItem.nWidth;
    pToolbar->SetItemData( nItemId, pAddonParams );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

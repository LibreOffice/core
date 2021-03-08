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

#include <string_view>

#include <uielement/toolbarmerger.hxx>
#include <framework/generictoolbarcontroller.hxx>

#include <uielement/buttontoolbarcontroller.hxx>
#include <uielement/comboboxtoolbarcontroller.hxx>
#include <uielement/dropdownboxtoolbarcontroller.hxx>
#include <uielement/edittoolbarcontroller.hxx>
#include <uielement/imagebuttontoolbarcontroller.hxx>
#include <uielement/spinfieldtoolbarcontroller.hxx>
#include <uielement/togglebuttontoolbarcontroller.hxx>
#include <uielement/FixedTextToolbarController.hxx>
#include <uielement/FixedImageToolbarController.hxx>

namespace framework
{

const char MERGE_TOOLBAR_URL[]             = "URL";
const char MERGE_TOOLBAR_TITLE[]           = "Title";
const char MERGE_TOOLBAR_CONTEXT[]         = "Context";
const char MERGE_TOOLBAR_TARGET[]          = "Target";
const char MERGE_TOOLBAR_CONTROLTYPE[]     = "ControlType";
const char MERGE_TOOLBAR_WIDTH[]           = "Width";

const char16_t MERGECOMMAND_ADDAFTER[]     = u"AddAfter";
const char16_t MERGECOMMAND_ADDBEFORE[]    = u"AddBefore";
const char16_t MERGECOMMAND_REPLACE[]      = u"Replace";
const char16_t MERGECOMMAND_REMOVE[]       = u"Remove";

const char16_t MERGEFALLBACK_ADDLAST[]     = u"AddLast";
const char16_t MERGEFALLBACK_ADDFIRST[]    = u"AddFirst";
const char16_t MERGEFALLBACK_IGNORE[]      = u"Ignore";

const char16_t TOOLBARCONTROLLER_BUTTON[]  = u"Button";
const char16_t TOOLBARCONTROLLER_COMBOBOX[] = u"Combobox";
const char16_t TOOLBARCONTROLLER_EDIT[]    = u"Editfield";
const char16_t TOOLBARCONTROLLER_SPINFIELD[] = u"Spinfield";
const char16_t TOOLBARCONTROLLER_IMGBUTTON[] = u"ImageButton";
const char16_t TOOLBARCONTROLLER_DROPDOWNBOX[] = u"Dropdownbox";
const char16_t TOOLBARCONTROLLER_DROPDOWNBTN[] = u"DropdownButton";
const char16_t TOOLBARCONTROLLER_TOGGLEDDBTN[] = u"ToggleDropdownButton";
const char16_t TOOLBARCONTROLLER_FIXEDIMAGE[] = u"FixedImage";
const char16_t TOOLBARCONTROLLER_FIXEDTEXT[] = u"FixedText";

const char   TOOLBOXITEM_SEPARATOR_STR[]   = "private:separator";

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
    std::u16string_view rModuleIdentifier )
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
*/
void ToolBarMerger::ConvertSeqSeqToVector(
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
                                 aAddonToolbarItem.aTarget,
                                 aAddonToolbarItem.aContext,
                                 aAddonToolbarItem.aControlType,
                                 aAddonToolbarItem.nWidth );
        rContainer.push_back( aAddonToolbarItem );
    }
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
    OUString& rTarget,
    OUString& rContext,
    OUString& rControlType,
    sal_uInt16& rWidth )
{
    for ( beans::PropertyValue const & prop : rSequence )
    {
        if ( prop.Name == MERGE_TOOLBAR_URL )
            prop.Value >>= rCommandURL;
        else if ( prop.Name == MERGE_TOOLBAR_TITLE )
            prop.Value >>= rLabel;
        else if ( prop.Name == MERGE_TOOLBAR_CONTEXT )
            prop.Value >>= rContext;
        else if ( prop.Name == MERGE_TOOLBAR_TARGET )
            prop.Value >>= rTarget;
        else if ( prop.Name == MERGE_TOOLBAR_CONTROLTYPE )
            prop.Value >>= rControlType;
        else if ( prop.Name == MERGE_TOOLBAR_WIDTH )
        {
            sal_Int32 aValue = 0;
            prop.Value >>= aValue;
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
    const ToolBox*   pToolbar,
    std::u16string_view rReferencePoint )
{
    ReferenceToolbarPathInfo aResult;
    aResult.bResult  = false;
    aResult.nPos     = ToolBox::ITEM_NOTFOUND;

    const ToolBox::ImplToolItems::size_type nSize( pToolbar->GetItemCount() );

    for ( ToolBox::ImplToolItems::size_type i = 0; i < nSize; i++ )
    {
        const ToolBoxItemId nItemId = pToolbar->GetItemId( i );
        if ( nItemId > ToolBoxItemId(0) )
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
    ToolBox::ImplToolItems::size_type      nPos,
    ToolBoxItemId&                         rItemId,
    CommandToInfoMap&                      rCommandMap,
    std::u16string_view                    rModuleIdentifier,
    std::u16string_view                    rMergeCommand,
    const OUString&                        rMergeCommandParameter,
    const AddonToolbarItemContainer&       rItems )
{
    if ( rMergeCommand == MERGECOMMAND_ADDAFTER )
        MergeItems( pToolbar, nPos, 1, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand == MERGECOMMAND_ADDBEFORE )
        MergeItems( pToolbar, nPos, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand == MERGECOMMAND_REPLACE )
        ReplaceItem( pToolbar, nPos, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand == MERGECOMMAND_REMOVE )
        RemoveItems( pToolbar, nPos, rMergeCommandParameter );
    else
        return false;
    return true;
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
    ToolBoxItemId&                   rItemId,
    CommandToInfoMap&                rCommandMap,
    std::u16string_view       rModuleIdentifier,
    std::u16string_view       rMergeCommand,
    std::u16string_view       rMergeFallback,
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
            MergeItems( pToolbar, 0, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
        else if ( rMergeFallback == MERGEFALLBACK_ADDLAST )
            MergeItems( pToolbar, ToolBox::APPEND, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
        else
            return false;
        return true;
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
*/
void ToolBarMerger::MergeItems(
    ToolBox*                               pToolbar,
    ToolBox::ImplToolItems::size_type      nPos,
    sal_uInt16                             nModIndex,
    ToolBoxItemId&                         rItemId,
    CommandToInfoMap&                      rCommandMap,
    std::u16string_view                    rModuleIdentifier,
    const AddonToolbarItemContainer&       rAddonToolbarItems )
{
    const sal_Int32 nSize( rAddonToolbarItems.size() );

    for ( sal_Int32 i = 0; i < nSize; i++ )
    {
        const AddonToolbarItem& rItem = rAddonToolbarItems[i];
        if ( IsCorrectContext( rItem.aContext, rModuleIdentifier ))
        {
            ToolBox::ImplToolItems::size_type nInsPos = nPos;
            if (nInsPos != ToolBox::APPEND)
            {
                nInsPos += nModIndex+i;
                if ( nInsPos > pToolbar->GetItemCount() )
                    nInsPos = ToolBox::APPEND;
            }

            if ( rItem.aCommandURL == TOOLBOXITEM_SEPARATOR_STR )
                pToolbar->InsertSeparator( nInsPos );
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

                ToolBarMerger::CreateToolbarItem( pToolbar, nInsPos, rItemId, rItem );
            }

            ++rItemId;
        }
    }
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
*/
void ToolBarMerger::ReplaceItem(
    ToolBox*                               pToolbar,
    ToolBox::ImplToolItems::size_type      nPos,
    ToolBoxItemId&                         rItemId,
    CommandToInfoMap&                      rCommandMap,
    std::u16string_view                    rModuleIdentifier,
    const AddonToolbarItemContainer&       rAddonToolbarItems )
{
    pToolbar->RemoveItem( nPos );
    MergeItems( pToolbar, nPos, 0, rItemId, rCommandMap, rModuleIdentifier, rAddonToolbarItems );
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
*/
void ToolBarMerger::RemoveItems(
    ToolBox*                  pToolbar,
    ToolBox::ImplToolItems::size_type nPos,
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
rtl::Reference<::cppu::OWeakObject> ToolBarMerger::CreateController(
    const uno::Reference< uno::XComponentContext >& rxContext,
    const uno::Reference< frame::XFrame > & xFrame,
    ToolBox*               pToolbar,
    const OUString& rCommandURL,
    ToolBoxItemId          nId,
    sal_uInt16             nWidth,
    std::u16string_view rControlType )
{
    rtl::Reference<::cppu::OWeakObject> pResult;

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
    else if ( rControlType == TOOLBARCONTROLLER_FIXEDIMAGE )
        pResult = new FixedImageToolbarController( rxContext, xFrame, pToolbar, nId, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_FIXEDTEXT )
        pResult = new FixedTextToolbarController( rxContext, xFrame, pToolbar, nId, rCommandURL );
    else if ( rControlType == TOOLBARCONTROLLER_TOGGLEDDBTN )
        pResult = new ToggleButtonToolbarController( rxContext, xFrame, pToolbar, nId,
                                                     ToggleButtonToolbarController::Style::ToggleDropDownButton, rCommandURL );
    else
        pResult = new GenericToolbarController( rxContext, xFrame, pToolbar, nId, rCommandURL );

    return pResult;
}

void ToolBarMerger::CreateToolbarItem( ToolBox* pToolbar, ToolBox::ImplToolItems::size_type nPos, ToolBoxItemId nItemId, const AddonToolbarItem& rItem )
{
    pToolbar->InsertItem( nItemId, rItem.aLabel, ToolBoxItemBits::NONE, nPos );
    pToolbar->SetItemCommand( nItemId, rItem.aCommandURL );
    pToolbar->SetQuickHelpText( nItemId, rItem.aLabel );
    pToolbar->SetItemText( nItemId, rItem.aLabel );
    pToolbar->EnableItem( nItemId );
    pToolbar->SetItemState( nItemId, TRISTATE_FALSE );

    // Use the user data to store add-on specific data with the toolbar item
    AddonsParams* pAddonParams = new AddonsParams;
    pAddonParams->aControlType = rItem.aControlType;
    pAddonParams->nWidth       = rItem.nWidth;
    pToolbar->SetItemData( nItemId, pAddonParams );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <uielement/toolbarmerger.hxx>
#include <uielement/generictoolbarcontroller.hxx>
#include <framework/imageproducer.hxx>

#include <svtools/miscopt.hxx>

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
     IDL description of com::sun::star::frame::XModuleManager

 @param
     rModuleIdentifier

     A string describing a module identifier. See IDL
     description of com::sun::star::frame::XModuleManager.

 @result
     The result is true if the rContext is an empty string
     or rModuleIdentifier is part of the context string.

*/
bool ToolBarMerger::IsCorrectContext(
    const ::rtl::OUString& rContext,
    const ::rtl::OUString& rModuleIdentifier )
{
    return (( rContext.getLength() == 0 ) || ( rContext.indexOf( rModuleIdentifier ) >= 0 ));
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
    const uno::Sequence< uno::Sequence< beans::PropertyValue > > rSequence,
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
    const uno::Sequence< beans::PropertyValue > rSequence,
    ::rtl::OUString& rCommandURL,
    ::rtl::OUString& rLabel,
    ::rtl::OUString& rImageIdentifier,
    ::rtl::OUString& rTarget,
    ::rtl::OUString& rContext,
    ::rtl::OUString& rControlType,
    sal_uInt16&      rWidth )
{
    for ( sal_Int32 i = 0; i < rSequence.getLength(); i++ )
    {
        if ( rSequence[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGE_TOOLBAR_URL ) ) )
            rSequence[i].Value >>= rCommandURL;
        else if ( rSequence[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGE_TOOLBAR_TITLE ) ) )
            rSequence[i].Value >>= rLabel;
        else if ( rSequence[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGE_TOOLBAR_IMAGEID ) ) )
            rSequence[i].Value >>= rImageIdentifier;
        else if ( rSequence[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGE_TOOLBAR_CONTEXT ) ) )
            rSequence[i].Value >>= rContext;
        else if ( rSequence[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGE_TOOLBAR_TARGET ) ) )
            rSequence[i].Value >>= rTarget;
        else if ( rSequence[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGE_TOOLBAR_CONTROLTYPE ) ) )
            rSequence[i].Value >>= rControlType;
        else if ( rSequence[i].Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGE_TOOLBAR_WIDTH ) ) )
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
    const ::rtl::OUString& rReferencePoint )
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
            const ::rtl::OUString rCmd = pToolbar->GetItemCommand( nItemId );
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
     xFrame

     Must be a valid reference to a frame.

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
    const uno::Reference< frame::XFrame >& xFrame,
    ToolBox*                               pToolbar,
    sal_uInt16                             nPos,
    sal_uInt16&                            rItemId,
    CommandToInfoMap&                      rCommandMap,
    const ::rtl::OUString&                 rModuleIdentifier,
    const ::rtl::OUString&                 rMergeCommand,
    const ::rtl::OUString&                 rMergeCommandParameter,
    const AddonToolbarItemContainer&       rItems )
{
    if ( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGECOMMAND_ADDAFTER ) ) )
        return MergeItems( xFrame, pToolbar, nPos, 1, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGECOMMAND_ADDBEFORE ) ) )
        return MergeItems( xFrame, pToolbar, nPos, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGECOMMAND_REPLACE ) ) )
        return ReplaceItem( xFrame, pToolbar, nPos, rItemId, rCommandMap, rModuleIdentifier, rItems );
    else if ( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGECOMMAND_REMOVE ) ) )
        return RemoveItems( pToolbar, nPos, rMergeCommandParameter );

    return false;
}

/**
 Processes a merge fallback operation.

 @param
     xFrame

     Must be a valid reference to a frame.

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
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame,
    ToolBox*                         pToolbar,
    sal_uInt16                       /*nPos*/,
    sal_uInt16&                      rItemId,
    CommandToInfoMap&                rCommandMap,
    const ::rtl::OUString&           rModuleIdentifier,
    const ::rtl::OUString&           rMergeCommand,
    const ::rtl::OUString&           rMergeFallback,
    const AddonToolbarItemContainer& rItems )
{
    if (( rMergeFallback.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGEFALLBACK_IGNORE ))) ||
        ( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(  MERGECOMMAND_REPLACE ))) ||
        ( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(  MERGECOMMAND_REMOVE  ))) )
    {
        return true;
    }
    else if (( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGECOMMAND_ADDBEFORE ))) ||
             ( rMergeCommand.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGECOMMAND_ADDAFTER ))) )
    {
        if ( rMergeFallback.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGEFALLBACK_ADDFIRST )))
            return MergeItems( xFrame, pToolbar, 0, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
        else if ( rMergeFallback.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( MERGEFALLBACK_ADDLAST )))
            return MergeItems( xFrame, pToolbar, TOOLBOX_APPEND, 0, rItemId, rCommandMap, rModuleIdentifier, rItems );
    }

    return false;
}

/**
 Merges (adds) toolbar items into an existing toolbar.

 @param
     xFrame

     Must be a valid reference to a frame.

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
    const uno::Reference< frame::XFrame >& rFrame,
    ToolBox*                               pToolbar,
    sal_uInt16                             nPos,
    sal_uInt16                             nModIndex,
    sal_uInt16&                            rItemId,
    CommandToInfoMap&                      rCommandMap,
    const ::rtl::OUString&                 rModuleIdentifier,
    const AddonToolbarItemContainer&       rAddonToolbarItems )
{
    const sal_Int32 nSize( rAddonToolbarItems.size() );

    uno::Reference< frame::XFrame > xFrame( rFrame );

    sal_uInt16 nIndex( 0 );
    for ( sal_Int32 i = 0; i < nSize; i++ )
    {
        const AddonToolbarItem& rItem = rAddonToolbarItems[i];
        if ( IsCorrectContext( rItem.aContext, rModuleIdentifier ))
        {
            sal_Int32 nInsPos = nPos+nModIndex+i;
            if ( nInsPos > sal_Int32( pToolbar->GetItemCount() ))
                nInsPos = TOOLBOX_APPEND;

            if ( rItem.aCommandURL.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBOXITEM_SEPARATOR_STR )))
                pToolbar->InsertSeparator( sal_uInt16( nInsPos ));
            else
            {
                CommandToInfoMap::iterator pIter = rCommandMap.find( rItem.aCommandURL );
                if ( pIter == rCommandMap.end())
                {
                    CommandInfo aCmdInfo;
                    aCmdInfo.nId = rItemId;
                    rCommandMap.insert( CommandToInfoMap::value_type( rItem.aCommandURL, aCmdInfo ));
                }
                else
                {
                    pIter->second.aIds.push_back( rItemId );
                }

                ToolBarMerger::CreateToolbarItem( pToolbar, rCommandMap, sal_uInt16( nInsPos ), rItemId, rItem );
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
     xFrame

     Must be a valid reference to a frame.

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
    const uno::Reference< frame::XFrame >& xFrame,
    ToolBox*                               pToolbar,
    sal_uInt16                             nPos,
    sal_uInt16&                            rItemId,
    CommandToInfoMap&                      rCommandMap,
    const ::rtl::OUString&                 rModuleIdentifier,
    const AddonToolbarItemContainer&       rAddonToolbarItems )
{
    pToolbar->RemoveItem( nPos );
    return MergeItems( xFrame, pToolbar, nPos, 0, rItemId, rCommandMap, rModuleIdentifier, rAddonToolbarItems );
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
    const ::rtl::OUString&    rMergeCommandParameter )
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
    uno::Reference< lang::XMultiServiceFactory > xSMGR,
    uno::Reference< frame::XFrame > xFrame,
    ToolBox*               pToolbar,
    const ::rtl::OUString& rCommandURL,
    sal_uInt16             nId,
    sal_uInt16             nWidth,
    const ::rtl::OUString& rControlType )
{
    ::cppu::OWeakObject* pResult( 0 );

    if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_BUTTON )))
        pResult = new ButtonToolbarController( xSMGR, pToolbar, rCommandURL );
    else if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_COMBOBOX )))
        pResult = new ComboboxToolbarController( xSMGR, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_EDIT )))
        pResult = new EditToolbarController( xSMGR, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_SPINFIELD )))
        pResult = new SpinfieldToolbarController( xSMGR, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_IMGBUTTON )))
        pResult = new ImageButtonToolbarController( xSMGR, xFrame, pToolbar, nId, rCommandURL );
    else if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_DROPDOWNBOX )))
        pResult = new DropdownToolbarController( xSMGR, xFrame, pToolbar, nId, nWidth, rCommandURL );
    else if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_DROPDOWNBTN )))
        pResult = new ToggleButtonToolbarController( xSMGR, xFrame, pToolbar, nId,
                                                     ToggleButtonToolbarController::STYLE_DROPDOWNBUTTON, rCommandURL );
    else if ( rControlType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( TOOLBARCONTROLLER_TOGGLEDDBTN )))
        pResult = new ToggleButtonToolbarController( xSMGR, xFrame, pToolbar, nId,
                                                     ToggleButtonToolbarController::STYLE_TOGGLE_DROPDOWNBUTTON, rCommandURL );
    else
        pResult = new GenericToolbarController( xSMGR, xFrame, pToolbar, nId, rCommandURL );

    return pResult;
}

void ToolBarMerger::CreateToolbarItem( ToolBox* pToolbar, CommandToInfoMap& rCommandMap, sal_uInt16 nPos, sal_uInt16 nItemId, const AddonToolbarItem& rItem )
{
    pToolbar->InsertItem( nItemId, rItem.aLabel, 0, nPos );
    pToolbar->SetItemCommand( nItemId, rItem.aCommandURL );
    pToolbar->SetQuickHelpText( nItemId, rItem.aLabel );
    pToolbar->SetItemText( nItemId, rItem.aLabel );
    pToolbar->EnableItem( nItemId, sal_True );
    pToolbar->SetItemState( nItemId, STATE_NOCHECK );

    CommandToInfoMap::iterator pIter = rCommandMap.find( rItem.aCommandURL );
    if ( pIter != rCommandMap.end() )
        pIter->second.nWidth = rItem.nWidth;

    // Use the user data to store add-on specific data with the toolbar item
    AddonsParams* pAddonParams = new AddonsParams;
    pAddonParams->aImageId     = rItem.aImageIdentifier;
    pAddonParams->aTarget      = rItem.aTarget;
    pAddonParams->aControlType = rItem.aControlType;
    pToolbar->SetItemData( nItemId, pAddonParams );
}

} // namespace framework

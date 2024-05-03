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

#include <fltoptint.hxx>
#include <sfx2/brokenpackageint.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/sfxuno.hxx>
#include <sfxslots.hxx>

#include <sal/config.h>
#include <sal/log.hxx>
#include <comphelper/interaction.hxx>
#include <osl/diagnose.h>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <tools/debug.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/document/BrokenPackageRequest.hpp>
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/ucb/XContent.hpp>

#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;

// needs to be converted to a better data structure
SfxFormalArgument const aFormalArgs[] = {
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "SuggestedSaveAsName", SID_DEFAULTFILENAME },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "SuggestedSaveAsDir", SID_DEFAULTFILEPATH },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "VersionAuthor", SID_DOCINFO_AUTHOR },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "VersionComment", SID_DOCINFO_COMMENTS },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "DontTerminateEdit", FN_PARAM_1 },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "VersionMajor", SID_DOCINFO_MAJOR },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "FilterOptions", SID_FILE_FILTEROPTIONS },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "FilterName", SID_FILTER_NAME },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "Margin1", SID_RULER_MARGIN1 },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "Margin2", SID_RULER_MARGIN2 },
//    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "FileName", SID_FILE_NAME },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "URL", SID_FILE_NAME },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "OpenFlags", SID_OPTIONS },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "Overwrite", SID_OVERWRITE },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "Password", SID_PASSWORD },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "PasswordInteraction", SID_PASSWORDINTERACTION },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "Referer", SID_REFERER },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "SaveTo", SID_SAVETO },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "TemplateName", SID_TEMPLATE_NAME },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "TemplateRegion", SID_TEMPLATE_REGIONNAME },
//    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "Region", SID_TEMPLATE_REGIONNAME },
//    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "Name", SID_TEMPLATE_NAME },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "Unpacked", SID_UNPACK },
    { reinterpret_cast<SfxType*>(&aSfxInt16Item_Impl), "Version", SID_VERSION },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "SaveACopy", SID_SAVEACOPYITEM },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "NoFileSync", SID_NO_FILE_SYNC },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "NoThumbnail", SID_NO_THUMBNAIL },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "NoEmbDataSet", SID_NO_EMBEDDED_DS },
    { reinterpret_cast<SfxType*>(&aSfxBoolItem_Impl), "IsRedactMode", SID_IS_REDACT_MODE },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "RedactionStyle", SID_REDACTION_STYLE },
    { reinterpret_cast<SfxType*>(&aSfxStringItem_Impl), "AdditionsTag", FN_PARAM_ADDITIONS_TAG },
};

sal_uInt16 const nMediaArgsCount = SAL_N_ELEMENTS(aFormalArgs);

constexpr OUString sTemplateRegionName = u"TemplateRegionName"_ustr;
constexpr OUString sTemplateName = u"TemplateName"_ustr;
constexpr OUString sAsTemplate = u"AsTemplate"_ustr;
constexpr OUString sOpenNewView = u"OpenNewView"_ustr;
constexpr OUString sViewId = u"ViewId"_ustr;
constexpr OUString sPluginMode = u"PluginMode"_ustr;
constexpr OUString sReadOnly = u"ReadOnly"_ustr;
constexpr OUString sDdeReconnect = u"DDEReconnect"_ustr;
constexpr OUString sStartPresentation = u"StartPresentation"_ustr;
constexpr OUString sFrameName = u"FrameName"_ustr;
constexpr OUString sMediaType = u"MediaType"_ustr;
constexpr OUString sPostData = u"PostData"_ustr;
constexpr OUString sCharacterSet = u"CharacterSet"_ustr;
constexpr OUString sInputStream = u"InputStream"_ustr;
constexpr OUString sStream = u"Stream"_ustr;
constexpr OUString sOutputStream = u"OutputStream"_ustr;
constexpr OUString sHidden = u"Hidden"_ustr;
constexpr OUString sPreview = u"Preview"_ustr;
constexpr OUString sViewOnly = u"ViewOnly"_ustr;
constexpr OUString sDontEdit = u"DontEdit"_ustr;
constexpr OUString sSilent = u"Silent"_ustr;
constexpr OUString sJumpMark = u"JumpMark"_ustr;
constexpr OUString sSalvagedFile = u"SalvagedFile"_ustr;
constexpr OUString sStatusInd = u"StatusIndicator"_ustr;
constexpr OUString sModel = u"Model"_ustr;
constexpr OUString sFrame = u"Frame"_ustr;
constexpr OUString sViewData = u"ViewData"_ustr;
constexpr OUString sFilterData = u"FilterData"_ustr;
constexpr OUString sSelectionOnly = u"SelectionOnly"_ustr;
constexpr OUString sMacroExecMode = u"MacroExecutionMode"_ustr;
constexpr OUString sUpdateDocMode = u"UpdateDocMode"_ustr;
constexpr OUString sMinimized = u"Minimized"_ustr;
constexpr OUString sInteractionHdl = u"InteractionHandler"_ustr;
constexpr OUString sUCBContent = u"UCBContent"_ustr;
constexpr OUString sRepairPackage = u"RepairPackage"_ustr;
constexpr OUString sDocumentTitle = u"DocumentTitle"_ustr;
constexpr OUString sComponentData = u"ComponentData"_ustr;
constexpr OUString sComponentContext = u"ComponentContext"_ustr;
constexpr OUString sDocumentBaseURL = u"DocumentBaseURL"_ustr;
constexpr OUString sHierarchicalDocumentName = u"HierarchicalDocumentName"_ustr;
constexpr OUString sCopyStreamIfPossible = u"CopyStreamIfPossible"_ustr;
constexpr OUString sNoAutoSave = u"NoAutoSave"_ustr;
constexpr OUString sFolderName = u"FolderName"_ustr;
constexpr OUString sUseSystemDialog = u"UseSystemDialog"_ustr;
constexpr OUString sStandardDir = u"StandardDir"_ustr;
constexpr OUString sDenyList = u"DenyList"_ustr;
constexpr OUString sModifyPasswordInfo = u"ModifyPasswordInfo"_ustr;
constexpr OUString sSuggestedSaveAsDir = u"SuggestedSaveAsDir"_ustr;
constexpr OUString sSuggestedSaveAsName = u"SuggestedSaveAsName"_ustr;
constexpr OUString sEncryptionData = u"EncryptionData"_ustr;
constexpr OUString sFailOnWarning = u"FailOnWarning"_ustr;
constexpr OUString sDocumentService = u"DocumentService"_ustr;
constexpr OUString sFilterProvider = u"FilterProvider"_ustr;
constexpr OUString sImageFilter = u"ImageFilter"_ustr;
constexpr OUString sLockContentExtraction = u"LockContentExtraction"_ustr;
constexpr OUString sLockExport = u"LockExport"_ustr;
constexpr OUString sLockPrint = u"LockPrint"_ustr;
constexpr OUString sLockSave = u"LockSave"_ustr;
constexpr OUString sLockEditDoc = u"LockEditDoc"_ustr;
constexpr OUString sReplaceable = u"Replaceable"_ustr;

static bool isMediaDescriptor( sal_uInt16 nSlotId )
{
    return ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC ||
             nSlotId == SID_SAVEASDOC || nSlotId == SID_SAVEDOC ||
             nSlotId == SID_SAVETO || nSlotId == SID_SAVEACOPY ||
             nSlotId == SID_EXPORTDOCASPDF || nSlotId == SID_DIRECTEXPORTDOCASPDF ||
             nSlotId == SID_EXPORTDOCASEPUB || nSlotId == SID_DIRECTEXPORTDOCASEPUB ||
             nSlotId == SID_REDACTDOC || nSlotId == SID_AUTOREDACTDOC ||
             nSlotId == SID_SAVEACOPYITEM);
}

void TransformParameters( sal_uInt16 nSlotId, const uno::Sequence<beans::PropertyValue>& rArgs, SfxAllItemSet& rSet, const SfxSlot* pSlot )
{
    if ( !pSlot )
        pSlot = SFX_SLOTPOOL().GetSlot( nSlotId );

    if ( !pSlot )
        return;

    if ( nSlotId == SID_OPENURL )
        nSlotId = SID_OPENDOC;

    const sal_Int32 nCount = rArgs.getLength();
    if ( !nCount )
        return;

    const beans::PropertyValue* pPropsVal = rArgs.getConstArray();
    if ( !pSlot->IsMode(SfxSlotMode::METHOD) )
    {
        // slot is a property
        const SfxType* pType = pSlot->GetType();
        std::unique_ptr<SfxPoolItem> pItem(pType->CreateItem());

        if ( !pItem )
        {
            SAL_WARN( "sfx", "No creator method for item: " << nSlotId );
            return;
        }

        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(nSlotId);
        bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == MapUnit::MapTwip );
        pItem->SetWhich( nWhich );
        sal_uInt16 nSubCount = pType->nAttribs;

        const beans::PropertyValue& rProp = pPropsVal[0];
        const OUString& rName = rProp.Name;
        if ( nCount == 1 && rName == pSlot->pUnoName )
        {
            // there is only one parameter and its name matches the name of the property,
            // so it's either a simple property or a complex property in one single UNO struct
            if( pItem->PutValue( rProp.Value, bConvertTwips ? CONVERT_TWIPS : 0 ) )
                // only use successfully converted items
                rSet.Put( std::move(pItem) );
            else
            {
                SAL_WARN( "sfx", "Property not convertible: " << pSlot->pUnoName );
            }
        }
#ifdef DBG_UTIL
        else if ( nSubCount == 0 )
        {
            // for a simple property there can be only one parameter and its name *must* match
            SAL_WARN("sfx.appl", "Property name does not match: " << rName);
        }
#endif
        else
        {
            // there is more than one parameter and the property is a complex one
#ifdef DBG_UTIL
            // if the dispatch API is used for UI purposes or from the testtool,
            // it is possible to skip some or all arguments,
            // but it indicates an error for macro recording;
            // so this should be notified as a warning only
            if ( nCount != nSubCount )
            {
                SAL_INFO("sfx.appl", "MacroPlayer: wrong number of parameters for slot: " << nSlotId );
            }
#endif
            // complex property; collect sub items from the parameter set and reconstruct complex item
            sal_uInt16 nFound=0;
            for ( const beans::PropertyValue& rPropValue : rArgs )
            {
                sal_uInt16 nSub;
                for ( nSub=0; nSub<nSubCount; nSub++ )
                {
                    // search sub item by name
                    OUString aStr = pSlot->pUnoName + "." + OUString::createFromAscii(pType->aAttrib[nSub].pName);
                    if ( rPropValue.Name == aStr )
                    {
                        sal_uInt8 nSubId = static_cast<sal_uInt8>(static_cast<sal_Int8>(pType->aAttrib[nSub].nAID));
                        if ( bConvertTwips )
                            nSubId |= CONVERT_TWIPS;
                        if ( pItem->PutValue( rPropValue.Value, nSubId ) )
                            nFound++;
                        else
                        {
                            SAL_WARN( "sfx.appl", "Property not convertible: " << pSlot->pUnoName);
                        }
                        break;
                    }
                }

                // there was a parameter with a name that didn't match to any of the members
                SAL_WARN_IF( nSub >= nSubCount, "sfx.appl", "Property name does not match: " << rPropValue.Name );
            }

            // at least one part of the complex item must be present; other parts can have default values
            if ( nFound > 0 )
                rSet.Put( std::move(pItem) );
        }

        return;
    }

#ifdef DBG_UTIL
    // detect parameters that don't match to any formal argument or one of its members
    sal_Int32 nFoundArgs = 0;
#endif
    // slot is a method
    bool bIsMediaDescriptor = isMediaDescriptor( nSlotId );
    sal_uInt16 nMaxArgs = bIsMediaDescriptor ? nMediaArgsCount : pSlot->nArgDefCount;
    for ( sal_uInt16 nArgs=0; nArgs<nMaxArgs; nArgs++ )
    {
        const SfxFormalArgument &rArg = bIsMediaDescriptor ? aFormalArgs[nArgs] : pSlot->GetFormalArgument( nArgs );
        std::unique_ptr<SfxPoolItem> pItem(rArg.CreateItem());
        if ( !pItem )
        {
            SAL_WARN( "sfx", "No creator method for argument: " << rArg.pName );
            return;
        }

        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(rArg.nSlotId);
        bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == MapUnit::MapTwip );
        pItem->SetWhich( nWhich );
        const SfxType* pType = rArg.pType;
        sal_uInt16 nSubCount = pType->nAttribs;
        if ( nSubCount == 0 )
        {
            // "simple" (base type) argument
            auto pProp = std::find_if(rArgs.begin(), rArgs.end(),
                [&rArg](const beans::PropertyValue& rProp) { return rProp.Name.equalsAscii(rArg.pName); });
            if (pProp != rArgs.end())
            {
#ifdef DBG_UTIL
                ++nFoundArgs;
#endif
                if( pItem->PutValue( pProp->Value, 0 ) )
                    // only use successfully converted items
                    rSet.Put( std::move(pItem) );
                else
                {
                    SAL_WARN( "sfx", "Property not convertible: " << rArg.pName );
                }
            }
        }
        else
        {
            // complex argument, could be passed in one struct
            bool bAsWholeItem = false;
            for ( const beans::PropertyValue& rProp : rArgs )
            {
                const OUString& rName = rProp.Name;
                if ( rName == OUString(rArg.pName, strlen(rArg.pName), RTL_TEXTENCODING_UTF8) )
                {
                    bAsWholeItem = true;
#ifdef DBG_UTIL
                    ++nFoundArgs;
#endif
                    if( pItem->PutValue( rProp.Value, 0 ) )
                        // only use successfully converted items
                        rSet.Put( std::move(pItem) );
                    else
                    {
                        SAL_WARN( "sfx", "Property not convertible: " << rArg.pName );
                    }
                }
            }

            if ( !bAsWholeItem )
            {
                // complex argument; collect sub items from argument array and reconstruct complex item
                // only put item if at least one member was found and had the correct type
                // (is this a good idea?! Should we ask for *all* members?)
                bool bRet = false;
                for ( const beans::PropertyValue& rProp : rArgs )
                {
                    for ( sal_uInt16 nSub=0; nSub<nSubCount; nSub++ )
                    {
                        // search sub item by name
                        OString aStr = OString::Concat(rArg.pName) + "." + pType->aAttrib[nSub].pName;
                        if ( rProp.Name.equalsAsciiL(aStr.getStr(), aStr.getLength()) )
                        {
                            // at least one member found ...
                            bRet = true;
#ifdef DBG_UTIL
                            ++nFoundArgs;
#endif
                            sal_uInt8 nSubId = static_cast<sal_uInt8>(static_cast<sal_Int8>(pType->aAttrib[nSub].nAID));
                            if ( bConvertTwips )
                                nSubId |= CONVERT_TWIPS;
                            if (!pItem->PutValue( rProp.Value, nSubId ) )
                            {
                                // ... but it was not convertible
                                bRet = false;
                                SAL_WARN( "sfx", "Property not convertible: " << rArg.pName );
                            }

                            break;
                        }
                    }
                }

                if ( bRet )
                    // only use successfully converted items
                    rSet.Put( std::move(pItem) );

            }
        }
    }

    // special additional parameters for some slots not seen in the slot definitions
    // Some of these slots are not considered to be used for macro recording, because they shouldn't be recorded as slots,
    // but as dispatching or factory or arbitrary URLs to the frame
    // Some also can use additional arguments that are not recordable (will be changed later,
    // f.e. "SaveAs" shouldn't support parameters not in the slot definition!)
    if ( nSlotId == SID_NEWWINDOW )
    {
        for ( const beans::PropertyValue& rProp : rArgs )
        {
            const OUString& rName = rProp.Name;
            if ( rName == sFrame )
            {
                Reference< XFrame > xFrame;
                OSL_VERIFY( rProp.Value >>= xFrame );
                rSet.Put( SfxUnoFrameItem( SID_FILLFRAME, xFrame ) );
            }
            else
            if ( rName == sHidden )
            {
                bool bVal = false;
                if (rProp.Value >>= bVal)
                    rSet.Put( SfxBoolItem( SID_HIDDEN, bVal ) );
            }
        }
    }
    else if ( bIsMediaDescriptor )
    {
        for ( const beans::PropertyValue& rProp : rArgs )
        {
#ifdef DBG_UTIL
            ++nFoundArgs;
#endif
            const OUString& aName = rProp.Name;
            if ( aName == sModel )
                rSet.Put( SfxUnoAnyItem( SID_DOCUMENT, rProp.Value ) );
            else if ( aName == sComponentData )
            {
                rSet.Put( SfxUnoAnyItem( SID_COMPONENTDATA, rProp.Value ) );
            }
            else if ( aName == sComponentContext )
            {
                rSet.Put( SfxUnoAnyItem( SID_COMPONENTCONTEXT, rProp.Value ) );
            }
            else if ( aName == sStatusInd )
            {
                Reference<task::XStatusIndicator> xVal;
                bool bOK = (rProp.Value >>= xVal);
                DBG_ASSERT( bOK, "invalid type for StatusIndicator" );
                if (bOK && xVal.is())
                    rSet.Put( SfxUnoAnyItem( SID_PROGRESS_STATUSBAR_CONTROL, rProp.Value ) );
            }
            else if ( aName == sInteractionHdl )
            {
                Reference<task::XInteractionHandler> xVal;
                bool bOK = (rProp.Value >>= xVal);
                DBG_ASSERT( bOK, "invalid type for InteractionHandler" );
                if (bOK && xVal.is())
                    rSet.Put( SfxUnoAnyItem( SID_INTERACTIONHANDLER, rProp.Value ) );
            }
            else if ( aName == sViewData )
                rSet.Put( SfxUnoAnyItem( SID_VIEW_DATA, rProp.Value ) );
            else if ( aName == sFilterData )
                rSet.Put( SfxUnoAnyItem( SID_FILTER_DATA, rProp.Value ) );
            else if ( aName == sInputStream )
            {
                Reference< XInputStream > xVal;
                bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for InputStream" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_INPUTSTREAM, rProp.Value ) );
            }
            else if ( aName == sStream )
            {
                Reference< XInputStream > xVal;
                bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for Stream" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_STREAM, rProp.Value ) );
            }
            else if ( aName == sUCBContent )
            {
                Reference< XContent > xVal;
                bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for UCBContent" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_CONTENT, rProp.Value ) );
            }
            else if ( aName == sOutputStream )
            {
                Reference< XOutputStream > xVal;
                bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for OutputStream" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_OUTPUTSTREAM, rProp.Value ) );
            }
            else if ( aName == sPostData )
            {
                Reference< XInputStream > xVal;
                bool bOK = (rProp.Value >>= xVal);
                DBG_ASSERT( bOK, "invalid type for PostData" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_POSTDATA, rProp.Value ) );
            }
            else if ( aName == sFrame )
            {
                Reference< XFrame > xFrame;
                bool bOK = (rProp.Value >>= xFrame);
                DBG_ASSERT( bOK, "invalid type for Frame" );
                if (bOK)
                    rSet.Put( SfxUnoFrameItem( SID_FILLFRAME, xFrame ) );
            }
            else if ( aName == sAsTemplate )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for AsTemplate" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_TEMPLATE, bVal ) );
            }
            else if ( aName == sOpenNewView )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for OpenNewView" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_OPEN_NEW_VIEW, bVal ) );
            }
            else if ( aName == sFailOnWarning )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for FailOnWarning" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_FAIL_ON_WARNING, bVal ) );
            }
            else if ( aName == sViewId )
            {
                sal_Int16 nVal = -1;
                bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for ViewId" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_VIEW_ID, nVal ) );
            }
            else if ( aName == sPluginMode )
            {
                sal_Int16 nVal = -1;
                bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for PluginMode" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_PLUGIN_MODE, nVal ) );
            }
            else if ( aName == sReadOnly )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ReadOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_DOC_READONLY, bVal ) );
            }
            else if ( aName == sDdeReconnect )
            {
                bool bVal = true;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for DDEReconnect" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_DDE_RECONNECT_ONLOAD, bVal ) );
            }
            else if ( aName == sStartPresentation )
            {
                sal_uInt16 nVal = 0;
                bool bOK = (rProp.Value >>= nVal);
                DBG_ASSERT( bOK, "invalid type for StartPresentation" );
                if (bOK)
                    rSet.Put(SfxUInt16Item(SID_DOC_STARTPRESENTATION, nVal));
            }
            else if ( aName == sSelectionOnly )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for SelectionOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_SELECTION, bVal ) );
            }
            else if ( aName == sHidden )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Hidden" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_HIDDEN, bVal ) );
            }
            else if ( aName == sMinimized )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Minimized" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_MINIMIZED, bVal ) );
            }
            else if ( aName == sSilent )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Silent" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_SILENT, bVal ) );
            }
            else if ( aName == sPreview )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Preview" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_PREVIEW, bVal ) );
            }
            else if ( aName == sViewOnly )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ViewOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_VIEWONLY, bVal ) );
            }
            else if ( aName == sDontEdit )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ViewOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_EDITDOC, !bVal ) );
            }
            else if ( aName == sUseSystemDialog )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ViewOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_FILE_DIALOG, bVal ) );
            }
            else if ( aName == sStandardDir )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for StandardDir" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_STANDARD_DIR, sVal ) );
            }
            else if ( aName == sDenyList )
            {
                uno::Sequence<OUString> xVal;
                bool bOK = (rProp.Value >>= xVal);
                DBG_ASSERT( bOK, "invalid type or value for DenyList" );
                if (bOK)
                {
                    SfxStringListItem stringList(SID_DENY_LIST);
                    stringList.SetStringList( xVal );
                    rSet.Put( stringList );
                }
            }
            else if ( aName == "FileName" )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for FileName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_FILE_NAME, sVal ) );
            }
            else if ( aName == sSalvagedFile )
            {
                OUString sVal;
                bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type or value for SalvagedFile" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOC_SALVAGE, sVal ) );
            }
            else if ( aName == sFolderName )
            {
                OUString sVal;
                bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type or value for FolderName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_PATH, sVal ) );
            }
            else if ( aName == sFrameName )
            {
                OUString sVal;
                bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type for FrameName" );
                if (bOK && !sVal.isEmpty())
                    rSet.Put( SfxStringItem( SID_TARGETNAME, sVal ) );
            }
            else if ( aName == sMediaType )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for MediaType" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_CONTENTTYPE, sVal ) );
            }
            else if ( aName == sTemplateName )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for TemplateName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_TEMPLATE_NAME, sVal ) );
            }
            else if ( aName == sTemplateRegionName )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for TemplateRegionName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_TEMPLATE_REGIONNAME, sVal ) );
            }
            else if ( aName == sJumpMark )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for JumpMark" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_JUMPMARK, sVal ) );
            }
            else if ( aName == sCharacterSet )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for CharacterSet" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_CHARSET, sVal ) );
            }
            else if ( aName == "FilterFlags" )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for FilterFlags" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sVal ) );
            }
            else if ( aName == sImageFilter )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for FilterFlags" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_CONVERT_IMAGES, sVal ) );
            }
            else if ( aName == sMacroExecMode )
            {
                sal_Int16 nVal =-1;
                bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for MacroExecMode" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_MACROEXECMODE, nVal ) );
            }
            else if ( aName == sUpdateDocMode )
            {
                sal_Int16 nVal =-1;
                bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for UpdateDocMode" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_UPDATEDOCMODE, nVal ) );
            }
            else if ( aName == sRepairPackage )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for RepairPackage" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_REPAIRPACKAGE, bVal ) );
            }
            else if ( aName == sDocumentTitle )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for DocumentTitle" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOCINFO_TITLE, sVal ) );
            }
            else if ( aName == sDocumentBaseURL )
            {
                OUString sVal;
                // the base url can be set to empty ( for embedded objects for example )
                bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type or value for DocumentBaseURL" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOC_BASEURL, sVal ) );
            }
            else if ( aName == sHierarchicalDocumentName )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for HierarchicalDocumentName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOC_HIERARCHICALNAME, sVal ) );
            }
            else if ( aName == sCopyStreamIfPossible )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for CopyStreamIfPossible" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_COPY_STREAM_IF_POSSIBLE, bVal ) );
            }
            else if ( aName == sNoAutoSave )
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for NoAutoSave" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_NOAUTOSAVE, bVal ) );
            }
            else if ( aName == sModifyPasswordInfo )
            {
                rSet.Put( SfxUnoAnyItem( SID_MODIFYPASSWORDINFO, rProp.Value ) );
            }
            else if ( aName == sEncryptionData )
            {
                rSet.Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, rProp.Value ) );
            }
            else if ( aName == sSuggestedSaveAsDir )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for SuggestedSaveAsDir" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_SUGGESTEDSAVEASDIR, sVal ) );
            }
            else if ( aName == sSuggestedSaveAsName )
            {
                OUString sVal;
                bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for SuggestedSaveAsName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_SUGGESTEDSAVEASNAME, sVal ) );
            }
            else if (aName == sDocumentService)
            {
                OUString aVal;
                bool bOK = ((rProp.Value >>= aVal) && !aVal.isEmpty());
                if (bOK)
                    rSet.Put(SfxStringItem(SID_DOC_SERVICE, aVal));
            }
            else if (aName == sFilterProvider)
            {
                OUString aVal;
                bool bOK = ((rProp.Value >>= aVal) && !aVal.isEmpty());
                if (bOK)
                    rSet.Put(SfxStringItem(SID_FILTER_PROVIDER, aVal));
            }
            else if (aName == sLockContentExtraction)
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for LockContentExtraction" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_LOCK_CONTENT_EXTRACTION, bVal ) );
            }
            else if (aName == sLockExport)
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for LockExport" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_LOCK_EXPORT, bVal ) );
            }
            else if (aName == sLockPrint)
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for LockPrint" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_LOCK_PRINT, bVal ) );
            }
            else if (aName == sLockSave)
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for LockSave" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_LOCK_SAVE, bVal ) );
            }
            else if (aName == sLockEditDoc)
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for LockEditDoc" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_LOCK_EDITDOC, bVal ) );
            }
            else if (aName == sReplaceable)
            {
                bool bVal = false;
                bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT(bOK, "invalid type for Replaceable");
                if (bOK)
                    rSet.Put(SfxBoolItem(SID_REPLACEABLE, bVal));
            }
#ifdef DBG_UTIL
            else
                --nFoundArgs;
#endif
        }
    }
    // API to raise options dialog with a specified options ab page (#i83757#)
    else
    {
        // transform parameter "OptionsPageURL" of slot "OptionsTreeDialog"
        if ( "OptionsTreeDialog" == pSlot->pUnoName )
        {
            auto pProp = std::find_if(rArgs.begin(), rArgs.end(),
                [](const PropertyValue& rProp) { return rProp.Name == "OptionsPageURL" || rProp.Name == "OptionsPageID"; });
            if (pProp != rArgs.end())
            {
                OUString sURL;
                sal_uInt16 nPageID;
                if ( pProp->Name == "OptionsPageURL" && (pProp->Value >>= sURL) )
                    rSet.Put( SfxStringItem( SID_OPTIONS_PAGEURL, sURL ) );
                else if ( pProp->Name == "OptionsPageID" && (pProp->Value >>= nPageID) )
                    rSet.Put( SfxUInt16Item( SID_OPTIONS_PAGEID, nPageID ) );
            }
        }
    }
#ifdef DBG_UTIL
    if ( nFoundArgs == nCount )
    {
        // except for the "special" slots: assure that every argument was convertible
        SAL_INFO( "sfx.appl", "MacroPlayer: Some properties didn't match to any formal argument for slot: "<< pSlot->pUnoName );
    }
#endif
}

void TransformItems( sal_uInt16 nSlotId, const SfxItemSet& rSet, uno::Sequence<beans::PropertyValue>& rArgs, const SfxSlot* pSlot )
{
    if ( !pSlot )
        pSlot = SFX_SLOTPOOL().GetSlot( nSlotId );

    if ( !pSlot)
        return;

    if ( nSlotId == SID_OPENURL )
        nSlotId = SID_OPENDOC;
    if ( nSlotId == SID_SAVEASREMOTE )
        nSlotId = SID_SAVEASDOC;

    // find number of properties to avoid permanent reallocations in the sequence
    sal_Int32 nProps=0;

#ifdef DBG_UTIL
    // trace number of items and compare with number of properties for debugging purposes
    sal_Int32 nItems=0;
#endif

    const SfxType *pType = pSlot->GetType();
    if ( !pSlot->IsMode(SfxSlotMode::METHOD) )
    {
        // slot is a property
        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(nSlotId);
        if ( rSet.GetItemState( nWhich ) == SfxItemState::SET ) //???
        {
            sal_uInt16 nSubCount = pType->nAttribs;
            if ( nSubCount )
                // it's a complex property, we want it split into simple types
                // so we expect to get as many items as we have (sub) members
                nProps = nSubCount;
            else
                // simple property: we expect to get exactly one item
                nProps++;
        }
        else
        {
            // we will not rely on the "toggle" ability of some property slots
            SAL_WARN( "sfx", "Processing property slot without argument: " << nSlotId );
        }

#ifdef DBG_UTIL
        nItems++;
#endif
    }
    else
    {
        // slot is a method
        bool bIsMediaDescriptor = isMediaDescriptor( nSlotId );
        sal_uInt16 nFormalArgs = bIsMediaDescriptor ? nMediaArgsCount : pSlot->GetFormalArgumentCount();
        for ( sal_uInt16 nArg=0; nArg<nFormalArgs; ++nArg )
        {
            // check every formal argument of the method
            const SfxFormalArgument &rArg = bIsMediaDescriptor ? aFormalArgs[nArg] : pSlot->GetFormalArgument( nArg );

            sal_uInt16 nWhich = rSet.GetPool()->GetWhich( rArg.nSlotId );
            if ( rSet.GetItemState( nWhich ) == SfxItemState::SET ) //???
            {
                sal_uInt16 nSubCount = rArg.pType->nAttribs;
                if ( nSubCount )
                    // argument has a complex type, we want it split into simple types
                    // so for this argument we expect to get as many items as we have (sub) members
                    nProps += nSubCount;
                else
                    // argument of simple type: we expect to get exactly one item for it
                    nProps++;
#ifdef DBG_UTIL
                nItems++;
#endif
            }
        }

        // special treatment for slots that are *not* meant to be recorded as slots (except SaveAs/To)
        if ( bIsMediaDescriptor )
        {
            sal_Int32 nAdditional=0;
            if ( rSet.GetItemState( SID_PROGRESS_STATUSBAR_CONTROL ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_INTERACTIONHANDLER ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_SALVAGE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_PATH ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FILE_DIALOG ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_STANDARD_DIR ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DENY_LIST ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_CONTENT ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_INPUTSTREAM ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_STREAM ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_OUTPUTSTREAM ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TEMPLATE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_OPEN_NEW_VIEW ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FAIL_ON_WARNING ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_VIEW_ID ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_VIEW_DATA ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FILTER_DATA ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_PLUGIN_MODE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_READONLY ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DDE_RECONNECT_ONLOAD ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_STARTPRESENTATION ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SELECTION ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_CONTENTTYPE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_POSTDATA ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FILLFRAME ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_CHARSET ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TARGETNAME ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TEMPLATE_NAME ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TEMPLATE_REGIONNAME ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_HIDDEN ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_MINIMIZED ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_PREVIEW ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_VIEWONLY ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_EDITDOC ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SILENT ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_JUMPMARK ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOCUMENT ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_MACROEXECMODE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_UPDATEDOCMODE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_REPAIRPACKAGE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOCINFO_TITLE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_COMPONENTDATA ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_COMPONENTCONTEXT ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_BASEURL ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_HIERARCHICALNAME ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_COPY_STREAM_IF_POSSIBLE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_NOAUTOSAVE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_MODIFYPASSWORDINFO ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SUGGESTEDSAVEASDIR ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_ENCRYPTIONDATA ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SUGGESTEDSAVEASNAME ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_SERVICE ) == SfxItemState::SET )
                nAdditional++;
            if (rSet.HasItem(SID_FILTER_PROVIDER))
                ++nAdditional;
            if ( rSet.GetItemState( SID_CONVERT_IMAGES ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_LOCK_CONTENT_EXTRACTION ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_LOCK_EXPORT ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_LOCK_PRINT ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_LOCK_SAVE ) == SfxItemState::SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_LOCK_EDITDOC ) == SfxItemState::SET )
                nAdditional++;
            if (rSet.GetItemState(SID_REPLACEABLE) == SfxItemState::SET)
                nAdditional++;

            // consider additional arguments
            nProps += nAdditional;
#ifdef DBG_UTIL
            nItems += nAdditional;
#endif
        }
    }

#ifdef DBG_UTIL
    // now check the itemset: is there any item that is not convertible using the list of formal arguments
    // or the table of additional items?!
    if ( rSet.Count() != nItems )
    {
        // detect unknown item and present error message
        for ( auto const & rPair : rSet.GetRanges() )
        {
            sal_uInt16 nStartWhich = rPair.first;
            sal_uInt16 nEndWhich = rPair.second;
            for(sal_uInt16 nId = nStartWhich; nId <= nEndWhich; ++nId)
            {
                if ( rSet.GetItemState(nId) < SfxItemState::SET ) //???
                    // not really set
                    continue;

                if ( !pSlot->IsMode(SfxSlotMode::METHOD) && nId == rSet.GetPool()->GetWhich( pSlot->GetSlotId() ) )
                    continue;

                bool bIsMediaDescriptor = isMediaDescriptor( nSlotId );
                sal_uInt16 nFormalArgs = bIsMediaDescriptor ? nMediaArgsCount : pSlot->nArgDefCount;
                sal_uInt16 nArg;
                for ( nArg=0; nArg<nFormalArgs; ++nArg )
                {
                    const SfxFormalArgument &rArg = bIsMediaDescriptor ? aFormalArgs[nArg] : pSlot->GetFormalArgument( nArg );
                    sal_uInt16 nWhich = rSet.GetPool()->GetWhich( rArg.nSlotId );
                    if ( nId == nWhich )
                        break;
                }

                if ( nArg<nFormalArgs )
                    continue;

                if ( bIsMediaDescriptor )
                {
                    if ( nId == SID_DOCFRAME )
                        continue;
                    if ( nId == SID_PROGRESS_STATUSBAR_CONTROL )
                        continue;
                    if ( nId == SID_INTERACTIONHANDLER )
                        continue;
                    if ( nId == SID_VIEW_DATA )
                        continue;
                    if ( nId == SID_FILTER_DATA )
                        continue;
                    if ( nId == SID_DOCUMENT )
                        continue;
                    if ( nId == SID_CONTENT )
                        continue;
                    if ( nId == SID_INPUTSTREAM )
                        continue;
                    if ( nId == SID_STREAM )
                        continue;
                    if ( nId == SID_OUTPUTSTREAM )
                        continue;
                    if ( nId == SID_POSTDATA )
                        continue;
                    if ( nId == SID_FILLFRAME )
                        continue;
                    if ( nId == SID_TEMPLATE )
                        continue;
                    if ( nId == SID_OPEN_NEW_VIEW )
                        continue;
                    if ( nId == SID_VIEW_ID )
                        continue;
                    if ( nId == SID_PLUGIN_MODE )
                        continue;
                    if ( nId == SID_DOC_READONLY )
                        continue;
                    if ( nId == SID_DOC_STARTPRESENTATION )
                        continue;
                    if ( nId == SID_SELECTION )
                        continue;
                    if ( nId == SID_HIDDEN )
                        continue;
                    if ( nId == SID_MINIMIZED )
                        continue;
                    if ( nId == SID_SILENT )
                        continue;
                    if ( nId == SID_PREVIEW )
                        continue;
                    if ( nId == SID_VIEWONLY )
                        continue;
                    if ( nId == SID_EDITDOC )
                        continue;
                    if ( nId == SID_TARGETNAME )
                        continue;
                    if ( nId == SID_DOC_SALVAGE )
                        continue;
                    if ( nId == SID_PATH )
                        continue;
                    if ( nId == SID_FILE_DIALOG )
                        continue;
                    if ( nId == SID_STANDARD_DIR )
                        continue;
                    if ( nId == SID_DENY_LIST )
                        continue;
                    if ( nId == SID_CONTENTTYPE )
                        continue;
                    if ( nId == SID_TEMPLATE_NAME )
                        continue;
                    if ( nId == SID_TEMPLATE_REGIONNAME )
                        continue;
                    if ( nId == SID_JUMPMARK )
                        continue;
                    if ( nId == SID_CHARSET )
                        continue;
                    if ( nId == SID_MACROEXECMODE )
                        continue;
                    if ( nId == SID_UPDATEDOCMODE )
                        continue;
                    if ( nId == SID_REPAIRPACKAGE )
                        continue;
                    if ( nId == SID_DOCINFO_TITLE )
                        continue;
                    if ( nId == SID_COMPONENTDATA )
                        continue;
                    if ( nId == SID_COMPONENTCONTEXT )
                        continue;
                    if ( nId == SID_DOC_BASEURL )
                        continue;
                    if ( nId == SID_DOC_HIERARCHICALNAME )
                        continue;
                    if ( nId == SID_COPY_STREAM_IF_POSSIBLE )
                        continue;
                    if ( nId == SID_NOAUTOSAVE )
                        continue;
                    if ( nId == SID_ENCRYPTIONDATA )
                        continue;
                    if ( nId == SID_DOC_SERVICE )
                        continue;
                    if (nId == SID_FILTER_PROVIDER)
                        continue;
                    if ( nId == SID_CONVERT_IMAGES )
                        continue;

                    // used only internally
                    if ( nId == SID_SAVETO )
                        continue;
                    if ( nId == SID_SAVEACOPYITEM )
                        continue;
                    if ( nId == SID_MODIFYPASSWORDINFO )
                        continue;
                    if ( nId == SID_SUGGESTEDSAVEASDIR )
                        continue;
                    if ( nId == SID_SUGGESTEDSAVEASNAME )
                        continue;
                    if ( nId == SID_LOCK_CONTENT_EXTRACTION )
                        continue;
                    if ( nId == SID_LOCK_EXPORT )
                        continue;
                    if ( nId == SID_LOCK_PRINT )
                        continue;
                    if ( nId == SID_LOCK_SAVE )
                        continue;
                    if ( nId == SID_LOCK_EDITDOC )
                        continue;
                    if (nId == SID_REPLACEABLE)
                        continue;
                }

                OString aDbg = "Unknown item detected: " + OString::number(static_cast<sal_Int32>(nId));
                DBG_ASSERT(nArg<nFormalArgs, aDbg.getStr());
            }
        }
    }
#endif

    if ( !nProps )
        return;

    // convert every item into a property
    uno::Sequence<beans::PropertyValue> aSequ(nProps);
    beans::PropertyValue *pValue = aSequ.getArray();

    sal_Int32 nActProp=0;
    if ( !pSlot->IsMode(SfxSlotMode::METHOD) )
    {
        // slot is a property
        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(nSlotId);
        bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == MapUnit::MapTwip );
        const SfxPoolItem* pItem = rSet.GetItem<SfxPoolItem>(nWhich, false);
        if ( pItem ) //???
        {
            sal_uInt16 nSubCount = pType->nAttribs;
            if ( !nSubCount )
            {
                pValue[nActProp].Name = pSlot->pUnoName;
                if ( !pItem->QueryValue( pValue[nActProp].Value ) )
                {
                    SAL_WARN( "sfx", "Item not convertible: " << nSlotId );
                }
            }
            else
            {
                // complex type, add a property value for every member of the struct
                for ( sal_uInt16 n=1; n<=nSubCount; ++n )
                {
                    sal_uInt8 nSubId = static_cast<sal_uInt8>(static_cast<sal_Int8>(pType->aAttrib[n-1].nAID));
                    if ( bConvertTwips )
                        nSubId |= CONVERT_TWIPS;

                    DBG_ASSERT(( pType->aAttrib[n-1].nAID ) <= 127, "Member ID out of range" );
                    pValue[nActProp].Name = pSlot->pUnoName +
                        "." +
                        OUString::createFromAscii( pType->aAttrib[n-1].pName );
                    if ( !pItem->QueryValue( pValue[nActProp++].Value, nSubId ) )
                    {
                        SAL_WARN( "sfx", "Sub item " << pType->aAttrib[n-1].nAID
                                    << " not convertible in slot: " << nSlotId );
                    }
                }
            }
        }

        rArgs = aSequ;
        return;
    }

    // slot is a method
    sal_uInt16 nFormalArgs = pSlot->GetFormalArgumentCount();
    for ( sal_uInt16 nArg=0; nArg<nFormalArgs; ++nArg )
    {
        const SfxFormalArgument &rArg = pSlot->GetFormalArgument( nArg );
        sal_uInt16 nWhich = rSet.GetPool()->GetWhich( rArg.nSlotId );
        bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == MapUnit::MapTwip );
        const SfxPoolItem* pItem = rSet.GetItem<SfxPoolItem>(nWhich, false);
        if ( pItem ) //???
        {
            sal_uInt16 nSubCount = rArg.pType->nAttribs;
            if ( !nSubCount )
            {
                pValue[nActProp].Name = OUString::createFromAscii( rArg.pName ) ;
                if ( !pItem->QueryValue( pValue[nActProp++].Value ) )
                {
                    SAL_WARN( "sfx", "Item not convertible: " << rArg.nSlotId );
                }
            }
            else
            {
                // complex type, add a property value for every member of the struct
                for ( sal_uInt16 n = 1; n <= nSubCount; ++n )
                {
                    sal_uInt8 nSubId = static_cast<sal_uInt8>(static_cast<sal_Int8>(rArg.pType->aAttrib[n-1].nAID));
                    if ( bConvertTwips )
                        nSubId |= CONVERT_TWIPS;

                    DBG_ASSERT((rArg.pType->aAttrib[n-1].nAID) <= 127, "Member ID out of range" );
                    pValue[nActProp].Name = OUString::createFromAscii( rArg.pName ) +
                        "." +
                        OUString::createFromAscii( rArg.pType->aAttrib[n-1].pName ) ;
                    if ( !pItem->QueryValue( pValue[nActProp++].Value, nSubId ) )
                    {
                        SAL_WARN( "sfx", "Sub item "
                                    << rArg.pType->aAttrib[n-1].nAID
                                    << " not convertible in slot: "
                                    << rArg.nSlotId );
                    }
                }
            }
        }
    }

    if ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC || nSlotId == SID_SAVEASDOC ||  nSlotId == SID_SAVEDOC ||
         nSlotId == SID_SAVETO || nSlotId == SID_EXPORTDOCASPDF || nSlotId == SID_DIRECTEXPORTDOCASPDF ||
         nSlotId == SID_EXPORTDOCASEPUB || nSlotId == SID_DIRECTEXPORTDOCASEPUB ||
         nSlotId == SID_REDACTDOC || nSlotId == SID_AUTOREDACTDOC || nSlotId == SID_SAVEACOPY )
    {
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_COMPONENTDATA, false) )
        {
            pValue[nActProp].Name = sComponentData;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_COMPONENTCONTEXT, false) )
        {
            pValue[nActProp].Name = sComponentContext;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_PROGRESS_STATUSBAR_CONTROL, false) )
        {
            pValue[nActProp].Name = sStatusInd;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_INTERACTIONHANDLER, false) )
        {
            pValue[nActProp].Name = sInteractionHdl;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_VIEW_DATA, false) )
        {
            pValue[nActProp].Name = sViewData;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_FILTER_DATA, false) )
        {
            pValue[nActProp].Name = sFilterData;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_DOCUMENT, false) )
        {
            pValue[nActProp].Name = sModel;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_CONTENT, false) )
        {
            pValue[nActProp].Name = sUCBContent;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_INPUTSTREAM, false) )
        {
            pValue[nActProp].Name = sInputStream;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_STREAM, false) )
        {
            pValue[nActProp].Name = sStream;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_OUTPUTSTREAM, false) )
        {
            pValue[nActProp].Name = sOutputStream;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_POSTDATA, false) )
        {
            pValue[nActProp].Name = sPostData;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxPoolItem *pItem = nullptr; SfxItemState::SET == rSet.GetItemState( SID_FILLFRAME, false, &pItem) )
        {
            pValue[nActProp].Name = sFrame;
            if ( auto pUsrAnyItem = dynamic_cast< const SfxUnoAnyItem *>( pItem ) )
            {
                OSL_FAIL( "TransformItems: transporting an XFrame via an SfxUnoAnyItem is not deprecated!" );
                pValue[nActProp++].Value = pUsrAnyItem->GetValue();
            }
            else if ( auto pUnoFrameItem = dynamic_cast< const SfxUnoFrameItem *>( pItem ) )
                pValue[nActProp++].Value <<= pUnoFrameItem->GetFrame();
            else
                OSL_FAIL( "TransformItems: invalid item type for SID_FILLFRAME!" );
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_TEMPLATE, false) )
        {
            pValue[nActProp].Name = sAsTemplate;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_OPEN_NEW_VIEW, false) )
        {
            pValue[nActProp].Name = sOpenNewView;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_FAIL_ON_WARNING, false) )
        {
            pValue[nActProp].Name = sFailOnWarning;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxUInt16Item *pItem = rSet.GetItemIfSet( SID_VIEW_ID, false) )
        {
            pValue[nActProp].Name = sViewId;
            pValue[nActProp++].Value <<= static_cast<sal_Int16>(pItem->GetValue());
        }
        if ( const SfxUInt16Item *pItem = rSet.GetItemIfSet( SID_PLUGIN_MODE, false) )
        {
            pValue[nActProp].Name = sPluginMode;
            pValue[nActProp++].Value <<= static_cast<sal_Int16>(pItem->GetValue());
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_DOC_READONLY, false) )
        {
            pValue[nActProp].Name = sReadOnly;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_DDE_RECONNECT_ONLOAD, false) )
        {
            pValue[nActProp].Name = sDdeReconnect;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if (const SfxUInt16Item* pItem = rSet.GetItemIfSet(SID_DOC_STARTPRESENTATION, false))
        {
            pValue[nActProp].Name = sStartPresentation;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_SELECTION, false) )
        {
            pValue[nActProp].Name = sSelectionOnly;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_HIDDEN, false) )
        {
            pValue[nActProp].Name = sHidden;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_MINIMIZED, false) )
        {
            pValue[nActProp].Name = sMinimized;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_SILENT, false) )
        {
            pValue[nActProp].Name = sSilent;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_PREVIEW, false) )
        {
            pValue[nActProp].Name = sPreview;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_VIEWONLY, false) )
        {
            pValue[nActProp].Name = sViewOnly;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_EDITDOC, false) )
        {
            pValue[nActProp].Name = sDontEdit;
            pValue[nActProp++].Value <<= !pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_FILE_DIALOG, false) )
        {
            pValue[nActProp].Name = sUseSystemDialog;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_STANDARD_DIR, false) )
        {
            pValue[nActProp].Name = sStandardDir;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringListItem *pItem = rSet.GetItemIfSet( SID_DENY_LIST, false) )
        {
            pValue[nActProp].Name = sDenyList;

            css::uno::Sequence< OUString > aList;
            pItem->GetStringList( aList );
            pValue[nActProp++].Value <<= aList ;
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_TARGETNAME, false) )
        {
            pValue[nActProp].Name = sFrameName;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_DOC_SALVAGE, false) )
        {
            pValue[nActProp].Name = sSalvagedFile;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_PATH, false) )
        {
            pValue[nActProp].Name = sFolderName;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_CONTENTTYPE, false) )
        {
            pValue[nActProp].Name = sMediaType;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_TEMPLATE_NAME, false) )
        {
            pValue[nActProp].Name = sTemplateName;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_TEMPLATE_REGIONNAME, false) )
        {
            pValue[nActProp].Name = sTemplateRegionName;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_JUMPMARK, false) )
        {
            pValue[nActProp].Name = sJumpMark;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }

        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_CHARSET, false) )
        {
            pValue[nActProp].Name = sCharacterSet;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxUInt16Item *pItem = rSet.GetItemIfSet( SID_MACROEXECMODE, false) )
        {
            pValue[nActProp].Name = sMacroExecMode;
            pValue[nActProp++].Value <<= static_cast<sal_Int16>(pItem->GetValue());
        }
        if ( const SfxUInt16Item *pItem = rSet.GetItemIfSet( SID_UPDATEDOCMODE, false) )
        {
            pValue[nActProp].Name = sUpdateDocMode;
            pValue[nActProp++].Value <<= static_cast<sal_Int16>(pItem->GetValue());
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_REPAIRPACKAGE, false) )
        {
            pValue[nActProp].Name = sRepairPackage;
            pValue[nActProp++].Value <<= pItem->GetValue() ;
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_DOCINFO_TITLE, false) )
        {
            pValue[nActProp].Name = sDocumentTitle;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_DOC_BASEURL, false) )
        {
            pValue[nActProp].Name = sDocumentBaseURL;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_DOC_HIERARCHICALNAME, false) )
        {
            pValue[nActProp].Name = sHierarchicalDocumentName;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_COPY_STREAM_IF_POSSIBLE, false) )
        {
            pValue[nActProp].Name = sCopyStreamIfPossible;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_NOAUTOSAVE, false) )
        {
            pValue[nActProp].Name = sNoAutoSave;
            pValue[nActProp++].Value <<= pItem->GetValue() ;
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_MODIFYPASSWORDINFO, false) )
        {
            pValue[nActProp].Name = sModifyPasswordInfo;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxUnoAnyItem *pItem = rSet.GetItemIfSet( SID_ENCRYPTIONDATA, false) )
        {
            pValue[nActProp].Name = sEncryptionData;
            pValue[nActProp++].Value = pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_SUGGESTEDSAVEASDIR, false) )
        {
            pValue[nActProp].Name = sSuggestedSaveAsDir;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_SUGGESTEDSAVEASNAME, false) )
        {
            pValue[nActProp].Name = sSuggestedSaveAsName;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxStringItem *pItem = rSet.GetItemIfSet( SID_DOC_SERVICE, false) )
        {
            pValue[nActProp].Name = sDocumentService;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if (const SfxStringItem *pItem = rSet.GetItemIfSet(SID_FILTER_PROVIDER))
        {
            pValue[nActProp].Name = sFilterProvider;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if (const SfxStringItem *pItem = rSet.GetItemIfSet(SID_CONVERT_IMAGES))
        {
            pValue[nActProp].Name = sImageFilter;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_LOCK_CONTENT_EXTRACTION, false) )
        {
            pValue[nActProp].Name = sLockContentExtraction;
            pValue[nActProp++].Value <<= pItem->GetValue() ;
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_LOCK_EXPORT, false) )
        {
            pValue[nActProp].Name = sLockExport;
            pValue[nActProp++].Value <<= pItem->GetValue() ;
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_LOCK_PRINT, false) )
        {
            pValue[nActProp].Name = sLockPrint;
            pValue[nActProp++].Value <<= pItem->GetValue() ;
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_LOCK_SAVE, false) )
        {
            pValue[nActProp].Name = sLockSave;
            pValue[nActProp++].Value <<= pItem->GetValue() ;
        }
        if ( const SfxBoolItem *pItem = rSet.GetItemIfSet( SID_LOCK_EDITDOC, false) )
        {
            pValue[nActProp].Name = sLockEditDoc;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
        if (const SfxBoolItem *pItem = rSet.GetItemIfSet(SID_REPLACEABLE, false))
        {
            pValue[nActProp].Name = sReplaceable;
            pValue[nActProp++].Value <<= pItem->GetValue();
        }
    }

    rArgs = aSequ;
}

void SAL_CALL FilterOptionsContinuation::setFilterOptions(
                const uno::Sequence<beans::PropertyValue>& rProps )
{
    rProperties = rProps;
}

uno::Sequence< beans::PropertyValue > SAL_CALL
    FilterOptionsContinuation::getFilterOptions()
{
    return rProperties;
}


RequestFilterOptions::RequestFilterOptions( uno::Reference< frame::XModel > const & rModel,
                              const uno::Sequence< beans::PropertyValue >& rProperties )
{
    uno::Reference< uno::XInterface > temp2;
    document::FilterOptionsRequest aOptionsRequest( OUString(),
                                                    temp2,
                                                    rModel,
                                                    rProperties );

    m_aRequest <<= aOptionsRequest;

    m_xAbort  = new comphelper::OInteractionAbort;
    m_xOptions = new FilterOptionsContinuation;
}

uno::Any SAL_CALL RequestFilterOptions::getRequest()
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL RequestFilterOptions::getContinuations()
{
    return { m_xAbort, m_xOptions };
}


class RequestPackageReparation_Impl : public ::cppu::WeakImplHelper< task::XInteractionRequest >
{
    uno::Any m_aRequest;
    rtl::Reference<comphelper::OInteractionApprove> m_xApprove;
    rtl::Reference<comphelper::OInteractionDisapprove>  m_xDisapprove;

public:
    explicit RequestPackageReparation_Impl( const OUString& aName );
    bool    isApproved() const;
    virtual uno::Any SAL_CALL getRequest() override;
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations() override;
};

RequestPackageReparation_Impl::RequestPackageReparation_Impl( const OUString& aName )
{
    uno::Reference< uno::XInterface > temp2;
    document::BrokenPackageRequest aBrokenPackageRequest( OUString(), temp2, aName );
    m_aRequest <<= aBrokenPackageRequest;
    m_xApprove = new comphelper::OInteractionApprove;
    m_xDisapprove = new comphelper::OInteractionDisapprove;
}

bool RequestPackageReparation_Impl::isApproved() const
{
    return m_xApprove->wasSelected();
}

uno::Any SAL_CALL RequestPackageReparation_Impl::getRequest()
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL RequestPackageReparation_Impl::getContinuations()
{
    return { m_xApprove, m_xDisapprove };
}

RequestPackageReparation::RequestPackageReparation( const OUString& aName )
    : mxImpl(new RequestPackageReparation_Impl( aName ))
{
}

RequestPackageReparation::~RequestPackageReparation()
{
}

bool RequestPackageReparation::isApproved() const
{
    return mxImpl->isApproved();
}

css::uno::Reference < task::XInteractionRequest > RequestPackageReparation::GetRequest() const
{
    return mxImpl;
}


class NotifyBrokenPackage_Impl : public ::cppu::WeakImplHelper< task::XInteractionRequest >
{
    uno::Any m_aRequest;
    rtl::Reference<comphelper::OInteractionAbort>  m_xAbort;

public:
    explicit NotifyBrokenPackage_Impl(const OUString& rName);
    virtual uno::Any SAL_CALL getRequest() override;
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations() override;
};

NotifyBrokenPackage_Impl::NotifyBrokenPackage_Impl( const OUString& aName )
{
    uno::Reference< uno::XInterface > temp2;
    document::BrokenPackageRequest aBrokenPackageRequest( OUString(), temp2, aName );
    m_aRequest <<= aBrokenPackageRequest;
    m_xAbort = new comphelper::OInteractionAbort;
}

uno::Any SAL_CALL NotifyBrokenPackage_Impl::getRequest()
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL NotifyBrokenPackage_Impl::getContinuations()
{
    return { m_xAbort };
}

NotifyBrokenPackage::NotifyBrokenPackage( const OUString& aName )
    : mxImpl(new NotifyBrokenPackage_Impl( aName ))
{
}

NotifyBrokenPackage::~NotifyBrokenPackage()
{
}

css::uno::Reference < task::XInteractionRequest > NotifyBrokenPackage::GetRequest() const
{
    return mxImpl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

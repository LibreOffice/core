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

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

#include "sfx2/appuno.hxx"

#include "backingcomp.hxx"
#include "SfxDocumentMetaData.hxx"
#include "appbaslib.hxx"
#include "doctemplates.hxx"
#include "eventsupplier.hxx"
#include "fltoptint.hxx"
#include "frmload.hxx"
#include "iframe.hxx"
#include "objshimp.hxx"
#include "plugin.hxx"
#include "sfx2/app.hxx"
#include "sfx2/brokenpackageint.hxx"
#include "sfx2/dispatch.hxx"
#include "sfx2/dllapi.h"
#include "sfx2/docfile.hxx"
#include "sfx2/fcontnr.hxx"
#include "sfx2/frame.hxx"
#include "sfx2/module.hxx"
#include "sfx2/msg.hxx"
#include "sfx2/msgpool.hxx"
#include "sfx2/objsh.hxx"
#include "sfx2/request.hxx"
#include "sfx2/sfxbasecontroller.hxx"
#include "sfx2/sfxsids.hrc"
#include "sfx2/sfxuno.hxx"
#include "sfx2/unoctitm.hxx"
#include "sfxslots.hxx"
#include "sfxtypes.hxx"
#include "shutdownicon.hxx"
#include "xpackcreator.hxx"

#include "sal/config.h"
#include <basic/basmgr.hxx>
#include <basic/sberrors.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbuno.hxx>
#include <basic/sbx.hxx>
#include <basic/sbxcore.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/sbxobj.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <framework/documentundoguard.hxx>
#include <osl/mutex.hxx>
#include <ownsubfilterservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/lckbitem.hxx>
#include <svl/ownlist.hxx>
#include <svl/rectitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <tools/config.hxx>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/FrameActionEvent.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/registry/RegistryValueType.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/ucb/XContent.hpp>

#include <boost/scoped_ptr.hpp>

#define PROTOCOLHANDLER_SERVICENAME     "com.sun.star.frame.ProtocolHandler"

using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;

// needs to be converted to a better data structure
SfxFormalArgument aFormalArgs[] = {
    SFX_ARGUMENT(SID_DEFAULTFILENAME,"SuggestedSaveAsName",SfxStringItem),
    SFX_ARGUMENT(SID_DEFAULTFILEPATH,"SuggestedSaveAsDir",SfxStringItem),
    SFX_ARGUMENT(SID_DOCINFO_AUTHOR,"VersionAuthor",SfxStringItem),
    SFX_ARGUMENT(SID_DOCINFO_COMMENTS,"VersionComment",SfxStringItem),
    SFX_ARGUMENT(SID_DOCINFO_MAJOR,"VersionMajor",SfxBoolItem),
    SFX_ARGUMENT(SID_FILE_FILTEROPTIONS,"FilterOptions",SfxStringItem),
    SFX_ARGUMENT(SID_FILTER_NAME,"FilterName",SfxStringItem),
//    SFX_ARGUMENT(SID_FILE_NAME,"FileName",SfxStringItem),
    SFX_ARGUMENT(SID_FILE_NAME,"URL",SfxStringItem),
    SFX_ARGUMENT(SID_OPTIONS,"OpenFlags",SfxStringItem),
    SFX_ARGUMENT(SID_OVERWRITE,"Overwrite",SfxBoolItem),
    SFX_ARGUMENT(SID_PASSWORD,"Password",SfxStringItem),
    SFX_ARGUMENT(SID_PASSWORDINTERACTION,"PasswordInteraction",SfxBoolItem),
    SFX_ARGUMENT(SID_REFERER,"Referer",SfxStringItem),
    SFX_ARGUMENT(SID_SAVETO,"SaveTo",SfxBoolItem),
    SFX_ARGUMENT(SID_TEMPLATE_NAME,"TemplateName",SfxStringItem),
    SFX_ARGUMENT(SID_TEMPLATE_REGIONNAME,"TemplateRegion",SfxStringItem),
//    SFX_ARGUMENT(SID_TEMPLATE_REGIONNAME,"Region",SfxStringItem),
//    SFX_ARGUMENT(SID_TEMPLATE_NAME,"Name",SfxStringItem),
    SFX_ARGUMENT(SID_UNPACK,"Unpacked",SfxBoolItem),
    SFX_ARGUMENT(SID_VERSION,"Version",SfxInt16Item),
    SFX_ARGUMENT(SID_SAVEACOPYITEM,"SaveACopy",SfxBoolItem),
};

static sal_uInt16 nMediaArgsCount = sizeof(aFormalArgs) / sizeof (SfxFormalArgument);

static char const sTemplateRegionName[] = "TemplateRegionName";
static char const sTemplateName[] = "TemplateName";
static char const sAsTemplate[] = "AsTemplate";
static char const sOpenNewView[] = "OpenNewView";
static char const sViewId[] = "ViewId";
static char const sPluginMode[] = "PluginMode";
static char const sReadOnly[] = "ReadOnly";
static char const sDdeReconnect[] = "DDEReconnect";
static char const sStartPresentation[] = "StartPresentation";
static char const sFrameName[] = "FrameName";
static char const sMediaType[] = "MediaType";
static char const sPostData[] = "PostData";
static char const sCharacterSet[] = "CharacterSet";
static char const sInputStream[] = "InputStream";
static char const sStream[] = "Stream";
static char const sOutputStream[] = "OutputStream";
static char const sHidden[] = "Hidden";
static char const sPreview[] = "Preview";
static char const sViewOnly[] = "ViewOnly";
static char const sDontEdit[] = "DontEdit";
static char const sSilent[] = "Silent";
static char const sJumpMark[] = "JumpMark";
static char const sFileName[] = "FileName";
static char const sSalvagedFile[] = "SalvagedFile";
static char const sStatusInd[] = "StatusIndicator";
static char const sModel[] = "Model";
static char const sFrame[] = "Frame";
static char const sViewData[] = "ViewData";
static char const sFilterData[] = "FilterData";
static char const sSelectionOnly[] = "SelectionOnly";
static char const sFilterFlags[] = "FilterFlags";
static char const sMacroExecMode[] = "MacroExecutionMode";
static char const sUpdateDocMode[] = "UpdateDocMode";
static char const sMinimized[] = "Minimized";
static char const sInteractionHdl[] = "InteractionHandler";
static char const sUCBContent[] = "UCBContent";
static char const sRepairPackage[] = "RepairPackage";
static char const sDocumentTitle[] = "DocumentTitle";
static char const sComponentData[] = "ComponentData";
static char const sComponentContext[] = "ComponentContext";
static char const sDocumentBaseURL[] = "DocumentBaseURL";
static char const sHierarchicalDocumentName[] = "HierarchicalDocumentName";
static char const sCopyStreamIfPossible[] = "CopyStreamIfPossible";
static char const sNoAutoSave[] = "NoAutoSave";
static char const sFolderName[] = "FolderName";
static char const sUseSystemDialog[] = "UseSystemDialog";
static char const sStandardDir[] = "StandardDir";
static char const sBlackList[] = "BlackList";
static char const sModifyPasswordInfo[] = "ModifyPasswordInfo";
static char const sSuggestedSaveAsDir[] = "SuggestedSaveAsDir";
static char const sSuggestedSaveAsName[] = "SuggestedSaveAsName";
static char const sEncryptionData[] = "EncryptionData";
static char const sFailOnWarning[] = "FailOnWarning";
static char const sDocumentService[] = "DocumentService";
static char const sFilterProvider[] = "FilterProvider";

static bool isMediaDescriptor( sal_uInt16 nSlotId )
{
    return ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC ||
             nSlotId == SID_SAVEASDOC || nSlotId == SID_SAVEDOC ||
             nSlotId == SID_SAVETO || nSlotId == SID_EXPORTDOCASPDF ||
             nSlotId == SID_DIRECTEXPORTDOCASPDF || nSlotId == SID_SAVEACOPY ||
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
    if ( nSlotId == SID_SAVEASURL )
        nSlotId = SID_SAVEASDOC;

    sal_Int32 nCount = rArgs.getLength();
    if ( !nCount )
        return;

    const beans::PropertyValue* pPropsVal = rArgs.getConstArray();
    if ( !pSlot->IsMode(SFX_SLOT_METHOD) )
    {
        // slot is a property
        const SfxType* pType = pSlot->GetType();
        boost::scoped_ptr<SfxPoolItem> pItem(pType->CreateItem());

        if ( !pItem )
        {
#ifdef DBG_UTIL
            OStringBuffer aStr("No creator method for item: ");
            aStr.append(static_cast<sal_Int32>(nSlotId));
            OSL_FAIL(aStr.getStr());
#endif
            return;
        }

        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(nSlotId);
        sal_Bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
        pItem->SetWhich( nWhich );
        sal_uInt16 nSubCount = pType->nAttribs;

        const beans::PropertyValue& rProp = pPropsVal[0];
        OUString aName = rProp.Name;
        if ( nCount == 1 && aName == OUString( pSlot->pUnoName, strlen( pSlot->pUnoName ), RTL_TEXTENCODING_UTF8 ) )
        {
            // there is only one parameter and its name matches the name of the property,
            // so it's either a simple property or a complex property in one single UNO struct
            if( pItem->PutValue( rProp.Value, bConvertTwips ? CONVERT_TWIPS : 0 ) )
                // only use successfully converted items
                rSet.Put( *pItem );
#ifdef DBG_UTIL
            else
            {
                OStringBuffer aStr("Property not convertible: ");
                aStr.append(pSlot->pUnoName);
                OSL_FAIL( aStr.getStr() );
            }
#endif
        }
#ifdef DBG_UTIL
        else if ( nSubCount == 0 )
        {
            // for a simple property there can be only one parameter and its name *must* match
            OUStringBuffer aStr("Property name does not match: ");
            aStr.append(aName);
            OSL_FAIL( aStr.getStr() );
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
                OStringBuffer aStr("MacroPlayer: wrong number of parameters for slot: ");
                aStr.append(static_cast<sal_Int32>(nSlotId));
                DBG_WARNING(aStr.getStr());
            }
#endif
            // complex property; collect sub items from the parameter set and reconstruct complex item
            sal_uInt16 nFound=0;
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                const beans::PropertyValue& rPropValue = pPropsVal[n];
                sal_uInt16 nSub;
                for ( nSub=0; nSub<nSubCount; nSub++ )
                {
                    // search sub item by name
                    OStringBuffer aStr;
                    aStr.append(pSlot->pUnoName).append('.').append(pType->aAttrib[nSub].pName);
                    if ( rPropValue.Name.equalsAsciiL(aStr.getStr(), aStr.getLength()) )
                    {
                        sal_uInt8 nSubId = (sal_uInt8) (sal_Int8) pType->aAttrib[nSub].nAID;
                        if ( bConvertTwips )
                            nSubId |= CONVERT_TWIPS;
                        if ( pItem->PutValue( rPropValue.Value, nSubId ) )
                            nFound++;
#ifdef DBG_UTIL
                        else
                        {
                            OStringBuffer aDbgStr(RTL_CONSTASCII_STRINGPARAM("Property not convertible: "));
                            aDbgStr.append(pSlot->pUnoName);
                            OSL_FAIL( aDbgStr.getStr() );
                        }
#endif
                        break;
                    }
                }

#ifdef DBG_UTIL
                if ( nSub >= nSubCount )
                {
                    // there was a parameter with a name that didn't match to any of the members
                    OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("Property name does not match: "));
                    aStr.append(OUStringToOString(rPropValue.Name, RTL_TEXTENCODING_UTF8));
                    OSL_FAIL( aStr.getStr() );
                }
#endif
            }

            // at least one part of the complex item must be present; other parts can have default values
            if ( nFound > 0 )
                rSet.Put( *pItem );
        }

        return;
    }

    OSL_ASSERT(nCount > 0);

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
        boost::scoped_ptr<SfxPoolItem> pItem(rArg.CreateItem());
        if ( !pItem )
        {
#ifdef DBG_UTIL
            OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("No creator method for argument: "));
            aStr.append(rArg.pName);
            OSL_FAIL( aStr.getStr() );
#endif
            return;
        }

        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(rArg.nSlotId);
        sal_Bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
        pItem->SetWhich( nWhich );
        const SfxType* pType = rArg.pType;
        sal_uInt16 nSubCount = pType->nAttribs;
        if ( nSubCount == 0 )
        {
            // "simple" (base type) argument
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                const beans::PropertyValue& rProp = pPropsVal[n];
                OUString aName = rProp.Name;
                if ( aName == OUString( rArg.pName, strlen(rArg.pName), RTL_TEXTENCODING_UTF8 )  )
                {
#ifdef DBG_UTIL
                    ++nFoundArgs;
#endif
                    if( pItem->PutValue( rProp.Value ) )
                        // only use successfully converted items
                        rSet.Put( *pItem );
#ifdef DBG_UTIL
                    else
                    {
                        OStringBuffer aStr("Property not convertible: ");
                        aStr.append(rArg.pName);
                        OSL_FAIL( aStr.getStr() );
                    }
#endif
                    break;
                }
            }
        }
        else
        {
            // complex argument, could be passed in one struct
            sal_Bool bAsWholeItem = sal_False;
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                const beans::PropertyValue& rProp = pPropsVal[n];
                OUString aName = rProp.Name;
                if ( aName == OUString(rArg.pName, strlen(rArg.pName), RTL_TEXTENCODING_UTF8) )
                {
                    bAsWholeItem = sal_True;
#ifdef DBG_UTIL
                    ++nFoundArgs;
#endif
                    if( pItem->PutValue( rProp.Value ) )
                        // only use successfully converted items
                        rSet.Put( *pItem );
#ifdef DBG_UTIL
                    else
                    {
                        OStringBuffer aStr("Property not convertible: ");
                        aStr.append(rArg.pName);
                        OSL_FAIL( aStr.getStr() );
                    }
#endif
                }
            }

            if ( !bAsWholeItem )
            {
                // complex argument; collect sub items from argument array and reconstruct complex item
                // only put item if at least one member was found and had the correct type
                // (is this a good idea?! Should we ask for *all* members?)
                sal_Bool bRet = sal_False;
                for ( sal_uInt16 n=0; n<nCount; n++ )
                {
                    const beans::PropertyValue& rProp = pPropsVal[n];
                    for ( sal_uInt16 nSub=0; nSub<nSubCount; nSub++ )
                    {
                        // search sub item by name
                        OStringBuffer aStr;
                        aStr.append(rArg.pName).append('.').append(pType->aAttrib[nSub].pName);
                        if ( rProp.Name.equalsAsciiL(aStr.getStr(), aStr.getLength()) )
                        {
                            // at least one member found ...
                            bRet = sal_True;
#ifdef DBG_UTIL
                            ++nFoundArgs;
#endif
                            sal_uInt8 nSubId = (sal_uInt8) (sal_Int8) pType->aAttrib[nSub].nAID;
                            if ( bConvertTwips )
                                nSubId |= CONVERT_TWIPS;
                            if (!pItem->PutValue( rProp.Value, nSubId ) )
                            {
                                // ... but it was not convertible
                                bRet = sal_False;
#ifdef DBG_UTIL
                                OStringBuffer aDbgStr(RTL_CONSTASCII_STRINGPARAM("Property not convertible: "));
                                aDbgStr.append(rArg.pName);
                                OSL_FAIL( aDbgStr.getStr() );
#endif
                            }

                            break;
                        }
                    }
                }

                if ( bRet )
                    // only use successfully converted items
                    rSet.Put( *pItem );

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
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
            const beans::PropertyValue& rProp = pPropsVal[n];
            OUString aName = rProp.Name;
            if ( aName == sFrame )
            {
                Reference< XFrame > xFrame;
                OSL_VERIFY( rProp.Value >>= xFrame );
                rSet.Put( SfxUnoFrameItem( SID_FILLFRAME, xFrame ) );
            }
            else
            if ( aName == sHidden )
            {
                sal_Bool bVal = sal_False;
                if (rProp.Value >>= bVal)
                    rSet.Put( SfxBoolItem( SID_HIDDEN, bVal ) );
            }
        }
    }
    else if ( bIsMediaDescriptor )
    {
        for ( sal_uInt16 n=0; n<nCount; n++ )
        {
#ifdef DBG_UTIL
            ++nFoundArgs;
#endif
            const beans::PropertyValue& rProp = pPropsVal[n];
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
                sal_Bool bOK = (rProp.Value >>= xVal);
                DBG_ASSERT( bOK, "invalid type for StatusIndicator" );
                if (bOK && xVal.is())
                    rSet.Put( SfxUnoAnyItem( SID_PROGRESS_STATUSBAR_CONTROL, rProp.Value ) );
            }
            else if ( aName == sInteractionHdl )
            {
                Reference<task::XInteractionHandler> xVal;
                sal_Bool bOK = (rProp.Value >>= xVal);
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
                sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for InputStream" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_INPUTSTREAM, rProp.Value ) );
            }
            else if ( aName == sStream )
            {
                Reference< XInputStream > xVal;
                sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for Stream" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_STREAM, rProp.Value ) );
            }
            else if ( aName == sUCBContent )
            {
                Reference< XContent > xVal;
                sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for UCBContent" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_CONTENT, rProp.Value ) );
            }
            else if ( aName == sOutputStream )
            {
                Reference< XOutputStream > xVal;
                sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
                DBG_ASSERT( bOK, "invalid type for OutputStream" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_OUTPUTSTREAM, rProp.Value ) );
            }
            else if ( aName == sPostData )
            {
                Reference< XInputStream > xVal;
                sal_Bool bOK = (rProp.Value >>= xVal);
                DBG_ASSERT( bOK, "invalid type for PostData" );
                if (bOK)
                    rSet.Put( SfxUnoAnyItem( SID_POSTDATA, rProp.Value ) );
            }
            else if ( aName == sFrame )
            {
                Reference< XFrame > xFrame;
                sal_Bool bOK = (rProp.Value >>= xFrame);
                DBG_ASSERT( bOK, "invalid type for Frame" );
                if (bOK)
                    rSet.Put( SfxUnoFrameItem( SID_FILLFRAME, xFrame ) );
            }
            else if ( aName == sAsTemplate )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for AsTemplate" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_TEMPLATE, bVal ) );
            }
            else if ( aName == sOpenNewView )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for OpenNewView" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_OPEN_NEW_VIEW, bVal ) );
            }
            else if ( aName == sFailOnWarning )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for FailOnWarning" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_FAIL_ON_WARNING, bVal ) );
            }
            else if ( aName == sViewId )
            {
                sal_Int16 nVal = -1;
                sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for ViewId" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_VIEW_ID, nVal ) );
            }
            else if ( aName == sPluginMode )
            {
                sal_Int16 nVal = -1;
                sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for PluginMode" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_PLUGIN_MODE, nVal ) );
            }
            else if ( aName == sReadOnly )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ReadOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_DOC_READONLY, bVal ) );
            }
            else if ( aName == sDdeReconnect )
            {
                sal_Bool bVal = sal_True;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for DDEReconnect" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_DDE_RECONNECT_ONLOAD, bVal ) );
            }
            else if ( aName == sStartPresentation )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for StartPresentation" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_DOC_STARTPRESENTATION, bVal ) );
            }
            else if ( aName == sSelectionOnly )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for SelectionOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_SELECTION, bVal ) );
            }
            else if ( aName == sHidden )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Hidden" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_HIDDEN, bVal ) );
            }
            else if ( aName == sMinimized )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Minimized" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_MINIMIZED, bVal ) );
            }
            else if ( aName == sSilent )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Silent" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_SILENT, bVal ) );
            }
            else if ( aName == sPreview )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for Preview" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_PREVIEW, bVal ) );
            }
            else if ( aName == sViewOnly )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ViewOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_VIEWONLY, bVal ) );
            }
            else if ( aName == sDontEdit )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ViewOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_EDITDOC, !bVal ) );
            }
            else if ( aName == sUseSystemDialog )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for ViewOnly" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_FILE_DIALOG, bVal ) );
            }
            else if ( aName == sStandardDir )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for StandardDir" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_STANDARD_DIR, sVal ) );
            }
            else if ( aName == sBlackList )
            {
                uno::Sequence<OUString> xVal;
                sal_Bool bOK = (rProp.Value >>= xVal);
                DBG_ASSERT( bOK, "invalid type or value for BlackList" );
                if (bOK)
                {
                    SfxStringListItem stringList(SID_BLACK_LIST);
                    stringList.SetStringList( xVal );
                    rSet.Put( stringList );
                }
            }
            else if ( aName == sFileName )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for FileName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_FILE_NAME, sVal ) );
            }
            else if ( aName == sSalvagedFile )
            {
                OUString sVal;
                sal_Bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type or value for SalvagedFile" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOC_SALVAGE, sVal ) );
            }
            else if ( aName == sFolderName )
            {
                OUString sVal;
                sal_Bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type or value for FolderName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_PATH, sVal ) );
            }
            else if ( aName == sFrameName )
            {
                OUString sVal;
                sal_Bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type for FrameName" );
                if (bOK && !sVal.isEmpty())
                    rSet.Put( SfxStringItem( SID_TARGETNAME, sVal ) );
            }
            else if ( aName == sMediaType )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for MediaType" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_CONTENTTYPE, sVal ) );
            }
            else if ( aName == sTemplateName )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for TemplateName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_TEMPLATE_NAME, sVal ) );
            }
            else if ( aName == sTemplateRegionName )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for TemplateRegionName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_TEMPLATE_REGIONNAME, sVal ) );
            }
            else if ( aName == sJumpMark )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for JumpMark" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_JUMPMARK, sVal ) );
            }
            else if ( aName == sCharacterSet )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for CharacterSet" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_CHARSET, sVal ) );
            }
            else if ( aName == sFilterFlags )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for FilterFlags" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sVal ) );
            }
            else if ( aName == sMacroExecMode )
            {
                sal_Int16 nVal =-1;
                sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for MacroExecMode" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_MACROEXECMODE, nVal ) );
            }
            else if ( aName == sUpdateDocMode )
            {
                sal_Int16 nVal =-1;
                sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
                DBG_ASSERT( bOK, "invalid type for UpdateDocMode" );
                if (bOK)
                    rSet.Put( SfxUInt16Item( SID_UPDATEDOCMODE, nVal ) );
            }
            else if ( aName == sRepairPackage )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for RepairPackage" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_REPAIRPACKAGE, bVal ) );
            }
            else if ( aName == sDocumentTitle )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for DocumentTitle" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOCINFO_TITLE, sVal ) );
            }
            else if ( aName == sDocumentBaseURL )
            {
                OUString sVal;
                // the base url can be set to empty ( for embedded objects for example )
                sal_Bool bOK = (rProp.Value >>= sVal);
                DBG_ASSERT( bOK, "invalid type or value for DocumentBaseURL" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOC_BASEURL, sVal ) );
            }
            else if ( aName == sHierarchicalDocumentName )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for HierarchicalDocumentName" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_DOC_HIERARCHICALNAME, sVal ) );
            }
            else if ( aName == sCopyStreamIfPossible )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
                DBG_ASSERT( bOK, "invalid type for CopyStreamIfPossible" );
                if (bOK)
                    rSet.Put( SfxBoolItem( SID_COPY_STREAM_IF_POSSIBLE, bVal ) );
            }
            else if ( aName == sNoAutoSave )
            {
                sal_Bool bVal = sal_False;
                sal_Bool bOK = (rProp.Value >>= bVal);
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
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
                DBG_ASSERT( bOK, "invalid type or value for SuggestedSaveAsDir" );
                if (bOK)
                    rSet.Put( SfxStringItem( SID_SUGGESTEDSAVEASDIR, sVal ) );
            }
            else if ( aName == sSuggestedSaveAsName )
            {
                OUString sVal;
                sal_Bool bOK = ((rProp.Value >>= sVal) && !sVal.isEmpty());
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
        OUString sSlotName( "OptionsTreeDialog" );
        OUString sPropName( "OptionsPageURL" );
        if ( sSlotName == OUString( pSlot->pUnoName, strlen(pSlot->pUnoName), RTL_TEXTENCODING_UTF8 ) )
        {
            for ( sal_uInt16 n = 0; n < nCount; ++n )
            {
                const PropertyValue& rProp = pPropsVal[n];
                OUString sName( rProp.Name );
                if ( sName == sPropName )
                {
                    OUString sURL;
                    if ( rProp.Value >>= sURL )
                        rSet.Put( SfxStringItem( SID_OPTIONS_PAGEURL, sURL ) );
                    break;
                }
            }
        }
    }
#ifdef DB_UTIL
    if ( nFoundArgs == nCount )
    {
        // except for the "special" slots: assure that every argument was convertible
        OStringBuffer aStr("MacroPlayer: Some properties didn't match to any formal argument for slot: ");
        aStr.append(pSlot->pUnoName);
        DBG_WARNING( aStr.getStr() );
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
    if ( nSlotId == SID_SAVEASURL )
        nSlotId = SID_SAVEASDOC;

    // find number of properties to avoid permanent reallocations in the sequence
    sal_Int32 nProps=0;

#ifdef DBG_UTIL
    // trace number of items and compare with number of properties for debugging purposes
    sal_Int32 nItems=0;
#endif

    const SfxType *pType = pSlot->GetType();
    if ( !pSlot->IsMode(SFX_SLOT_METHOD) )
    {
        // slot is a property
        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(nSlotId);
        if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET ) //???
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
#ifdef DBG_UTIL
        else
        {
            // we will not rely on the "toggle" ability of some property slots
            OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                "Processing property slot without argument: "));
            aStr.append(static_cast<sal_Int32>(nSlotId));
            OSL_FAIL(aStr.getStr());
        }
#endif

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
            const SfxFormalArgument &rArg = pSlot->GetFormalArgument( nArg );

            sal_uInt16 nWhich = rSet.GetPool()->GetWhich( rArg.nSlotId );
            if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET ) //???
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
            if ( rSet.GetItemState( SID_PROGRESS_STATUSBAR_CONTROL ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_INTERACTIONHANDLER ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_SALVAGE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_PATH ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FILE_DIALOG ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_STANDARD_DIR ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_BLACK_LIST ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_CONTENT ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_INPUTSTREAM ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_STREAM ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_OUTPUTSTREAM ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TEMPLATE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_OPEN_NEW_VIEW ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FAIL_ON_WARNING ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_VIEW_ID ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_VIEW_DATA ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FILTER_DATA ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_PLUGIN_MODE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_READONLY ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DDE_RECONNECT_ONLOAD ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_STARTPRESENTATION ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SELECTION ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_CONTENTTYPE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_POSTDATA ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_FILLFRAME ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_CHARSET ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TARGETNAME ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TEMPLATE_NAME ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TEMPLATE_REGIONNAME ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_HIDDEN ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_MINIMIZED ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_PREVIEW ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_VIEWONLY ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_EDITDOC ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SILENT ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_JUMPMARK ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOCUMENT ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_MACROEXECMODE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_UPDATEDOCMODE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_REPAIRPACKAGE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOCINFO_TITLE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_COMPONENTDATA ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_COMPONENTCONTEXT ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_BASEURL ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_HIERARCHICALNAME ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_COPY_STREAM_IF_POSSIBLE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_NOAUTOSAVE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_MODIFYPASSWORDINFO ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SUGGESTEDSAVEASDIR ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_ENCRYPTIONDATA ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_SUGGESTEDSAVEASNAME ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_SERVICE ) == SFX_ITEM_SET )
                nAdditional++;
            if (rSet.HasItem(SID_FILTER_PROVIDER))
                ++nAdditional;

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
        const sal_uInt16 *pRanges = rSet.GetRanges();
        while ( *pRanges )
        {
            for(sal_uInt16 nId = *pRanges++; nId <= *pRanges; ++nId)
            {
                if ( rSet.GetItemState(nId) < SFX_ITEM_SET ) //???
                    // not really set
                    continue;

                if ( !pSlot->IsMode(SFX_SLOT_METHOD) && nId == rSet.GetPool()->GetWhich( pSlot->GetSlotId() ) )
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
                    if ( nId == SID_BLACK_LIST )
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
               }

                OStringBuffer aDbg(RTL_CONSTASCII_STRINGPARAM(
                    "Unknown item detected: "));
                aDbg.append(static_cast<sal_Int32>(nId));
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
    if ( !pSlot->IsMode(SFX_SLOT_METHOD) )
    {
        // slot is a property
        sal_uInt16 nWhich = rSet.GetPool()->GetWhich(nSlotId);
        sal_Bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
        SFX_ITEMSET_ARG( &rSet, pItem, SfxPoolItem, nWhich, sal_False );
        if ( pItem ) //???
        {
            sal_uInt16 nSubCount = pType->nAttribs;
            if ( !nSubCount )
            {
                pValue[nActProp].Name = OUString::createFromAscii(pSlot->pUnoName) ;
                if ( !pItem->QueryValue( pValue[nActProp].Value ) )
                {
                    OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                        "Item not convertible: "));
                    aStr.append(static_cast<sal_Int32>(nSlotId));
                    OSL_FAIL(aStr.getStr());
                }
            }
            else
            {
                // complex type, add a property value for every member of the struct
                for ( sal_uInt16 n=1; n<=nSubCount; ++n )
                {
                    sal_uInt8 nSubId = (sal_uInt8) (sal_Int8) pType->aAttrib[n-1].nAID;
                    if ( bConvertTwips )
                        nSubId |= CONVERT_TWIPS;

                    DBG_ASSERT(( pType->aAttrib[n-1].nAID ) <= 127, "Member ID out of range" );
                    OUString aName( OUString::createFromAscii( pSlot->pUnoName ) ) ;
                    aName += ".";
                    aName += OUString::createFromAscii( pType->aAttrib[n-1].pName ) ;
                    pValue[nActProp].Name = aName;
                    if ( !pItem->QueryValue( pValue[nActProp++].Value, nSubId ) )
                    {
                        OStringBuffer aStr("Sub item ");
                        aStr.append(static_cast<sal_Int32>(
                            pType->aAttrib[n-1].nAID));
                        aStr.append(" not convertible in slot: ");
                        aStr.append(static_cast<sal_Int32>(nSlotId));
                        OSL_FAIL( aStr.getStr() );
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
        sal_Bool bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
        SFX_ITEMSET_ARG( &rSet, pItem, SfxPoolItem, nWhich, sal_False );
        if ( pItem ) //???
        {
            sal_uInt16 nSubCount = rArg.pType->nAttribs;
            if ( !nSubCount )
            {
                pValue[nActProp].Name = OUString::createFromAscii( rArg.pName ) ;
                if ( !pItem->QueryValue( pValue[nActProp++].Value ) )
                {
                    OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM(
                        "Item not convertible: "));
                    aStr.append(static_cast<sal_Int32>(rArg.nSlotId));
                    OSL_FAIL(aStr.getStr());
                }
            }
            else
            {
                // complex type, add a property value for every member of the struct
                for ( sal_uInt16 n = 1; n <= nSubCount; ++n )
                {
                    sal_uInt8 nSubId = (sal_uInt8) (sal_Int8) rArg.pType->aAttrib[n-1].nAID;
                    if ( bConvertTwips )
                        nSubId |= CONVERT_TWIPS;

                    DBG_ASSERT((rArg.pType->aAttrib[n-1].nAID) <= 127, "Member ID out of range" );
                    OUString aName( OUString::createFromAscii( rArg.pName ) ) ;
                    aName += ".";
                    aName += OUString::createFromAscii( rArg.pType->aAttrib[n-1].pName ) ;
                    pValue[nActProp].Name = aName;
                    if ( !pItem->QueryValue( pValue[nActProp++].Value, nSubId ) )
                    {
                        OStringBuffer aStr("Sub item ");
                        aStr.append(static_cast<sal_Int32>(
                            rArg.pType->aAttrib[n-1].nAID));
                        aStr.append(" not convertible in slot: ");
                        aStr.append(static_cast<sal_Int32>(rArg.nSlotId));
                        OSL_FAIL(aStr.getStr());
                    }
                }
            }
        }
    }

    if ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC || nSlotId == SID_SAVEASDOC ||  nSlotId == SID_SAVEDOC ||
         nSlotId == SID_SAVETO || nSlotId == SID_EXPORTDOCASPDF || nSlotId == SID_DIRECTEXPORTDOCASPDF ||
         nSlotId == SID_SAVEACOPY )
    {
        const SfxPoolItem *pItem=0;
        if ( rSet.GetItemState( SID_COMPONENTDATA, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sComponentData);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_COMPONENTCONTEXT, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sComponentContext);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_PROGRESS_STATUSBAR_CONTROL, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sStatusInd);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_INTERACTIONHANDLER, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sInteractionHdl);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_VIEW_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sViewData);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_FILTER_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sFilterData);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_DOCUMENT, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sModel);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_CONTENT, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sUCBContent);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_INPUTSTREAM, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sInputStream);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_STREAM, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sStream);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_OUTPUTSTREAM, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sOutputStream);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_POSTDATA, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sPostData);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_FILLFRAME, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sFrame);
            if ( pItem->ISA( SfxUsrAnyItem ) )
            {
                OSL_FAIL( "TransformItems: transporting an XFrame via an SfxUsrAnyItem is not deprecated!" );
                pValue[nActProp++].Value = static_cast< const SfxUsrAnyItem* >( pItem )->GetValue();
            }
            else if ( pItem->ISA( SfxUnoFrameItem ) )
                pValue[nActProp++].Value <<= static_cast< const SfxUnoFrameItem* >( pItem )->GetFrame();
            else
                OSL_FAIL( "TransformItems: invalid item type for SID_FILLFRAME!" );
        }
        if ( rSet.GetItemState( SID_TEMPLATE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sAsTemplate);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_OPEN_NEW_VIEW, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sOpenNewView);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_FAIL_ON_WARNING, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(RTL_CONSTASCII_USTRINGPARAM(sFailOnWarning));
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_VIEW_ID, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sViewId);
            pValue[nActProp++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_PLUGIN_MODE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sPluginMode);
            pValue[nActProp++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_DOC_READONLY, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sReadOnly);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_DDE_RECONNECT_ONLOAD, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sDdeReconnect);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_DOC_STARTPRESENTATION, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sStartPresentation);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_SELECTION, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sSelectionOnly);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_HIDDEN, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sHidden);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_MINIMIZED, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sMinimized);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_SILENT, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sSilent);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_PREVIEW, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sPreview);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_VIEWONLY, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sViewOnly);
            pValue[nActProp++].Value <<= (sal_Bool) (( ((SfxBoolItem*)pItem)->GetValue() ));
        }
        if ( rSet.GetItemState( SID_EDITDOC, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sDontEdit);
            pValue[nActProp++].Value <<= (sal_Bool) (!( ((SfxBoolItem*)pItem)->GetValue() ));
        }
        if ( rSet.GetItemState( SID_FILE_DIALOG, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sUseSystemDialog);
            pValue[nActProp++].Value <<= (sal_Bool) ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_STANDARD_DIR, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sStandardDir);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_BLACK_LIST, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sBlackList);

            com::sun::star::uno::Sequence< OUString > aList;
            ((SfxStringListItem*)pItem)->GetStringList( aList );
            pValue[nActProp++].Value <<= aList ;
        }
        if ( rSet.GetItemState( SID_TARGETNAME, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sFrameName);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_DOC_SALVAGE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sSalvagedFile);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_PATH, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sFolderName);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_CONTENTTYPE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sMediaType);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue())  );
        }
        if ( rSet.GetItemState( SID_TEMPLATE_NAME, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sTemplateName);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue())  );
        }
        if ( rSet.GetItemState( SID_TEMPLATE_REGIONNAME, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sTemplateRegionName);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue())  );
        }
        if ( rSet.GetItemState( SID_JUMPMARK, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sJumpMark);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue())  );
        }

        if ( rSet.GetItemState( SID_CHARSET, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sCharacterSet);
            pValue[nActProp++].Value <<= (  OUString(((SfxStringItem*)pItem)->GetValue())  );
        }
        if ( rSet.GetItemState( SID_MACROEXECMODE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sMacroExecMode);
            pValue[nActProp++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_UPDATEDOCMODE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sUpdateDocMode);
            pValue[nActProp++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_REPAIRPACKAGE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sRepairPackage);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_DOCINFO_TITLE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sDocumentTitle);
            pValue[nActProp++].Value <<= ( OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_DOC_BASEURL, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sDocumentBaseURL);
            pValue[nActProp++].Value <<= ( OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_DOC_HIERARCHICALNAME, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sHierarchicalDocumentName);
            pValue[nActProp++].Value <<= ( OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_COPY_STREAM_IF_POSSIBLE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sCopyStreamIfPossible);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_NOAUTOSAVE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sNoAutoSave);
            pValue[nActProp++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_MODIFYPASSWORDINFO, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sModifyPasswordInfo);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_ENCRYPTIONDATA, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sEncryptionData);
            pValue[nActProp++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
        }
        if ( rSet.GetItemState( SID_SUGGESTEDSAVEASDIR, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sSuggestedSaveAsDir);
            pValue[nActProp++].Value <<= ( OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_SUGGESTEDSAVEASNAME, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sSuggestedSaveAsName);
            pValue[nActProp++].Value <<= ( OUString(((SfxStringItem*)pItem)->GetValue()) );
        }
        if ( rSet.GetItemState( SID_DOC_SERVICE, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            pValue[nActProp].Name = OUString(sDocumentService);
            pValue[nActProp++].Value <<= OUString(static_cast<const SfxStringItem*>(pItem)->GetValue());
        }
        if (rSet.HasItem(SID_FILTER_PROVIDER, &pItem))
        {
            pValue[nActProp].Name = sFilterProvider;
            pValue[nActProp++].Value <<= OUString(static_cast<const SfxStringItem*>(pItem)->GetValue());
        }
    }

    rArgs = aSequ;
}

SFX_IMPL_XSERVICEINFO( SfxMacroLoader, PROTOCOLHANDLER_SERVICENAME, "com.sun.star.comp.sfx2.SfxMacroLoader" )
SFX_IMPL_SINGLEFACTORY( SfxMacroLoader )

void SAL_CALL SfxMacroLoader::initialize( const uno::Sequence<uno::Any>& aArguments ) throw (uno::Exception, uno::RuntimeException)
{
    Reference < XFrame > xFrame;
    if ( aArguments.getLength() )
    {
        aArguments[0] >>= xFrame;
        m_xFrame = xFrame;
    }
}

SfxObjectShell* SfxMacroLoader::GetObjectShell_Impl()
{
    SfxObjectShell* pDocShell = NULL;
    Reference < XFrame > xFrame( m_xFrame.get(), UNO_QUERY );
    if ( xFrame.is() )
    {
        SfxFrame* pFrame=0;
        for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
        {
            if ( pFrame->GetFrameInterface() == xFrame )
                break;
        }

        if ( pFrame )
            pDocShell = pFrame->GetCurrentDocument();
    }

    return pDocShell;
}

// -----------------------------------------------------------------------
uno::Reference<frame::XDispatch> SAL_CALL SfxMacroLoader::queryDispatch(
    const util::URL&   aURL            ,
    const OUString&               /*sTargetFrameName*/,
    sal_Int32                            /*nSearchFlags*/    ) throw( uno::RuntimeException )
{
    uno::Reference<frame::XDispatch> xDispatcher;
    if(aURL.Complete.startsWith("macro:"))
        xDispatcher = this;
    return xDispatcher;
}

// -----------------------------------------------------------------------
uno::Sequence< uno::Reference<frame::XDispatch> > SAL_CALL
                SfxMacroLoader::queryDispatches( const uno::Sequence < frame::DispatchDescriptor >& seqDescriptor )
                    throw( uno::RuntimeException )
{
    sal_Int32 nCount = seqDescriptor.getLength();
    uno::Sequence< uno::Reference<frame::XDispatch> > lDispatcher(nCount);
    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = this->queryDispatch( seqDescriptor[i].FeatureURL,
                                              seqDescriptor[i].FrameName,
                                              seqDescriptor[i].SearchFlags );
    return lDispatcher;
}

// -----------------------------------------------------------------------
void SAL_CALL SfxMacroLoader::dispatchWithNotification(
    const util::URL& aURL, const uno::Sequence<beans::PropertyValue>& /*lArgs*/,
    const uno::Reference<frame::XDispatchResultListener>& xListener )
              throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Any aAny;
    ErrCode nErr = loadMacro( aURL.Complete, aAny, GetObjectShell_Impl() );
    if( xListener.is() )
    {
        // always call dispatchFinished(), because we didn't load a document but
        // executed a macro instead!
        frame::DispatchResultEvent aEvent;

        aEvent.Source = static_cast< ::cppu::OWeakObject* >(this);
        if( nErr == ERRCODE_NONE )
            aEvent.State = frame::DispatchResultState::SUCCESS;
        else
            aEvent.State = frame::DispatchResultState::FAILURE;

        xListener->dispatchFinished( aEvent ) ;
    }
}

uno::Any SAL_CALL SfxMacroLoader::dispatchWithReturnValue(
    const util::URL& aURL, const uno::Sequence<beans::PropertyValue>& )
        throw (uno::RuntimeException)
{
    uno::Any aRet;
    loadMacro( aURL.Complete, aRet, GetObjectShell_Impl() );
    return aRet;
}

void SAL_CALL SfxMacroLoader::dispatch(
    const util::URL& aURL, const uno::Sequence<beans::PropertyValue>& /*lArgs*/ )
        throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Any aAny;
    loadMacro( aURL.Complete, aAny, GetObjectShell_Impl() );
}

void SAL_CALL SfxMacroLoader::addStatusListener(
    const uno::Reference< frame::XStatusListener >& ,
    const util::URL&                                                    )
              throw (uno::RuntimeException)
{
    /* TODO
            How we can handle different listener for further coming or currently running dispatch() jobs
            without any inconsistency!
     */
}

// -----------------------------------------------------------------------
void SAL_CALL SfxMacroLoader::removeStatusListener(
    const uno::Reference< frame::XStatusListener >&,
    const util::URL&                                                  )
        throw (uno::RuntimeException)
{
}

ErrCode SfxMacroLoader::loadMacro( const OUString& rURL, com::sun::star::uno::Any& rRetval, SfxObjectShell* pSh )
    throw ( uno::RuntimeException )
{
#ifdef DISABLE_SCRIPTING
    (void) rURL;
    (void) rRetval;
    (void) pSh;
    return ERRCODE_BASIC_PROC_UNDEFINED;
#else
    SfxObjectShell* pCurrent = pSh;
    if ( !pCurrent )
        // all not full qualified names use the BASIC of the given or current document
        pCurrent = SfxObjectShell::Current();

    // 'macro:///lib.mod.proc(args)' => macro of App-BASIC
    // 'macro://[docname|.]/lib.mod.proc(args)' => macro of current or qualified document
    // 'macro://obj.method(args)' => direct API call, execute it via App-BASIC
    OUString aMacro( rURL );
    sal_Int32 nHashPos = aMacro.indexOf( '/', 8 );
    sal_Int32 nArgsPos = aMacro.indexOf( '(' );
    BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
    BasicManager *pBasMgr = 0;
    ErrCode nErr = ERRCODE_NONE;

    // should a macro function be executed ( no direct API call)?
    if ( -1 != nHashPos && nHashPos < nArgsPos )
    {
        // find BasicManager
        SfxObjectShell* pDoc = NULL;
        OUString aBasMgrName( INetURLObject::decode(aMacro.copy( 8, nHashPos-8 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET) );
        if ( aBasMgrName.isEmpty() )
            pBasMgr = pAppMgr;
        else if ( aBasMgrName == "." )
        {
            // current/actual document
            pDoc = pCurrent;
            if (pDoc)
                pBasMgr = pDoc->GetBasicManager();
        }
        else
        {
            // full qualified name, find document by name
            for ( SfxObjectShell *pObjSh = SfxObjectShell::GetFirst();
                    pObjSh && !pBasMgr;
                    pObjSh = SfxObjectShell::GetNext(*pObjSh) )
                if ( aBasMgrName == pObjSh->GetTitle(SFX_TITLE_APINAME) )
                {
                    pDoc = pObjSh;
                    pBasMgr = pDoc->GetBasicManager();
                }
        }

        if ( pBasMgr )
        {
            const bool bIsAppBasic = ( pBasMgr == pAppMgr );
            const bool bIsDocBasic = ( pBasMgr != pAppMgr );

            if ( pDoc )
            {
                // security check for macros from document basic if an SFX doc is given
                if ( !pDoc->AdjustMacroMode( OUString() ) )
                    // check forbids execution
                    return ERRCODE_IO_ACCESSDENIED;
            }
            else if ( pDoc && pDoc->GetMedium() )
            {
                pDoc->AdjustMacroMode( OUString() );
                SFX_ITEMSET_ARG( pDoc->GetMedium()->GetItemSet(), pUpdateDocItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False);
                SFX_ITEMSET_ARG( pDoc->GetMedium()->GetItemSet(), pMacroExecModeItem, SfxUInt16Item, SID_MACROEXECMODE, sal_False);
                if ( pUpdateDocItem && pMacroExecModeItem
                  && pUpdateDocItem->GetValue() == document::UpdateDocMode::NO_UPDATE
                  && pMacroExecModeItem->GetValue() == document::MacroExecMode::NEVER_EXECUTE )
                    return ERRCODE_IO_ACCESSDENIED;
            }

            // find BASIC method
            OUString aQualifiedMethod( INetURLObject::decode(aMacro.copy( nHashPos+1 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET) );
            OUString aArgs;
            if ( -1 != nArgsPos )
            {
                // remove arguments from macro name
                aArgs = aQualifiedMethod.copy( nArgsPos - nHashPos - 1 );
                aQualifiedMethod = aQualifiedMethod.copy( 0, nArgsPos - nHashPos - 1 );
            }

            if ( pBasMgr->HasMacro( aQualifiedMethod ) )
            {
                Any aOldThisComponent;
                const bool bSetDocMacroMode = ( pDoc != NULL ) && bIsDocBasic;
                const bool bSetGlobalThisComponent = ( pDoc != NULL ) && bIsAppBasic;
                if ( bSetDocMacroMode )
                {
                    // mark document: it executes an own macro, so it's in a modal mode
                    pDoc->SetMacroMode_Impl( sal_True );
                }

                if ( bSetGlobalThisComponent )
                {
                    // document is executed via AppBASIC, adjust ThisComponent variable
                    aOldThisComponent = pAppMgr->SetGlobalUNOConstant( "ThisComponent", makeAny( pDoc->GetModel() ) );
                }

                // just to let the shell be alive
                SfxObjectShellRef xKeepDocAlive = pDoc;

                {
                    // attempt to protect the document against the script tampering with its Undo Context
                    ::std::auto_ptr< ::framework::DocumentUndoGuard > pUndoGuard;
                    if ( bIsDocBasic )
                        pUndoGuard.reset( new ::framework::DocumentUndoGuard( pDoc->GetModel() ) );

                    // execute the method
                    SbxVariableRef retValRef = new SbxVariable;
                    nErr = pBasMgr->ExecuteMacro( aQualifiedMethod, aArgs, retValRef );
                    if ( nErr == ERRCODE_NONE )
                        rRetval = sbxToUnoValue( retValRef );
                }

                if ( bSetGlobalThisComponent )
                {
                    pAppMgr->SetGlobalUNOConstant( "ThisComponent", aOldThisComponent );
                }

                if ( bSetDocMacroMode )
                {
                    // remove flag for modal mode
                    pDoc->SetMacroMode_Impl( sal_False );
                }
            }
            else
                nErr = ERRCODE_BASIC_PROC_UNDEFINED;
        }
        else
            nErr = ERRCODE_IO_NOTEXISTS;
    }
    else
    {
        // direct API call on a specified object
        OUStringBuffer aCall;
        aCall.append('[').append(INetURLObject::decode(aMacro.copy(6), INET_HEX_ESCAPE,
            INetURLObject::DECODE_WITH_CHARSET));
        aCall.append(']');
        pAppMgr->GetLib(0)->Execute(aCall.makeStringAndClear());
        nErr = SbxBase::GetError();
    }

    SbxBase::ResetError();
    return nErr;
#endif
}

 /* XServiceInfo */
OUString SAL_CALL SfxAppDispatchProvider::getImplementationName() throw( css::uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SfxAppDispatchProvider::supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException )
{
    css::uno::Sequence< OUString > seqServiceNames = getSupportedServiceNames();
    const OUString*         pArray          = seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True;
        }
    }
    return sal_False;
}

/* XServiceInfo */
css::uno::Sequence< OUString > SAL_CALL SfxAppDispatchProvider::getSupportedServiceNames() throw( css::uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
css::uno::Sequence< OUString > SfxAppDispatchProvider::impl_getStaticSupportedServiceNames()
{
    css::uno::Sequence< OUString > seqServiceNames( 2 );
    seqServiceNames.getArray()[0] = "com.sun.star.frame.DispatchProvider";
    seqServiceNames.getArray()[1] = "com.sun.star.frame.AppDispatchProvider";
    return seqServiceNames;
}

/* Helper for XServiceInfo */
OUString SfxAppDispatchProvider::impl_getStaticImplementationName()
{
    return OUString( "com.sun.star.comp.sfx2.AppDispatchProvider" );
}

/* Helper for registry */
css::uno::Reference< css::uno::XInterface > SAL_CALL SfxAppDispatchProvider::impl_createInstance( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceManager ) throw( css::uno::Exception )
{
    return css::uno::Reference< css::uno::XInterface >( *new SfxAppDispatchProvider( xServiceManager ) );
}

SFX_IMPL_SINGLEFACTORY( SfxAppDispatchProvider );

void SAL_CALL SfxAppDispatchProvider::initialize( const uno::Sequence<uno::Any>& aArguments ) throw (uno::Exception, uno::RuntimeException)
{
    Reference < XFrame > xFrame;
    if ( aArguments.getLength() )
    {
        aArguments[0] >>= xFrame;
        m_xFrame = xFrame;
    }
}

Reference < XDispatch > SAL_CALL SfxAppDispatchProvider::queryDispatch(
    const util::URL& aURL,
    const OUString& /*sTargetFrameName*/,
    FrameSearchFlags /*eSearchFlags*/ ) throw( RuntimeException )
{
    sal_uInt16                  nId( 0 );
    sal_Bool                bMasterCommand( sal_False );
    Reference < XDispatch > xDisp;
    const SfxSlot* pSlot = 0;
    SfxDispatcher* pAppDisp = SFX_APP()->GetAppDispatcher_Impl();
    if ( aURL.Protocol.compareToAscii( "slot:" ) == COMPARE_EQUAL ||
         aURL.Protocol.compareToAscii( "commandId:" ) == COMPARE_EQUAL )
    {
        nId = (sal_uInt16) aURL.Path.toInt32();
        SfxShell* pShell;
        pAppDisp->GetShellAndSlot_Impl( nId, &pShell, &pSlot, sal_True, sal_True );
    }
    else if ( aURL.Protocol.compareToAscii( ".uno:" ) == COMPARE_EQUAL )
    {
        // Support ".uno" commands. Map commands to slotid
        bMasterCommand = SfxOfficeDispatch::IsMasterUnoCommand( aURL );
        if ( bMasterCommand )
            pSlot = pAppDisp->GetSlot( SfxOfficeDispatch::GetMasterUnoCommand( aURL ) );
        else
            pSlot = pAppDisp->GetSlot( aURL.Main );
    }

    if ( pSlot )
    {
        SfxOfficeDispatch* pDispatch = new SfxOfficeDispatch( pAppDisp, pSlot, aURL ) ;
        pDispatch->SetFrame(m_xFrame);
        pDispatch->SetMasterUnoCommand( bMasterCommand );
        xDisp = pDispatch;
    }

    return xDisp;
}

Sequence< Reference < XDispatch > > SAL_CALL SfxAppDispatchProvider::queryDispatches( const Sequence < DispatchDescriptor >& seqDescriptor )
throw( RuntimeException )
{
    sal_Int32 nCount = seqDescriptor.getLength();
    uno::Sequence< uno::Reference < frame::XDispatch > > lDispatcher(nCount);
    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = this->queryDispatch( seqDescriptor[i].FeatureURL,
                                              seqDescriptor[i].FrameName,
                                              seqDescriptor[i].SearchFlags );
    return lDispatcher;
}

Sequence< sal_Int16 > SAL_CALL SfxAppDispatchProvider::getSupportedCommandGroups()
throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    std::list< sal_Int16 > aGroupList;
    SfxSlotPool* pAppSlotPool = &SFX_APP()->GetAppSlotPool_Impl();

    const sal_uIntPtr nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );

    // Gruppe anw"ahlen ( Gruppe 0 ist intern )
    for ( sal_uInt16 i=0; i<pAppSlotPool->GetGroupCount(); i++ )
    {
        pAppSlotPool->SeekGroup( i );
        const SfxSlot* pSfxSlot = pAppSlotPool->FirstSlot();
        while ( pSfxSlot )
        {
            if ( pSfxSlot->GetMode() & nMode )
            {
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                aGroupList.push_back( nCommandGroup );
                break;
            }
            pSfxSlot = pAppSlotPool->NextSlot();
        }
    }

    uno::Sequence< sal_Int16 > aSeq =
        comphelper::containerToSequence< sal_Int16, std::list< sal_Int16 > >( aGroupList );

    return aSeq;
}

Sequence< frame::DispatchInformation > SAL_CALL SfxAppDispatchProvider::getConfigurableDispatchInformation( sal_Int16 nCmdGroup )
throw (uno::RuntimeException)
{
    std::list< frame::DispatchInformation > aCmdList;

    SolarMutexGuard aGuard;
    SfxSlotPool* pAppSlotPool = &SFX_APP()->GetAppSlotPool_Impl();

    if ( pAppSlotPool )
    {
        const sal_uIntPtr   nMode( SFX_SLOT_TOOLBOXCONFIG|SFX_SLOT_ACCELCONFIG|SFX_SLOT_MENUCONFIG );
        OUString aCmdPrefix( ".uno:" );

        // Gruppe anw"ahlen ( Gruppe 0 ist intern )
        for ( sal_uInt16 i=0; i<pAppSlotPool->GetGroupCount(); i++ )
        {
            pAppSlotPool->SeekGroup( i );
            const SfxSlot* pSfxSlot = pAppSlotPool->FirstSlot();
            if ( pSfxSlot )
            {
                sal_Int16 nCommandGroup = MapGroupIDToCommandGroup( pSfxSlot->GetGroupId() );
                if ( nCommandGroup == nCmdGroup )
                {
                    while ( pSfxSlot )
                    {
                        if ( pSfxSlot->GetMode() & nMode )
                        {
                            frame::DispatchInformation aCmdInfo;
                            OUStringBuffer aBuf( aCmdPrefix );
                            aBuf.appendAscii( pSfxSlot->GetUnoName() );
                            aCmdInfo.Command = aBuf.makeStringAndClear();
                            aCmdInfo.GroupId = nCommandGroup;
                            aCmdList.push_back( aCmdInfo );
                        }
                        pSfxSlot = pAppSlotPool->NextSlot();
                    }
                }
            }
        }
    }

    uno::Sequence< frame::DispatchInformation > aSeq =
        comphelper::containerToSequence< frame::DispatchInformation, std::list< frame::DispatchInformation > >( aCmdList );

    return aSeq;
}

#ifdef TEST_HANDLERS
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/awt/XKeyHandler.hpp>
#include <com/sun/star/awt/XMouseClickHandler.hpp>

class TestKeyHandler: public ::cppu::WeakImplHelper2
<
    com::sun::star::awt::XKeyHandler,
    com::sun::star::lang::XServiceInfo
>
{
public:
    TestKeyHandler( const com::sun::star::uno::Reference < lang::XMultiServiceFactory >& ){}

    SFX_DECL_XSERVICEINFO
    virtual sal_Bool SAL_CALL keyPressed( const awt::KeyEvent& aEvent ) throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL keyReleased( const awt::KeyEvent& aEvent ) throw (uno::RuntimeException);
    virtual void SAL_CALL disposing( const lang::EventObject& Source)
        throw (uno::RuntimeException);
};

class TestMouseClickHandler: public ::cppu::WeakImplHelper2
<
    com::sun::star::awt::XMouseClickHandler,
    com::sun::star::lang::XServiceInfo
>
{
public:
    TestMouseClickHandler( const com::sun::star::uno::Reference < lang::XMultiServiceFactory >& ){}

    SFX_DECL_XSERVICEINFO
    virtual sal_Bool SAL_CALL mousePressed( const awt::MouseEvent& e ) throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL mouseReleased( const awt::MouseEvent& e ) throw (uno::RuntimeException);
    virtual void SAL_CALL disposing( const lang::EventObject& Source)
        throw (uno::RuntimeException);
};

sal_Bool SAL_CALL TestKeyHandler::keyPressed( const awt::KeyEvent& aEvent ) throw (uno::RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL TestKeyHandler::keyReleased( const awt::KeyEvent& aEvent ) throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL TestKeyHandler::disposing( const lang::EventObject& Source) throw (uno::RuntimeException)
{
}

sal_Bool SAL_CALL TestMouseClickHandler::mousePressed( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    return sal_False;
}

sal_Bool SAL_CALL TestMouseClickHandler::mouseReleased( const awt::MouseEvent& e ) throw (uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL TestMouseClickHandler::disposing( const lang::EventObject& Source) throw (uno::RuntimeException)
{
}

SFX_IMPL_XSERVICEINFO( TestKeyHandler, "com.sun.star.task.Job", "com.sun.star.comp.Office.KeyHandler");
SFX_IMPL_XSERVICEINFO( TestMouseClickHandler, "com.sun.star.task.Job", "com.sun.star.comp.Office.MouseClickHandler");
SFX_IMPL_SINGLEFACTORY( TestKeyHandler );
SFX_IMPL_SINGLEFACTORY( TestMouseClickHandler );
#endif
// -----------------------------------------------------------------------

extern "C" {

SFX2_DLLPUBLIC void* SAL_CALL sfx_component_getFactory(
    const sal_Char* pImplementationName ,
    void*           pServiceManager     ,
    void*                                 )
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        // Define variables which are used in following macros.
        uno::Reference<uno::XInterface> xFactory;
        uno::Reference<lang::XMultiServiceFactory> xServiceManager( reinterpret_cast<lang::XMultiServiceFactory*>( pServiceManager ) ) ;

        //=============================================================================
        //  Add new macro line to handle new service.
        //  !!! ATTENTION !!!
        //      Write no ";" at end of line and dont forget "else" ! (see macro)
        //=============================================================================
        IF_NAME_CREATECOMPONENTFACTORY( BackingComp )
        IF_NAME_CREATECOMPONENTFACTORY( SfxGlobalEvents_Impl )
        IF_NAME_CREATECOMPONENTFACTORY( SfxFrameLoader_Impl )
        IF_NAME_CREATECOMPONENTFACTORY( SfxMacroLoader )
        IF_NAME_CREATECOMPONENTFACTORY( SfxAppDispatchProvider )
        IF_NAME_CREATECOMPONENTFACTORY( SfxDocTplService )
        IF_NAME_CREATECOMPONENTFACTORY( ShutdownIcon )
        IF_NAME_CREATECOMPONENTFACTORY( SfxApplicationScriptLibraryContainer )
        IF_NAME_CREATECOMPONENTFACTORY( SfxApplicationDialogLibraryContainer )
#ifdef TEST_HANDLERS
        IF_NAME_CREATECOMPONENTFACTORY( TestKeyHandler )
        IF_NAME_CREATECOMPONENTFACTORY( TestMouseClickHandler )
#endif
        IF_NAME_CREATECOMPONENTFACTORY( OPackageStructureCreator )
        IF_NAME_CREATECOMPONENTFACTORY( ::sfx2::PluginObject )
        IF_NAME_CREATECOMPONENTFACTORY( ::sfx2::IFrameObject )
        IF_NAME_CREATECOMPONENTFACTORY( ::sfx2::OwnSubFilterService )
        if ( ::comp_SfxDocumentMetaData::_getImplementationName().equals(
                 OUString::createFromAscii( pImplementationName ) ) )
        {
            xFactory = ::cppu::createSingleComponentFactory(
            ::comp_SfxDocumentMetaData::_create,
            ::comp_SfxDocumentMetaData::_getImplementationName(),
            ::comp_SfxDocumentMetaData::_getSupportedServiceNames());
        }
        if ( ::comp_CompatWriterDocProps::_getImplementationName().equals(
                 OUString::createFromAscii( pImplementationName ) ) )
        {
            xFactory = ::cppu::createSingleComponentFactory(
            ::comp_CompatWriterDocProps::_create,
            ::comp_CompatWriterDocProps::_getImplementationName(),
            ::comp_CompatWriterDocProps::_getSupportedServiceNames());
        }

        // Factory is valid - service was found.

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }
    // Return with result of this operation.
    return pReturn ;
}
} // extern "C"

//=========================================================================

void SAL_CALL FilterOptionsContinuation::setFilterOptions(
                const uno::Sequence<beans::PropertyValue>& rProps )
        throw (uno::RuntimeException)
{
    rProperties = rProps;
}

uno::Sequence< beans::PropertyValue > SAL_CALL
    FilterOptionsContinuation::getFilterOptions()
        throw (uno::RuntimeException)
{
    return rProperties;
}

//=========================================================================

RequestFilterOptions::RequestFilterOptions( uno::Reference< frame::XModel > rModel,
                              uno::Sequence< beans::PropertyValue > rProperties )
{
    OUString temp;
    uno::Reference< uno::XInterface > temp2;
    document::FilterOptionsRequest aOptionsRequest( temp,
                                                                      temp2,
                                                                      rModel,
                                                                      rProperties );

    m_aRequest <<= aOptionsRequest;

    m_pAbort  = new comphelper::OInteractionAbort;
    m_pOptions = new FilterOptionsContinuation;

    m_lContinuations.realloc( 2 );
    m_lContinuations[0] = uno::Reference< task::XInteractionContinuation >( m_pAbort  );
    m_lContinuations[1] = uno::Reference< task::XInteractionContinuation >( m_pOptions );
}

uno::Any SAL_CALL RequestFilterOptions::getRequest()
        throw( uno::RuntimeException )
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL RequestFilterOptions::getContinuations()
        throw( uno::RuntimeException )
{
    return m_lContinuations;
}

//=========================================================================
class RequestPackageReparation_Impl : public ::cppu::WeakImplHelper1< task::XInteractionRequest >
{
    uno::Any m_aRequest;
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > m_lContinuations;
    comphelper::OInteractionApprove* m_pApprove;
    comphelper::OInteractionDisapprove*  m_pDisapprove;

public:
    RequestPackageReparation_Impl( OUString aName );
    sal_Bool    isApproved();
    virtual uno::Any SAL_CALL getRequest() throw( uno::RuntimeException );
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations()
        throw( uno::RuntimeException );
};

RequestPackageReparation_Impl::RequestPackageReparation_Impl( OUString aName )
{
    OUString temp;
    uno::Reference< uno::XInterface > temp2;
    document::BrokenPackageRequest aBrokenPackageRequest( temp,
                                                                                 temp2,
                                                                              aName );
       m_aRequest <<= aBrokenPackageRequest;
    m_pApprove = new comphelper::OInteractionApprove;
    m_pDisapprove = new comphelper::OInteractionDisapprove;
       m_lContinuations.realloc( 2 );
       m_lContinuations[0] = uno::Reference< task::XInteractionContinuation >( m_pApprove );
       m_lContinuations[1] = uno::Reference< task::XInteractionContinuation >( m_pDisapprove );
}

sal_Bool RequestPackageReparation_Impl::isApproved()
{
    return m_pApprove->wasSelected();
}

uno::Any SAL_CALL RequestPackageReparation_Impl::getRequest()
        throw( uno::RuntimeException )
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL RequestPackageReparation_Impl::getContinuations()
        throw( uno::RuntimeException )
{
    return m_lContinuations;
}

RequestPackageReparation::RequestPackageReparation( OUString aName )
{
    pImp = new RequestPackageReparation_Impl( aName );
    pImp->acquire();
}

RequestPackageReparation::~RequestPackageReparation()
{
    pImp->release();
}

sal_Bool RequestPackageReparation::isApproved()
{
    return pImp->isApproved();
}

com::sun::star::uno::Reference < task::XInteractionRequest > RequestPackageReparation::GetRequest()
{
    return com::sun::star::uno::Reference < task::XInteractionRequest >(pImp);
}

//=========================================================================
class NotifyBrokenPackage_Impl : public ::cppu::WeakImplHelper1< task::XInteractionRequest >
{
    uno::Any m_aRequest;
    uno::Sequence< uno::Reference< task::XInteractionContinuation > > m_lContinuations;
    comphelper::OInteractionAbort*  m_pAbort;

public:
    NotifyBrokenPackage_Impl( OUString aName );
    virtual uno::Any SAL_CALL getRequest() throw( uno::RuntimeException );
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations()
        throw( uno::RuntimeException );
};

NotifyBrokenPackage_Impl::NotifyBrokenPackage_Impl( OUString aName )
{
    OUString temp;
    uno::Reference< uno::XInterface > temp2;
    document::BrokenPackageRequest aBrokenPackageRequest( temp,
                                                                                 temp2,
                                                                              aName );
       m_aRequest <<= aBrokenPackageRequest;
    m_pAbort  = new comphelper::OInteractionAbort;
       m_lContinuations.realloc( 1 );
       m_lContinuations[0] = uno::Reference< task::XInteractionContinuation >( m_pAbort  );
}

uno::Any SAL_CALL NotifyBrokenPackage_Impl::getRequest()
        throw( uno::RuntimeException )
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > >
    SAL_CALL NotifyBrokenPackage_Impl::getContinuations()
        throw( uno::RuntimeException )
{
    return m_lContinuations;
}

NotifyBrokenPackage::NotifyBrokenPackage( OUString aName )
{
    pImp = new NotifyBrokenPackage_Impl( aName );
    pImp->acquire();
}

NotifyBrokenPackage::~NotifyBrokenPackage()
{
    pImp->release();
}

com::sun::star::uno::Reference < task::XInteractionRequest > NotifyBrokenPackage::GetRequest()
{
    return com::sun::star::uno::Reference < task::XInteractionRequest >(pImp);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

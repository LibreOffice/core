/*************************************************************************
 *
 *  $RCSfile: appuno.cxx,v $
 *
 *  $Revision: 1.68 $
 *
 *  last change: $Author: mba $ $Date: 2002-07-24 17:57:22 $
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

#include "appuno.hxx"

#include <svtools/sbx.hxx>
#include <svtools/itempool.hxx>

#ifndef _SFXRECTITEM_HXX //autogen
#include <svtools/rectitem.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _WLDCRD_HXX //autogen
#include <tools/wldcrd.hxx>
#endif

#include <tools/urlobj.hxx>

#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _INET_CONFIG_HXX //autogen
#include <inet/inetcfg.hxx>
#endif
#ifndef __SBX_SBXMETHOD_HXX
#include <svtools/sbxmeth.hxx>
#endif
#ifndef _SB_SBMETH_HXX
#include <basic/sbmeth.hxx>
#endif
#ifndef _SBX_SBXOBJECT_HXX
#include <svtools/sbxobj.hxx>
#endif
#ifndef _SB_SBERRORS_HXX
#include <basic/sberrors.hxx>
#endif
#ifndef _BASMGR_HXX
#include <basic/basmgr.hxx>
#endif
#ifndef _BASIC_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif

#include <svtools/sbxcore.hxx>
#include <svtools/ownlist.hxx>
#include <svtools/lckbitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/intitem.hxx>
#include <svtools/eitem.hxx>

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTIONEVENT_HPP_
#include <com/sun/star/frame/FrameActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_CONTAINEREVENT_HPP_
#include <com/sun/star/container/ContainerEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_REGISTRYVALUETYPE_HPP_
#include <com/sun/star/registry/RegistryValueType.hpp>
#endif
#include <comphelper/processfactory.hxx>
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBUTTON_HPP_
#include <com/sun/star/awt/XButton.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTEVENT_HPP_
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif

#include <tools/cachestr.hxx>
#include <osl/mutex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::rtl;

#pragma hdrstop

#include "sfxtypes.hxx"
#include "sfxuno.hxx"
#include "appdata.hxx"
#include "app.hxx"
#include "sfxsids.hrc"
#include "msg.hxx"
#include "msgpool.hxx"
#include "request.hxx"
#include "module.hxx"
#include "fcontnr.hxx"
#include "frmload.hxx"
#include "frame.hxx"
#include "sfxbasic.hxx"
#include "objsh.hxx"
#include "objuno.hxx"
#include "unoctitm.hxx"
#include "dispatch.hxx"
#include "doctemplates.hxx"
#include "shutdownicon.hxx"
#include "scriptcont.hxx"
#include "dlgcont.hxx"
#include "objshimp.hxx"
#include "fltoptint.hxx"
#include "docfile.hxx"

#define FRAMELOADER_SERVICENAME         "com.sun.star.frame.FrameLoader"
#define PROTOCOLHANDLER_SERVICENAME     "com.sun.star.frame.ProtocolHandler"

static const String sTemplateRegionName   = String::CreateFromAscii( "TemplateRegionName"   );
static const String sTemplateName   = String::CreateFromAscii( "TemplateName"   );
static const String sAsTemplate     = String::CreateFromAscii( "AsTemplate"     );
static const String sOpenNewView    = String::CreateFromAscii( "OpenNewView"    );
static const String sViewId         = String::CreateFromAscii( "ViewId"         );
static const String sPluginMode     = String::CreateFromAscii( "PluginMode"     );
static const String sReadOnly       = String::CreateFromAscii( "ReadOnly"       );
static const String sFrameName      = String::CreateFromAscii( "FrameName"      );
static const String sMediaType      = String::CreateFromAscii( "MediaType"    );
static const String sPostData       = String::CreateFromAscii( "PostData"       );
static const String sPosSize        = String::CreateFromAscii( "PosSize"        );
static const String sCharacterSet   = String::CreateFromAscii( "CharacterSet"   );
static const String sInputStream    = String::CreateFromAscii( "InputStream"    );
static const String sOutputStream   = String::CreateFromAscii( "OutputStream"    );
static const String sHidden         = String::CreateFromAscii( "Hidden"         );
static const String sPreview        = String::CreateFromAscii( "Preview"        );
static const String sSilent         = String::CreateFromAscii( "Silent"         );
static const String sJumpMark       = String::CreateFromAscii( "JumpMark"       );
static const String sFileName       = String::CreateFromAscii( "FileName"       );
static const String sOrigURL        = String::CreateFromAscii( "OriginalURL"    );
static const String sSalvageURL     = String::CreateFromAscii( "SalvagedFile"   );
static const String sStatusInd      = String::CreateFromAscii( "StatusIndicator" );
static const String sModel          = String::CreateFromAscii( "Model" );
static const String sFrame          = String::CreateFromAscii( "Frame" );
static const String sViewData       = String::CreateFromAscii( "ViewData" );
static const String sFilterData     = String::CreateFromAscii( "FilterData" );
static const String sSelectionOnly  = String::CreateFromAscii( "SelectionOnly" );
static const String sFilterFlags    = String::CreateFromAscii( "FilterFlags" );
static const String sMacroExecMode  = String::CreateFromAscii( "MacroExecutionMode" );
static const String sUpdateDocMode  = String::CreateFromAscii( "UpdateDocMode" );
static const String sMinimized      = String::CreateFromAscii( "Minimized" );

void TransformParameters( sal_uInt16 nSlotId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs, SfxAllItemSet& rSet, const SfxSlot* pSlot )
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

    const ::com::sun::star::beans::PropertyValue* pPropsVal = rArgs.getConstArray();
    if ( !pSlot->IsMode(SFX_SLOT_METHOD) )
    {
        // slot is a property
        const SfxType* pType = pSlot->GetType();
        SfxPoolItem* pItem = pType->CreateItem();
        if ( !pItem )
        {
#ifdef DBG_UTIL
            ByteString aStr( "No creator method for item: ");
            aStr += ByteString::CreateFromInt32( nSlotId );
            DBG_ERROR( aStr.GetBuffer() );
#endif
            return;
        }

        USHORT nWhich = rSet.GetPool()->GetWhich(nSlotId);
        BOOL bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
        pItem->SetWhich( nWhich );
        USHORT nSubCount = pType->nAttribs;
        if ( nSubCount == 0 )
        {
            // simple property
#ifdef DBG_UTIL
            // this indicates an error only for macro recording; if the dispatch API is used for
            // UI purposes or from the testtool, it is possible to use the "toggle" ability of
            // some property slots, so this should be notified as a warning only
            if ( nCount != 1 )
            {
                ByteString aStr( "MacroPlayer: wrong number of parameters for slot: ");
                aStr += ByteString::CreateFromInt32( nSlotId );
                DBG_WARNING( aStr.GetBuffer() );
            }
#endif
            if ( nCount )
            {
                const ::com::sun::star::beans::PropertyValue& rProp = pPropsVal[0];
                String aName = rProp.Name;
                if ( aName.CompareToAscii( pSlot->pUnoName ) == COMPARE_EQUAL )
                {
                    if( pItem->PutValue( rProp.Value ) )
                        // only use successfully converted items
                        rSet.Put( *pItem );
#ifdef DBG_UTIL
                    else
                    {
                        ByteString aStr( "MacroPlayer: Property not convertable: ");
                        aStr += pSlot->pUnoName;
                        DBG_WARNING( aStr.GetBuffer() );
                    }
#endif
                }
#ifdef DBG_UTIL
                else
                {
                    // for a simple property the name of the only argument *must* match
                    ByteString aStr( "MacroPlayer: Property name does not match: ");
                    aStr += ByteString( aName, RTL_TEXTENCODING_UTF8 );
                    DBG_WARNING( aStr.GetBuffer() );
                }
#endif
            }
        }
        else
        {
#ifdef DBG_UTIL
            // this indicates an error only for macro recording; if the dispatch API is used for
            // UI purposes or from the testtool, it is possible to skip some or all arguments,
            // so this should be notified as a warning only
            if ( nCount != nSubCount )
            {
                ByteString aStr( "MacroPlayer: wrong number of parameters for slot: ");
                aStr += ByteString::CreateFromInt32( nSlotId );
                DBG_WARNING( aStr.GetBuffer() );
            }
#endif
            // complex property; collect sub items from the parameter set and reconstruct complex item
            USHORT nFound=0;
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                const ::com::sun::star::beans::PropertyValue& rProp = pPropsVal[n];
                for ( USHORT nSub=0; nSub<nSubCount; nSub++ )
                {
                    // search sub item by name
                    ByteString aStr( pSlot->pUnoName );
                    aStr += '.';
                    aStr += ByteString( pType->aAttrib[nSub].pName );
                    const char* pName = aStr.GetBuffer();
                    if ( rProp.Name.compareToAscii( pName ) == COMPARE_EQUAL )
                    {
                        BYTE nSubId = (BYTE) (sal_Int8) pType->aAttrib[nSub].nAID;
                        if ( bConvertTwips )
                            nSubId |= CONVERT_TWIPS;
                        if ( pItem->PutValue( rProp.Value, nSubId ) )
                            ++nFound;
                        break;
                    }
                }

#ifdef DBG_UTIL
                if ( nSub >= nSubCount )
                {
                    // for complex property slots every passed argument *must* match to the name of a member of the item
                    ByteString aStr( "MacroPlayer: Property name does not match: ");
                    aStr += ByteString( String(rProp.Name), RTL_TEXTENCODING_UTF8 );
                    DBG_WARNING( aStr.GetBuffer() );
                }
#endif
            }

            if ( nFound == nSubCount )
                // only use completely converted items
                rSet.Put( *pItem );
#ifdef DBG_UTIL
            else
            {
                ByteString aStr( "MacroPlayer: Complex property not convertable: ");
                aStr += pSlot->pUnoName;
                DBG_WARNING( aStr.GetBuffer() );
            }
#endif
        }

        delete pItem;
    }
    else if ( nCount )
    {
#ifdef DBG_UTIL
        // for debugging purposes: detect parameters that don't match to any formal argument or one of its members
        sal_Int32 nFoundArgs = 0;
#endif
        // slot is a method
        for ( sal_uInt16 nArgs=0; nArgs<pSlot->nArgDefCount; nArgs++ )
        {
            const SfxFormalArgument &rArg = pSlot->GetFormalArgument( nArgs );
            SfxPoolItem* pItem = rArg.CreateItem();
            if ( !pItem )
            {
#ifdef DBG_UTIL
                ByteString aStr( "No creator method for argument: ");
                aStr += rArg.pName;
                DBG_ERROR( aStr.GetBuffer() );
#endif
                return;
            }

            USHORT nWhich = rSet.GetPool()->GetWhich(rArg.nSlotId);
            BOOL bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
            pItem->SetWhich( nWhich );
            const SfxType* pType = rArg.pType;
            USHORT nSubCount = pType->nAttribs;
            if ( nSubCount == 0 )
            {
                // "simple" (base type) argument
                for ( sal_uInt16 n=0; n<nCount; n++ )
                {
                    const ::com::sun::star::beans::PropertyValue& rProp = pPropsVal[n];
                    String aName = rProp.Name;
                    if ( aName.CompareToAscii(rArg.pName) == COMPARE_EQUAL )
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
                            ByteString aStr( "MacroPlayer: Property not convertable: ");
                            aStr += rArg.pName;
                            DBG_WARNING( aStr.GetBuffer() );
                        }
#endif
                        break;
                    }
                }
            }
            else
            {
                // complex argument; collect sub items from argument arry and reconstruct complex item
                BOOL bRet = TRUE;
                for ( sal_uInt16 n=0; n<nCount; n++ )
                {
                    const ::com::sun::star::beans::PropertyValue& rProp = pPropsVal[n];
                    for ( USHORT nSub=0; nSub<nSubCount; nSub++ )
                    {
                        // search sub item by name
                        ByteString aStr( rArg.pName );
                        aStr += '.';
                        aStr += pType->aAttrib[nSub].pName;
                        const char* pName = aStr.GetBuffer();
                        if ( rProp.Name.compareToAscii( pName ) == COMPARE_EQUAL )
                        {
#ifdef DBG_UTIL
                            ++nFoundArgs;
#endif
                            BYTE nSubId = (BYTE) (sal_Int8) pType->aAttrib[nSub].nAID;
                            if ( bConvertTwips )
                                nSubId |= CONVERT_TWIPS;
                            if (!pItem->PutValue( rProp.Value, nSubId ) )
                                bRet = FALSE;
                            break;
                        }
                    }
                }

                if ( bRet )
                    // only use completely converted items
                    rSet.Put( *pItem );
            }

            delete pItem;
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
                const ::com::sun::star::beans::PropertyValue& rProp = pPropsVal[n];
                String aName = rProp.Name;
                if ( aName == sFrame )
                    rSet.Put( SfxUnoAnyItem( SID_FILLFRAME, rProp.Value ) );
                else if ( aName == sHidden && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_HIDDEN, *((sal_Bool*)rProp.Value.getValue()) ) );
            }
        }
        else if ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC || nSlotId == SID_SAVEASDOC || nSlotId == SID_SAVETO )
        {
            for ( sal_uInt16 n=0; n<nCount; n++ )
            {
                const ::com::sun::star::beans::PropertyValue& rProp = pPropsVal[n];
                String aName = rProp.Name;
                if ( aName == sModel )
                    rSet.Put( SfxUnoAnyItem( SID_DOCUMENT, rProp.Value ) );
                else if ( aName == sStatusInd )
                    rSet.Put( SfxUnoAnyItem( SID_PROGRESS_STATUSBAR_CONTROL, rProp.Value ) );
                else if ( aName == sViewData )
                    rSet.Put( SfxUnoAnyItem( SID_VIEW_DATA, rProp.Value ) );
                else if ( aName == sFilterData )
                    rSet.Put( SfxUnoAnyItem( SID_FILTER_DATA, rProp.Value ) );
                else if ( aName == sInputStream && rProp.Value.getValueType() == ::getCppuType( (Reference < XInputStream >*)0 ) )
                    rSet.Put( SfxUnoAnyItem( SID_INPUTSTREAM, rProp.Value ) );
                else if ( aName == sOutputStream && rProp.Value.getValueType() == ::getCppuType( (Reference < XOutputStream >*)0 ) )
                    rSet.Put( SfxUnoAnyItem( SID_OUTPUTSTREAM, rProp.Value ) );
                else if ( aName == sPostData && rProp.Value.getValueType() == ::getCppuType( (Reference < XInputStream >*)0 ) )
                    rSet.Put( SfxUnoAnyItem( SID_POSTDATA, rProp.Value ) );
                else if ( aName == sAsTemplate && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_TEMPLATE, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sOpenNewView && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_OPEN_NEW_VIEW, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sViewId && rProp.Value.getValueType() == ::getCppuType((const sal_Int16*)0) )
                    rSet.Put( SfxUInt16Item( SID_VIEW_ID, *((sal_Int16*)rProp.Value.getValue()) ) );
                else if ( aName == sPluginMode && rProp.Value.getValueType() == ::getCppuType((const sal_Int16*)0) )
                    rSet.Put( SfxUInt16Item( SID_PLUGIN_MODE, *((sal_Int16*)rProp.Value.getValue()) ) );
                else if ( aName == sReadOnly && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_DOC_READONLY, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sSelectionOnly && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_SELECTION, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sHidden && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_HIDDEN, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sMinimized && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_MINIMIZEWINS, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sSilent && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_SILENT, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sPreview && rProp.Value.getValueType() == ::getBooleanCppuType() )
                    rSet.Put( SfxBoolItem( SID_PREVIEW, *((sal_Bool*)rProp.Value.getValue()) ) );
                else if ( aName == sFileName && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_FILE_NAME, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sOrigURL && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_ORIGURL, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sSalvageURL && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_DOC_SALVAGE, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sFrameName && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_TARGETNAME, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sMediaType && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_CONTENTTYPE, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sTemplateName && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_TEMPLATE_NAME, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sTemplateRegionName && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_TEMPLATE_REGIONNAME, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sJumpMark && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_JUMPMARK, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sCharacterSet && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_CHARSET, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sFilterFlags && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                    rSet.Put( SfxStringItem( SID_FILE_FILTEROPTIONS, *((::rtl::OUString*)rProp.Value.getValue()) ) );
                else if ( aName == sPosSize && rProp.Value.getValueType() == ::getCppuType((const ::rtl::OUString*)0) )
                {
                    String aPar = *((::rtl::OUString*)rProp.Value.getValue());
                    Size aSize;
                    Point aPos;
                    DBG_ASSERT( sal_False, "TransformParameters()\nProperty \"PosSize\" isn't supported yet!\n" );
                }
                else if ( aName == sMacroExecMode )
                {
                    sal_Int16 nValue;
                    if ( rProp.Value >>= nValue )
                        rSet.Put( SfxUInt16Item( SID_MACROEXECMODE, nValue ) );
                }
                else if ( aName == sUpdateDocMode )
                {
                    sal_Int16 nValue;
                    if ( rProp.Value >>= nValue )
                        rSet.Put( SfxUInt16Item( SID_UPDATEDOCMODE, nValue ) );
                }

            }
        }
#ifdef DB_UTIL
        else if ( nFoundArgs == nCount )
        {
            // except for the "special" slots: assure that every argument was convertable
            ByteString aStr( "MacroPlayer: Some properties didn't match to any formal argument for slot: ");
            aStr += pSlot->pUnoName;
            DBG_WARNING( aStr.GetBuffer() );
        }
#endif
    }
}

void TransformItems( sal_uInt16 nSlotId, const SfxItemSet& rSet, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs, const SfxSlot* pSlot )
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
        USHORT nWhich = rSet.GetPool()->GetWhich(nSlotId);
        if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET ) //???
        {
            USHORT nSubCount = pType->nAttribs;
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
            ByteString aStr( "Processing property slot without argument: ");
            aStr += ByteString::CreateFromInt32( nSlotId );
            DBG_ERROR( aStr.GetBuffer() );
        }
#endif

#ifdef DBG_UTIL
        nItems++;
#endif
    }
    else
    {
        // slot is a method
        USHORT nFormalArgs = pSlot->GetFormalArgumentCount();
        for ( USHORT nArg=0; nArg<nFormalArgs; ++nArg )
        {
            // check every formal argument of the method
            const SfxFormalArgument &rArg = pSlot->GetFormalArgument( nArg );
            USHORT nWhich = rSet.GetPool()->GetWhich( rArg.nSlotId );
            if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET ) //???
            {
                USHORT nSubCount = rArg.pType->nAttribs;
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
        if ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC || nSlotId == SID_SAVEASDOC || nSlotId == SID_SAVETO )
        {
            sal_Int32 nAdditional=0;
            if ( rSet.GetItemState( SID_PROGRESS_STATUSBAR_CONTROL ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_ORIGURL ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_DOC_SALVAGE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_INPUTSTREAM ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_OUTPUTSTREAM ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_TEMPLATE ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_OPEN_NEW_VIEW ) == SFX_ITEM_SET )
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
            if ( rSet.GetItemState( SID_SELECTION ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_CONTENTTYPE ) == SFX_ITEM_SET )
                nAdditional++;
    //        if ( rSet.GetItemState( SID_VIEW_POS_SIZE ) == SFX_ITEM_SET )
    //            nAdditional++;
            if ( rSet.GetItemState( SID_POSTDATA ) == SFX_ITEM_SET )
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
            if ( rSet.GetItemState( SID_MINIMIZEWINS ) == SFX_ITEM_SET )
                nAdditional++;
            if ( rSet.GetItemState( SID_PREVIEW ) == SFX_ITEM_SET )
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

            // consider additional arguments
            nProps += nAdditional;
#ifdef DBG_UTIL
            nItems += nAdditional;
#endif
        }
    }

#ifdef DBG_UTIL
    // now check the itemset: is there any item that is not convertable using the list of formal arguments
    // or the table of additional items?!
    if ( rSet.Count() != nItems )
    {
        // detect unknown item and present error message
        const USHORT *pRanges = rSet.GetRanges();
        while ( *pRanges )
        {
            for(USHORT nId = *pRanges++; nId <= *pRanges; ++nId)
            {
                if ( rSet.GetItemState(nId) < SFX_ITEM_SET ) //???
                    // not really set
                    continue;

                if ( !pSlot->IsMode(SFX_SLOT_METHOD) && nId == rSet.GetPool()->GetWhich( pSlot->GetSlotId() ) )
                    continue;

                USHORT nFormalArgs = pSlot->GetFormalArgumentCount();
                for ( USHORT nArg=0; nArg<nFormalArgs; ++nArg )
                {
                    const SfxFormalArgument &rArg = pSlot->GetFormalArgument( nArg );
                    USHORT nWhich = rSet.GetPool()->GetWhich( rArg.nSlotId );
                    if ( nId == nWhich )
                        break;
                }

                if ( nArg<nFormalArgs )
                    continue;

                if ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC || nSlotId == SID_SAVEASDOC || nSlotId == SID_SAVETO )
                {
                    if ( nId == SID_SAVETO )
                        // used only internally
                        continue;
                    if ( nId == SID_DOCFRAME )
                        continue;
                    if ( nId == SID_PROGRESS_STATUSBAR_CONTROL )
                        continue;
                    if ( nId == SID_VIEW_DATA )
                        continue;
                    if ( nId == SID_FILTER_DATA )
                        continue;
                    if ( nId == SID_DOCUMENT )
                        continue;
                    if ( nId == SID_INPUTSTREAM )
                        continue;
                    if ( nId == SID_OUTPUTSTREAM )
                        continue;
                    if ( nId == SID_POSTDATA )
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
                    if ( nId == SID_SELECTION )
                        continue;
                    if ( nId == SID_HIDDEN )
                        continue;
                    if ( nId == SID_MINIMIZEWINS )
                        continue;
                    if ( nId == SID_SILENT )
                        continue;
                    if ( nId == SID_PREVIEW )
                        continue;
                    if ( nId == SID_TARGETNAME )
                        continue;
                    if ( nId == SID_ORIGURL )
                        continue;
                    if ( nId == SID_DOC_SALVAGE )
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
                }

                ByteString aDbg( "Unknown item detected: ");
                aDbg += ByteString::CreateFromInt32( nId );
                DBG_ASSERT( nArg<nFormalArgs, aDbg.GetBuffer() );
            }
        }
    }
#endif

    if ( !nProps )
        return;

    // convert every item into a property
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aSequ( nProps );
    ::com::sun::star::beans::PropertyValue *pValue = aSequ.getArray();
    nProps = 0;
    if ( !pSlot->IsMode(SFX_SLOT_METHOD) )
    {
        // slot is a property
        USHORT nWhich = rSet.GetPool()->GetWhich(nSlotId);
        BOOL bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
        SFX_ITEMSET_ARG( &rSet, pItem, SfxPoolItem, nWhich, sal_False );
        if ( pItem ) //???
        {
            USHORT nSubCount = pType->nAttribs;
            if ( !nSubCount )
            {
                //rPool.FillVariable( *pItem, *pVar, eUserMetric );
                pValue[nProps].Name = String( String::CreateFromAscii( pSlot->pUnoName ) ) ;
                if ( !pItem->QueryValue( pValue[nProps].Value ) )
                {
                    ByteString aStr( "Item not convertable: ");
                    aStr += ByteString::CreateFromInt32(nSlotId);
                    DBG_ERROR( aStr.GetBuffer() );
                }
            }
            else
            {
                // complex type, add a property value for every member of the struct
                for ( USHORT n=1; n<=nSubCount; ++n )
                {
                    //rPool.FillVariable( *pItem, *pVar, eUserMetric );
                    BYTE nSubId = (BYTE) (sal_Int8) pType->aAttrib[n-1].nAID;
                    if ( bConvertTwips )
                        nSubId |= CONVERT_TWIPS;

                    DBG_ASSERT( nSubId <= 255, "Member ID out of range" );
                    String aName( String::CreateFromAscii( pSlot->pUnoName ) ) ;
                    aName += '.';
                    aName += String( String::CreateFromAscii( pType->aAttrib[n-1].pName ) ) ;
                    pValue[nProps].Name = aName;
                    if ( !pItem->QueryValue( pValue[nProps++].Value, nSubId ) )
                    {
                        ByteString aStr( "Sub item ");
                        aStr += ByteString::CreateFromInt32( pType->aAttrib[n-1].nAID );
                        aStr += " not convertable in slot: ";
                        aStr += ByteString::CreateFromInt32(nSlotId);
                        DBG_ERROR( aStr.GetBuffer() );
                    }
                }
            }
        }
    }
    else
    {
        // slot is a method
        USHORT nFormalArgs = pSlot->GetFormalArgumentCount();
        for ( USHORT nArg=0; nArg<nFormalArgs; ++nArg )
        {
            const SfxFormalArgument &rArg = pSlot->GetFormalArgument( nArg );
            USHORT nWhich = rSet.GetPool()->GetWhich( rArg.nSlotId );
            BOOL bConvertTwips = ( rSet.GetPool()->GetMetric( nWhich ) == SFX_MAPUNIT_TWIP );
            SFX_ITEMSET_ARG( &rSet, pItem, SfxPoolItem, nWhich, sal_False );
            if ( pItem ) //???
            {
                USHORT nSubCount = rArg.pType->nAttribs;
                if ( !nSubCount )
                {
                    //rPool.FillVariable( *pItem, *pVar, eUserMetric );
                    pValue[nProps].Name = String( String::CreateFromAscii( rArg.pName ) ) ;
                    if ( !pItem->QueryValue( pValue[nProps++].Value ) )
                    {
                        ByteString aStr( "Item not convertable: ");
                        aStr += ByteString::CreateFromInt32(rArg.nSlotId);
                        DBG_ERROR( aStr.GetBuffer() );
                    }
                }
                else
                {
                    // complex type, add a property value for every member of the struct
                    for ( USHORT n = 1; n <= nSubCount; ++n )
                    {
                        //rPool.FillVariable( rItem, *pVar, eUserMetric );
                        BYTE nSubId = (BYTE) (sal_Int8) rArg.pType->aAttrib[n-1].nAID;
                        if ( bConvertTwips )
                            nSubId |= CONVERT_TWIPS;

                        DBG_ASSERT( nSubId <= 255, "Member ID out of range" );
                        String aName( String::CreateFromAscii( rArg.pName ) ) ;
                        aName += '.';
                        aName += String( String::CreateFromAscii( rArg.pType->aAttrib[n-1].pName ) ) ;
                        pValue[nProps].Name = aName;
                        if ( !pItem->QueryValue( pValue[nProps++].Value, nSubId ) )
                        {
                            ByteString aStr( "Sub item ");
                            aStr += ByteString::CreateFromInt32( rArg.pType->aAttrib[n-1].nAID );
                            aStr += " not convertable in slot: ";
                            aStr += ByteString::CreateFromInt32(rArg.nSlotId);
                            DBG_ERROR( aStr.GetBuffer() );
                        }
                    }
                }
            }
        }

        if ( nSlotId == SID_OPENDOC || nSlotId == SID_EXPORTDOC || nSlotId == SID_SAVEASDOC || nSlotId == SID_SAVETO )
        {
            const SfxPoolItem *pItem=0;
            if ( rSet.GetItemState( SID_PROGRESS_STATUSBAR_CONTROL, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sStatusInd;
                pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_VIEW_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sViewData;
                pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_FILTER_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sFilterData;
                pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_DOCUMENT, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sModel;
                pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_INPUTSTREAM, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sInputStream;
                pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_OUTPUTSTREAM, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sOutputStream;
                pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_POSTDATA, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sPostData;
                pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_TEMPLATE, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sAsTemplate;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_OPEN_NEW_VIEW, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sOpenNewView;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_VIEW_ID, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sViewId;
                pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_PLUGIN_MODE, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sPluginMode;
                pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_DOC_READONLY, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sReadOnly;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_SELECTION, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sSelectionOnly;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_HIDDEN, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sHidden;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_MINIMIZEWINS, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sMinimized;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_SILENT, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sSilent;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_PREVIEW, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sPreview;
                pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_TARGETNAME, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sFrameName;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
            }
            if ( rSet.GetItemState( SID_ORIGURL, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sOrigURL;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
            }
            if ( rSet.GetItemState( SID_DOC_SALVAGE, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sSalvageURL;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
            }
            if ( rSet.GetItemState( SID_CONTENTTYPE, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sMediaType;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
            }
            if ( rSet.GetItemState( SID_TEMPLATE_NAME, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sTemplateName;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
            }
            if ( rSet.GetItemState( SID_TEMPLATE_REGIONNAME, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sTemplateRegionName;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
            }
            if ( rSet.GetItemState( SID_JUMPMARK, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sJumpMark;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
            }

            SFX_ITEMSET_ARG( &rSet, pRectItem, SfxRectangleItem, SID_VIEW_POS_SIZE, sal_False );
            if ( pRectItem )
            {
    //            pValue[nProps].Name = sPosSize;
    //            Rectangle aRect = pRectItem->GetValue();
                DBG_ERROR("PosSizeItem not supported yet!");
            }

            if ( rSet.GetItemState( SID_CHARSET, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sCharacterSet;
                pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
            }
            if ( rSet.GetItemState( SID_MACROEXECMODE, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sMacroExecMode;
                pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
            }
            if ( rSet.GetItemState( SID_UPDATEDOCMODE, sal_False, &pItem ) == SFX_ITEM_SET )
            {
                pValue[nProps].Name = sUpdateDocMode;
                pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
            }
        }
    }

    rArgs = aSequ;
}

SFX_IMPL_XINTERFACE_3( SfxMacroLoader, OWeakObject, ::com::sun::star::frame::XDispatchProvider, ::com::sun::star::frame::XNotifyingDispatch, ::com::sun::star::frame::XDispatch )
SFX_IMPL_XTYPEPROVIDER_3( SfxMacroLoader, ::com::sun::star::frame::XDispatchProvider, ::com::sun::star::frame::XNotifyingDispatch, ::com::sun::star::frame::XDispatch )
SFX_IMPL_XSERVICEINFO( SfxMacroLoader, PROTOCOLHANDLER_SERVICENAME, "com.sun.star.comp.sfx2.SfxMacroLoader" )
SFX_IMPL_SINGLEFACTORY( SfxMacroLoader )

// -----------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL SfxMacroLoader::queryDispatch( const ::com::sun::star::util::URL&   aURL            ,
                                                                                                               const ::rtl::OUString&               sTargetFrameName,
                                                                                                                     sal_Int32                      nSearchFlags    ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > xDispatcher;
    if(aURL.Complete.compareToAscii("macro:",6)==0)
        xDispatcher = this;
    return xDispatcher;
}

// -----------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > > SAL_CALL
                SfxMacroLoader::queryDispatches( const ::com::sun::star::uno::Sequence < ::com::sun::star::frame::DispatchDescriptor >& seqDescriptor )
                    throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int32 nCount = seqDescriptor.getLength();
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference < ::com::sun::star::frame::XDispatch > > lDispatcher(nCount);
    for( sal_Int32 i=0; i<nCount; ++i )
        lDispatcher[i] = this->queryDispatch( seqDescriptor[i].FeatureURL,
                                              seqDescriptor[i].FrameName,
                                              seqDescriptor[i].SearchFlags );
    return lDispatcher;
}

// -----------------------------------------------------------------------
void SAL_CALL SfxMacroLoader::dispatchWithNotification( const ::com::sun::star::util::URL&                                                          aURL      ,
                                                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&            lArgs     ,
                                                        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchResultListener >& xListener )
              throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_uInt32 nPropertyCount = lArgs.getLength();
    ::rtl::OUString aReferer;
    for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( lArgs[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("Referer")) )
        {
            lArgs[nProperty].Value >>= aReferer;
            break;
        }
    }

    // call from UI?
    ErrCode nErr = ERRCODE_NONE;
    if ( aReferer.compareToAscii("private:select", 12) == 0 )
        nErr = loadMacro( aURL.Complete, SfxObjectShell::Current() );
    else
        nErr = loadMacro( aURL.Complete );

    if( xListener.is() )
    {
        // always call dispatchFinished(), because we didn't load a document but
        // executed a macro instead!
        ::com::sun::star::frame::DispatchResultEvent aEvent;

        aEvent.Source = static_cast< ::cppu::OWeakObject* >(this);
        if( nErr == ERRCODE_NONE )
            aEvent.State = ::com::sun::star::frame::DispatchResultState::SUCCESS;
        else
            aEvent.State = ::com::sun::star::frame::DispatchResultState::FAILURE;

        xListener->dispatchFinished( aEvent ) ;
    }
}

// -----------------------------------------------------------------------
void SAL_CALL SfxMacroLoader::dispatch( const ::com::sun::star::util::URL&                                               aURL  ,
                                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArgs )
              throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    sal_uInt32 nPropertyCount = lArgs.getLength();
    ::rtl::OUString aReferer;
    for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( lArgs[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("Referer")) )
        {
            lArgs[nProperty].Value >>= aReferer;
            break;
        }
    }

    // call from UI?
    if ( aReferer.compareToAscii("private:select", 12) == 0 )
        loadMacro( aURL.Complete, SfxObjectShell::Current() );
    else
        loadMacro( aURL.Complete );
}

// -----------------------------------------------------------------------
void SAL_CALL SfxMacroLoader::addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl ,
                                                 const ::com::sun::star::util::URL&                                                  aURL     )
              throw (::com::sun::star::uno::RuntimeException)
{
    /* TODO
            How we can handle different listener for further coming or currently running dispatch() jobs
            without any inconsistency!
     */
}

// -----------------------------------------------------------------------
void SAL_CALL SfxMacroLoader::removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& xControl ,
                                                    const ::com::sun::star::util::URL&                                                  aURL     )
        throw (::com::sun::star::uno::RuntimeException)
{
}

// -----------------------------------------------------------------------
SfxMacroLoader::SfxMacroLoader( com::sun::star::uno::Reference < class com::sun::star::lang::XMultiServiceFactory > const &)
{
}

// -----------------------------------------------------------------------
SfxMacroLoader::~SfxMacroLoader()
{
}

// -----------------------------------------------------------------------
ErrCode SfxMacroLoader::loadMacro( const ::rtl::OUString& rURL, SfxObjectShell* pSh )
    throw ( ::com::sun::star::uno::RuntimeException )
{
    SfxApplication* pApp = SFX_APP();
    pApp->EnterBasicCall();
    SfxObjectShell* pCurrent = pSh;
    if ( !pCurrent )
        // all not full qualified names use the BASIC of the given or current document
        pCurrent = SfxObjectShell::Current();

    // 'macro:///lib.mod.proc(args)' => macro of App-BASIC
    // 'macro://[docname|.]/lib.mod.proc(args)' => macro of current or qualified document
    // 'macro://obj.method(args)' => direct API call, execute it via App-BASIC
    String aMacro( rURL );
    sal_uInt16 nHashPos = aMacro.Search( '/', 8 );
    sal_uInt16 nArgsPos = aMacro.Search( '(' );
    BasicManager *pAppMgr = SFX_APP()->GetBasicManager();
    BasicManager *pBasMgr = 0;
    ErrCode nErr = ERRCODE_NONE;

    // should a macro function be executed ( no direct API call)?
    if ( STRING_NOTFOUND != nHashPos && nHashPos < nArgsPos )
    {
        // find BasicManager
        SfxObjectShell* pDoc = NULL;
        String aBasMgrName( INetURLObject::decode(aMacro.Copy( 8, nHashPos-8 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET) );
        if ( !aBasMgrName.Len() )
            pBasMgr = pAppMgr;
        else if ( aBasMgrName.EqualsAscii(".") )
        {
            // current/actual document
            pDoc = pCurrent;
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
            if ( pSh && pDoc )
            {
                // security check for macros from document basic if an SFX context (pSh) is given
                pDoc->AdjustMacroMode( String() );
                if( pDoc->Get_Impl()->nMacroMode == ::com::sun::star::document::MacroExecMode::NEVER_EXECUTE )
                    // check forbids execution
                    return ERRCODE_IO_ACCESSDENIED;;
            }

            // find BASIC method
            String aQualifiedMethod( INetURLObject::decode(aMacro.Copy( nHashPos+1 ), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET) );
            String aArgs;
            if ( STRING_NOTFOUND != nArgsPos )
            {
                // remove arguments from macro name
                aArgs = aQualifiedMethod.Copy( nArgsPos - nHashPos - 1 );
                aQualifiedMethod.Erase( nArgsPos - nHashPos - 1 );
            }

            SbxMethod *pMethod = SfxQueryMacro( pBasMgr, aQualifiedMethod );
            if ( pMethod )
            {
                // arguments must be quoted
                String aQuotedArgs;
                if ( aArgs.Len()<2 || aArgs.GetBuffer()[1] == '\"')
                    // no args or already quoted args
                    aQuotedArgs = aArgs;
                else
                {
                    // quote parameters
                    aArgs.Erase(0,1);
                    aArgs.Erase( aArgs.Len()-1,1);

                    aQuotedArgs = '(';

                    sal_uInt16 nCount = aArgs.GetTokenCount(',');
                    for ( sal_uInt16 n=0; n<nCount; n++ )
                    {
                        aQuotedArgs += '\"';
                        aQuotedArgs += aArgs.GetToken( n, ',' );
                        aQuotedArgs += '\"';
                        if ( n<nCount-1 )
                            aQuotedArgs += ',';
                    }

                    aQuotedArgs += ')';
                }

                SbxBaseRef xOldVar;
                SbxVariable *pCompVar = NULL;
                if ( pSh )
                {
                    if ( pBasMgr != pAppMgr )
                        // mark document: it executes an own macro, so it's in a modal mode
                        pSh->SetMacroMode_Impl( TRUE );
                    if ( pBasMgr == pAppMgr )
                    {
                        // document is executed via AppBASIC, adjust "ThisComponent" variable
                        StarBASIC* pBas = pAppMgr->GetLib(0);
                        pCompVar = pBas->Find( DEFINE_CONST_UNICODE("ThisComponent"), SbxCLASS_OBJECT );
                        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                                xInterface ( pSh->GetModel() , ::com::sun::star::uno::UNO_QUERY );
                        ::com::sun::star::uno::Any aAny;
                        aAny <<= xInterface;
                        if ( pCompVar )
                        {
                            xOldVar = pCompVar->GetObject();
                            pCompVar->PutObject( GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aAny ) );
                        }
                        else
                        {
                            SbxObjectRef xUnoObj = GetSbUnoObject( DEFINE_CONST_UNICODE("ThisComponent"), aAny );
                            xUnoObj->SetFlag( SBX_DONTSTORE );
                            pBas->Insert( xUnoObj );
                            pCompVar = pBas->Find( DEFINE_CONST_UNICODE("ThisComponent"), SbxCLASS_OBJECT );
                        }
                    }
                }

                // add quoted arguments and do the call
                String aCall( '[' );
                aCall += pMethod->GetName();
                aCall += aQuotedArgs;
                aCall += ']';

                // execute function using its Sbx parent,
                pMethod->GetParent()->Execute( aCall );
                nErr = SbxBase::GetError();
                if ( pCompVar )
                    // reset "ThisComponent" to prior value
                    pCompVar->PutObject( xOldVar );

                if ( pSh )
                    // remove flag for modal mode
                    pSh->SetMacroMode_Impl( FALSE );
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
        String aCall( '[' );
        aCall += INetURLObject::decode(aMacro.Copy(6), INET_HEX_ESCAPE, INetURLObject::DECODE_WITH_CHARSET);
        aCall += ']';
        pAppMgr->GetLib(0)->Execute( aCall );
        nErr = SbxBase::GetError();
    }

    pApp->LeaveBasicCall();
    SbxBase::ResetError();
    return nErr;
}

SFX_IMPL_XSERVICEINFO( SfxAppDispatchProvider, "com.sun.star.frame.DispatchProvider", "com.sun.star.comp.sfx2.AppDispatchProvider" )                                                                \
SFX_IMPL_ONEINSTANCEFACTORY( SfxAppDispatchProvider );

Reference < XDispatch > SAL_CALL SfxAppDispatchProvider::queryDispatch( const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& sTargetFrameName,
                    FrameSearchFlags eSearchFlags ) throw( RuntimeException )
{
    USHORT nId = 0;
    Reference < XDispatch > xDisp;
    if ( aURL.Protocol.compareToAscii( "slot:" ) == COMPARE_EQUAL ||
         aURL.Protocol.compareToAscii( "commandId:" ) == COMPARE_EQUAL )
    {
        nId = (USHORT) aURL.Path.toInt32();
    }

    if ( aURL.Protocol.compareToAscii( ".uno:" ) == COMPARE_EQUAL )
    {
        // Support ".uno" commands. Map commands to slotid
        nId = SFX_APP()->GetAppDispatcher_Impl()->GetSlotId( aURL.Main );
    }

    if ( nId && SFX_APP()->GetAppDispatcher_Impl()->HasSlot_Impl( nId ) )
        xDisp = new SfxOfficeDispatch( SFX_APP()->GetAppDispatcher_Impl(), nId, aURL ) ;

    return xDisp;
}

Sequence< Reference < XDispatch > > SAL_CALL SfxAppDispatchProvider::queryDispatches( const Sequence < DispatchDescriptor >& seqDescriptor )
                        throw( RuntimeException )
{
    return Sequence< Reference < XDispatch > >();
}

// -----------------------------------------------------------------------

#define IMPLEMENTATION_NAME "com.sun.comp.jsimport.IchitaroImportFilter"
#define SERVICE_NAME        "com.sun.star.document.ImportFilter"

extern "C" {

void SAL_CALL component_getImplementationEnvironment(   const   sal_Char**          ppEnvironmentTypeName   ,
                                                                uno_Environment**   ppEnvironment           )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL component_writeInfo(  void*   pServiceManager ,
                                        void*   pRegistryKey    )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >        xKey( reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( pRegistryKey ) )   ;

    // Eigentliche Implementierung und ihre Services registrieren
    ::rtl::OUString aImpl;
    ::rtl::OUString aTempStr;
    ::rtl::OUString aKeyStr;
    Reference< XRegistryKey > xNewKey;
    Reference< XRegistryKey > xLoaderKey;

    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxAppDispatchProvider::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.frame.DispatchProvider") );

    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxStandaloneDocumentInfoObject::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.document.StandaloneDocumentInfo") );

    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxFrameLoader_Impl::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    Sequence < ::rtl::OUString > aServices = SfxFrameLoader_Impl::impl_getStaticSupportedServiceNames();
    sal_Int32 nCount = aServices.getLength();
    for ( sal_Int16 i=0; i<nCount; i++ )
        xNewKey->createKey( aServices.getConstArray()[i] );

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/Loader"));
    xNewKey = xKey->createKey( aTempStr );

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/Loader"));
    xLoaderKey = xKey->createKey( aTempStr );
    xNewKey = xLoaderKey->createKey( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Pattern")) );
    xNewKey->setAsciiValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("private:factory/*" )) );

    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxMacroLoader::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.frame.FrameLoader") );

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/Loader"));
    xNewKey = xKey->createKey( aTempStr );

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/Loader"));
    xLoaderKey = xKey->createKey( aTempStr );
    xNewKey = xLoaderKey->createKey( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Pattern")) );
    xNewKey->setAsciiValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("macro:*" )) );

    // - sfx document templates
    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxDocTplService::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.frame.DocumentTemplates") );

    // quickstart wrapper service
    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += ShutdownIcon::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.office.Quickstart") );

    // script library container service
    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxScriptLibraryContainer::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.script.ScriptLibraryContainer") );

    // application script library container service
    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxApplicationScriptLibraryContainer::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.script.ApplicationScriptLibraryContainer") );

    // dialog library container service
    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxDialogLibraryContainer::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.script.DialogLibraryContainer") );

    // application dialog library container service
    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += SfxApplicationDialogLibraryContainer::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.script.ApplicationDialogLibraryContainer") );

#if 0
    if (pRegistryKey)
    {
            try
            {
                Reference< XRegistryKey > xKey(
                    reinterpret_cast< XRegistryKey * >( pRegistryKey ) );

                Reference< XRegistryKey > xNewKey = xKey->createKey(
                    ::rtl::OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) );
                xNewKey->createKey( ::rtl::OUString::createFromAscii( SERVICE_NAME ) );

                return sal_True;
            }
            catch (InvalidRegistryException &)
            {
                OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
            }
    }
#endif
    return sal_True;
}

void* SAL_CALL component_getFactory(    const   sal_Char*   pImplementationName ,
                                                void*       pServiceManager     ,
                                                void*       pRegistryKey        )
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        // Define variables which are used in following macros.
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >   xFactory                                                                                                ;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    xServiceManager( reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ) ) ;

        //=============================================================================
        //  Add new macro line to handle new service.
        //
        //  !!! ATTENTION !!!
        //      Write no ";" at end of line and dont forget "else" ! (see macro)
        //=============================================================================
        IF_NAME_CREATECOMPONENTFACTORY( SfxFrameLoader_Impl )
        IF_NAME_CREATECOMPONENTFACTORY( SfxMacroLoader )
        IF_NAME_CREATECOMPONENTFACTORY( SfxStandaloneDocumentInfoObject )
        IF_NAME_CREATECOMPONENTFACTORY( SfxAppDispatchProvider )
        IF_NAME_CREATECOMPONENTFACTORY( SfxDocTplService )
        IF_NAME_CREATECOMPONENTFACTORY( ShutdownIcon )
        IF_NAME_CREATECOMPONENTFACTORY( SfxScriptLibraryContainer )
        IF_NAME_CREATECOMPONENTFACTORY( SfxDialogLibraryContainer )
        IF_NAME_CREATECOMPONENTFACTORY( SfxApplicationScriptLibraryContainer )
        IF_NAME_CREATECOMPONENTFACTORY( SfxApplicationDialogLibraryContainer )

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
#if 0
        if (!xFactory.is() && pServiceManager )
        {
                ::rtl::OUString aImplementationName = ::rtl::OUString::createFromAscii( pImplementationName );
                if (aImplementationName == ::rtl::OUString::createFromAscii( IMPLEMENTATION_NAME ) )
                {
                    xFactory = ::cppu::createSingleFactory( xServiceManager, aImplementationName,
                                                IchitaroImportFilter_CreateInstance,
                                                IchitaroImportFilter::getSupportedServiceNames_Static() );
                }
                if (xFactory.is())
                {
                    xFactory->acquire();
                    pReturn = xFactory.get();
                }
        }
#endif
    }
    // Return with result of this operation.
    return pReturn ;
}
} // extern "C"

//=========================================================================

void SAL_CALL FilterOptionsContinuation::setFilterOptions(
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProps )
        throw (::com::sun::star::uno::RuntimeException)
{
    rProperties = rProps;
}

::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
    FilterOptionsContinuation::getFilterOptions()
        throw (::com::sun::star::uno::RuntimeException)
{
    return rProperties;
}

//=========================================================================

RequestFilterOptions::RequestFilterOptions( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > rModel,
                              ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rProperties )
{
    ::rtl::OUString temp;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > temp2;
    ::com::sun::star::document::FilterOptionsRequest aOptionsRequest( temp,
                                                                         temp2,
                                                                      rModel,
                                                                      rProperties );

       m_aRequest <<= aOptionsRequest;

       m_pAbort  = new ContinuationAbort;
       m_pOptions = new FilterOptionsContinuation;

       m_lContinuations.realloc( 2 );
       m_lContinuations[0] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pAbort  );
       m_lContinuations[1] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pOptions );
}

::com::sun::star::uno::Any SAL_CALL RequestFilterOptions::getRequest()
        throw( ::com::sun::star::uno::RuntimeException )
{
    return m_aRequest;
}

::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >
    SAL_CALL RequestFilterOptions::getContinuations()
        throw( ::com::sun::star::uno::RuntimeException )
{
    return m_lContinuations;
}



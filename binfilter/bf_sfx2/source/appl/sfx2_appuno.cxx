/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#pragma warning( disable : 4290 )
#endif

#include "appuno.hxx"

#include <bf_svtools/itempool.hxx>
#include <bf_svtools/rectitem.hxx>
#include <tools/urlobj.hxx>
#include <bf_basic/sbmeth.hxx>
#include <bf_basic/basmgr.hxx>
#include <bf_basic/sbxcore.hxx>
#include <bf_basic/sbx.hxx>
#include <bf_svtools/stritem.hxx>
#include <bf_svtools/intitem.hxx>
#include <bf_svtools/eitem.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#include <com/sun/star/document/BrokenPackageRequest.hpp>

using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::document;

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "app.hxx"
#include "sfxsids.hrc"
#include "objuno.hxx"
#include "fltoptint.hxx"
#include "brokenpackageint.hxx"
#include "eventsupplier.hxx"
#include "namecont.hxx"
#include "dlgcont.hxx"
#include "scriptcont.hxx"

namespace binfilter {

TYPEINIT1(SfxUsrAnyItem, SfxPoolItem);

/*N*/ SfxUsrAnyItem::SfxUsrAnyItem( sal_uInt16 nWhich, const ::com::sun::star::uno::Any& rAny )
/*N*/   : SfxPoolItem( nWhich )
/*N*/ {
/*N*/   aValue = rAny;
/*N*/ }

/*N*/ int SfxUsrAnyItem::operator==( const SfxPoolItem &rItem ) const
/*N*/ {
/*N*/ //   return rItem.ISA( SfxUsrAnyItem ) && ((SfxUsrAnyItem&)rItem).aValue == aValue;
/*N*/   return sal_False;
/*N*/ }


/*N*/ SfxPoolItem* SfxUsrAnyItem::Clone( SfxItemPool *) const
/*N*/ {
/*N*/     return new SfxUsrAnyItem( Which(), aValue );
/*N*/ }

#define PROTOCOLHANDLER_SERVICENAME     "com.sun.star.frame.ProtocolHandler"

static const String sTemplateRegionName   = String::CreateFromAscii( "TemplateRegionName"   );
static const String sTemplateName   = String::CreateFromAscii( "TemplateName"   );
static const String sAsTemplate     = String::CreateFromAscii( "AsTemplate"     );
static const String sOpenNewView    = String::CreateFromAscii( "OpenNewView"    );
static const String sViewId         = String::CreateFromAscii( "ViewId"         );
static const String sPluginMode     = String::CreateFromAscii( "PluginMode"     );
static const String sReadOnly       = String::CreateFromAscii( "ReadOnly"       );
static const String sStartPresentation = String::CreateFromAscii( "StartPresentation"       );
static const String sFrameName      = String::CreateFromAscii( "FrameName"      );
static const String sMediaType      = String::CreateFromAscii( "MediaType"    );
static const String sPostData       = String::CreateFromAscii( "PostData"       );
static const String sCharacterSet   = String::CreateFromAscii( "CharacterSet"   );
static const String sInputStream    = String::CreateFromAscii( "InputStream"    );
static const String sOutputStream   = String::CreateFromAscii( "OutputStream"    );
static const String sHidden         = String::CreateFromAscii( "Hidden"         );
static const String sPreview        = String::CreateFromAscii( "Preview"        );
static const String sViewOnly       = String::CreateFromAscii( "ViewOnly"       );
static const String sDontEdit       = String::CreateFromAscii( "DontEdit"       );
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
static const String sInteractionHdl = String::CreateFromAscii( "InteractionHandler" );
static const String sWindowState    = String::CreateFromAscii( "WindowState" );
static const String sUCBContent     = String::CreateFromAscii( "UCBContent" );
static const String sRepairPackage  = String::CreateFromAscii( "RepairPackage" );
static const String sDocumentTitle  = String::CreateFromAscii( "DocumentTitle" );
static const String sURL  = String::CreateFromAscii( "URL" );
static const String sFilterName  = String::CreateFromAscii( "FilterName" );
static const String sOpenFlags  = String::CreateFromAscii( "OpenFlags" );
static const String sPassword  = String::CreateFromAscii( "Password" );
static const String sReferer  = String::CreateFromAscii( "Referer" );
static const String sFilterOptions  = String::CreateFromAscii( "FilterOptions" );
static const String sVersionComment  = String::CreateFromAscii( "VersionComment" );
static const String sVersionAuthor  = String::CreateFromAscii( "VersionAuthor" );
static const String sVersion  = String::CreateFromAscii( "Version" );
static const String sOverwrite  = String::CreateFromAscii( "Overwrite" );
static const String sUnpacked  = String::CreateFromAscii( "Unpacked" );

/*N*/ void TransformParameters( sal_uInt16 nSlotId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs, SfxAllItemSet& rSet )
/*N*/ {
/*N*/     sal_Int32 nCount = rArgs.getLength();
/*N*/     const ::com::sun::star::beans::PropertyValue* pPropsVal = rArgs.getConstArray();
/*N*/     if ( nCount )
/*N*/     {
/*N*/         if ( nSlotId == SID_OPENDOC || nSlotId == SID_SAVEASDOC )
/*N*/         {
/*N*/             for ( sal_uInt16 n=0; n<nCount; n++ )
/*N*/             {
/*N*/                 const ::com::sun::star::beans::PropertyValue& rProp = pPropsVal[n];
/*N*/                 String aName = rProp.Name;
/*N*/                 if ( aName == sURL )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for FileName" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_FILE_NAME, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sFilterName )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for FilterName" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_FILTER_NAME, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sOpenFlags )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for OpenFlags" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_OPTIONS, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sPassword )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for Password" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_PASSWORD, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sReferer )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for Referer" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_REFERER, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sFilterOptions )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for FilterOptions" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sVersionComment )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for Comment" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_DOCINFO_COMMENTS, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sVersionAuthor )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for Author" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_DOCINFO_AUTHOR, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sVersion )
/*N*/                      {
/*N*/                         sal_Int16 nVal = -1;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
/*N*/                         DBG_ASSERT( bOK, "invalid type for Version" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxInt16Item( SID_VERSION, nVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sOverwrite )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for Overwrite" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_OVERWRITE, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sUnpacked )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for Unpacked" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_UNPACK, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sModel )
/*N*/                     rSet.Put( SfxUnoAnyItem( SID_DOCUMENT, rProp.Value ) );
/*N*/                 else if ( aName == sStatusInd )
/*N*/                      {
/*N*/                         Reference< ::com::sun::star::task::XStatusIndicator > xVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
/*N*/                         DBG_ASSERT( bOK, "invalid type for StatusIndicator" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUnoAnyItem( SID_PROGRESS_STATUSBAR_CONTROL, rProp.Value ) );
/*N*/                      }
/*N*/                 else if ( aName == sInteractionHdl )
/*N*/                      {
/*N*/                         Reference< ::com::sun::star::task::XInteractionHandler > xVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
/*N*/                         DBG_ASSERT( bOK, "invalid type for InteractionHandler" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUnoAnyItem( SID_INTERACTIONHANDLER, rProp.Value ) );
/*N*/                      }
/*N*/                 else if ( aName == sViewData )
/*N*/                     rSet.Put( SfxUnoAnyItem( SID_VIEW_DATA, rProp.Value ) );
/*N*/                 else if ( aName == sFilterData )
/*N*/ 					rSet.Put( SfxUnoAnyItem( SID_FILTER_DATA, rProp.Value ) );
/*N*/                 else if ( aName == sInputStream )
/*N*/                      {
/*N*/                         Reference< XInputStream > xVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
/*N*/                         DBG_ASSERT( bOK, "invalid type for InputStream" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUnoAnyItem( SID_INPUTSTREAM, rProp.Value ) );
/*N*/                      }
/*N*/                 else if ( aName == sUCBContent )
/*N*/                      {
/*N*/                         Reference< XContent > xVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
/*N*/                         DBG_ASSERT( bOK, "invalid type for UCBContent" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUnoAnyItem( SID_CONTENT, rProp.Value ) );
/*N*/                      }
/*N*/                 else if ( aName == sOutputStream )
/*N*/                      {
/*N*/                         Reference< XOutputStream > xVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= xVal) && xVal.is());
/*N*/                         DBG_ASSERT( bOK, "invalid type for OutputStream" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUnoAnyItem( SID_OUTPUTSTREAM, rProp.Value ) );
/*N*/                      }
/*N*/                 else if ( aName == sPostData )
/*N*/                      {
/*N*/                         Reference< XInputStream > xVal;
/*N*/                         sal_Bool bOK = (rProp.Value >>= xVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for PostData" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUnoAnyItem( SID_POSTDATA, rProp.Value ) );
/*N*/                      }
/*N*/                 else if ( aName == sAsTemplate )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for AsTemplate" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_TEMPLATE, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sOpenNewView )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for OpenNewView" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_OPEN_NEW_VIEW, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sViewId )
/*N*/                      {
/*N*/                         sal_Int16 nVal = -1;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
/*N*/                         DBG_ASSERT( bOK, "invalid type for ViewId" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUInt16Item( SID_VIEW_ID, nVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sPluginMode )
/*N*/                      {
/*N*/                         sal_Int16 nVal = -1;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
/*N*/                         DBG_ASSERT( bOK, "invalid type for PluginMode" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxUInt16Item( SID_PLUGIN_MODE, nVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sReadOnly )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for ReadOnly" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_DOC_READONLY, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sStartPresentation )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for StartPresentation" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_DOC_STARTPRESENTATION, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sSelectionOnly )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for SelectionOnly" );
/*N*/                         if (bOK)
/*N*/                            rSet.Put( SfxBoolItem( SID_SELECTION, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sHidden )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for Hidden" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_HIDDEN, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sMinimized )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for Minimized" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_MINIMIZEWINS, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sSilent )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for Silent" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_SILENT, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sPreview )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for Preview" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_PREVIEW, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sViewOnly )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for ViewOnly" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_VIEWONLY, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sDontEdit )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for ViewOnly" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxBoolItem( SID_EDITDOC, !bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sFileName )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for FileName" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_FILE_NAME, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sOrigURL )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for OrigURL" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_ORIGURL, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sSalvageURL )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = (rProp.Value >>= sVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for SalvageURL" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_DOC_SALVAGE, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sFrameName )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = (rProp.Value >>= sVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for FrameName" );
/*N*/                         if (bOK && sVal.getLength())
/*N*/                             rSet.Put( SfxStringItem( SID_TARGETNAME, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sMediaType )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for MediaType" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_CONTENTTYPE, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sWindowState )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for WindowState" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_WIN_POSSIZE, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sTemplateName )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for TemplateName" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_TEMPLATE_NAME, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sTemplateRegionName )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for TemplateRegionName" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_TEMPLATE_REGIONNAME, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sJumpMark )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for JumpMark" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_JUMPMARK, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sCharacterSet )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for CharacterSet" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_CHARSET, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sFilterFlags )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for FilterFlags" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_FILE_FILTEROPTIONS, sVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sMacroExecMode )
/*N*/                 {
/*N*/                     sal_Int16 nVal =-1;
/*N*/                     sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
/*N*/                     DBG_ASSERT( bOK, "invalid type for MacroExecMode" );
/*N*/                     if (bOK)
/*N*/                         rSet.Put( SfxUInt16Item( SID_MACROEXECMODE, nVal ) );
/*N*/                 }
/*N*/                 else if ( aName == sUpdateDocMode )
/*N*/ 				{
/*N*/                     sal_Int16 nVal =-1;
/*N*/                     sal_Bool bOK = ((rProp.Value >>= nVal) && (nVal != -1));
/*N*/                     DBG_ASSERT( bOK, "invalid type for UpdateDocMode" );
/*N*/                     if (bOK)
/*N*/                         rSet.Put( SfxUInt16Item( SID_UPDATEDOCMODE, nVal ) );
/*N*/ 				}
/*N*/                 else if ( aName == sRepairPackage )
/*N*/                      {
/*N*/                         sal_Bool bVal = sal_False;
/*N*/                         sal_Bool bOK = (rProp.Value >>= bVal);
/*N*/                         DBG_ASSERT( bOK, "invalid type for RepairPackage" );
/*N*/                         if (bOK)
/*N*/                            rSet.Put( SfxBoolItem( SID_REPAIRPACKAGE, bVal ) );
/*N*/                      }
/*N*/                 else if ( aName == sDocumentTitle )
/*N*/                      {
/*N*/                         ::rtl::OUString sVal;
/*N*/                         sal_Bool bOK = ((rProp.Value >>= sVal) && sVal.getLength());
/*N*/                         DBG_ASSERT( bOK, "invalid type or value for DocumentTitle" );
/*N*/                         if (bOK)
/*N*/                             rSet.Put( SfxStringItem( SID_DOCINFO_TITLE, sVal ) );
/*N*/                      }
/*N*/ 
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ }


/*N*/ void TransformItems( sal_uInt16 nSlotId, const SfxItemSet& rSet, ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rArgs )
/*N*/ {
/*N*/     // find number of properties to avoid permanent reallocations in the sequence
/*N*/     sal_Int32 nProps=0;
/*N*/ 
/*N*/         // special treatment for slots that are *not* meant to be recorded as slots (except SaveAs/To)
/*N*/         if ( nSlotId == SID_OPENDOC || nSlotId == SID_SAVEASDOC )
/*N*/         {
/*N*/             sal_Int32 nAdditional=0;
/*N*/             if ( rSet.GetItemState( SID_FILTER_NAME ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_OPTIONS ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_PASSWORD ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_REFERER ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_FILE_FILTEROPTIONS ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_DOCINFO_COMMENTS ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_DOCINFO_AUTHOR ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_VERSION ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_OVERWRITE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_UNPACK ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_FILE_NAME ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_PROGRESS_STATUSBAR_CONTROL ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_INTERACTIONHANDLER ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_ORIGURL ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_DOC_SALVAGE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_CONTENT ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_INPUTSTREAM ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_OUTPUTSTREAM ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_TEMPLATE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_OPEN_NEW_VIEW ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_VIEW_ID ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_VIEW_DATA ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_FILTER_DATA ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_PLUGIN_MODE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_DOC_READONLY ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_DOC_STARTPRESENTATION ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_SELECTION ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_CONTENTTYPE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_WIN_POSSIZE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_POSTDATA ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_CHARSET ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_TARGETNAME ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_TEMPLATE_NAME ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_TEMPLATE_REGIONNAME ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_HIDDEN ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_MINIMIZEWINS ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_PREVIEW ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_VIEWONLY ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_EDITDOC ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_SILENT ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_JUMPMARK ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_DOCUMENT ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_MACROEXECMODE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_UPDATEDOCMODE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_REPAIRPACKAGE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/             if ( rSet.GetItemState( SID_DOCINFO_TITLE ) == SFX_ITEM_SET )
/*N*/                 nAdditional++;
/*N*/ 
/*N*/             // consider additional arguments
/*N*/             nProps += nAdditional;
/*N*/         }
/*N*/ 
/*N*/     if ( !nProps )
/*N*/         return;
/*N*/ 
/*N*/     // convert every item into a property
/*N*/     ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> aSequ( nProps );
/*N*/     ::com::sun::star::beans::PropertyValue *pValue = aSequ.getArray();
/*N*/     nProps = 0;
/*N*/ 
/*N*/         if ( nSlotId == SID_OPENDOC || nSlotId == SID_SAVEASDOC )
/*N*/         {
/*N*/             const SfxPoolItem *pItem=0;

/*N*/             if ( rSet.GetItemState( SID_FILE_NAME, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sURL;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
                  }
/*N*/             if ( rSet.GetItemState( SID_FILTER_NAME, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sFilterName;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_OPTIONS, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sOpenFlags;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_PASSWORD, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sPassword;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_REFERER, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sReferer;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_FILE_FILTEROPTIONS, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sFilterOptions;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_DOCINFO_COMMENTS, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sVersionComment;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_DOCINFO_AUTHOR, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sVersionAuthor;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_VERSION, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sVersion;
/*N*/                 pValue[nProps++].Value <<= (sal_Int16)((SfxInt16Item*)pItem)->GetValue();
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_OVERWRITE, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sOverwrite;
/*N*/                 pValue[nProps++].Value <<= (sal_Bool) ((SfxBoolItem*)pItem)->GetValue();
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_UNPACK, sal_False, &pItem  ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sUnpacked;
/*N*/                 pValue[nProps++].Value <<= (sal_Bool) ((SfxBoolItem*)pItem)->GetValue();
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_PROGRESS_STATUSBAR_CONTROL, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sStatusInd;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_INTERACTIONHANDLER, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sInteractionHdl;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_VIEW_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sViewData;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_FILTER_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sFilterData;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_DOCUMENT, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sModel;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_CONTENT, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sUCBContent;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_INPUTSTREAM, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sInputStream;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_OUTPUTSTREAM, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sOutputStream;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_POSTDATA, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sPostData;
/*N*/                 pValue[nProps++].Value = ( ((SfxUnoAnyItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_TEMPLATE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sAsTemplate;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_OPEN_NEW_VIEW, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sOpenNewView;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_VIEW_ID, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sViewId;
/*N*/                 pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_PLUGIN_MODE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sPluginMode;
/*N*/                 pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_DOC_READONLY, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sReadOnly;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_DOC_STARTPRESENTATION, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sStartPresentation;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_SELECTION, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sSelectionOnly;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_HIDDEN, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sHidden;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_MINIMIZEWINS, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sMinimized;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_SILENT, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sSilent;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_PREVIEW, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sPreview;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_VIEWONLY, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sViewOnly;
/*N*/                 pValue[nProps++].Value <<= (sal_Bool) (( ((SfxBoolItem*)pItem)->GetValue() ));
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_EDITDOC, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sDontEdit;
/*N*/                 pValue[nProps++].Value <<= (sal_Bool) (!( ((SfxBoolItem*)pItem)->GetValue() ));
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_TARGETNAME, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sFrameName;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_ORIGURL, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sOrigURL;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_DOC_SALVAGE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sSalvageURL;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_CONTENTTYPE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sMediaType;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_WIN_POSSIZE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sWindowState;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_TEMPLATE_NAME, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sTemplateName;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_TEMPLATE_REGIONNAME, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sTemplateRegionName;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_JUMPMARK, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sJumpMark;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
/*N*/             }
/*N*/ 
/*N*/             if ( rSet.GetItemState( SID_CHARSET, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sCharacterSet;
/*N*/                 pValue[nProps++].Value <<= (  ::rtl::OUString(((SfxStringItem*)pItem)->GetValue())  );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_MACROEXECMODE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sMacroExecMode;
/*N*/                 pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_UPDATEDOCMODE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sUpdateDocMode;
/*N*/                 pValue[nProps++].Value <<= ( (sal_Int16) ((SfxUInt16Item*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_REPAIRPACKAGE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sRepairPackage;
/*N*/                 pValue[nProps++].Value <<= ( ((SfxBoolItem*)pItem)->GetValue() );
/*N*/             }
/*N*/             if ( rSet.GetItemState( SID_DOCINFO_TITLE, sal_False, &pItem ) == SFX_ITEM_SET )
/*N*/             {
/*N*/                 pValue[nProps].Name = sDocumentTitle;
/*N*/                 pValue[nProps++].Value <<= ( ::rtl::OUString(((SfxStringItem*)pItem)->GetValue()) );
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/     rArgs = aSequ;
/*N*/ }

// -----------------------------------------------------------------------

extern "C" {

/*N*/ 
/*N*/ sal_Bool SAL_CALL sfx2_component_writeInfo(	void*	pServiceManager	,
/*N*/ 										void*	pRegistryKey	)
/*N*/ {
/*N*/ 	::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >		xKey( reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( pRegistryKey ) )	;
/*N*/ 
/*N*/     // Eigentliche Implementierung und ihre Services registrieren
/*N*/     ::rtl::OUString aImpl;
/*N*/     ::rtl::OUString aTempStr;
/*N*/     ::rtl::OUString aKeyStr;
/*N*/     Reference< XRegistryKey > xNewKey;
/*N*/     Reference< XRegistryKey > xLoaderKey;
/*N*/ 
/*N*/     // global app event broadcaster
/*N*/     aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
/*N*/     aImpl += SfxGlobalEvents_Impl::impl_getStaticImplementationName();
/*N*/ 
/*N*/     aTempStr = aImpl;
/*N*/     aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
/*N*/     xNewKey = xKey->createKey( aTempStr );
/*N*/     xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.frame.GlobalEventBroadcaster") );
/*N*/ 
/*N*/     // standalone document info
/*N*/     aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
/*N*/     aImpl += SfxStandaloneDocumentInfoObject::impl_getStaticImplementationName();
/*N*/ 
/*N*/     aTempStr = aImpl;
/*N*/     aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
/*N*/     xNewKey = xKey->createKey( aTempStr );
/*N*/     xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.document.StandaloneDocumentInfo") );
/*N*/ 
/*N*/ 	// script library container service
/*N*/     aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
/*N*/     aImpl += SfxScriptLibraryContainer::impl_getStaticImplementationName();
/*N*/ 
/*N*/     aTempStr = aImpl;
/*N*/     aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
/*N*/     xNewKey = xKey->createKey( aTempStr );
/*N*/     xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.script.ScriptLibraryContainer") );
/*N*/ 
/*N*/ 	// dialog library container service
/*N*/     aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
/*N*/     aImpl += SfxDialogLibraryContainer::impl_getStaticImplementationName();
/*N*/ 
/*N*/     aTempStr = aImpl;
/*N*/     aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
/*N*/     xNewKey = xKey->createKey( aTempStr );
/*N*/     xNewKey->createKey( ::rtl::OUString::createFromAscii("com.sun.star.script.DialogLibraryContainer") );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }
/*N*/ 
/*N*/ void* SAL_CALL sfx2_component_getFactory(	const	sal_Char*	pImplementationName	,
/*N*/ 												void*		pServiceManager		,
/*N*/ 												void*		pRegistryKey		)
/*N*/ {
/*N*/ 	// Set default return value for this operation - if it failed.
/*N*/ 	void* pReturn = NULL ;
/*N*/ 
/*N*/ 	if	(
/*N*/ 			( pImplementationName	!=	NULL ) &&
/*N*/ 			( pServiceManager		!=	NULL )
/*N*/ 		)
/*N*/ 	{
/*N*/ 		// Define variables which are used in following macros.
/*N*/ 		::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >	xFactory																								;
/*N*/ 		::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	xServiceManager( reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ) )	;

        //=============================================================================
        //  Add new macro line to handle new service.
        //
        //	!!! ATTENTION !!!
        //		Write no ";" at end of line and dont forget "else" ! (see macro)
        //=============================================================================
/*N*/         IF_NAME_CREATECOMPONENTFACTORY( SfxGlobalEvents_Impl )
/*N*/         IF_NAME_CREATECOMPONENTFACTORY( SfxStandaloneDocumentInfoObject )
/*N*/ 		IF_NAME_CREATECOMPONENTFACTORY( SfxScriptLibraryContainer )
/*N*/ 		IF_NAME_CREATECOMPONENTFACTORY( SfxDialogLibraryContainer )
/*N*/         // Factory is valid - service was found.
/*N*/ 		if ( xFactory.is() )
/*N*/ 		{
/*N*/ 			xFactory->acquire();
/*N*/ 			pReturn = xFactory.get();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Return with result of this operation.
/*N*/ 	return pReturn ;
/*N*/ }
} // extern "C"

//=========================================================================

/*N*/ void SAL_CALL FilterOptionsContinuation::setFilterOptions(
/*N*/ 				const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProps )
/*N*/ 		throw (::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	rProperties = rProps;
/*N*/ }
/*N*/ 
/*N*/ ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
/*N*/ 	FilterOptionsContinuation::getFilterOptions()
/*N*/ 		throw (::com::sun::star::uno::RuntimeException)
/*N*/ {
/*N*/ 	return rProperties;
/*N*/ }
/*N*/ 
/*N*/ //=========================================================================
/*N*/ 
/*N*/ RequestFilterOptions::RequestFilterOptions( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > rModel,
/*N*/ 							  ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rProperties )
/*N*/ {
/*N*/ 	::rtl::OUString temp;
/*N*/ 	::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > temp2;
/*N*/ 	::com::sun::star::document::FilterOptionsRequest aOptionsRequest( temp,
/*N*/                                                        				  temp2,
/*N*/ 																	  rModel,
/*N*/ 																	  rProperties );
/*N*/ 
/*N*/    	m_aRequest <<= aOptionsRequest;
/*N*/ 
/*N*/    	m_pAbort  = new ContinuationAbort;
/*N*/    	m_pOptions = new FilterOptionsContinuation;
/*N*/ 
/*N*/    	m_lContinuations.realloc( 2 );
/*N*/    	m_lContinuations[0] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pAbort  );
/*N*/    	m_lContinuations[1] = ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >( m_pOptions );
/*N*/ }
/*N*/ 
/*N*/ ::com::sun::star::uno::Any SAL_CALL RequestFilterOptions::getRequest()
/*N*/ 		throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	return m_aRequest;
/*N*/ }
/*N*/ 
/*N*/ ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >
/*N*/ 	SAL_CALL RequestFilterOptions::getContinuations()
/*N*/ 		throw( ::com::sun::star::uno::RuntimeException )
/*N*/ {
/*N*/ 	return m_lContinuations;
/*N*/ }
}

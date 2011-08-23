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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_XSYSTEMSHELLEXECUTE_HPP_
#include <com/sun/star/system/XSystemShellExecute.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XTYPEDETECTION_HPP_
#include <com/sun/star/document/XTypeDetection.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_SYSTEMSHELLEXECUTEFLAGS_HPP_
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_MACROEXECMODE_HPP_
#include <com/sun/star/document/MacroExecMode.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_UPDATEDOCMODE_HPP_
#include <com/sun/star/document/UpdateDocMode.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _SFXINTITEM_HXX //autogen
#include <bf_svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <bf_svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <bf_svtools/eitem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <bf_so3/svstor.hxx>
#endif
#ifndef _SFXDOCTEMPL_HXX //autogen
#include <doctempl.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <bf_svtools/sfxecode.hxx>
#endif

#include <bf_svtools/ehdl.hxx>
#include "bf_basic/sbxobj.hxx"
#include <bf_svtools/urihelper.hxx>
#include <unotools/localfilehelper.hxx>

#include <bf_svtools/itemset.hxx>

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <bf_svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <bf_svtools/moduleoptions.hxx>
#endif
#ifndef _SVTOOLS_TEMPLDLG_HXX
#include <bf_svtools/templdlg.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#include <bf_svtools/extendedsecurityoptions.hxx>
#include <bf_svtools/docpasswdrequest.hxx>

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "app.hxx"
#include "appdata.hxx"
#include "cfgmgr.hxx"
#include "docfile.hxx"
#include "docinf.hxx"
#include "fcontnr.hxx"
#include "objitem.hxx"
#include "objsh.hxx"
#include "objshimp.hxx"
#include "openflag.hxx"
#include "referers.hxx"
#include "request.hxx"
#include "appimp.hxx"
#include "appuno.hxx"
#include "docfac.hxx"
#include "sfxsids.hrc"

#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>

#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
#endif
namespace binfilter {

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::system;
using namespace ::com::sun::star::task;
using namespace ::cppu;
//using namespace sfx2;

//--------------------------------------------------------------------

/*N*/ sal_uInt32 CheckPasswd_Impl
/*N*/ (
/*N*/     //Window *pWin,       // Parent des Dialogs
/*N*/     SfxObjectShell* pDoc,
/*N*/ 	SfxItemPool &rPool,	// Pool, falls ein Set erzeugt werden mus
/*N*/ 	SfxMedium *pFile	// das Medium, dessen Passwort gfs. erfragt werden soll
/*N*/ )

/*  [Beschreibung]

    Zu einem Medium das Passwort erfragen; funktioniert nur, wenn es sich
    um einen Storage handelt.
    Wenn in der Documentinfo das Passwort-Flag gesetzt ist, wird
    das Passwort vom Benutzer per Dialog erfragt und an dem Set
    des Mediums gesetzt; das Set wird, wenn nicht vorhanden, erzeugt.

*/

/*N*/ {
/*N*/     ULONG nRet=0;
/*N*/     if( ( !pFile->GetFilter() || pFile->GetFilter()->UsesStorage() ) )
/*N*/     {
/*N*/         SvStorageRef aRef = pFile->GetStorage();
/*N*/         if(aRef.Is())
/*N*/         {
/*N*/             sal_Bool bIsEncrypted = sal_False;
/*N*/             ::com::sun::star::uno::Any aAny;
/*N*/             if ( aRef->GetProperty( ::rtl::OUString::createFromAscii("HasEncryptedEntries"), aAny ) )
/*N*/                 aAny >>= bIsEncrypted;
/*N*/             else
/*N*/             {
/*N*/                 SfxDocumentInfo aInfo;
/*N*/                 bIsEncrypted = ( aInfo.Load(aRef) && aInfo.IsPasswd() );
/*N*/             }
/*N*/
/*N*/             if ( bIsEncrypted )
/*N*/             {
                nRet = ERRCODE_SFX_CANTGETPASSWD;
                SfxItemSet *pSet = pFile->GetItemSet();
                if( pSet )
                {
                    Reference< ::com::sun::star::task::XInteractionHandler > xInteractionHandler;

                    SFX_ITEMSET_ARG( pSet, pxInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False );
                    if( pxInteractionItem && ( pxInteractionItem->GetValue() >>= xInteractionHandler )
                     && xInteractionHandler.is() )
                    {
                        RequestDocumentPassword* pPasswordRequest = new RequestDocumentPassword(
                            ::com::sun::star::task::PasswordRequestMode_PASSWORD_ENTER,
                            INetURLObject( pFile->GetOrigURL() ).GetName( INetURLObject::DECODE_WITH_CHARSET ) );

                        Reference< XInteractionRequest > rRequest( pPasswordRequest );
                        xInteractionHandler->handle( rRequest );

                        if ( pPasswordRequest->isPassword() )
                        {
                            pSet->Put( SfxStringItem( SID_PASSWORD, pPasswordRequest->getPassword() ) );
                            nRet = ERRCODE_NONE;
                        }
                        else
                            nRet = ERRCODE_IO_ABORT;
                    }
                }
/*?*/             }
/*N*/         }
/*N*/     }
/*N*/     return nRet;
/*N*/ }


}

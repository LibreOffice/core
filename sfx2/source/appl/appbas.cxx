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

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/uno/Reference.h>
#include <basic/basrdll.hxx>
#include <officecfg/Office/Common.hxx>
#include <svl/macitem.hxx>
#include <basic/sbxfac.hxx>
#include <basic/sbx.hxx>
#include <vcl/gradient.hxx>
#include <svl/rectitem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <basic/sbmod.hxx>
#include <svl/whiter.hxx>
#include <basic/sbmeth.hxx>
#include <basic/sbstar.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <basic/sbuno.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/bootstrap.hxx>

#include <sfx2/appuno.hxx>
#include <sfx2/module.hxx>
#include "arrdecl.hxx"
#include <sfx2/app.hxx>
#include "sfxtypes.hxx"
#include "sfx2/sfxresid.hxx"
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include "sfx2/tplpitem.hxx"
#include "sfx2/minfitem.hxx"
#include "app.hrc"
#include <sfx2/evntconf.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dinfdlg.hxx>
#include "appdata.hxx"
#include "appbas.hxx"
#include "sfx2/sfxhelp.hxx"
#include "sfx2/basmgr.hxx"
#include "sorgitm.hxx"
#include "appbaslib.hxx"
#include <basic/basicmanagerrepository.hxx>

#define ITEMID_SEARCH SID_SEARCH_ITEM

#include <svl/srchitem.hxx>
#include <osl/socket.hxx>

#if defined(LIBO_MERGELIBS) || defined(DISABLE_DYNLOADING)
/* Avoid clash with the ones from svx/source/form/typemap.cxx */
#define aSfxBoolItem_Impl sfx2_source_appl_appbas_aSfxBoolItem_Impl
#ifdef DISABLE_DYNLOADING
#define aSfxInt16Item_Impl sfx2_source_appl_appbas_aSfxInt16Item_Impl
#define aSfxUnoFrameItem_Impl sfx2_source_appl_appbas_aSfxUnoFrameItem_Impl
#endif
#define aSfxStringItem_Impl sfx2_source_appl_appbas_aSfxStringItem_Impl
#define aSfxUInt16Item_Impl sfx2_source_appl_appbas_aSfxUInt16Item_Impl
#define aSfxUInt32Item_Impl sfx2_source_appl_appbas_aSfxUInt32Item_Impl
#define aSfxVoidItem_Impl sfx2_source_appl_appbas_aSfxVoidtem_Impl
#endif

#define SFX_TYPEMAP
#define Selection
#include "sfxslots.hxx"

#if defined(LIBO_MERGELIBS) || defined(DISABLE_DYNLOADING)
#undef aSfxBoolItem_Impl
#ifdef DISABLE_DYNLOADING
#undef aSfxInt16Item_Impl
#undef aSfxUnoFrameItem_Impl
#endif
#undef aSfxStringItem_Impl
#undef aSfxUInt16Item_Impl
#undef aSfxUInt32Item_Impl
#undef aSfxVoidItem_Impl
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;

using ::basic::BasicManagerRepository;

//=========================================================================
sal_uInt16 SfxApplication::SaveBasicAndDialogContainer() const
{
    if ( pAppData_Impl->pBasicManager->isValid() )
        pAppData_Impl->pBasicManager->storeAllLibraries();
    return 0;
}

//--------------------------------------------------------------------

SbxVariable* MakeVariable( StarBASIC *pBas, SbxObject *pObject,
           const char *pName, sal_uInt32 nSID, SbxDataType eType, SbxClassType eClassType )
{
#ifdef DISABLE_SCRIPTING
    (void) pBas;
    (void) pObject;
    (void) pName;
    (void) nSID;
    (void) eType;
    (void) eClassType;
    return 0;
#else
    SbxVariable *pVar = pBas->Make( rtl::OUString::createFromAscii(pName), eClassType, eType ); //SbxCLASS_PROPERTY
    pVar->SetUserData( nSID );
    pVar->SetFlag( SBX_DONTSTORE );
    pObject->StartListening( pVar->GetBroadcaster() );
    return pVar;
#endif
}

//--------------------------------------------------------------------

BasicManager* SfxApplication::GetBasicManager()
{
#ifdef DISABLE_SCRIPTING
    return 0;
#else
    return BasicManagerRepository::getApplicationBasicManager( true );
#endif
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxApplication::GetDialogContainer()
{
#ifdef DISABLE_SCRIPTING
    Reference< XLibraryContainer >  dummy;
    return dummy;
#else
    if ( !pAppData_Impl->pBasicManager->isValid() )
        GetBasicManager();
    return pAppData_Impl->pBasicManager->getLibraryContainer( SfxBasicManagerHolder::DIALOGS );
#endif
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxApplication::GetBasicContainer()
{
#ifdef DISABLE_SCRIPTING
    Reference< XLibraryContainer >  dummy;
    return dummy;
#else
    if ( !pAppData_Impl->pBasicManager->isValid() )
        GetBasicManager();
    return pAppData_Impl->pBasicManager->getLibraryContainer( SfxBasicManagerHolder::SCRIPTS );
#endif
}

//--------------------------------------------------------------------

StarBASIC* SfxApplication::GetBasic()
{
#ifdef DISABLE_SCRIPTING
    return 0;
#else
    return GetBasicManager()->GetLib(0);
#endif
}

//-------------------------------------------------------------------------
void SfxApplication::PropExec_Impl( SfxRequest &rReq )
{
#ifdef DISABLE_SCRIPTING
    (void) rReq;
#else
    rReq.GetArgs();
    sal_uInt16 nSID = rReq.GetSlot();
    switch ( nSID )
    {
        case SID_CREATE_BASICOBJECT:
        {
            SFX_REQUEST_ARG(rReq, pItem, SfxStringItem, nSID, sal_False);
            if ( pItem )
            {
                SbxObject* pObject = SbxBase::CreateObject( pItem->GetValue() );
                pObject->AddRef();
                rReq.Done();
            }
            break;
        }

        case SID_DELETE_BASICOBJECT:
        {
            break;
        }

        case SID_ATTR_UNDO_COUNT:
        {
            SFX_REQUEST_ARG(rReq, pCountItem, SfxUInt16Item, nSID, sal_False);
            boost::shared_ptr< comphelper::ConfigurationChanges > batch(
                comphelper::ConfigurationChanges::create());
            officecfg::Office::Common::Undo::Steps::set(
                pCountItem->GetValue(), batch);
            batch->commit();
            break;
        }

        case SID_WIN_VISIBLE:
        {
            break;
        }

        case SID_OFFICE_CUSTOMERNUMBER:
        {
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, nSID, sal_False);

            if ( pStringItem )
                SvtUserOptions().SetCustomerNumber( pStringItem->GetValue() );
            break;
        }
    }
#endif
}

//-------------------------------------------------------------------------
void SfxApplication::PropState_Impl( SfxItemSet &rSet )
{
#ifdef DISABLE_SCRIPTING
    (void) rSet;
#else
    SfxWhichIter aIter(rSet);
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
            case SID_PROGNAME:
                rSet.Put( SfxStringItem( SID_PROGNAME, GetName() ) );
                break;

            case SID_ACTIVEDOCUMENT:
                rSet.Put( SfxObjectItem( SID_ACTIVEDOCUMENT, SfxObjectShell::Current() ) );
                break;

            case SID_APPLICATION:
                rSet.Put( SfxObjectItem( SID_APPLICATION, this ) );
                break;

            case SID_PROGFILENAME:
                rSet.Put( SfxStringItem( SID_PROGFILENAME, Application::GetAppFileName() ) );
                break;

            case SID_ATTR_UNDO_COUNT:
                rSet.Put(
                    SfxUInt16Item(
                        SID_ATTR_UNDO_COUNT,
                        officecfg::Office::Common::Undo::Steps::get()));
                break;

            case SID_UPDATE_VERSION:
                rSet.Put( SfxUInt32Item( SID_UPDATE_VERSION, SUPD ) );
                break;

            case SID_OFFICE_CUSTOMERNUMBER:
            {
                rSet.Put( SfxStringItem( nSID, SvtUserOptions().GetCustomerNumber() ) );
                break;
            }
        }
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

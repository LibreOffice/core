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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.h>
#include <basic/basrdll.hxx>
#include <tools/urlobj.hxx>
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

#include <unotools/undoopt.hxx>
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
#include <vos/socket.hxx>

#define SFX_TYPEMAP
#define Selection
#include "sfxslots.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::script;

using ::basic::BasicManagerRepository;

//========================================================================

//------------------------------------------------------------------------
String lcl_GetVersionString(ResMgr* /*pAppData_ImplResMgr*/)
{
    ::rtl::OUString aDefault;
    String aVersion( utl::Bootstrap::getBuildIdData( aDefault ));

    if ( aVersion.Len() == 0 )
    {
        DBG_ERROR( "No BUILDID in bootstrap file found" );
    }

    aVersion.Erase( 0, aVersion.Search( ':' ) + 1 );
    aVersion.Erase( aVersion.Search( ')' ) );
    return aVersion;
}

//=========================================================================
sal_uInt16 SfxApplication::SaveBasicManager() const
{
    return 0;
}

//--------------------------------------------------------------------
sal_uInt16 SfxApplication::SaveBasicAndDialogContainer() const
{
    if ( pAppData_Impl->pBasicManager->isValid() )
        pAppData_Impl->pBasicManager->storeAllLibraries();
    return 0;
}

//--------------------------------------------------------------------

void SfxApplication::RegisterBasicConstants
(
    const char*,        // Prefix vor Konstanten-Namen
    const SfxConstant*, // Array von <SfxConstant> Instanzen
    sal_uInt16          // Anahl der Kontanten in pConsts
)

/*  [Beschreibung]

    Diese Methode meldet Konstanten beim BASIC an. Sie sollte on-demand
    (in GetSbxObject() der Applikation) gerufen werden. Das Array mu\s
    alphabetisch nach den Namen sortiert sein!

    Durch den Prefix kann Speicher gespart und das Suchen beschleunigt
    werden. Im StarOffice soll der Prefix "so" verwendet werden.


    [Beispiel]

    const SfxConstant __FAR_DATA aConstants[] =
    {
        SFX_BOOL_CONSTANT( "False", sal_False ),
        SFX_BOOL_CONSTANT( "True",  sal_True ),
    };

    ...
    SFX_APP()->RegisterBasicConstants( 0, aConstants, 2 );
    ...

*/

{
//  DBG_ASSERT( pAppData_Impl->pBasicMgr, "no basic available" );

//  pAppData_Impl->pBasicMgr->GetLib(0)->Insert(
//              new SfxConstants_Impl( pPrefix, pConsts, nCount ) );
}

//--------------------------------------------------------------------

SbxVariable* MakeVariable( StarBASIC *pBas, SbxObject *pObject,
           const char *pName, sal_uInt32 nSID, SbxDataType eType, SbxClassType eClassType )
{
    SbxVariable *pVar = pBas->Make( String::CreateFromAscii(pName), eClassType, eType ); //SbxCLASS_PROPERTY
    pVar->SetUserData( nSID );
    pVar->SetFlag( SBX_DONTSTORE );
    pObject->StartListening( pVar->GetBroadcaster() );
    return pVar;
}

//--------------------------------------------------------------------

BasicManager* SfxApplication::GetBasicManager()
{
    return BasicManagerRepository::getApplicationBasicManager( true );
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxApplication::GetDialogContainer()
{
    if ( !pAppData_Impl->pBasicManager->isValid() )
        GetBasicManager();
    return pAppData_Impl->pBasicManager->getLibraryContainer( SfxBasicManagerHolder::DIALOGS );
}

//--------------------------------------------------------------------

Reference< XLibraryContainer > SfxApplication::GetBasicContainer()
{
    if ( !pAppData_Impl->pBasicManager->isValid() )
        GetBasicManager();
    return pAppData_Impl->pBasicManager->getLibraryContainer( SfxBasicManagerHolder::SCRIPTS );
}

//--------------------------------------------------------------------

StarBASIC* SfxApplication::GetBasic()
{
    return GetBasicManager()->GetLib(0);
}

//-------------------------------------------------------------------------
void SfxApplication::PropExec_Impl( SfxRequest &rReq )
{
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
//(mba)                rReq.SetReturnValue( SfxObjectItem( 0, pObject ) );
                rReq.Done();
            }
            break;
        }

        case SID_DELETE_BASICOBJECT:
        {
            SFX_REQUEST_ARG(rReq, pItem, SfxObjectItem, nSID, sal_False);
            if ( pItem )
            {
//(mba)                SbxObject* pObject = pItem->GetObject();
//(mba)                pObject->ReleaseRef();
            }
            break;
        }

        case SID_ATTR_UNDO_COUNT:
        {
            SFX_REQUEST_ARG(rReq, pCountItem, SfxUInt16Item, nSID, sal_False);
            SvtUndoOptions().SetUndoCount( pCountItem->GetValue() );
            break;
        }

        case SID_WIN_VISIBLE:
        {
            break;
        }

        case SID_STATUSBARTEXT:
        {
            SFX_REQUEST_ARG(rReq, pStringItem, SfxStringItem, nSID, sal_False);
            String aText = pStringItem->GetValue();
            if ( aText.Len() )
                GetpApp()->ShowStatusText( aText );
            else
                GetpApp()->HideStatusText();
            break;
        }

        case SID_OFFICE_PRIVATE_USE:
        case SID_OFFICE_COMMERCIAL_USE:
        {
            DBG_ASSERT( sal_False, "SfxApplication::PropExec_Impl()\nSID_OFFICE_PRIVATE_USE & SID_OFFICE_COMMERCIAL_USE are obsolete!\n" );
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
}

//-------------------------------------------------------------------------
void SfxApplication::PropState_Impl( SfxItemSet &rSet )
{
//  SfxViewFrame *pFrame = SfxViewFrame::Current();
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
                rSet.Put( SfxUInt16Item( SID_ATTR_UNDO_COUNT, sal::static_int_cast< sal_uInt16 >( SvtUndoOptions().GetUndoCount() ) ) );
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
}


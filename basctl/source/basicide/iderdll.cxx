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
#include "precompiled_basctl.hxx"


#include <ide_pch.hxx>


#include <svheader.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/genlink.hxx>


#include <svl/solar.hrc>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <svx/svxids.hrc>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <basobj.hxx>
#include <bastypes.hxx>
#include <basdoc.hxx>
#include <basicmod.hxx>
#include <propbrw.hxx>


#define ITEMID_SEARCH   0
#include <svl/srchitem.hxx>
#include <com/sun/star/script/XLibraryContainerPassword.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;


static BasicIDEDLL* pBasicIDEDLL = 0;

BasicIDEDLL* BasicIDEDLL::GetDLL()
{
    return pBasicIDEDLL;
}

IDEResId::IDEResId( sal_uInt16 nId ):
    ResId( nId, *(*(BasicIDEModule**)GetAppData(SHL_IDE))->GetResMgr() )
{
}

BasicIDEDLL::BasicIDEDLL()
{
    pBasicIDEDLL = this;
    pShell = 0;
    pExtraData = 0;

    GetExtraData(); // damit GlobalErrorHdl gesetzt wird.
}

BasicIDEDLL::~BasicIDEDLL()
{
    delete pExtraData;
    *(BasicIDEDLL**)GetAppData(SHL_IDE) = NULL;
}

void BasicIDEDLL::Init()
{
    if ( pBasicIDEDLL )
        return;

    SfxObjectFactory* pFact = &BasicDocShell::Factory();
    (void)pFact;

    ByteString aResMgrName( "basctl" );
    ResMgr* pMgr = ResMgr::CreateResMgr(
        aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );

    BASIC_MOD() = new BasicIDEModule( pMgr, &BasicDocShell::Factory() );

    new BasicIDEDLL;
    SfxModule* pMod = BASIC_MOD();

    SfxObjectFactory& rFactory = BasicDocShell::Factory();
    rFactory.SetDocumentServiceName( String::CreateFromAscii( "com.sun.star.script.BasicIDE" ) );

    BasicDocShell::RegisterInterface( pMod );
    BasicIDEShell::RegisterFactory( SVX_INTERFACE_BASIDE_VIEWSH );
    BasicIDEShell::RegisterInterface( pMod );

    PropBrwMgr::RegisterChildWindow();
}

/*************************************************************************
|*
|* Deinitialisierung
|*
\************************************************************************/
void BasicIDEDLL::Exit()
{
    // the BasicIDEModule must be destroyed
    BasicIDEModule** ppShlPtr = (BasicIDEModule**) GetAppData(SHL_IDE);
    delete (*ppShlPtr);
    (*ppShlPtr) = NULL;
    DELETEZ( pBasicIDEDLL );
}

BasicIDEData* BasicIDEDLL::GetExtraData()
{
    if ( !pExtraData )
        pExtraData = new BasicIDEData;
     return pExtraData;
}

BasicIDEData::BasicIDEData() : aObjCatPos( INVPOSITION, INVPOSITION )
{
    nBasicDialogCount = 0;
    bChoosingMacro = sal_False;
    bShellInCriticalSection = sal_False;
    pSearchItem = new SvxSearchItem( SID_SEARCH_ITEM );

    StarBASIC::SetGlobalBreakHdl( LINK( this, BasicIDEData, GlobalBasicBreakHdl ) );

    pAccelerator = 0;
}

BasicIDEData::~BasicIDEData()
{
    // ErrorHdl zuruecksetzen ist zwar sauberer, aber diese Instanz wird
    // sowieso sehr spaet, nach dem letzten Basic, zerstoert.
    // Durch den Aufruf werden dann aber wieder AppDaten erzeugt und nicht
    // mehr zerstoert => MLK's beim Purify
//  StarBASIC::SetGlobalErrorHdl( Link() );
//  StarBASIC::SetGlobalBreakHdl( Link() );
//  StarBASIC::setGlobalStarScriptListener( XEngineListenerRef() );

    delete pSearchItem;
    //delete pAccelerator;
}

SvxSearchItem& BasicIDEData::GetSearchItem() const
{
    return *pSearchItem;
}

void BasicIDEData::SetSearchItem( const SvxSearchItem& rItem )
{
    delete pSearchItem;
    pSearchItem = (SvxSearchItem*)rItem.Clone();
}

IMPL_LINK( BasicIDEData, GlobalBasicBreakHdl, StarBASIC *, pBasic )
{
    long nRet = 0;
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    if ( pIDEShell )
    {
        BasicManager* pBasMgr = BasicIDE::FindBasicManager( pBasic );
        if ( pBasMgr )
        {
            // Hier lande ich zweimal, wenn Step into protected Basic
            // => schlecht, wenn Passwortabfrage 2x, ausserdem sieht man in
            // dem PasswordDlg nicht, fuer welche Lib...
            // => An dieser Stelle keine Passwort-Abfrage starten
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            OSL_ENSURE( aDocument.isValid(), "BasicIDEData::GlobalBasicBreakHdl: no document for the basic manager!" );
            if ( aDocument.isValid() )
            {
                ::rtl::OUString aOULibName( pBasic->GetName() );
                Reference< script::XLibraryContainer > xModLibContainer( aDocument.getLibraryContainer( E_SCRIPTS ), UNO_QUERY );
                if ( xModLibContainer.is() && xModLibContainer->hasByName( aOULibName ) )
                {
                    Reference< script::XLibraryContainerPassword > xPasswd( xModLibContainer, UNO_QUERY );
                    if ( xPasswd.is() && xPasswd->isLibraryPasswordProtected( aOULibName ) && !xPasswd->isLibraryPasswordVerified( aOULibName ) )
                    {
                           // Ein Step-Out muesste mich aus den geschuetzten Bereich befoerdern...
                        nRet = SbDEBUG_STEPOUT;
                    }
                    else
                    {
                          nRet = pIDEShell->CallBasicBreakHdl( pBasic );
                    }
                }
            }
        }
    }

    return nRet;
}

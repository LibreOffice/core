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

#include "docsignature.hxx"

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#include <ide_pch.hxx>
#include <basic/sbx.hxx>
#include "basicrenderable.hxx"

#include <com/sun/star/frame/XTitle.hpp>

#include <vcl/sound.hxx>
#include <basidesh.hxx>
#include <basidesh.hrc>
#include <baside2.hxx>
#include <basdoc.hxx>
#include <basobj.hxx>
#include <svtools/texteng.hxx>
#include <svtools/textview.hxx>
#include <svtools/xtextedt.hxx>
#include <tools/diagnose_ex.h>
#include <sfx2/sfxdefs.hxx>
#include <sfx2/signaturestate.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
namespace css = ::com::sun::star;

IMPL_LINK_INLINE_START( BasicIDEShell, ObjectDialogCancelHdl, ObjectCatalog *, EMPTYARG )
{
    ShowObjectDialog( sal_False, sal_True );
    return 0;
}
IMPL_LINK_INLINE_END( BasicIDEShell, ObjectDialogCancelHdl, ObjectCatalog *, EMPTYARG )

/*
IMPL_LINK( BasicIDEShell, ObjectDialogInsertHdl, ObjectCatalog *, pObjCat )
{
    if ( !pCurWin )
        return 0;

    if ( dynamic_cast< ModulWindow* >(pCurWin) )
    {
        ModulWindow* pEditWin = (ModulWindow*)pCurWin;
        pEditWin->InsertFromObjectCatalog( pObjCat );
    }
    else
        Sound::Beep();

    return 0;
}
*/

Reference< view::XRenderable > BasicIDEShell::GetRenderable()
{
    return Reference< view::XRenderable >( new basicide::BasicRenderable( pCurWin ) );
}

#if 0
sal_uInt16 __EXPORT BasicIDEShell::Print( SfxProgress &rProgress, sal_Bool bIsAPI, PrintDialog *pPrintDialog )
{
    if ( pCurWin )
    {
        SfxPrinter* pPrinter = GetPrinter( sal_True );
        if ( pPrinter )
        {
            SfxViewShell::Print( rProgress, bIsAPI, pPrintDialog );
            pCurWin->PrintData( pPrinter );
        }
    }
    return 0;
}
#endif

sal_Bool BasicIDEShell::HasSelection( sal_Bool /* bText */ ) const
{
    sal_Bool bSel = sal_False;
    ModulWindow* pModulWindow = dynamic_cast< ModulWindow* >(pCurWin);

    if ( pModulWindow )
    {
        TextView* pEditView = pModulWindow->GetEditView();
        if ( pEditView && pEditView->HasSelection() )
            bSel = sal_True;
    }
    return bSel;
}

String BasicIDEShell::GetSelectionText( sal_Bool bWholeWord )
{
    String aText;
    ModulWindow* pModulWindow = dynamic_cast< ModulWindow* >(pCurWin);

    if ( pModulWindow )
    {
        TextView* pEditView = pModulWindow->GetEditView();
        if ( pEditView )
        {
            if ( bWholeWord && !pEditView->HasSelection() )
            {
                // String aStrCurrentDelimiters = pEngine->GetWordDelimiters();
                // pEngine->SetWordDelimiters( " .,;\"'" );
                aText = pEditView->GetTextEngine()->GetWord( pEditView->GetSelection().GetEnd() );
                // pEngine->SetWordDelimiters( aStrCurrentDelimiters );
            }
            else
            {
                TextSelection aSel = pEditView->GetSelection();
                if ( !bWholeWord || ( aSel.GetStart().GetPara() == aSel.GetEnd().GetPara() ) )
                    aText = pEditView->GetSelected();
            }
        }
    }
    return aText;
}

SfxPrinter* __EXPORT BasicIDEShell::GetPrinter( sal_Bool bCreate )
{
    if ( pCurWin ) // && dynamic_cast< ModulWindow* >(pCurWin) )
    {
        BasicDocShell* pDocShell = (BasicDocShell*)GetViewFrame()->GetObjectShell();
        DBG_ASSERT( pDocShell, "DocShell ?!" );
        return pDocShell->GetPrinter( bCreate );
    }
    return 0;
}

sal_uInt16 __EXPORT BasicIDEShell::SetPrinter( SfxPrinter *pNewPrinter, sal_uInt16 nDiffFlags, bool )
{
    (void)nDiffFlags;
    BasicDocShell* pDocShell = (BasicDocShell*)GetViewFrame()->GetObjectShell();
    DBG_ASSERT( pDocShell, "DocShell ?!" );
    pDocShell->SetPrinter( pNewPrinter );
    return 0;
}

void BasicIDEShell::SetMDITitle()
{
    String aTitle;

    if ( m_aCurLibName.Len() )
    {
        LibraryLocation eLocation = m_aCurDocument.getLibraryLocation( m_aCurLibName );
        aTitle = m_aCurDocument.getTitle( eLocation );
        aTitle += '.';
        aTitle += m_aCurLibName;
    }
    else
    {
        aTitle = String( IDEResId( RID_STR_ALL ) );
    }

    ::basctl::DocumentSignature aCurSignature( m_aCurDocument );
    if ( aCurSignature.getScriptingSignatureState() == SIGNATURESTATE_SIGNATURES_OK )
    {
        aTitle += String::CreateFromAscii( " " );
        aTitle += String( IDEResId( RID_STR_SIGNED ) );
        aTitle += String::CreateFromAscii( " " );
    }

    SfxViewFrame* pViewFrame = GetViewFrame();
    if ( pViewFrame )
    {
        SfxObjectShell* pShell = pViewFrame->GetObjectShell();
        if ( pShell && aTitle != pShell->GetTitle( SFX_TITLE_CAPTION ) )
        {
            pShell->SetTitle( aTitle );
            pShell->SetModified( sal_False );
        }

        css::uno::Reference< css::frame::XController > xController = GetController ();
        css::uno::Reference< css::frame::XTitle >      xTitle      (xController, css::uno::UNO_QUERY);
        if (xTitle.is ())
            xTitle->setTitle (aTitle);
    }
}

void BasicIDEShell::DestroyModulWindowLayout()
{
    delete pModulLayout;
    pModulLayout = 0;
}


void BasicIDEShell::UpdateModulWindowLayout( bool bBasicStopped )
{
    if ( pModulLayout )
    {
        pModulLayout->GetStackWindow().UpdateCalls();
        pModulLayout->GetWatchWindow().UpdateWatches( bBasicStopped );
    }
}

void BasicIDEShell::CreateModulWindowLayout()
{
    pModulLayout = new ModulWindowLayout( &GetViewFrame()->GetWindow() );
}

ModulWindow* BasicIDEShell::CreateBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName )
{
    bCreatingWindow = sal_True;

    sal_uLong nKey = 0;
    ModulWindow* pWin = 0;

    String aLibName( rLibName );
    String aModName( rModName );

    if ( !aLibName.Len() )
        aLibName = String::CreateFromAscii( "Standard" );

    uno::Reference< container::XNameContainer > xLib = rDocument.getOrCreateLibrary( E_SCRIPTS, aLibName );

    if ( !aModName.Len() )
        aModName = rDocument.createObjectName( E_SCRIPTS, aLibName );

    // Vielleicht gibt es ein suspendiertes?
    pWin = FindBasWin( rDocument, aLibName, aModName, sal_False, sal_True );

    if ( !pWin )
    {
        ::rtl::OUString aModule;
        bool bSuccess = false;
        if ( rDocument.hasModule( aLibName, aModName ) )
            bSuccess = rDocument.getModule( aLibName, aModName, aModule );
        else
            bSuccess = rDocument.createModule( aLibName, aModName, sal_True, aModule );

        if ( bSuccess )
        {
            pWin = FindBasWin( rDocument, aLibName, aModName, sal_False, sal_True );
            if( !pWin )
            {
                // new module window
                pWin = new ModulWindow( pModulLayout, rDocument, aLibName, aModName, aModule );
                nKey = InsertWindowInTable( pWin );
            }
            else // we've gotten called recursively ( via listener from createModule above ), get outta here
                return pWin;
        }
    }
    else
    {
        pWin->SetStatus( pWin->GetStatus() & ~BASWIN_SUSPENDED );
        IDEBaseWindow* pTmp = aIDEWindowTable.First();
        while ( pTmp && !nKey )
        {
            if ( pTmp == pWin )
                nKey = aIDEWindowTable.GetCurKey();
            pTmp = aIDEWindowTable.Next();
        }
        DBG_ASSERT( nKey, "CreateBasWin: Kein Key- Fenster nicht gefunden!" );
    }
    if( nKey && xLib.is() && rDocument.isInVBAMode() )
    {
        // display a nice friendly name in the ObjectModule tab,
        // combining the objectname and module name, e.g. Sheet1 ( Financials )
        String sObjName;
        ModuleInfoHelper::getObjectName( xLib, rModName, sObjName );
        if( sObjName.Len() )
        {
            aModName.AppendAscii(" (").Append(sObjName).AppendAscii(")");
        }
    }
    pTabBar->InsertPage( (sal_uInt16)nKey, aModName );
    pTabBar->Sort();
    pWin->GrabScrollBars( &aHScrollBar, &aVScrollBar );
    if ( !pCurWin )
        SetCurWindow( pWin, sal_False, sal_False );

    bCreatingWindow = sal_False;
    return pWin;
}

ModulWindow* BasicIDEShell::FindBasWin( const ScriptDocument& rDocument, const String& rLibName, const String& rModName, sal_Bool bCreateIfNotExist, sal_Bool bFindSuspended )
{
    ModulWindow* pModWin = 0;
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin && !pModWin )
    {
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && dynamic_cast< ModulWindow* >(pWin) )
        {
            if ( !rLibName.Len() )  // nur irgendeins finden...
                pModWin = (ModulWindow*)pWin;
            else if ( pWin->IsDocument( rDocument ) && pWin->GetLibName() == rLibName && pWin->GetName() == rModName )
                pModWin = (ModulWindow*)pWin;
        }
        pWin = aIDEWindowTable.Next();
    }
    if ( !pModWin && bCreateIfNotExist )
        pModWin = CreateBasWin( rDocument, rLibName, rModName );

    return pModWin;
}

void __EXPORT BasicIDEShell::Move()
{
    ModulWindow* pModulWindow = dynamic_cast< ModulWindow* >(pCurWin);

    if ( pModulWindow )
        pModulWindow->FrameWindowMoved();
}

void __EXPORT BasicIDEShell::ShowCursor( bool bOn )
{
    ModulWindow* pModulWindow = dynamic_cast< ModulWindow* >(pCurWin);

    if ( pModulWindow )
        pModulWindow->ShowCursor( (sal_Bool)bOn );
}

// Hack for #101048
sal_Int32 getBasicIDEShellCount( void );

// Nur wenn Basicfenster oben:
void __EXPORT BasicIDEShell::ExecuteBasic( SfxRequest& rReq )
{
    if ( !pCurWin || !dynamic_cast< ModulWindow* >(pCurWin) )
        return;

    pCurWin->ExecuteCommand( rReq );
    sal_Int32 nCount = getBasicIDEShellCount();
    if( nCount )
        CheckWindows();
}


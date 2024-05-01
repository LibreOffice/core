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

#include <basidesh.hxx>
#include <iderdll.hxx>
#include "iderdll2.hxx"
#include "macrodlg.hxx"
#include "moduldlg.hxx"
#include <iderid.hxx>
#include <strings.hrc>
#include "baside2.hxx"

#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <basic/sbmeth.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/sequence.hxx>
#include <framework/documentundoguard.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <vector>
#include <algorithm>
#include <basic/basmgr.hxx>
namespace basctl
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;

extern "C" {
    SAL_DLLPUBLIC_EXPORT rtl_uString* basicide_choose_macro(void* pParent, void* pOnlyInDocument_AsXModel, void* pDocFrame_AsXFrame, sal_Bool bChooseOnly )
    {
        Reference< frame::XModel > aDocument( static_cast< frame::XModel* >( pOnlyInDocument_AsXModel ) );
        Reference< frame::XFrame > aDocFrame( static_cast< frame::XFrame* >( pDocFrame_AsXFrame ) );
        OUString aScriptURL = basctl::ChooseMacro(static_cast<weld::Window*>(pParent), aDocument, aDocFrame, bChooseOnly);
        rtl_uString* pScriptURL = aScriptURL.pData;
        rtl_uString_acquire( pScriptURL );

        return pScriptURL;
    }
    SAL_DLLPUBLIC_EXPORT void basicide_macro_organizer(void *pParent, void* pDocFrame_AsXFrame, sal_Int16 nTabId)
    {
        SAL_INFO("basctl.basicide","in basicide_macro_organizer");
        Reference< frame::XFrame > aDocFrame( static_cast< frame::XFrame* >( pDocFrame_AsXFrame ) );
        basctl::Organize(static_cast<weld::Window*>(pParent), aDocFrame, nTabId);
    }
}

void Organize(weld::Window* pParent, const css::uno::Reference<css::frame::XFrame>& xDocFrame, sal_Int16 tabId)
{
    EnsureIde();

    auto xDlg(std::make_shared<OrganizeDialog>(pParent, xDocFrame, tabId));
    weld::DialogController::runAsync(xDlg, [](int) {});
}

bool IsValidSbxName( std::u16string_view rName )
{
    for ( size_t nChar = 0; nChar < rName.size(); nChar++ )
    {
        sal_Unicode c = rName[nChar];
        bool bValid = (
            ( c >= 'A' && c <= 'Z' ) ||
            ( c >= 'a' && c <= 'z' ) ||
            ( c >= '0' && c <= '9' && nChar ) ||
            ( c == '_' )
        );
        if ( !bValid )
            return false;
    }
    return true;
}

Sequence< OUString > GetMergedLibraryNames( const Reference< script::XLibraryContainer >& xModLibContainer, const Reference< script::XLibraryContainer >& xDlgLibContainer )
{
    // create a list of module library names
    std::vector<OUString> aLibList;
    if ( xModLibContainer.is() )
    {
        const Sequence< OUString > aModLibNames = xModLibContainer->getElementNames();
        aLibList.insert( aLibList.end(), aModLibNames.begin(), aModLibNames.end() );
    }

    // create a list of dialog library names
    if ( xDlgLibContainer.is() )
    {
        const Sequence< OUString > aDlgLibNames = xDlgLibContainer->getElementNames();
        aLibList.insert( aLibList.end(), aDlgLibNames.begin(), aDlgLibNames.end() );
    }

    // sort list
    auto const sort = comphelper::string::NaturalStringSorter(
        comphelper::getProcessComponentContext(),
        Application::GetSettings().GetUILanguageTag().getLocale());
    std::sort(aLibList.begin(), aLibList.end(),
              [&sort](const OUString& rLHS, const OUString& rRHS) {
                  return sort.compare(rLHS, rRHS) < 0;
              });
    // remove duplicates
    std::vector<OUString>::iterator aIterEnd = std::unique( aLibList.begin(), aLibList.end() );
    aLibList.erase( aIterEnd, aLibList.end() );

    return comphelper::containerToSequence(aLibList);
}

bool RenameModule (
    weld::Widget* pErrorParent,
    const ScriptDocument& rDocument,
    const OUString& rLibName,
    const OUString& rOldName,
    const OUString& rNewName
)
{
    if ( !rDocument.hasModule( rLibName, rOldName ) )
    {
        SAL_WARN( "basctl.basicide","basctl::RenameModule: old module name is invalid!" );
        return false;
    }

    if ( rDocument.hasModule( rLibName, rNewName ) )
    {
        std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(pErrorParent,
                                                    VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_SBXNAMEALLREADYUSED2)));
        xError->run();
        return false;
    }

    // #i74440
    if ( rNewName.isEmpty() )
    {
        std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(pErrorParent,
                                                    VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_BADSBXNAME)));
        xError->run();
        return false;
    }

    if ( !rDocument.renameModule( rLibName, rOldName, rNewName ) )
        return false;

    Shell* pShell = GetShell();
    if (!pShell)
        return true;
    VclPtr<ModulWindow> pWin = pShell->FindBasWin(rDocument, rLibName, rNewName, false, true);
    if (!pWin)
        return true;

    // set new name in window
    pWin->SetName( rNewName );

    // set new module in module window
    pWin->SetSbModule( pWin->GetBasic()->FindModule( rNewName ) );

    // update tabwriter
    sal_uInt16 nId = pShell->GetWindowId( pWin );
    SAL_WARN_IF( nId == 0 , "basctl.basicide", "No entry in Tabbar!");
    if ( nId )
    {
        TabBar& rTabBar = pShell->GetTabBar();
        rTabBar.SetPageText(nId, rNewName);
        rTabBar.Sort();
        rTabBar.MakeVisible(rTabBar.GetCurPageId());
    }
    return true;
}

namespace
{
    struct MacroExecutionData
    {
        ScriptDocument  aDocument;
        SbMethodRef     xMethod;

        MacroExecutionData()
            :aDocument( ScriptDocument::NoDocument )
        {
        }
    };

    class MacroExecution
    {
    public:
        DECL_STATIC_LINK( MacroExecution, ExecuteMacroEvent, void*, void );
    };

    IMPL_STATIC_LINK( MacroExecution, ExecuteMacroEvent, void*, p, void )
    {
        MacroExecutionData* i_pData = static_cast<MacroExecutionData*>(p);
        ENSURE_OR_RETURN_VOID( i_pData, "wrong MacroExecutionData" );
        // take ownership of the data
        std::unique_ptr< MacroExecutionData > pData( i_pData );

        SAL_WARN_IF( (pData->xMethod->GetParent()->GetFlags() & SbxFlagBits::ExtSearch) == SbxFlagBits::NONE, "basctl.basicide","No EXTSEARCH!" );

        // in case this is a document-local macro, try to protect the document's Undo Manager from
        // flawed scripts
        std::optional< ::framework::DocumentUndoGuard > pUndoGuard;
        if ( pData->aDocument.isDocument() )
            pUndoGuard.emplace( pData->aDocument.getDocument() );

        RunMethod( pData->xMethod.get() );
    }
}

OUString ChooseMacro(weld::Window* pParent,
                     const uno::Reference< frame::XModel >& rxLimitToDocument,
                     const uno::Reference< frame::XFrame >& xDocFrame,
                     bool bChooseOnly)
{
    EnsureIde();

    GetExtraData()->ChoosingMacro() = true;

    OUString aScriptURL;
    SbMethod* pMethod = nullptr;

    MacroChooser aChooser(pParent, xDocFrame);
    if ( bChooseOnly || !SvtModuleOptions::IsBasicIDE() )
        aChooser.SetMode(MacroChooser::ChooseOnly);

    if ( !bChooseOnly && rxLimitToDocument.is() )
    {
        // Hack!
        aChooser.SetMode(MacroChooser::Recording);
    }

    short nRetValue = aChooser.run();

    GetExtraData()->ChoosingMacro() = false;

    switch ( nRetValue )
    {
        case Macro_OkRun:
        {
            bool bError = false;

            pMethod = aChooser.GetMacro();
            if ( !pMethod && aChooser.GetMode() == MacroChooser::Recording )
                pMethod = aChooser.CreateMacro();

            if ( !pMethod )
                break;

            SbModule* pModule = pMethod->GetModule();
            if ( !pModule )
            {
                SAL_WARN( "basctl.basicide", "basctl::ChooseMacro: No Module found!" );
                break;
            }

            StarBASIC* pBasic = dynamic_cast<StarBASIC*>(pModule->GetParent());
            if ( !pBasic )
            {
                SAL_WARN( "basctl.basicide", "basctl::ChooseMacro: No Basic found!" );
                break;
            }

            BasicManager* pBasMgr = FindBasicManager( pBasic );
            if ( !pBasMgr )
            {
                SAL_WARN( "basctl.basicide", "basctl::ChooseMacro: No BasicManager found!" );
                break;
            }

            // name
            OUString aName = pBasic->GetName() + "." + pModule->GetName() + "." + pMethod->GetName();

            // location
            OUString aLocation;
            ScriptDocument aDocument( ScriptDocument::getDocumentForBasicManager( pBasMgr ) );
            if ( aDocument.isDocument() )
            {
                // document basic
                aLocation = "document" ;

                if ( rxLimitToDocument.is() )
                {
                    uno::Reference< frame::XModel > xLimitToDocument( rxLimitToDocument );

                    uno::Reference< document::XEmbeddedScripts > xScripts( rxLimitToDocument, UNO_QUERY );
                    if ( !xScripts.is() )
                    {   // the document itself does not support embedding scripts
                        uno::Reference< document::XScriptInvocationContext > xContext( rxLimitToDocument, UNO_QUERY );
                        if ( xContext.is() )
                            xScripts = xContext->getScriptContainer();
                        if ( xScripts.is() )
                        {   // but it is able to refer to a document which actually does support this
                            xLimitToDocument.set( xScripts, UNO_QUERY );
                            if ( !xLimitToDocument.is() )
                            {
                                SAL_WARN_IF(!xLimitToDocument.is(), "basctl.basicide", "basctl::ChooseMacro: a script container which is no document!?" );
                                xLimitToDocument = rxLimitToDocument;
                            }
                        }
                    }

                    if ( xLimitToDocument != aDocument.getDocument() )
                    {
                        // error
                        bError = true;
                        std::unique_ptr<weld::MessageDialog> xError(Application::CreateMessageDialog(nullptr,
                                                                    VclMessageType::Warning, VclButtonsType::Ok, IDEResId(RID_STR_ERRORCHOOSEMACRO)));
                        xError->run();
                    }
                }
            }
            else
            {
                // application basic
                aLocation = "application" ;
            }

            // script URL
            if ( !bError )
            {
                aScriptURL = "vnd.sun.star.script:" + aName + "?language=Basic&location=" + aLocation;
            }

            if ( !rxLimitToDocument.is() )
            {
                MacroExecutionData* pExecData = new MacroExecutionData;
                pExecData->aDocument = aDocument;
                pExecData->xMethod = pMethod;   // keep alive until the event has been processed
                Application::PostUserEvent( LINK( nullptr, MacroExecution, ExecuteMacroEvent ), pExecData );
            }
        }
        break;
    }

    return aScriptURL;
}

Sequence< OUString > GetMethodNames( const ScriptDocument& rDocument, const OUString& rLibName, const OUString& rModName )
{
    Sequence< OUString > aSeqMethods;

    // get module
    OUString aOUSource;
    if ( rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        BasicManager* pBasMgr = rDocument.getBasicManager();
        StarBASIC* pSb = pBasMgr ? pBasMgr->GetLib( rLibName ) : nullptr;
        SbModule* pMod = pSb ? pSb->FindModule( rModName ) : nullptr;

        SbModuleRef xModule;
        // Only reparse modules if ScriptDocument source is out of sync
        // with basic's Module
        if ( !pMod || pMod->GetSource32() != aOUSource )
        {
            xModule = new SbModule( rModName );
            xModule->SetSource32( aOUSource );
            pMod = xModule.get();
        }

        sal_uInt32 nCount = pMod->GetMethods()->Count();
        sal_uInt32 nRealCount = nCount;
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pMod->GetMethods()->Get(i));
            if( pMethod->IsHidden() )
                --nRealCount;
        }
        aSeqMethods.realloc( nRealCount );

        sal_uInt32 iTarget = 0;
        for ( sal_uInt32 i = 0 ; i < nCount; ++i )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pMod->GetMethods()->Get(i));
            if( pMethod->IsHidden() )
                continue;
            assert(pMethod && "Method not found! (NULL)");
            aSeqMethods.getArray()[ iTarget++ ] = pMethod->GetName();
        }
    }

    return aSeqMethods;
}

bool HasMethod (
    ScriptDocument const& rDocument,
    OUString const& rLibName,
    OUString const& rModName,
    OUString const& rMethName
)
{
    bool bHasMethod = false;

    OUString aOUSource;
    if ( rDocument.hasModule( rLibName, rModName ) && rDocument.getModule( rLibName, rModName, aOUSource ) )
    {
        // Check if we really need to scan the source ( again )
        BasicManager* pBasMgr = rDocument.getBasicManager();
        StarBASIC* pSb = pBasMgr ? pBasMgr->GetLib( rLibName ) : nullptr;
        SbModule* pMod = pSb ? pSb->FindModule( rModName ) : nullptr;
        SbModuleRef xModule;
        // Only reparse modules if ScriptDocument source is out of sync
        // with basic's Module
        if ( !pMod || pMod->GetSource32() != aOUSource )
        {
            xModule = new SbModule( rModName );
            xModule->SetSource32( aOUSource );
            pMod = xModule.get();
        }
        SbxArray* pMethods = pMod->GetMethods().get();
        if ( pMethods )
        {
            SbMethod* pMethod = static_cast<SbMethod*>(pMethods->Find( rMethName, SbxClassType::Method ));
            if ( pMethod && !pMethod->IsHidden() )
                bHasMethod = true;
        }
    }

    return bHasMethod;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

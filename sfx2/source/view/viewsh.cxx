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

#include <config_features.h>

#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/toolbox.hxx>
#include <svl/intitem.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <cppuhelper/implbase.hxx>

#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/soerr.hxx>
#include <svtools/embedhlp.hxx>

#include <basic/basmgr.hxx>
#include <basic/sbuno.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/settings.hxx>

#include <sfx2/app.hxx>
#include "view.hrc"
#include <sfx2/viewsh.hxx>
#include "viewimp.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/request.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include "arrdecl.hxx"
#include <sfx2/docfac.hxx>
#include "sfxlocal.hrc"
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/mailmodelapi.hxx>
#include "bluthsndapi.hxx"
#include <sfx2/viewfrm.hxx>
#include <sfx2/event.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/ipclient.hxx>
#include "workwin.hxx"
#include <sfx2/objface.hxx>
#include <sfx2/docfilt.hxx>
#include "openuriexternally.hxx"
#include <shellimpl.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::cppu;

#define SfxViewShell
#include "sfxslots.hxx"



class SfxClipboardChangeListener : public ::cppu::WeakImplHelper<
    datatransfer::clipboard::XClipboardListener >
{
public:
    SfxClipboardChangeListener( SfxViewShell* pView, const uno::Reference< datatransfer::clipboard::XClipboardNotifier >& xClpbrdNtfr );
    virtual ~SfxClipboardChangeListener();

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& rEventObject )
        throw ( uno::RuntimeException, std::exception ) override;

    // XClipboardListener
    virtual void SAL_CALL changedContents( const datatransfer::clipboard::ClipboardEvent& rEventObject )
        throw ( uno::RuntimeException, std::exception ) override;

    void DisconnectViewShell() { m_pViewShell = nullptr; }
    void ChangedContents();

    enum AsyncExecuteCmd
    {
        ASYNCEXECUTE_CMD_DISPOSING,
        ASYNCEXECUTE_CMD_CHANGEDCONTENTS
    };

    struct AsyncExecuteInfo
    {
        AsyncExecuteInfo( AsyncExecuteCmd eCmd, uno::Reference< datatransfer::clipboard::XClipboardListener > xThis, SfxClipboardChangeListener* pListener ) :
            m_eCmd( eCmd ), m_xThis( xThis ), m_pListener( pListener ) {}

        AsyncExecuteCmd m_eCmd;
        uno::Reference< datatransfer::clipboard::XClipboardListener > m_xThis;
        SfxClipboardChangeListener* m_pListener;
    };

private:
    SfxViewShell* m_pViewShell;
    uno::Reference< datatransfer::clipboard::XClipboardNotifier > m_xClpbrdNtfr;
    uno::Reference< lang::XComponent > m_xCtrl;

    DECL_STATIC_LINK_TYPED( SfxClipboardChangeListener, AsyncExecuteHdl_Impl, void*, void );
};

SfxClipboardChangeListener::SfxClipboardChangeListener( SfxViewShell* pView, const uno::Reference< datatransfer::clipboard::XClipboardNotifier >& xClpbrdNtfr )
  : m_pViewShell( nullptr ), m_xClpbrdNtfr( xClpbrdNtfr )
{
    m_xCtrl.set( pView->GetController(), uno::UNO_QUERY );
    if ( m_xCtrl.is() )
    {
        m_xCtrl->addEventListener( uno::Reference < lang::XEventListener > ( static_cast < lang::XEventListener* >( this ) ) );
        m_pViewShell = pView;
    }
    if ( m_xClpbrdNtfr.is() )
    {
        m_xClpbrdNtfr->addClipboardListener( uno::Reference< datatransfer::clipboard::XClipboardListener >(
            static_cast< datatransfer::clipboard::XClipboardListener* >( this )));
    }
}

SfxClipboardChangeListener::~SfxClipboardChangeListener()
{
}

void SfxClipboardChangeListener::ChangedContents()
{
    const SolarMutexGuard aGuard;
    if( m_pViewShell )
    {
        SfxBindings& rBind = m_pViewShell->GetViewFrame()->GetBindings();
        rBind.Invalidate( SID_PASTE );
        rBind.Invalidate( SID_PASTE_SPECIAL );
        rBind.Invalidate( SID_CLIPBOARD_FORMAT_ITEMS );
    }
}

IMPL_STATIC_LINK_TYPED( SfxClipboardChangeListener, AsyncExecuteHdl_Impl, void*, p, void )
{
    AsyncExecuteInfo* pAsyncExecuteInfo = static_cast<AsyncExecuteInfo*>(p);
    if ( pAsyncExecuteInfo )
    {
        uno::Reference< datatransfer::clipboard::XClipboardListener > xThis( pAsyncExecuteInfo->m_xThis );
        if ( pAsyncExecuteInfo->m_pListener )
        {
            if ( pAsyncExecuteInfo->m_eCmd == ASYNCEXECUTE_CMD_DISPOSING )
                pAsyncExecuteInfo->m_pListener->DisconnectViewShell();
            else if ( pAsyncExecuteInfo->m_eCmd == ASYNCEXECUTE_CMD_CHANGEDCONTENTS )
                pAsyncExecuteInfo->m_pListener->ChangedContents();
        }
    }
    delete pAsyncExecuteInfo;
}

void SAL_CALL SfxClipboardChangeListener::disposing( const lang::EventObject& /*rEventObject*/ )
    throw ( uno::RuntimeException, std::exception )
{
    // Either clipboard or ViewShell is going to be destroyed -> no interest in listening anymore
    uno::Reference< lang::XComponent > xCtrl( m_xCtrl );
    uno::Reference< datatransfer::clipboard::XClipboardNotifier > xNotify( m_xClpbrdNtfr );

    uno::Reference< datatransfer::clipboard::XClipboardListener > xThis( static_cast< datatransfer::clipboard::XClipboardListener* >( this ));
    if ( xCtrl.is() )
        xCtrl->removeEventListener( uno::Reference < lang::XEventListener > ( static_cast < lang::XEventListener* >( this )));
    if ( xNotify.is() )
        xNotify->removeClipboardListener( xThis );

    // Make asynchronous call to avoid locking SolarMutex which is the
    // root for many deadlocks, especially in conjunction with the "Windows"
    // based single thread apartment clipboard code!
    AsyncExecuteInfo* pInfo = new AsyncExecuteInfo( ASYNCEXECUTE_CMD_DISPOSING, xThis, this );
    Application::PostUserEvent( LINK( nullptr, SfxClipboardChangeListener, AsyncExecuteHdl_Impl ), pInfo );
}

void SAL_CALL SfxClipboardChangeListener::changedContents( const datatransfer::clipboard::ClipboardEvent& )
    throw ( RuntimeException, std::exception )
{
    // Make asynchronous call to avoid locking SolarMutex which is the
    // root for many deadlocks, especially in conjunction with the "Windows"
    // based single thread apartment clipboard code!
    uno::Reference< datatransfer::clipboard::XClipboardListener > xThis( static_cast< datatransfer::clipboard::XClipboardListener* >( this ));
    AsyncExecuteInfo* pInfo = new AsyncExecuteInfo( ASYNCEXECUTE_CMD_CHANGEDCONTENTS, xThis, this );
    Application::PostUserEvent( LINK( nullptr, SfxClipboardChangeListener, AsyncExecuteHdl_Impl ), pInfo );
}



static OUString RetrieveLabelFromCommand(
    const OUString& rCommandURL,
    const css::uno::Reference< css::frame::XFrame >& rFrame )
{
    static css::uno::WeakReference< frame::XModuleManager2 > s_xModuleManager;
    static css::uno::WeakReference< container::XNameAccess > s_xNameAccess;

    OUString aLabel;
    css::uno::Reference< css::frame::XModuleManager2 > xModuleManager( s_xModuleManager );
    css::uno::Reference< css::container::XNameAccess > xNameAccess( s_xNameAccess );
    css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

    try
    {
        if ( !xModuleManager.is() )
        {
            xModuleManager = css::frame::ModuleManager::create(xContext);
            s_xModuleManager = xModuleManager;
        }

        OUString aModuleIdentifier = xModuleManager->identify( rFrame );

        if ( !xNameAccess.is() )
        {
            xNameAccess.set( css::frame::theUICommandDescription::get(xContext),
                             css::uno::UNO_QUERY_THROW );
            s_xNameAccess = xNameAccess;
        }

        css::uno::Any a = xNameAccess->getByName( aModuleIdentifier );
        css::uno::Reference< css::container::XNameAccess > xUICommands;
        a >>= xUICommands;

        OUString aStr;
        css::uno::Sequence< css::beans::PropertyValue > aPropSeq;

        a = xUICommands->getByName( rCommandURL );
        if ( a >>= aPropSeq )
        {
            for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
            {
                if ( aPropSeq[i].Name == "Label" )
                {
                    aPropSeq[i].Value >>= aStr;
                    break;
                }
            }
            aLabel = aStr;
        }
    }
    catch (const css::uno::Exception&)
    {
    }

    return aLabel;
}

class SfxInPlaceClientList
{
    typedef std::vector<SfxInPlaceClient*> DataType;
    DataType maData;

public:
    typedef DataType::iterator iterator;

    SfxInPlaceClient* at( size_t i ) { return maData.at(i); }

    iterator begin() { return maData.begin(); }
    iterator end() { return maData.end(); }

    void push_back( SfxInPlaceClient* p ) { maData.push_back(p); }
    void erase( iterator it ) { maData.erase(it); }
    size_t size() const { return maData.size(); }
};

SfxViewShell_Impl::SfxViewShell_Impl(SfxViewShellFlags const nFlags)
: aInterceptorContainer( aMutex )
,   m_bControllerSet(false)
,   m_nPrinterLocks(0)
,   m_bCanPrint(nFlags & SfxViewShellFlags::CAN_PRINT)
,   m_bHasPrintOptions(nFlags & SfxViewShellFlags::HAS_PRINTOPTIONS)
,   m_bPlugInsActive(true)
,   m_bIsShowView(!(nFlags & SfxViewShellFlags::NO_SHOW))
,   m_bGotOwnership(false)
,   m_bGotFrameOwnership(false)
,   m_nFamily(0xFFFF)   // undefined, default set by TemplateDialog
,   m_pController(nullptr)
,   mpIPClientList(nullptr)
,   m_pLibreOfficeKitViewCallback(nullptr)
,   m_pLibreOfficeKitViewData(nullptr)
{}

SfxViewShell_Impl::~SfxViewShell_Impl()
{
    DELETEZ(mpIPClientList);
}

SfxInPlaceClientList* SfxViewShell_Impl::GetIPClientList_Impl( bool bCreate ) const
{
    if (!mpIPClientList && bCreate)
        mpIPClientList = new SfxInPlaceClientList;
    return mpIPClientList;
}

SFX_IMPL_SUPERCLASS_INTERFACE(SfxViewShell,SfxShell)

void SfxViewShell::InitInterface_Impl()
{
}


/** search for a filter name dependent on type and module
 */
static OUString impl_retrieveFilterNameFromTypeAndModule(
    const css::uno::Reference< css::container::XContainerQuery >& rContainerQuery,
    const OUString& rType,
    const OUString& rModuleIdentifier,
    const sal_Int32 nFlags )
{
    // Retrieve filter from type
    css::uno::Sequence< css::beans::NamedValue > aQuery {
        { "Type", css::uno::makeAny( rType ) },
        { "DocumentService", css::uno::makeAny( rModuleIdentifier ) }
    };

    css::uno::Reference< css::container::XEnumeration > xEnumeration =
        rContainerQuery->createSubSetEnumerationByProperties( aQuery );

    OUString aFoundFilterName;
    while ( xEnumeration->hasMoreElements() )
    {
        ::comphelper::SequenceAsHashMap aFilterPropsHM( xEnumeration->nextElement() );
        OUString aFilterName = aFilterPropsHM.getUnpackedValueOrDefault(
            "Name",
            OUString() );

        sal_Int32 nFilterFlags = aFilterPropsHM.getUnpackedValueOrDefault(
            "Flags",
            sal_Int32( 0 ) );

        if ( nFilterFlags & nFlags )
        {
            aFoundFilterName = aFilterName;
            break;
        }
    }

    return aFoundFilterName;
}


/** search for an internal typename, which map to the current app module
    and map also to a "family" of file formats as e.g. PDF/MS Doc/OOo Doc.
 */
enum ETypeFamily
{
    E_MS_DOC,
    E_OOO_DOC
};

OUString impl_searchFormatTypeForApp(const css::uno::Reference< css::frame::XFrame >& xFrame     ,
                                                  ETypeFamily                                eTypeFamily)
{
    try
    {
        css::uno::Reference< css::uno::XComponentContext >  xContext      (::comphelper::getProcessComponentContext());
        css::uno::Reference< css::frame::XModuleManager2 >  xModuleManager(css::frame::ModuleManager::create(xContext));

        OUString sModule = xModuleManager->identify(xFrame);
        OUString sType   ;

        switch(eTypeFamily)
        {
            case E_MS_DOC:
            {
                if ( sModule == "com.sun.star.text.TextDocument" )
                    sType = "writer_MS_Word_97";
                else
                if ( sModule == "com.sun.star.sheet.SpreadsheetDocument" )
                    sType = "calc_MS_Excel_97";
                else
                if ( sModule == "com.sun.star.drawing.DrawingDocument" )
                    sType = "impress_MS_PowerPoint_97";
                else
                if ( sModule == "com.sun.star.presentation.PresentationDocument" )
                    sType = "impress_MS_PowerPoint_97";
            }
            break;

            case E_OOO_DOC:
            {
                if ( sModule == "com.sun.star.text.TextDocument" )
                    sType = "writer8";
                else
                if ( sModule == "com.sun.star.sheet.SpreadsheetDocument" )
                    sType = "calc8";
                else
                if ( sModule == "com.sun.star.drawing.DrawingDocument" )
                    sType = "draw8";
                else
                if ( sModule == "com.sun.star.presentation.PresentationDocument" )
                    sType = "impress8";
            }
            break;
        }

        return sType;
    }
    catch (const css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::uno::Exception&)
    {
    }

    return OUString();
}

void SfxViewShell::NewIPClient_Impl( SfxInPlaceClient *pIPClient )
{
    pImp->GetIPClientList_Impl()->push_back(pIPClient);
}

void SfxViewShell::IPClientGone_Impl( SfxInPlaceClient *pIPClient )
{
    SfxInPlaceClientList* pClientList = pImp->GetIPClientList_Impl();

    for( SfxInPlaceClientList::iterator it = pClientList->begin(); it != pClientList->end(); ++it )
    {
        if ( *it == pIPClient )
        {
            pClientList->erase( it );
            break;
        }
    }
}





void SfxViewShell::ExecMisc_Impl( SfxRequest &rReq )
{
    const sal_uInt16 nId = rReq.GetSlot();
    switch( nId )
    {
        case SID_STYLE_FAMILY :
        {
            const SfxUInt16Item* pItem = rReq.GetArg<SfxUInt16Item>(nId);
            if (pItem)
            {
                pImp->m_nFamily = pItem->GetValue();
            }
            break;
        }
        case SID_ACTIVATE_STYLE_APPLY:
        {
            uno::Reference< frame::XFrame > xFrame(
                GetViewFrame()->GetFrame().GetFrameInterface(),
                uno::UNO_QUERY);

            Reference< beans::XPropertySet > xPropSet( xFrame, UNO_QUERY );
            Reference< frame::XLayoutManager > xLayoutManager;
            if ( xPropSet.is() )
            {
                try
                {
                    Any aValue = xPropSet->getPropertyValue("LayoutManager");
                    aValue >>= xLayoutManager;
                    if ( xLayoutManager.is() )
                    {
                        OUString aTextResString( "private:resource/toolbar/textobjectbar" );
                        uno::Reference< ui::XUIElement > xElement = xLayoutManager->getElement( aTextResString );
                        if(!xElement.is())
                        {
                            OUString aFrameResString( "private:resource/toolbar/frameobjectbar" );
                            xElement = xLayoutManager->getElement( aFrameResString );
                        }
                        if(!xElement.is())
                        {
                            OUString aOleResString( "private:resource/toolbar/oleobjectbar" );
                            xElement = xLayoutManager->getElement( aOleResString );
                        }
                        if(xElement.is())
                        {
                            uno::Reference< awt::XWindow > xWin( xElement->getRealInterface(), uno::UNO_QUERY_THROW );
                            vcl::Window* pWin = VCLUnoHelper::GetWindow( xWin );
                            ToolBox* pTextToolbox = dynamic_cast< ToolBox* >( pWin );
                            if( pTextToolbox )
                            {
                                sal_uInt16 nItemCount = pTextToolbox->GetItemCount();
                                for( sal_uInt16 nItem = 0; nItem < nItemCount; ++nItem )
                                {
                                    sal_uInt16 nItemId = pTextToolbox->GetItemId( nItem );
                                    const OUString& rCommand = pTextToolbox->GetItemCommand( nItemId );
                                    if (rCommand == ".uno:StyleApply")
                                    {
                                        vcl::Window* pItemWin = pTextToolbox->GetItemWindow( nItemId );
                                        if( pItemWin )
                                            pItemWin->GrabFocus();
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                catch (const Exception&)
                {
                }
            }
            rReq.Done();
        }
        break;
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        case SID_MAIL_SENDDOCASMS:
        case SID_MAIL_SENDDOCASOOO:
        case SID_MAIL_SENDDOCASPDF:
        case SID_MAIL_SENDDOC:
        case SID_MAIL_SENDDOCASFORMAT:
        {
            SfxObjectShell* pDoc = GetObjectShell();
            if ( pDoc && pDoc->QueryHiddenInformation(
                             HiddenWarningFact::WhenSaving, &GetViewFrame()->GetWindow() ) != RET_YES )
                break;


            SfxMailModel  aModel;
            OUString aDocType;

            const SfxStringItem* pMailSubject = rReq.GetArg<SfxStringItem>(SID_MAIL_SUBJECT);
            if ( pMailSubject )
                aModel.SetSubject( pMailSubject->GetValue() );

            const SfxStringItem* pMailRecipient = rReq.GetArg<SfxStringItem>(SID_MAIL_RECIPIENT);
            if ( pMailRecipient )
            {
                OUString aRecipient( pMailRecipient->GetValue() );
                OUString aMailToStr("mailto:");

                if ( aRecipient.startsWith( aMailToStr ) )
                    aRecipient = aRecipient.copy( aMailToStr.getLength() );
                aModel.AddAddress( aRecipient, SfxMailModel::ROLE_TO );
            }
            const SfxStringItem* pMailDocType = rReq.GetArg<SfxStringItem>(SID_TYPE_NAME);
            if ( pMailDocType )
                aDocType = pMailDocType->GetValue();

            uno::Reference < frame::XFrame > xFrame( pFrame->GetFrame().GetFrameInterface() );
            SfxMailModel::SendMailResult eResult = SfxMailModel::SEND_MAIL_ERROR;

            if ( nId == SID_MAIL_SENDDOC )
                eResult = aModel.SaveAndSend( xFrame, OUString() );
            else if ( nId == SID_MAIL_SENDDOCASPDF )
                eResult = aModel.SaveAndSend( xFrame, "pdf_Portable_Document_Format");
            else if ( nId == SID_MAIL_SENDDOCASMS )
            {
                aDocType = impl_searchFormatTypeForApp(xFrame, E_MS_DOC);
                if (!aDocType.isEmpty())
                    eResult = aModel.SaveAndSend( xFrame, aDocType );
            }
            else if ( nId == SID_MAIL_SENDDOCASOOO )
            {
                aDocType = impl_searchFormatTypeForApp(xFrame, E_OOO_DOC);
                if (!aDocType.isEmpty())
                    eResult = aModel.SaveAndSend( xFrame, aDocType );
            }

            if ( eResult == SfxMailModel::SEND_MAIL_ERROR )
            {
                ScopedVclPtrInstance< MessageDialog > aBox(SfxGetpApp()->GetTopWindow(), SfxResId( STR_ERROR_SEND_MAIL ), VCL_MESSAGE_INFO);
                aBox->Execute();
                rReq.Ignore();
            }
            else
                rReq.Done();
        }
        break;

        case SID_BLUETOOTH_SENDDOC:
        {
            SfxBluetoothModel aModel;
            SfxObjectShell* pDoc = GetObjectShell();
            if ( pDoc && pDoc->QueryHiddenInformation(
                            HiddenWarningFact::WhenSaving, &GetViewFrame()->GetWindow() ) != RET_YES )
                break;
            uno::Reference < frame::XFrame > xFrame( pFrame->GetFrame().GetFrameInterface() );
            SfxMailModel::SendMailResult eResult = aModel.SaveAndSend( xFrame, OUString() );
            if( eResult == SfxMailModel::SEND_MAIL_ERROR )
            {
                    ScopedVclPtrInstance< MessageDialog > aBox(SfxGetpApp()->GetTopWindow(), SfxResId( STR_ERROR_SEND_MAIL ), VCL_MESSAGE_INFO);
                    aBox->Execute();
                    rReq.Ignore();
            }
            else
                rReq.Done();
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_WEBHTML:
        {
            static const char HTML_DOCUMENT_TYPE[] = "generic_HTML";
            static const char HTML_GRAPHIC_TYPE[]  = "graphic_HTML";
            const sal_Int32   FILTERFLAG_EXPORT    = 0x00000002;

            css::uno::Reference< lang::XMultiServiceFactory > xSMGR(::comphelper::getProcessServiceFactory(), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< uno::XComponentContext >     xContext(::comphelper::getProcessComponentContext(), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::frame::XFrame >         xFrame( pFrame->GetFrame().GetFrameInterface() );
            css::uno::Reference< css::frame::XModel >         xModel;

            css::uno::Reference< css::frame::XModuleManager2 > xModuleManager( css::frame::ModuleManager::create(xContext) );

            OUString aModule;
            try
            {
                aModule = xModuleManager->identify( xFrame );
            }
            catch (const css::uno::RuntimeException&)
            {
                throw;
            }
            catch (const css::uno::Exception&)
            {
            }

            if ( xFrame.is() )
            {
                css::uno::Reference< css::frame::XController > xController = xFrame->getController();
                if ( xController.is() )
                    xModel = xController->getModel();
            }

            // We need at least a valid module name and model reference
            css::uno::Reference< css::frame::XStorable > xStorable( xModel, css::uno::UNO_QUERY );
            if ( xModel.is() && xStorable.is() )
            {
                OUString aFilterName;
                OUString aTypeName( HTML_DOCUMENT_TYPE );
                OUString aFileName;
                OUString aExtension( "htm" );

                OUString aLocation = xStorable->getLocation();
                INetURLObject aFileObj( aLocation );

                bool bPrivateProtocol = ( aFileObj.GetProtocol() == INetProtocol::PrivSoffice );
                bool bHasLocation = !aLocation.isEmpty() && !bPrivateProtocol;

                css::uno::Reference< css::container::XContainerQuery > xContainerQuery(
                    xSMGR->createInstance( "com.sun.star.document.FilterFactory" ),
                    css::uno::UNO_QUERY_THROW );

                // Retrieve filter from type
                sal_Int32 nFilterFlags = FILTERFLAG_EXPORT;
                aFilterName = impl_retrieveFilterNameFromTypeAndModule( xContainerQuery, aTypeName, aModule, nFilterFlags );
                if ( aFilterName.isEmpty() )
                {
                    // Draw/Impress uses a different type. 2nd chance try to use alternative type name
                    aFilterName = impl_retrieveFilterNameFromTypeAndModule(
                        xContainerQuery, HTML_GRAPHIC_TYPE, aModule, nFilterFlags );
                }

                // No filter found => error
                // No type and no location => error
                if ( aFilterName.isEmpty() ||  aTypeName.isEmpty())
                {
                    rReq.Done();
                    return;
                }

                // Use provided save file name. If empty determine file name
                if ( !bHasLocation )
                {
                    // Create a default file name with the correct extension
                    const OUString aPreviewFileName( "webpreview" );
                    aFileName = aPreviewFileName;
                }
                else
                {
                    // Determine file name from model
                    INetURLObject aFObj( xStorable->getLocation() );
                    aFileName = aFObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::NO_DECODE );
                }

                OSL_ASSERT( !aFilterName.isEmpty() );
                OSL_ASSERT( !aFileName.isEmpty() );

                // Creates a temporary directory to store our predefined file into it.
                ::utl::TempFile aTempDir( nullptr, true );

                INetURLObject aFilePathObj( aTempDir.GetURL() );
                aFilePathObj.insertName( aFileName );
                aFilePathObj.setExtension( aExtension );

                OUString aFileURL = aFilePathObj.GetMainURL( INetURLObject::NO_DECODE );

                css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name  = "FilterName";
                aArgs[0].Value = css::uno::makeAny( aFilterName );

                // Store document in the html format
                try
                {
                    xStorable->storeToURL( aFileURL, aArgs );
                }
                catch (const io::IOException&)
                {
                    rReq.Done();
                    return;
                }

                rReq.Done(sfx2::openUriExternally(aFileURL, true));
                break;
            }
            else
            {
                rReq.Done();
                return;
            }
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_PLUGINS_ACTIVE:
        {
            const SfxBoolItem* pShowItem = rReq.GetArg<SfxBoolItem>(nId);
            bool const bActive = (pShowItem)
                ? pShowItem->GetValue()
                : !pImp->m_bPlugInsActive;
            // ggf. recorden
            if ( !rReq.IsAPI() )
                rReq.AppendItem( SfxBoolItem( nId, bActive ) );

            // Jetzt schon DONE aufrufen, da die Argumente evtl. einen Pool
            // benutzen, der demn"achst weg ist
            rReq.Done(true);

            // ausfuehren
            if (!pShowItem || (bActive != pImp->m_bPlugInsActive))
            {
                SfxFrame* pTopFrame = &GetFrame()->GetTopFrame();
                if ( pTopFrame != &GetFrame()->GetFrame() )
                {
                    // FramesetDocument
                    SfxViewShell *pShell = pTopFrame->GetCurrentViewFrame()->GetViewShell();
                    if ( pShell->GetInterface()->GetSlot( nId ) )
                        pShell->ExecuteSlot( rReq );
                    break;
                }

                SfxFrameIterator aIter( *pTopFrame );
                while ( pTopFrame )
                {
                    if ( pTopFrame->GetCurrentViewFrame() )
                    {
                        SfxViewShell *pView = pTopFrame->GetCurrentViewFrame()->GetViewShell();
                        if ( pView )
                        {
                            pView->pImp->m_bPlugInsActive = bActive;
                            Rectangle aVisArea = GetObjectShell()->GetVisArea();
                            VisAreaChanged(aVisArea);

                            // the plugins might need change in their state
                            SfxInPlaceClientList *pClients = pView->pImp->GetIPClientList_Impl(false);
                            if ( pClients )
                            {
                                for ( size_t n = 0; n < pClients->size(); n++)
                                {
                                    SfxInPlaceClient* pIPClient = pClients->at( n );
                                    if ( pIPClient )
                                        pView->CheckIPClient_Impl( pIPClient, aVisArea );
                                }
                            }
                        }
                    }

                    if ( !pTopFrame->GetParentFrame() )
                        pTopFrame = aIter.FirstFrame();
                    else
                        pTopFrame = aIter.NextFrame( *pTopFrame );
                }
            }

            break;
        }
    }
}



void SfxViewShell::GetState_Impl( SfxItemSet &rSet )
{

    SfxWhichIter aIter( rSet );
    for ( sal_uInt16 nSID = aIter.FirstWhich(); nSID; nSID = aIter.NextWhich() )
    {
        switch ( nSID )
        {
#if HAVE_FEATURE_MACOSX_SANDBOX
            case SID_BLUETOOTH_SENDDOC:
            case SID_MAIL_SENDDOC:
            case SID_MAIL_SENDDOCASFORMAT:
            case SID_MAIL_SENDDOCASMS:
            case SID_MAIL_SENDDOCASOOO:
            case SID_MAIL_SENDDOCASPDF:
                rSet.DisableItem(nSID);
                break;
#endif
            // Printer functions
            case SID_PRINTDOC:
            case SID_PRINTDOCDIRECT:
            case SID_SETUPPRINTER:
            case SID_PRINTER_NAME:
            {
                bool bEnabled = pImp->m_bCanPrint && !pImp->m_nPrinterLocks
                              && !Application::GetSettings().GetMiscSettings().GetDisablePrinting();
                if ( bEnabled )
                {
                    SfxPrinter *pPrinter = GetPrinter();

                    if ( SID_PRINTDOCDIRECT == nSID )
                    {
                        OUString aPrinterName;
                        if ( pPrinter != nullptr )
                            aPrinterName = pPrinter->GetName();
                        else
                            aPrinterName = Printer::GetDefaultPrinterName();
                        if ( !aPrinterName.isEmpty() )
                        {
                            uno::Reference < frame::XFrame > xFrame( pFrame->GetFrame().GetFrameInterface() );

                            OUStringBuffer aBuffer( 60 );
                            aBuffer.append( RetrieveLabelFromCommand(
                                ".uno:PrintDefault",
                                xFrame ));
                            aBuffer.append( " (" );
                            aBuffer.append( aPrinterName );
                            aBuffer.append(')');

                            rSet.Put( SfxStringItem( SID_PRINTDOCDIRECT, aBuffer.makeStringAndClear() ) );
                        }
                    }
                }
                break;
            }

            // PlugIns running
            case SID_PLUGINS_ACTIVE:
            {
                rSet.Put( SfxBoolItem( SID_PLUGINS_ACTIVE, !pImp->m_bPlugInsActive) );
                break;
            }
            case SID_STYLE_FAMILY :
            {
                rSet.Put( SfxUInt16Item( SID_STYLE_FAMILY, pImp->m_nFamily ) );
                break;
            }
        }
    }
}



void SfxViewShell::SetZoomFactor( const Fraction &rZoomX,
    const Fraction &rZoomY )
{
    DBG_ASSERT( GetWindow(), "no window" );
    MapMode aMap( GetWindow()->GetMapMode() );
    aMap.SetScaleX( rZoomX );
    aMap.SetScaleY( rZoomY );
    GetWindow()->SetMapMode( aMap );
}


ErrCode SfxViewShell::DoVerb(long /*nVerb*/)

/*  [Description]

    Virtual Method used to perform a Verb on a selected Object.
    Since this Object is only known by the derived classes, they must override
    DoVerb.
*/

{
    return ERRCODE_SO_NOVERBS;
}



void SfxViewShell::OutplaceActivated( bool bActive, SfxInPlaceClient* /*pClient*/ )
{
    if ( !bActive )
        GetFrame()->GetFrame().Appear();
}



void SfxViewShell::UIActivating( SfxInPlaceClient* /*pClient*/ )
{
    uno::Reference < frame::XFrame > xOwnFrame( pFrame->GetFrame().GetFrameInterface() );
    uno::Reference < frame::XFramesSupplier > xParentFrame( xOwnFrame->getCreator(), uno::UNO_QUERY );
    if ( xParentFrame.is() )
        xParentFrame->setActiveFrame( xOwnFrame );

    pFrame->GetBindings().HidePopups();
    pFrame->GetDispatcher()->Update_Impl( true );
}



void SfxViewShell::UIDeactivated( SfxInPlaceClient* /*pClient*/ )
{
    if ( !pFrame->GetFrame().IsClosing_Impl() || SfxViewFrame::Current() != pFrame )
        pFrame->GetDispatcher()->Update_Impl( true );
    pFrame->GetBindings().HidePopups(false);

    pFrame->GetBindings().InvalidateAll(true);
}



SfxInPlaceClient* SfxViewShell::FindIPClient
(
    const uno::Reference < embed::XEmbeddedObject >& xObj,
    vcl::Window*             pObjParentWin
)   const
{
    SfxInPlaceClientList *pClients = pImp->GetIPClientList_Impl(false);
    if ( !pClients )
        return nullptr;

    if( !pObjParentWin )
        pObjParentWin = GetWindow();
    for ( size_t n = 0; n < pClients->size(); n++)
    {
        SfxInPlaceClient *pIPClient = pClients->at( n );
        if ( pIPClient->GetObject() == xObj && pIPClient->GetEditWin() == pObjParentWin )
            return pIPClient;
    }

    return nullptr;
}



SfxInPlaceClient* SfxViewShell::GetIPClient() const
{
    return GetUIActiveClient();
}



SfxInPlaceClient* SfxViewShell::GetUIActiveIPClient_Impl() const
{
    // this method is needed as long as SFX still manages the border space for ChildWindows (see SfxFrame::Resize)
    SfxInPlaceClientList *pClients = pImp->GetIPClientList_Impl(false);
    if ( !pClients )
        return nullptr;

    for ( size_t n = 0; n < pClients->size(); n++)
    {
        SfxInPlaceClient* pIPClient = pClients->at( n );
        if ( pIPClient->IsUIActive() )
            return pIPClient;
    }

    return nullptr;
}

SfxInPlaceClient* SfxViewShell::GetUIActiveClient() const
{
    SfxInPlaceClientList *pClients = pImp->GetIPClientList_Impl(false);
    if ( !pClients )
        return nullptr;

    for ( size_t n = 0; n < pClients->size(); n++)
    {
        SfxInPlaceClient* pIPClient = pClients->at( n );
        if ( pIPClient->IsObjectUIActive() )
            return pIPClient;
    }

    return nullptr;
}



void SfxViewShell::Activate( bool bMDI )
{
    if ( bMDI )
    {
        SfxObjectShell *pSh = GetViewFrame()->GetObjectShell();
        if ( pSh->GetModel().is() )
            pSh->GetModel()->setCurrentController( GetViewFrame()->GetFrame().GetController() );

        SetCurrentDocument();
    }
}



void SfxViewShell::Deactivate(bool /*bMDI*/)
{
}



void SfxViewShell::AdjustPosSizePixel
(
    const Point&    /*rToolOffset*/,// Upper left corner Tools in Frame-Window
    const Size&     /*rSize*/       // All available sizes.
)

{
}



void SfxViewShell::Move()

/*  [Description]

    This virtual Method is called when the window displayed in the
    SfxViewShell gets a StarView-Move() notification.

    This base implementation does not have to be called.     .

    [Note]

    This Method can be used to cancel a selection, in order to catch the
    mouse movement which is due to moving a window.

    For now the notification does not work In-Place.
*/

{
}



void SfxViewShell::OuterResizePixel
(
    const Point&    /*rToolOffset*/,// Upper left corner Tools in Frame-Window
    const Size&     /*rSize*/       // All available sizes.
)

/*  [Description]

    Override this Method to be able to react to the size-change of
    the View. Thus the View is defined as the Edit window and also the
    attached Tools are defined (for example the ruler).

    The Edit window must not be changed either in size or position.

    The Vis-Area of SfxObjectShell, its scale and position can be changed
    here. The main use is to change the size of the Vis-Area.

    If the Border is changed due to the new calculation then this has to be set
    by <SfxViewShell::SetBorderPixel(const SvBorder&)>. The Positioning of Tools
    is only allowed after the calling of 'SetBorderPixel'.

    [Example]

    void AppViewSh::OuterViewResizePixel( const Point &rOfs, const Size &rSz )
    {
        // Calculate Tool position and size externally, do not set!
        // (due to the following Border calculation)
        Point aHLinPos...; Size aHLinSz...;
        ...

        // Calculate and Set a Border of Tools which matches rSize.
        SvBorder aBorder...
        SetBorderPixel( aBorder ); // Allow Positioning from here on.

        // Arrange Tools
        pHLin->SetPosSizePixel( aHLinPos, aHLinSz );
        ...
    }

    [Cross-reference]

        <SfxViewShell::InnerResizePixel(const Point&,const Size& rSize)>
*/

{
    SetBorderPixel( SvBorder() );
}



void SfxViewShell::InnerResizePixel
(
    const Point&    /*rToolOffset*/,// Upper left corner Tools in Frame-Window
    const Size&     /*rSize*/       // All available sizes.
)

/*  [Description]

    Override this Method to be able to react to the size-change of
    the Edit window.

    The Edit window must not be changed either in size or position.
    Neither the Vis-Area of SfxObjectShell nor its scale or position are
    allowed to be changed

    If the Border is changed due to the new calculation then is has to be set
    by <SfxViewShell::SetBorderPixel(const SvBorder&)>.
    The Positioning of Tools is only allowed after the calling of
    'SetBorderPixel'.


    [Note]

    void AppViewSh::InnerViewResizePixel( const Point &rOfs, const Size &rSz )
    {
        // Calculate Tool position and size internally, do not set!
        // (due to the following Border calculation)
        Point aHLinPos...; Size aHLinSz...;
        ...

        // Calculate and Set a Border of Tools which matches rSize.
        SvBorder aBorder...
        SetBorderPixel( aBorder ); // Allow Positioning from here on.

        // Arrange Tools
        pHLin->SetPosSizePixel( aHLinPos, aHLinSz );
        ...
    }

    [Cross-reference]

        <SfxViewShell::OuterResizePixel(const Point&,const Size& rSize)>
*/

{
    SetBorderPixel( SvBorder() );
}



void SfxViewShell::InvalidateBorder()
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    GetViewFrame()->InvalidateBorderImpl( this );
    if (pImp->m_pController.is())
    {
        pImp->m_pController->BorderWidthsChanged_Impl();
    }
}



void SfxViewShell::SetBorderPixel( const SvBorder &rBorder )
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    GetViewFrame()->SetBorderPixelImpl( this, rBorder );

    // notify related controller that border size is changed
    if (pImp->m_pController.is())
    {
        pImp->m_pController->BorderWidthsChanged_Impl();
    }
}



const SvBorder& SfxViewShell::GetBorderPixel() const
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    return GetViewFrame()->GetBorderPixelImpl( this );
}



void SfxViewShell::SetWindow
(
    vcl::Window*     pViewPort   // For example Null pointer in the Destructor.
)

/*  [Description]

    With this method the SfxViewShell is set in the data window. This is
    needed for the in-place container and for restoring the proper focus.

    Even in-place-active the conversion of the ViewPort Windows is forbidden.
*/

{
    if( pWindow == pViewPort )
        return;

    // Disconnect existing IP-Clients if possible
    DisconnectAllClients();

    //TODO: should we have a "ReconnectAllClients" method?
    DiscardClients_Impl();

    // Switch View-Port
    bool bHadFocus = pWindow && pWindow->HasChildPathFocus( true );
    pWindow = pViewPort;

    if( pWindow )
    {
        // Disable automatic GUI mirroring (right-to-left) for document windows
        pWindow->EnableRTL( false );
    }

    if ( bHadFocus && pWindow )
        pWindow->GrabFocus();
    //TODO/CLEANUP
    //Do we still need this Method?!
    //SfxGetpApp()->GrabFocus( pWindow );
}



SfxViewShell::SfxViewShell
(
    SfxViewFrame*     pViewFrame,     /*  <SfxViewFrame>, which will be
                                          displayed in this View */
    SfxViewShellFlags nFlags          /*  See <SfxViewShell-Flags> */
)

:   SfxShell(this)
,   pImp( new SfxViewShell_Impl(nFlags) )
,   pFrame(pViewFrame)
,   pSubShell(nullptr)
,   pWindow(nullptr)
,   bNoNewWindow( nFlags & SfxViewShellFlags::NO_NEWWINDOW )
,   mbPrinterSettingsModified(false)
{

    if ( pViewFrame->GetParentViewFrame() )
    {
        pImp->m_bPlugInsActive = pViewFrame->GetParentViewFrame()
            ->GetViewShell()->pImp->m_bPlugInsActive;
    }
    SetMargin( pViewFrame->GetMargin_Impl() );

    SetPool( &pViewFrame->GetObjectShell()->GetPool() );
    StartListening(*pViewFrame->GetObjectShell());

    // Insert into list
    SfxViewShellArr_Impl &rViewArr = SfxGetpApp()->GetViewShells_Impl();
    rViewArr.push_back(this);
}



SfxViewShell::~SfxViewShell()
{

    // Remove from list
    const SfxViewShell *pThis = this;
    SfxViewShellArr_Impl &rViewArr = SfxGetpApp()->GetViewShells_Impl();
    SfxViewShellArr_Impl::iterator it = std::find( rViewArr.begin(), rViewArr.end(), pThis );
    rViewArr.erase( it );

    if ( pImp->xClipboardListener.is() )
    {
        pImp->xClipboardListener->DisconnectViewShell();
        pImp->xClipboardListener = nullptr;
    }

    if (pImp->m_pController.is())
    {
        pImp->m_pController->ReleaseShell_Impl();
        pImp->m_pController.clear();
    }

    DELETEZ( pImp );
}

bool SfxViewShell::PrepareClose
(
    bool bUI     // TRUE: Allow Dialog and so on, FALSE: silent-mode
)
{
    SfxPrinter *pPrinter = GetPrinter();
    if ( pPrinter && pPrinter->IsPrinting() )
    {
        if ( bUI )
        {
            ScopedVclPtrInstance< MessageDialog > aInfoBox(&GetViewFrame()->GetWindow(), SfxResId( STR_CANT_CLOSE ), VCL_MESSAGE_INFO );
            aInfoBox->Execute();
        }

        return false;
    }

    if( GetViewFrame()->IsInModalMode() )
        return false;

    if( bUI && GetViewFrame()->GetDispatcher()->IsLocked() )
        return false;

    return true;
}



SfxViewShell* SfxViewShell::Current()
{
    SfxViewFrame *pCurrent = SfxViewFrame::Current();
    return pCurrent ? pCurrent->GetViewShell() : nullptr;
}



SfxViewShell* SfxViewShell::Get( const Reference< XController>& i_rController )
{
    if ( !i_rController.is() )
        return nullptr;

    for (   SfxViewShell* pViewShell = SfxViewShell::GetFirst( false );
            pViewShell;
            pViewShell = SfxViewShell::GetNext( *pViewShell, false )
        )
    {
        if ( pViewShell->GetController() == i_rController )
            return pViewShell;
    }
    return nullptr;
}



SdrView* SfxViewShell::GetDrawView() const

/*  [Description]

    This virtual Method has to be overloded by the sub classes, to be able
    make the Property-Editor available.

    The default implementation does always return zero.
*/

{
    return nullptr;
}



OUString SfxViewShell::GetSelectionText
(
    bool /*bCompleteWords*/ /*  FALSE (default)
                                Only the actual selected text is returned.

                                TRUE
                                The selected text is expanded so that only
                                whole words are returned. As word separators
                                these are used: white spaces and punctuation
                                ".,;" and single and double quotes.
                            */
)

/*  [Description]

    Override this Method to return a text that
    is included in the current selection. This is for example used when
    sending emails.

    When called with "CompleteWords == TRUE", it is for example sufficient
    with having the Cursor positioned somewhere within an URL in-order
    to have the entire URL returned.
*/

{
    return OUString();
}



bool SfxViewShell::HasSelection( bool ) const

/*  [Description]

    With this virtual Method can a for example a Dialog be queried, to
    check if something is selected in the current view. If the Parameter
    is <BOOL> TRUE then it is checked whether some text is selected.
*/

{
    return false;
}

void SfxViewShell::AddSubShell( SfxShell& rShell )
{
    pImp->aArr.push_back(&rShell);
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( pDisp->IsActive(*this) )
    {
        pDisp->Push(rShell);
        pDisp->Flush();
    }
}

void SfxViewShell::RemoveSubShell( SfxShell* pShell )
{
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( !pShell )
    {
        size_t nCount = pImp->aArr.size();
        if ( pDisp->IsActive(*this) )
        {
            for(size_t n = nCount; n > 0; --n)
                pDisp->Pop(*pImp->aArr[n - 1]);
            pDisp->Flush();
        }
        pImp->aArr.clear();
    }
    else
    {
        SfxShellArr_Impl::iterator i = std::find(pImp->aArr.begin(), pImp->aArr.end(), pShell);
        if(i != pImp->aArr.end())
        {
            pImp->aArr.erase(i);
            if(pDisp->IsActive(*this))
            {
                pDisp->RemoveShell_Impl(*pShell);
                pDisp->Flush();
            }
        }
    }
}

SfxShell* SfxViewShell::GetSubShell( sal_uInt16 nNo )
{
    sal_uInt16 nCount = pImp->aArr.size();
    if(nNo < nCount)
        return pImp->aArr[nCount - nNo - 1];
    return nullptr;
}

void SfxViewShell::PushSubShells_Impl( bool bPush )
{
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( bPush )
    {
        for(SfxShellArr_Impl::const_iterator i = pImp->aArr.begin(); i != pImp->aArr.end(); ++i)
            pDisp->Push(**i);
    }
    else if(!pImp->aArr.empty())
    {
        SfxShell& rPopUntil = *pImp->aArr[0];
        if ( pDisp->GetShellLevel( rPopUntil ) != USHRT_MAX )
            pDisp->Pop( rPopUntil, SfxDispatcherPopFlags::POP_UNTIL );
    }

    pDisp->Flush();
}



void SfxViewShell::WriteUserData( OUString&, bool )
{
}



void SfxViewShell::ReadUserData(const OUString&, bool )
{
}

void SfxViewShell::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >&, bool )
{
}

void SfxViewShell::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >&, bool )
{
}



// returns the first shell of spec. type viewing the specified doc.
SfxViewShell* SfxViewShell::GetFirst
(
    bool          bOnlyVisible,
    std::function< bool ( const SfxViewShell* ) > isViewShell
)
{
    // search for a SfxViewShell of the specified type
    SfxViewShellArr_Impl &rShells = SfxGetpApp()->GetViewShells_Impl();
    SfxViewFrameArr_Impl &rFrames = SfxGetpApp()->GetViewFrames_Impl();
    for ( size_t nPos = 0; nPos < rShells.size(); ++nPos )
    {
        SfxViewShell *pShell = rShells[nPos];
        if ( pShell )
        {
            // sometimes dangling SfxViewShells exist that point to a dead SfxViewFrame
            // these ViewShells shouldn't be accessible anymore
            // a destroyed ViewFrame is not in the ViewFrame array anymore, so checking this array helps
            for ( size_t n=0; n<rFrames.size(); ++n )
            {
                SfxViewFrame *pFrame = rFrames[n];
                if ( pFrame == pShell->GetViewFrame() )
                {
                    // only ViewShells with a valid ViewFrame will be returned
                    if ( ( !bOnlyVisible || pFrame->IsVisible() ) && (!isViewShell || isViewShell(pShell)))
                        return pShell;
                    break;
                }
            }
        }
    }

    return nullptr;
}


// returns the next shell of spec. type viewing the specified doc.

SfxViewShell* SfxViewShell::GetNext
(
    const SfxViewShell& rPrev,
    bool                bOnlyVisible,
    std::function<bool ( const SfxViewShell* )> isViewShell
)
{
    SfxViewShellArr_Impl &rShells = SfxGetpApp()->GetViewShells_Impl();
    SfxViewFrameArr_Impl &rFrames = SfxGetpApp()->GetViewFrames_Impl();
    sal_uInt16 nPos;
    for ( nPos = 0; nPos < rShells.size(); ++nPos )
        if ( rShells[nPos] == &rPrev )
            break;

    for ( ++nPos; nPos < rShells.size(); ++nPos )
    {
        SfxViewShell *pShell = rShells[nPos];
        if ( pShell )
        {
            // sometimes dangling SfxViewShells exist that point to a dead SfxViewFrame
            // these ViewShells shouldn't be accessible anymore
            // a destroyed ViewFrame is not in the ViewFrame array anymore, so checking this array helps
            for ( size_t n=0; n<rFrames.size(); ++n )
            {
                SfxViewFrame *pFrame = rFrames[n];
                if ( pFrame == pShell->GetViewFrame() )
                {
                    // only ViewShells with a valid ViewFrame will be returned
                    if ( ( !bOnlyVisible || pFrame->IsVisible() ) && (!isViewShell || isViewShell(pShell)) )
                        return pShell;
                    break;
                }
            }
        }
    }

    return nullptr;
}



void SfxViewShell::Notify( SfxBroadcaster& rBC,
                            const SfxHint& rHint )
{
    const SfxEventHint* pEventHint = dynamic_cast<const SfxEventHint*>(&rHint);
    if ( pEventHint )
    {
        switch ( pEventHint->GetEventId() )
        {
            case SFX_EVENT_LOADFINISHED:
            {
                if ( GetController().is() )
                {
                    // avoid access to dangling ViewShells
                    SfxViewFrameArr_Impl &rFrames = SfxGetpApp()->GetViewFrames_Impl();
                    for ( size_t n=0; n<rFrames.size(); ++n )
                    {
                        SfxViewFrame *frame = rFrames[n];
                        if ( frame == GetViewFrame() && &rBC == GetObjectShell() )
                        {
                            SfxItemSet* pSet = GetObjectShell()->GetMedium()->GetItemSet();
                            const SfxUnoAnyItem* pItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pSet, SID_VIEW_DATA, false);
                            if ( pItem )
                            {
                                pImp->m_pController->restoreViewData( pItem->GetValue() );
                                pSet->ClearItem( SID_VIEW_DATA );
                            }

                            break;
                        }
                    }
                }

                break;
            }
        }
    }
}

bool SfxViewShell::ExecKey_Impl(const KeyEvent& aKey)
{
    if (!pImp->m_xAccExec.get())
    {
        pImp->m_xAccExec = ::svt::AcceleratorExecute::createAcceleratorHelper();
        pImp->m_xAccExec->init(::comphelper::getProcessComponentContext(),
            pFrame->GetFrame().GetFrameInterface());
    }

    return pImp->m_xAccExec->execute(aKey.GetKeyCode());
}

void SfxViewShell::registerLibreOfficeKitViewCallback(LibreOfficeKitCallback pCallback, void* pData)
{
    pImp->m_pLibreOfficeKitViewCallback = pCallback;
    pImp->m_pLibreOfficeKitViewData = pData;
}

void SfxViewShell::libreOfficeKitViewCallback(int nType, const char* pPayload) const
{
    if (pImp->m_pLibreOfficeKitViewCallback)
        pImp->m_pLibreOfficeKitViewCallback(nType, pPayload, pImp->m_pLibreOfficeKitViewData);
}

bool SfxViewShell::KeyInput( const KeyEvent &rKeyEvent )

/*  [Description]

    This Method executes the KeyEvent 'rKeyEvent' of the Keys (Accelerator)
    configured either direct or indirect (for example by the Application)
    in the SfxViewShell.

    [Return value]

    bool                    TRUE
                            The Key (Accelerator) is configured and the
                            the associated Handler was called

                            FALSE
                            The Key (Accelerator) is not configured and
                            subsequently no Handler was called

    [Cross-reference]

    <SfxApplication::KeyInput(const KeyEvent&)>
*/
{
    return ExecKey_Impl(rKeyEvent);
}

bool SfxViewShell::GlobalKeyInput_Impl( const KeyEvent &rKeyEvent )
{
    return ExecKey_Impl(rKeyEvent);
}



void SfxViewShell::ShowCursor( bool /*bOn*/ )

/*  [Description]

    Subclasses must override this Method so that SFx can switch the
    Cursor on and off, for example while a <SfxProgress> is running.
*/

{
}



void SfxViewShell::ResetAllClients_Impl( SfxInPlaceClient *pIP )
{

    SfxInPlaceClientList *pClients = pImp->GetIPClientList_Impl(false);
    if ( !pClients )
        return;

    for ( size_t n = 0; n < pClients->size(); n++ )
    {
        SfxInPlaceClient* pIPClient = pClients->at( n );
        if( pIPClient != pIP )
            pIPClient->ResetObject();
    }
}



void SfxViewShell::DisconnectAllClients()
{
    SfxInPlaceClientList *pClients = pImp->GetIPClientList_Impl(false);
    if ( !pClients )
        return;

    for ( size_t n = 0; n < pClients->size(); )
        // clients will remove themselves from the list
        delete pClients->at( n );
}



void SfxViewShell::QueryObjAreaPixel( Rectangle& ) const
{
}



void SfxViewShell::VisAreaChanged(const Rectangle& /*rVisArea*/)
{
    SfxInPlaceClientList *pClients = pImp->GetIPClientList_Impl(false);
    if ( !pClients )
        return;

    for ( size_t n = 0; n < pClients->size(); n++)
    {
        SfxInPlaceClient* pIPClient = pClients->at( n );
        if ( pIPClient->IsObjectInPlaceActive() )
            // client is active, notify client that the VisArea might have changed
            pIPClient->VisAreaChanged();
    }
}


void SfxViewShell::CheckIPClient_Impl( SfxInPlaceClient *pIPClient, const Rectangle& rVisArea )
{
    if ( GetObjectShell()->IsInClose() )
        return;

    bool bAlwaysActive =
        ( ( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) != 0 );
    bool bActiveWhenVisible =
        ( (( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE ) != 0 ) ||
         svt::EmbeddedObjectRef::IsGLChart(pIPClient->GetObject()));

    // this method is called when either a client is created or the "Edit/Plugins" checkbox is checked
    if ( !pIPClient->IsObjectInPlaceActive() && pImp->m_bPlugInsActive )
    {
        // object in client is currently not active
        // check if the object wants to be activated always or when it becomes at least partially visible
        // TODO/LATER: maybe we should use the scaled area instead of the ObjArea?!
        if ( bAlwaysActive || (bActiveWhenVisible && rVisArea.IsOver(pIPClient->GetObjArea())) )
        {
            try
            {
                pIPClient->GetObject()->changeState( embed::EmbedStates::INPLACE_ACTIVE );
            }
            catch (const uno::Exception&)
            {
            }
        }
    }
    else if (!pImp->m_bPlugInsActive)
    {
        // object in client is currently active and "Edit/Plugins" checkbox is selected
        // check if the object wants to be activated always or when it becomes at least partially visible
        // in this case selecting of the "Edit/Plugin" checkbox should let such objects deactivate
        if ( bAlwaysActive || bActiveWhenVisible )
            pIPClient->GetObject()->changeState( embed::EmbedStates::RUNNING );
    }
}


void SfxViewShell::DiscardClients_Impl()

/*  [Description]

    The purpose of this Method is to prevent the saving of Objects when closing
    the Document, if the user has chosen to close without saving.
*/

{
    SfxInPlaceClientList *pClients = pImp->GetIPClientList_Impl(false);
    if ( !pClients )
        return;

    for ( size_t n = 0; n < pClients->size(); )
        delete pClients->at( n );
}



SfxObjectShell* SfxViewShell::GetObjectShell()
{
    return pFrame ? pFrame->GetObjectShell() : nullptr;
}



Reference< XModel > SfxViewShell::GetCurrentDocument() const
{
    Reference< XModel > xDocument;

    const SfxObjectShell* pDocShell( const_cast< SfxViewShell* >( this )->GetObjectShell() );
    OSL_ENSURE( pDocShell, "SfxViewFrame::GetCurrentDocument: no DocShell!?" );
    if ( pDocShell )
        xDocument = pDocShell->GetModel();
    return xDocument;
}



void SfxViewShell::SetCurrentDocument() const
{
    uno::Reference< frame::XModel > xDocument( GetCurrentDocument() );
    if ( xDocument.is() )
        SfxObjectShell::SetCurrentComponent( xDocument );
}



const Size& SfxViewShell::GetMargin() const
{
    return pImp->aMargin;
}



void SfxViewShell::SetMargin( const Size& rSize )
{
    // the default margin was verified using www.apple.com !!
    Size aMargin = rSize;
    if ( aMargin.Width() == -1 )
        aMargin.Width() = DEFAULT_MARGIN_WIDTH;
    if ( aMargin.Height() == -1 )
        aMargin.Height() = DEFAULT_MARGIN_HEIGHT;

    if ( aMargin != pImp->aMargin )
    {
        pImp->aMargin = aMargin;
        MarginChanged();
    }
}

void SfxViewShell::MarginChanged()
{
}

bool SfxViewShell::IsShowView_Impl() const
{
    return pImp->m_bIsShowView;
}

void SfxViewShell::JumpToMark( const OUString& rMark )
{
    SfxStringItem aMarkItem( SID_JUMPTOMARK, rMark );
    GetViewFrame()->GetDispatcher()->Execute(
        SID_JUMPTOMARK,
        SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
        &aMarkItem, 0L );
}

void SfxViewShell::SetController( SfxBaseController* pController )
{
    pImp->m_pController = pController;
    pImp->m_bControllerSet = true;

    // there should be no old listener, but if there is one, it should be disconnected
    if (  pImp->xClipboardListener.is() )
        pImp->xClipboardListener->DisconnectViewShell();

    pImp->xClipboardListener = new SfxClipboardChangeListener( this, GetClipboardNotifier() );
}

Reference < XController > SfxViewShell::GetController()
{
    return pImp->m_pController.get();
}

SfxBaseController* SfxViewShell::GetBaseController_Impl() const
{
    return pImp->m_pController.get();
}

void SfxViewShell::AddContextMenuInterceptor_Impl( const uno::Reference< ui::XContextMenuInterceptor >& xInterceptor )
{
    pImp->aInterceptorContainer.addInterface( xInterceptor );
}

void SfxViewShell::RemoveContextMenuInterceptor_Impl( const uno::Reference< ui::XContextMenuInterceptor >& xInterceptor )
{
    pImp->aInterceptorContainer.removeInterface( xInterceptor );
}

void Change( Menu* pMenu, SfxViewShell* pView )
{
    SfxDispatcher *pDisp = pView->GetViewFrame()->GetDispatcher();
    sal_uInt16 nCount = pMenu->GetItemCount();
    for ( sal_uInt16 nPos=0; nPos<nCount; ++nPos )
    {
        sal_uInt16 nId = pMenu->GetItemId(nPos);
        OUString aCmd = pMenu->GetItemCommand(nId);
        PopupMenu* pPopup = pMenu->GetPopupMenu(nId);
        if ( pPopup )
        {
            Change( pPopup, pView );
        }
        else if ( nId < 5000 )
        {
            if ( aCmd.startsWith(".uno:") )
            {
                for (sal_uInt16 nIdx=0;;)
                {
                    SfxShell *pShell=pDisp->GetShell(nIdx++);
                    if (pShell == nullptr)
                        break;
                    const SfxInterface *pIFace = pShell->GetInterface();
                    const SfxSlot* pSlot = pIFace->GetSlot( aCmd );
                    if ( pSlot )
                    {
                        pMenu->InsertItem( pSlot->GetSlotId(), pMenu->GetItemText( nId ),
                            pMenu->GetItemBits( nId ), OString(), nPos );
                        pMenu->SetItemCommand( pSlot->GetSlotId(), aCmd );
                        pMenu->RemoveItem( nPos+1 );
                        break;
                    }
                }
            }
        }
    }
}


bool SfxViewShell::TryContextMenuInterception( Menu& rIn, const OUString& rMenuIdentifier, Menu*& rpOut, ui::ContextMenuExecuteEvent aEvent )
{
    rpOut = nullptr;
    bool bModified = false;

    // create container from menu
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
        &rIn, &rMenuIdentifier );

    // get selection from controller
    aEvent.Selection.set( GetController(), uno::UNO_QUERY );

    // call interceptors
    ::cppu::OInterfaceIteratorHelper aIt( pImp->aInterceptorContainer );
    while( aIt.hasMoreElements() )
    {
        try
        {
            ui::ContextMenuInterceptorAction eAction;
            {
                SolarMutexReleaser rel;
                eAction = static_cast<ui::XContextMenuInterceptor*>(aIt.next())->notifyContextMenuExecute( aEvent );
            }
            switch ( eAction )
            {
                case ui::ContextMenuInterceptorAction_CANCELLED :
                    // interceptor does not want execution
                    return false;
                case ui::ContextMenuInterceptorAction_EXECUTE_MODIFIED :
                    // interceptor wants his modified menu to be executed
                    bModified = true;
                    break;
                case ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED :
                    // interceptor has modified menu, but allows for calling other interceptors
                    bModified = true;
                    continue;
                case ui::ContextMenuInterceptorAction_IGNORED :
                    // interceptor is indifferent
                    continue;
                default:
                    OSL_FAIL("Wrong return value of ContextMenuInterceptor!");
                    continue;
            }
        }
        catch (...)
        {
            aIt.remove();
        }

        break;
    }

    if ( bModified )
    {
        // container was modified, create a new window out of it
        rpOut = new PopupMenu;
        ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer( rpOut, aEvent.ActionTriggerContainer );

        Change( rpOut, this );
    }

    return true;
}

bool SfxViewShell::TryContextMenuInterception( Menu& rMenu, const OUString& rMenuIdentifier, css::ui::ContextMenuExecuteEvent aEvent )
{
    bool bModified = false;

    // create container from menu
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu( &rMenu, &rMenuIdentifier );

    // get selection from controller
    aEvent.Selection = css::uno::Reference< css::view::XSelectionSupplier >( GetController(), css::uno::UNO_QUERY );

    // call interceptors
    ::cppu::OInterfaceIteratorHelper aIt( pImp->aInterceptorContainer );
    while( aIt.hasMoreElements() )
    {
        try
        {
            css::ui::ContextMenuInterceptorAction eAction;
            {
                SolarMutexReleaser rel;
                eAction = static_cast< css::ui::XContextMenuInterceptor* >( aIt.next() )->notifyContextMenuExecute( aEvent );
            }
            switch ( eAction )
            {
                case css::ui::ContextMenuInterceptorAction_CANCELLED:
                    // interceptor does not want execution
                    return false;
                case css::ui::ContextMenuInterceptorAction_EXECUTE_MODIFIED:
                    // interceptor wants his modified menu to be executed
                    bModified = true;
                    break;
                case css::ui::ContextMenuInterceptorAction_CONTINUE_MODIFIED:
                    // interceptor has modified menu, but allows for calling other interceptors
                    bModified = true;
                    continue;
                case css::ui::ContextMenuInterceptorAction_IGNORED:
                    // interceptor is indifferent
                    continue;
                default:
                    SAL_WARN( "sfx.view", "Wrong return value of ContextMenuInterceptor!" );
                    continue;
            }
        }
        catch (...)
        {
            aIt.remove();
        }

        break;
    }

    if ( bModified )
    {
        rMenu.Clear();
        ::framework::ActionTriggerHelper::CreateMenuFromActionTriggerContainer( &rMenu, aEvent.ActionTriggerContainer );
    }

    return true;
}

void SfxViewShell::TakeOwnership_Impl()
{
    // currently there is only one reason to take Ownership: a hidden frame is printed
    // so the ViewShell will check this on EndPrint (->prnmon.cxx)
    pImp->m_bGotOwnership = true;
}

void SfxViewShell::TakeFrameOwnership_Impl()
{
    // currently there is only one reason to take Ownership: a hidden frame is printed
    // so the ViewShell will check this on EndPrint (->prnmon.cxx)
    pImp->m_bGotFrameOwnership = true;
}

bool SfxViewShell::HandleNotifyEvent_Impl( NotifyEvent& rEvent )
{
    if (pImp->m_pController.is())
        return pImp->m_pController->HandleEvent_Impl( rEvent );
    return false;
}

bool SfxViewShell::HasKeyListeners_Impl()
{
    return (pImp->m_pController.is())
        && pImp->m_pController->HasKeyListeners_Impl();
}

bool SfxViewShell::HasMouseClickListeners_Impl()
{
    return (pImp->m_pController.is())
        && pImp->m_pController->HasMouseClickListeners_Impl();
}

bool SfxViewShell::Escape()
{
    return GetViewFrame()->GetBindings().Execute( SID_TERMINATE_INPLACEACTIVATION );
}

Reference< view::XRenderable > SfxViewShell::GetRenderable()
{
    Reference< view::XRenderable >xRender;
    SfxObjectShell* pObj = GetObjectShell();
    if( pObj )
    {
        Reference< frame::XModel > xModel( pObj->GetModel() );
        if( xModel.is() )
            xRender.set( xModel, UNO_QUERY );
    }
    return xRender;
}

uno::Reference< datatransfer::clipboard::XClipboardNotifier > SfxViewShell::GetClipboardNotifier()
{
    uno::Reference< datatransfer::clipboard::XClipboardNotifier > xClipboardNotifier;
    if ( GetViewFrame() )
        xClipboardNotifier.set( GetViewFrame()->GetWindow().GetClipboard(), uno::UNO_QUERY );

    return xClipboardNotifier;
}

void SfxViewShell::AddRemoveClipboardListener( const uno::Reference < datatransfer::clipboard::XClipboardListener >& rClp, bool bAdd )
{
    try
    {
        if ( GetViewFrame() )
        {
            uno::Reference< datatransfer::clipboard::XClipboard > xClipboard( GetViewFrame()->GetWindow().GetClipboard() );
            if( xClipboard.is() )
            {
                uno::Reference< datatransfer::clipboard::XClipboardNotifier > xClpbrdNtfr( xClipboard, uno::UNO_QUERY );
                if( xClpbrdNtfr.is() )
                {
                    if( bAdd )
                        xClpbrdNtfr->addClipboardListener( rClp );
                    else
                        xClpbrdNtfr->removeClipboardListener( rClp );
                }
            }
        }
    }
    catch (const uno::Exception&)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/log.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/weld.hxx>
#include <svl/intitem.hxx>
#include <svtools/langhelp.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <cppuhelper/implbase.hxx>

#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/pathoptions.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/soerr.hxx>
#include <svtools/embedhlp.hxx>

#include <basic/basmgr.hxx>
#include <basic/sbuno.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandinfoprovider.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <officecfg/Setup.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewsh.hxx>
#include "viewimp.hxx"
#include <sfx2/sfxresid.hxx>
#include <sfx2/request.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <arrdecl.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxbasecontroller.hxx>
#include <sfx2/mailmodelapi.hxx>
#include <bluthsndapi.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/event.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/ipclient.hxx>
#include <workwin.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/lokhelper.hxx>
#include <openuriexternally.hxx>
#include <shellimpl.hxx>

#include <vector>
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::cppu;

#define ShellClass_SfxViewShell
#include <sfxslots.hxx>


class SfxClipboardChangeListener : public ::cppu::WeakImplHelper<
    datatransfer::clipboard::XClipboardListener >
{
public:
    SfxClipboardChangeListener( SfxViewShell* pView, const uno::Reference< datatransfer::clipboard::XClipboardNotifier >& xClpbrdNtfr );

    // XEventListener
    virtual void SAL_CALL disposing( const lang::EventObject& rEventObject ) override;

    // XClipboardListener
    virtual void SAL_CALL changedContents( const datatransfer::clipboard::ClipboardEvent& rEventObject ) override;

    void DisconnectViewShell() { m_pViewShell = nullptr; }
    void ChangedContents();

    enum AsyncExecuteCmd
    {
        ASYNCEXECUTE_CMD_DISPOSING,
        ASYNCEXECUTE_CMD_CHANGEDCONTENTS
    };

    struct AsyncExecuteInfo
    {
        AsyncExecuteInfo( AsyncExecuteCmd eCmd, SfxClipboardChangeListener* pListener ) :
            m_eCmd( eCmd ), m_xListener( pListener ) {}

        AsyncExecuteCmd const m_eCmd;
        rtl::Reference<SfxClipboardChangeListener> m_xListener;
    };

private:
    SfxViewShell* m_pViewShell;
    uno::Reference< datatransfer::clipboard::XClipboardNotifier > m_xClpbrdNtfr;
    uno::Reference< lang::XComponent > m_xCtrl;

    DECL_STATIC_LINK( SfxClipboardChangeListener, AsyncExecuteHdl_Impl, void*, void );
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

void SfxClipboardChangeListener::ChangedContents()
{
    const SolarMutexGuard aGuard;
    if (!m_pViewShell)
        return;

    SfxBindings& rBind = m_pViewShell->GetViewFrame()->GetBindings();
    rBind.Invalidate(SID_PASTE);
    rBind.Invalidate(SID_PASTE_SPECIAL);
    rBind.Invalidate(SID_CLIPBOARD_FORMAT_ITEMS);

    if (comphelper::LibreOfficeKit::isActive())
    {
        // In the future we might send the payload as well.
        SfxLokHelper::notifyAllViews(LOK_CALLBACK_CLIPBOARD_CHANGED, "");
    }
}

IMPL_STATIC_LINK( SfxClipboardChangeListener, AsyncExecuteHdl_Impl, void*, p, void )
{
    AsyncExecuteInfo* pAsyncExecuteInfo = static_cast<AsyncExecuteInfo*>(p);
    if ( pAsyncExecuteInfo )
    {
        if ( pAsyncExecuteInfo->m_xListener.is() )
        {
            if ( pAsyncExecuteInfo->m_eCmd == ASYNCEXECUTE_CMD_DISPOSING )
                pAsyncExecuteInfo->m_xListener->DisconnectViewShell();
            else if ( pAsyncExecuteInfo->m_eCmd == ASYNCEXECUTE_CMD_CHANGEDCONTENTS )
                pAsyncExecuteInfo->m_xListener->ChangedContents();
        }
    }
    delete pAsyncExecuteInfo;
}

void SAL_CALL SfxClipboardChangeListener::disposing( const lang::EventObject& /*rEventObject*/ )
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
    AsyncExecuteInfo* pInfo = new AsyncExecuteInfo( ASYNCEXECUTE_CMD_DISPOSING, this );
    Application::PostUserEvent( LINK( nullptr, SfxClipboardChangeListener, AsyncExecuteHdl_Impl ), pInfo );
}

void SAL_CALL SfxClipboardChangeListener::changedContents( const datatransfer::clipboard::ClipboardEvent& )
{
    // Make asynchronous call to avoid locking SolarMutex which is the
    // root for many deadlocks, especially in conjunction with the "Windows"
    // based single thread apartment clipboard code!
    AsyncExecuteInfo* pInfo = new AsyncExecuteInfo( ASYNCEXECUTE_CMD_CHANGEDCONTENTS, this );
    Application::PostUserEvent( LINK( nullptr, SfxClipboardChangeListener, AsyncExecuteHdl_Impl ), pInfo );
}

sal_uInt32 SfxViewShell_Impl::m_nLastViewShellId = 0;

SfxViewShell_Impl::SfxViewShell_Impl(SfxViewShellFlags const nFlags)
: aInterceptorContainer( aMutex )
,   m_bHasPrintOptions(nFlags & SfxViewShellFlags::HAS_PRINTOPTIONS)
,   m_nFamily(0xFFFF)   // undefined, default set by TemplateDialog
,   m_pLibreOfficeKitViewCallback(nullptr)
,   m_pLibreOfficeKitViewData(nullptr)
,   m_bTiledSearching(false)
,   m_nViewShellId(SfxViewShell_Impl::m_nLastViewShellId++)
{}

SfxViewShell_Impl::~SfxViewShell_Impl()
{
}

std::vector< SfxInPlaceClient* > *SfxViewShell_Impl::GetIPClients_Impl( bool bCreate ) const
{
    if (!mpIPClients && bCreate)
        mpIPClients.reset(new std::vector< SfxInPlaceClient* >);
    return mpIPClients.get();
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

static OUString impl_searchFormatTypeForApp(const css::uno::Reference< css::frame::XFrame >& xFrame     ,
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
                    sType = "writer_MS_Word_2007";
                else
                if ( sModule == "com.sun.star.sheet.SpreadsheetDocument" )
                    sType = "MS Excel 2007 XML";
                else
                if ( sModule == "com.sun.star.presentation.PresentationDocument" )
                    sType = "MS PowerPoint 2007 XML";
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
    pImpl->GetIPClients_Impl()->push_back(pIPClient);
}

void SfxViewShell::IPClientGone_Impl( SfxInPlaceClient const *pIPClient )
{
    std::vector< SfxInPlaceClient* > *pClients = pImpl->GetIPClients_Impl();

    auto it = std::find(pClients->begin(), pClients->end(), pIPClient);
    if (it != pClients->end())
        pClients->erase( it );
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
                pImpl->m_nFamily = pItem->GetValue();
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
                        uno::Reference< ui::XUIElement > xElement = xLayoutManager->getElement( "private:resource/toolbar/textobjectbar" );
                        if(!xElement.is())
                        {
                            xElement = xLayoutManager->getElement( "private:resource/toolbar/frameobjectbar" );
                        }
                        if(!xElement.is())
                        {
                            xElement = xLayoutManager->getElement( "private:resource/toolbar/oleobjectbar" );
                        }
                        if(xElement.is())
                        {
                            uno::Reference< awt::XWindow > xWin( xElement->getRealInterface(), uno::UNO_QUERY_THROW );
                            VclPtr<vcl::Window> pWin = VCLUnoHelper::GetWindow( xWin );
                            ToolBox* pTextToolbox = dynamic_cast< ToolBox* >( pWin.get() );
                            if( pTextToolbox )
                            {
                                ToolBox::ImplToolItems::size_type nItemCount = pTextToolbox->GetItemCount();
                                for( ToolBox::ImplToolItems::size_type nItem = 0; nItem < nItemCount; ++nItem )
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

        case SID_MAIL_SENDDOCASMS:
        case SID_MAIL_SENDDOCASOOO:
        case SID_MAIL_SENDDOCASPDF:
        case SID_MAIL_SENDDOC:
        case SID_MAIL_SENDDOCASFORMAT:
        {
            SfxObjectShell* pDoc = GetObjectShell();
            if ( pDoc && pDoc->QueryHiddenInformation(
                             HiddenWarningFact::WhenSaving, GetViewFrame()->GetWindow().GetFrameWeld() ) != RET_YES )
                break;


            SfxMailModel  aModel;
            OUString aDocType;

            const SfxStringItem* pMailRecipient = rReq.GetArg<SfxStringItem>(SID_MAIL_RECIPIENT);
            if ( pMailRecipient )
            {
                OUString aRecipient( pMailRecipient->GetValue() );
                OUString aMailToStr("mailto:");

                if ( aRecipient.startsWith( aMailToStr ) )
                    aRecipient = aRecipient.copy( aMailToStr.getLength() );
                aModel.AddToAddress( aRecipient );
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
                vcl::Window* pWin = SfxGetpApp()->GetTopWindow();
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                                         SfxResId(STR_ERROR_SEND_MAIL)));
                xBox->run();
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
                            HiddenWarningFact::WhenSaving, GetViewFrame()->GetWindow().GetFrameWeld() ) != RET_YES )
                break;
            uno::Reference < frame::XFrame > xFrame( pFrame->GetFrame().GetFrameInterface() );
            SfxMailModel::SendMailResult eResult = aModel.SaveAndSend( xFrame );
            if( eResult == SfxMailModel::SEND_MAIL_ERROR )
            {
                vcl::Window* pWin = SfxGetpApp()->GetTopWindow();
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                                         VclMessageType::Info, VclButtonsType::Ok,
                                                                         SfxResId(STR_ERROR_SEND_MAIL)));
                xBox->run();
                rReq.Ignore();
            }
            else
                rReq.Done();
        }
        break;

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        case SID_WEBHTML:
        {
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
                OUString aTypeName( "generic_HTML" );
                OUString aFileName;

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
                        xContainerQuery, "graphic_HTML", aModule, nFilterFlags );
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
                    aFileName = aFObj.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::NONE );
                }

                OSL_ASSERT( !aFilterName.isEmpty() );
                OSL_ASSERT( !aFileName.isEmpty() );

                // Creates a temporary directory to store our predefined file into it.
                ::utl::TempFile aTempDir( nullptr, true );

                INetURLObject aFilePathObj( aTempDir.GetURL() );
                aFilePathObj.insertName( aFileName );
                aFilePathObj.setExtension( "htm" );

                OUString aFileURL = aFilePathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

                css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
                aArgs[0].Name  = "FilterName";
                aArgs[0].Value <<= aFilterName;

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

                sfx2::openUriExternally(aFileURL, true);
                rReq.Done(true);
                break;
            }
            else
            {
                rReq.Done();
                return;
            }
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
                bool bEnabled = !Application::GetSettings().GetMiscSettings().GetDisablePrinting();
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
                            aBuffer.append( vcl::CommandInfoProvider::GetLabelForCommand(
                                ".uno:PrintDefault",
                                vcl::CommandInfoProvider::GetModuleIdentifier( xFrame ) ) );
                            aBuffer.append( " (" );
                            aBuffer.append( aPrinterName );
                            aBuffer.append(')');

                            rSet.Put( SfxStringItem( SID_PRINTDOCDIRECT, aBuffer.makeStringAndClear() ) );
                        }
                    }
                }
                break;
            }
            case SID_STYLE_FAMILY :
            {
                rSet.Put( SfxUInt16Item( SID_STYLE_FAMILY, pImpl->m_nFamily ) );
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


void SfxViewShell::OutplaceActivated( bool bActive )
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
    std::vector< SfxInPlaceClient* > *pClients = pImpl->GetIPClients_Impl(false);
    if ( !pClients )
        return nullptr;

    if( !pObjParentWin )
        pObjParentWin = GetWindow();
    for (SfxInPlaceClient* pIPClient : *pClients)
    {
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
    std::vector< SfxInPlaceClient* > *pClients = pImpl->GetIPClients_Impl(false);
    if ( !pClients )
        return nullptr;

    for (SfxInPlaceClient* pIPClient : *pClients)
    {
        if ( pIPClient->IsUIActive() )
            return pIPClient;
    }

    return nullptr;
}

SfxInPlaceClient* SfxViewShell::GetUIActiveClient() const
{
    std::vector< SfxInPlaceClient* > *pClients = pImpl->GetIPClients_Impl(false);
    if ( !pClients )
        return nullptr;

    const bool bIsTiledRendering = comphelper::LibreOfficeKit::isActive();

    for (SfxInPlaceClient* pIPClient : *pClients)
    {
        if ( pIPClient->IsObjectUIActive() || ( bIsTiledRendering && pIPClient->IsObjectInPlaceActive() ) )
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
    const Size&     /*rSize*/,      // All available sizes.
    bool
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
    if (pImpl->m_pController.is())
    {
        pImpl->m_pController->BorderWidthsChanged_Impl();
    }
}


void SfxViewShell::SetBorderPixel( const SvBorder &rBorder )
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    GetViewFrame()->SetBorderPixelImpl( this, rBorder );

    // notify related controller that border size is changed
    if (pImpl->m_pController.is())
    {
        pImpl->m_pController->BorderWidthsChanged_Impl();
    }
}


const SvBorder& SfxViewShell::GetBorderPixel() const
{
    DBG_ASSERT( GetViewFrame(), "SfxViewShell without SfxViewFrame" );

    return GetViewFrame()->GetBorderPixelImpl();
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
,   pImpl( new SfxViewShell_Impl(nFlags) )
,   pFrame(pViewFrame)
,   pWindow(nullptr)
,   bNoNewWindow( nFlags & SfxViewShellFlags::NO_NEWWINDOW )
,   mbPrinterSettingsModified(false)
,   maLOKLanguageTag("en-US", true)
{

    SetMargin( pViewFrame->GetMargin_Impl() );

    SetPool( &pViewFrame->GetObjectShell()->GetPool() );
    StartListening(*pViewFrame->GetObjectShell());

    // Insert into list
    SfxViewShellArr_Impl &rViewArr = SfxGetpApp()->GetViewShells_Impl();
    rViewArr.push_back(this);

    if (comphelper::LibreOfficeKit::isActive())
        pViewFrame->GetWindow().SetLOKNotifier(this, true);
}


SfxViewShell::~SfxViewShell()
{

    // Remove from list
    const SfxViewShell *pThis = this;
    SfxViewShellArr_Impl &rViewArr = SfxGetpApp()->GetViewShells_Impl();
    SfxViewShellArr_Impl::iterator it = std::find( rViewArr.begin(), rViewArr.end(), pThis );
    rViewArr.erase( it );

    if ( pImpl->xClipboardListener.is() )
    {
        pImpl->xClipboardListener->DisconnectViewShell();
        pImpl->xClipboardListener = nullptr;
    }

    if (pImpl->m_pController.is())
    {
        pImpl->m_pController->ReleaseShell_Impl();
        pImpl->m_pController.clear();
    }

    if (GetViewFrame()->GetWindow().GetLOKNotifier())
        GetViewFrame()->GetWindow().ReleaseLOKNotifier();
}

bool SfxViewShell::PrepareClose
(
    bool bUI     // TRUE: Allow Dialog and so on, FALSE: silent-mode
)
{
    if (GetViewFrame()->GetWindow().GetLOKNotifier())
        GetViewFrame()->GetWindow().ReleaseLOKNotifier();

    SfxPrinter *pPrinter = GetPrinter();
    if ( pPrinter && pPrinter->IsPrinting() )
    {
        if ( bUI )
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetViewFrame()->GetWindow().GetFrameWeld(),
                                                                     VclMessageType::Info, VclButtonsType::Ok,
                                                                     SfxResId(STR_CANT_CLOSE)));
            xBox->run();
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

    This virtual Method has to be overloaded by the sub classes, to be able
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
    pImpl->aArr.push_back(&rShell);
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
        size_t nCount = pImpl->aArr.size();
        if ( pDisp->IsActive(*this) )
        {
            for(size_t n = nCount; n > 0; --n)
                pDisp->Pop(*pImpl->aArr[n - 1]);
            pDisp->Flush();
        }
        pImpl->aArr.clear();
    }
    else
    {
        SfxShellArr_Impl::iterator i = std::find(pImpl->aArr.begin(), pImpl->aArr.end(), pShell);
        if(i != pImpl->aArr.end())
        {
            pImpl->aArr.erase(i);
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
    sal_uInt16 nCount = pImpl->aArr.size();
    if(nNo < nCount)
        return pImpl->aArr[nCount - nNo - 1];
    return nullptr;
}

void SfxViewShell::PushSubShells_Impl( bool bPush )
{
    SfxDispatcher *pDisp = pFrame->GetDispatcher();
    if ( bPush )
    {
        for (auto const& elem : pImpl->aArr)
            pDisp->Push(*elem);
    }
    else if(!pImpl->aArr.empty())
    {
        SfxShell& rPopUntil = *pImpl->aArr[0];
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

void SfxViewShell::ReadUserDataSequence ( const uno::Sequence < beans::PropertyValue >& )
{
}

void SfxViewShell::WriteUserDataSequence ( uno::Sequence < beans::PropertyValue >& )
{
}


// returns the number of current available shells of spec. type viewing the specified doc.
size_t SfxViewShell::GetActiveShells ( bool bOnlyVisible )
{
    size_t nShells = 0;

    // search for a SfxViewShell of the specified type
    SfxViewShellArr_Impl &rShells = SfxGetpApp()->GetViewShells_Impl();
    SfxViewFrameArr_Impl &rFrames = SfxGetpApp()->GetViewFrames_Impl();
    for (SfxViewShell* pShell : rShells)
    {
        if ( pShell )
        {
            // sometimes dangling SfxViewShells exist that point to a dead SfxViewFrame
            // these ViewShells shouldn't be accessible anymore
            // a destroyed ViewFrame is not in the ViewFrame array anymore, so checking this array helps
            for (SfxViewFrame* pFrame : rFrames)
            {
                if ( pFrame == pShell->GetViewFrame() )
                {
                    // only ViewShells with a valid ViewFrame will be returned
                    if ( !bOnlyVisible || pFrame->IsVisible() )
                        ++nShells;
                }
            }
        }
    }

    return nShells;
}


// returns the first shell of spec. type viewing the specified doc.
SfxViewShell* SfxViewShell::GetFirst
(
    bool          bOnlyVisible,
    const std::function< bool ( const SfxViewShell* ) >& isViewShell
)
{
    // search for a SfxViewShell of the specified type
    SfxViewShellArr_Impl &rShells = SfxGetpApp()->GetViewShells_Impl();
    SfxViewFrameArr_Impl &rFrames = SfxGetpApp()->GetViewFrames_Impl();
    for (SfxViewShell* pShell : rShells)
    {
        if ( pShell )
        {
            // sometimes dangling SfxViewShells exist that point to a dead SfxViewFrame
            // these ViewShells shouldn't be accessible anymore
            // a destroyed ViewFrame is not in the ViewFrame array anymore, so checking this array helps
            for (SfxViewFrame* pFrame : rFrames)
            {
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
    const std::function<bool ( const SfxViewShell* )>& isViewShell
)
{
    SfxViewShellArr_Impl &rShells = SfxGetpApp()->GetViewShells_Impl();
    SfxViewFrameArr_Impl &rFrames = SfxGetpApp()->GetViewFrames_Impl();
    size_t nPos;
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
            for (SfxViewFrame* pFrame : rFrames)
            {
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
    if ( !(pEventHint && pEventHint->GetEventId() == SfxEventHintId::LoadFinished) )
        return;

    if ( !GetController().is() )
        return;

    // avoid access to dangling ViewShells
    SfxViewFrameArr_Impl &rFrames = SfxGetpApp()->GetViewFrames_Impl();
    for (SfxViewFrame* frame : rFrames)
    {
        if ( frame == GetViewFrame() && &rBC == GetObjectShell() )
        {
            SfxItemSet* pSet = GetObjectShell()->GetMedium()->GetItemSet();
            const SfxUnoAnyItem* pItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pSet, SID_VIEW_DATA, false);
            if ( pItem )
            {
                pImpl->m_pController->restoreViewData( pItem->GetValue() );
                pSet->ClearItem( SID_VIEW_DATA );
            }
            break;
        }
    }
}

bool SfxViewShell::ExecKey_Impl(const KeyEvent& aKey)
{
    if (!pImpl->m_xAccExec)
    {
        pImpl->m_xAccExec = ::svt::AcceleratorExecute::createAcceleratorHelper();
        pImpl->m_xAccExec->init(::comphelper::getProcessComponentContext(),
            pFrame->GetFrame().GetFrameInterface());
    }

    return pImpl->m_xAccExec->execute(aKey.GetKeyCode());
}

void SfxViewShell::registerLibreOfficeKitViewCallback(LibreOfficeKitCallback pCallback, void* pData)
{
    pImpl->m_pLibreOfficeKitViewCallback = pCallback;
    pImpl->m_pLibreOfficeKitViewData = pData;

    afterCallbackRegistered();

    // Ask other views to tell us about their cursors.
    SfxViewShell* pViewShell = SfxViewShell::GetFirst();
    while (pViewShell)
    {
        pViewShell->NotifyCursor(this);
        pViewShell = SfxViewShell::GetNext(*pViewShell);
    }
}

void SfxViewShell::libreOfficeKitViewCallback(int nType, const char* pPayload) const
{
    if (comphelper::LibreOfficeKit::isTiledPainting())
        return;

    if (pImpl->m_bTiledSearching)
    {
        switch (nType)
        {
        case LOK_CALLBACK_TEXT_SELECTION:
        case LOK_CALLBACK_TEXT_VIEW_SELECTION:
        case LOK_CALLBACK_TEXT_SELECTION_START:
        case LOK_CALLBACK_TEXT_SELECTION_END:
        case LOK_CALLBACK_GRAPHIC_SELECTION:
        case LOK_CALLBACK_GRAPHIC_VIEW_SELECTION:
            return;
        }
    }

    if (pImpl->m_pLibreOfficeKitViewCallback)
        pImpl->m_pLibreOfficeKitViewCallback(nType, pPayload, pImpl->m_pLibreOfficeKitViewData);
}

void SfxViewShell::afterCallbackRegistered()
{
}

vcl::Window* SfxViewShell::GetEditWindowForActiveOLEObj() const
{
    vcl::Window* pEditWin = nullptr;
    SfxInPlaceClient* pIPClient = GetIPClient();
    if (pIPClient)
    {
        pEditWin = pIPClient->GetEditWin();
    }
    return pEditWin;
}

void SfxViewShell::SetLOKLanguageTag(const OUString& rBcp47LanguageTag)
{
    LanguageTag aTag = LanguageTag(rBcp47LanguageTag, true);

    css::uno::Sequence<OUString> inst(officecfg::Setup::Office::InstalledLocales::get()->getElementNames());
    LanguageTag aFallbackTag = LanguageTag(getInstalledLocaleForSystemUILanguage(inst, /* bRequestInstallIfMissing */ false, rBcp47LanguageTag), true).makeFallback();

    // If we want de-CH, and the de localisation is available, we don't want to use de-DE as then
    // the magic in Translate::get() won't turn ess-zet into double s. Possibly other similar cases?
    if (comphelper::LibreOfficeKit::isActive() && aTag.getLanguage() == aFallbackTag.getLanguage())
        maLOKLanguageTag = aTag;
    else
        maLOKLanguageTag = aFallbackTag;
}

void SfxViewShell::NotifyCursor(SfxViewShell* /*pViewShell*/) const
{
}

void SfxViewShell::setTiledSearching(bool bTiledSearching)
{
    pImpl->m_bTiledSearching = bTiledSearching;
}

int SfxViewShell::getPart() const
{
    return 0;
}

ViewShellId SfxViewShell::GetViewShellId() const
{
    return pImpl->m_nViewShellId;
}

void SfxViewShell::NotifyOtherViews(int nType, const OString& rKey, const OString& rPayload)
{
    SfxLokHelper::notifyOtherViews(this, nType, rKey, rPayload);
}

void SfxViewShell::NotifyOtherView(OutlinerViewShell* pOther, int nType, const OString& rKey, const OString& rPayload)
{
    auto pOtherShell = dynamic_cast<SfxViewShell*>(pOther);
    if (!pOtherShell)
        return;

    SfxLokHelper::notifyOtherView(this, pOtherShell, nType, rKey, rPayload);
}

void SfxViewShell::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SfxViewShell"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("id"), BAD_CAST(OString::number(static_cast<sal_Int32>(GetViewShellId())).getStr()));
    xmlTextWriterEndElement(pWriter);
}

bool SfxViewShell::KeyInput( const KeyEvent &rKeyEvent )

/*  [Description]

    This Method executes the KeyEvent 'rKeyEvent' of the Keys (Accelerator)
    configured either direct or indirect (for example by the Application)
    in the SfxViewShell.

    [Return value]

    bool                    TRUE
                            The Key (Accelerator) is configured and the
                            associated Handler was called

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


void SfxViewShell::ResetAllClients_Impl( SfxInPlaceClient const *pIP )
{

    std::vector< SfxInPlaceClient* > *pClients = pImpl->GetIPClients_Impl(false);
    if ( !pClients )
        return;

    for (SfxInPlaceClient* pIPClient : *pClients)
    {
        if( pIPClient != pIP )
            pIPClient->ResetObject();
    }
}


void SfxViewShell::DisconnectAllClients()
{
    std::vector< SfxInPlaceClient* > *pClients = pImpl->GetIPClients_Impl(false);
    if ( !pClients )
        return;

    for ( size_t n = 0; n < pClients->size(); )
        // clients will remove themselves from the list
        delete pClients->at( n );
}


void SfxViewShell::QueryObjAreaPixel( tools::Rectangle& ) const
{
}


void SfxViewShell::VisAreaChanged()
{
    std::vector< SfxInPlaceClient* > *pClients = pImpl->GetIPClients_Impl(false);
    if ( !pClients )
        return;

    for (SfxInPlaceClient* pIPClient : *pClients)
    {
        if ( pIPClient->IsObjectInPlaceActive() )
            // client is active, notify client that the VisArea might have changed
            pIPClient->VisAreaChanged();
    }
}


void SfxViewShell::CheckIPClient_Impl(
        SfxInPlaceClient const *const pIPClient, const tools::Rectangle& rVisArea)
{
    if ( GetObjectShell()->IsInClose() )
        return;

    bool bAlwaysActive =
        ( ( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::EMBED_ACTIVATEIMMEDIATELY ) != 0 );
    bool bActiveWhenVisible =
        ( pIPClient->GetObjectMiscStatus() & embed::EmbedMisc::MS_EMBED_ACTIVATEWHENVISIBLE ) != 0;

    // this method is called when a client is created
    if (pIPClient->IsObjectInPlaceActive())
        return;

    // object in client is currently not active
    // check if the object wants to be activated always or when it becomes at least partially visible
    // TODO/LATER: maybe we should use the scaled area instead of the ObjArea?!
    if (bAlwaysActive || (bActiveWhenVisible && rVisArea.IsOver(pIPClient->GetObjArea())))
    {
        try
        {
            pIPClient->GetObject()->changeState( embed::EmbedStates::INPLACE_ACTIVE );
        }
        catch (const uno::Exception& e)
        {
            SAL_WARN("sfx.view", "SfxViewShell::CheckIPClient_Impl: " << e);
        }
    }
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
    return pImpl->aMargin;
}


void SfxViewShell::SetMargin( const Size& rSize )
{
    // the default margin was verified using www.apple.com !!
    Size aMargin = rSize;
    if ( aMargin.Width() == -1 )
        aMargin.setWidth( DEFAULT_MARGIN_WIDTH );
    if ( aMargin.Height() == -1 )
        aMargin.setHeight( DEFAULT_MARGIN_HEIGHT );

    if ( aMargin != pImpl->aMargin )
    {
        pImpl->aMargin = aMargin;
        MarginChanged();
    }
}

void SfxViewShell::MarginChanged()
{
}

void SfxViewShell::JumpToMark( const OUString& rMark )
{
    SfxStringItem aMarkItem( SID_JUMPTOMARK, rMark );
    GetViewFrame()->GetDispatcher()->ExecuteList(
        SID_JUMPTOMARK,
        SfxCallMode::SYNCHRON|SfxCallMode::RECORD,
        { &aMarkItem });
}

void SfxViewShell::SetController( SfxBaseController* pController )
{
    pImpl->m_pController = pController;

    // there should be no old listener, but if there is one, it should be disconnected
    if (  pImpl->xClipboardListener.is() )
        pImpl->xClipboardListener->DisconnectViewShell();

    pImpl->xClipboardListener = new SfxClipboardChangeListener( this, GetClipboardNotifier() );
}

Reference < XController > SfxViewShell::GetController()
{
    return pImpl->m_pController.get();
}

SfxBaseController* SfxViewShell::GetBaseController_Impl() const
{
    return pImpl->m_pController.get();
}

void SfxViewShell::AddContextMenuInterceptor_Impl( const uno::Reference< ui::XContextMenuInterceptor >& xInterceptor )
{
    pImpl->aInterceptorContainer.addInterface( xInterceptor );
}

void SfxViewShell::RemoveContextMenuInterceptor_Impl( const uno::Reference< ui::XContextMenuInterceptor >& xInterceptor )
{
    pImpl->aInterceptorContainer.removeInterface( xInterceptor );
}

static void Change( Menu* pMenu, SfxViewShell* pView )
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


bool SfxViewShell::TryContextMenuInterception( Menu& rIn, const OUString& rMenuIdentifier, VclPtr<Menu>& rpOut, ui::ContextMenuExecuteEvent aEvent )
{
    rpOut = nullptr;
    bool bModified = false;

    // create container from menu
    aEvent.ActionTriggerContainer = ::framework::ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
        &rIn, &rMenuIdentifier );

    // get selection from controller
    aEvent.Selection.set( GetController(), uno::UNO_QUERY );

    // call interceptors
    ::comphelper::OInterfaceIteratorHelper2 aIt( pImpl->aInterceptorContainer );
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
        rpOut = VclPtr<PopupMenu>::Create();
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
    ::comphelper::OInterfaceIteratorHelper2 aIt( pImpl->aInterceptorContainer );
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

bool SfxViewShell::HandleNotifyEvent_Impl( NotifyEvent const & rEvent )
{
    if (pImpl->m_pController.is())
        return pImpl->m_pController->HandleEvent_Impl( rEvent );
    return false;
}

bool SfxViewShell::HasKeyListeners_Impl()
{
    return (pImpl->m_pController.is())
        && pImpl->m_pController->HasKeyListeners_Impl();
}

bool SfxViewShell::HasMouseClickListeners_Impl()
{
    return (pImpl->m_pController.is())
        && pImpl->m_pController->HasMouseClickListeners_Impl();
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

void SfxViewShell::notifyWindow(vcl::LOKWindowId nDialogId, const OUString& rAction, const std::vector<vcl::LOKPayloadItem>& rPayload) const
{
    SfxLokHelper::notifyWindow(this, nDialogId, rAction, rPayload);
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

weld::Window* SfxViewShell::GetFrameWeld() const
{
    return pWindow ? pWindow->GetFrameWeld() : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

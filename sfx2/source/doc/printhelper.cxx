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


#include "printhelper.hxx"

#include <com/sun/star/view/XPrintJob.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/view/PaperFormat.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/view/DuplexMode.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/lstner.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <unotools/tempfile.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <tools/globname.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase.hxx>
#include <vcl/settings.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/event.hxx>

#define SFX_PRINTABLESTATE_CANCELJOB    css::view::PrintableState(-2)

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

struct IMPL_PrintListener_DataContainer : public SfxListener
{
    SfxObjectShellRef                               m_pObjectShell;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aInterfaceContainer;
    uno::Reference< css::view::XPrintJob>           m_xPrintJob;
    css::uno::Sequence< css::beans::PropertyValue > m_aPrintOptions;

    explicit IMPL_PrintListener_DataContainer( ::osl::Mutex& aMutex)
            :   m_aInterfaceContainer   ( aMutex )
    {
    }


    void Notify(            SfxBroadcaster& aBC     ,
                    const   SfxHint&        aHint   ) override ;
};

static awt::Size impl_Size_Object2Struct( const Size& aSize )
{
    awt::Size aReturnValue;
    aReturnValue.Width  = aSize.Width()  ;
    aReturnValue.Height = aSize.Height() ;
    return aReturnValue ;
}

static Size impl_Size_Struct2Object( const awt::Size& aSize )
{
    Size aReturnValue;
    aReturnValue.setWidth( aSize.Width )  ;
    aReturnValue.setHeight( aSize.Height ) ;
    return aReturnValue ;
}

class SfxPrintJob_Impl : public cppu::WeakImplHelper
<
    css::view::XPrintJob
>
{
    IMPL_PrintListener_DataContainer* m_pData;

public:
    explicit SfxPrintJob_Impl( IMPL_PrintListener_DataContainer* pData );
    virtual Sequence< css::beans::PropertyValue > SAL_CALL getPrintOptions(  ) override;
    virtual Sequence< css::beans::PropertyValue > SAL_CALL getPrinter(  ) override;
    virtual Reference< css::view::XPrintable > SAL_CALL getPrintable(  ) override;
    virtual void SAL_CALL cancelJob() override;
};

SfxPrintJob_Impl::SfxPrintJob_Impl( IMPL_PrintListener_DataContainer* pData )
    : m_pData( pData )
{
}

Sequence< css::beans::PropertyValue > SAL_CALL SfxPrintJob_Impl::getPrintOptions()
{
    return m_pData->m_aPrintOptions;
}

Sequence< css::beans::PropertyValue > SAL_CALL SfxPrintJob_Impl::getPrinter()
{
    if( m_pData->m_pObjectShell.is() )
    {
        Reference < view::XPrintable > xPrintable( m_pData->m_pObjectShell->GetModel(), UNO_QUERY );
        if ( xPrintable.is() )
            return xPrintable->getPrinter();
    }
    return Sequence< css::beans::PropertyValue >();
}

Reference< css::view::XPrintable > SAL_CALL SfxPrintJob_Impl::getPrintable()
{
    Reference < view::XPrintable > xPrintable( m_pData->m_pObjectShell.is() ? m_pData->m_pObjectShell->GetModel() : nullptr, UNO_QUERY );
    return xPrintable;
}

void SAL_CALL SfxPrintJob_Impl::cancelJob()
{
    // FIXME: how to cancel PrintJob via API?!
    if( m_pData->m_pObjectShell.is() )
        m_pData->m_pObjectShell->Broadcast( SfxPrintingHint( SFX_PRINTABLESTATE_CANCELJOB ) );
}

SfxPrintHelper::SfxPrintHelper()
{
    m_pData.reset(new IMPL_PrintListener_DataContainer(m_aMutex));
}

void SAL_CALL SfxPrintHelper::initialize( const css::uno::Sequence< css::uno::Any >& aArguments )
{
    if ( !aArguments.getLength() )
        return;

    css::uno::Reference < css::frame::XModel > xModel;
    aArguments[0] >>= xModel;
    uno::Reference < lang::XUnoTunnel > xObj( xModel, uno::UNO_QUERY );
    uno::Sequence < sal_Int8 > aSeq( SvGlobalName( SFX_GLOBAL_CLASSID ).GetByteSequence() );
    sal_Int64 nHandle = xObj->getSomething( aSeq );
    if ( nHandle )
    {
        m_pData->m_pObjectShell = reinterpret_cast< SfxObjectShell* >( sal::static_int_cast< sal_IntPtr >( nHandle ));
        m_pData->StartListening(*m_pData->m_pObjectShell);
    }
}

SfxPrintHelper::~SfxPrintHelper()
{
}

namespace
{
    view::PaperFormat convertToPaperFormat(Paper eFormat)
    {
        view::PaperFormat eRet;
        switch (eFormat)
        {
            case PAPER_A3:
                eRet = view::PaperFormat_A3;
                break;
            case PAPER_A4:
                eRet = view::PaperFormat_A4;
                break;
            case PAPER_A5:
                eRet = view::PaperFormat_A5;
                break;
            case PAPER_B4_ISO:
                eRet = view::PaperFormat_B4;
                break;
            case PAPER_B5_ISO:
                eRet = view::PaperFormat_B5;
                break;
            case PAPER_LETTER:
                eRet = view::PaperFormat_LETTER;
                break;
            case PAPER_LEGAL:
                eRet = view::PaperFormat_LEGAL;
                break;
            case PAPER_TABLOID:
                eRet = view::PaperFormat_TABLOID;
                break;
            case PAPER_USER:
            default:
                eRet = view::PaperFormat_USER;
                break;
        }
        return eRet;
    }

    Paper convertToPaper(view::PaperFormat eFormat)
    {
        Paper eRet(PAPER_USER);
        switch (eFormat)
        {
            case view::PaperFormat_A3:
                eRet = PAPER_A3;
                break;
            case view::PaperFormat_A4:
                eRet = PAPER_A4;
                break;
            case view::PaperFormat_A5:
                eRet = PAPER_A5;
                break;
            case view::PaperFormat_B4:
                eRet = PAPER_B4_ISO;
                break;
            case view::PaperFormat_B5:
                eRet = PAPER_B5_ISO;
                break;
            case view::PaperFormat_LETTER:
                eRet = PAPER_LETTER;
                break;
            case view::PaperFormat_LEGAL:
                eRet = PAPER_LEGAL;
                break;
            case view::PaperFormat_TABLOID:
                eRet = PAPER_TABLOID;
                break;
            case view::PaperFormat_USER:
                eRet = PAPER_USER;
                break;
            case view::PaperFormat::PaperFormat_MAKE_FIXED_SIZE:
                break;
            //deliberate no default to force warn on a new papersize
        }
        return eRet;
    }
}


//  XPrintable


uno::Sequence< beans::PropertyValue > SAL_CALL SfxPrintHelper::getPrinter()
{
    // object already disposed?
    SolarMutexGuard aGuard;

    // search for any view of this document that is currently printing
    const Printer *pPrinter = nullptr;
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.is() ? SfxViewFrame::GetFirst( m_pData->m_pObjectShell.get(), false ) : nullptr;
    SfxViewFrame* pFirst = pViewFrm;
    while ( pViewFrm && !pPrinter )
    {
        pPrinter = pViewFrm->GetViewShell()->GetActivePrinter();
        pViewFrm = SfxViewFrame::GetNext( *pViewFrm, m_pData->m_pObjectShell.get(), false );
    }

    // if no view is printing currently, use the permanent SfxPrinter instance
    if ( !pPrinter && pFirst )
        pPrinter = pFirst->GetViewShell()->GetPrinter(true);

    if ( !pPrinter )
        return uno::Sequence< beans::PropertyValue >();

    uno::Sequence< beans::PropertyValue > aPrinter(8);

    aPrinter.getArray()[7].Name = "CanSetPaperSize";
    aPrinter.getArray()[7].Value <<= pPrinter->HasSupport( PrinterSupport::SetPaperSize );

    aPrinter.getArray()[6].Name = "CanSetPaperFormat";
    aPrinter.getArray()[6].Value <<= pPrinter->HasSupport( PrinterSupport::SetPaper );

    aPrinter.getArray()[5].Name = "CanSetPaperOrientation";
    aPrinter.getArray()[5].Value <<= pPrinter->HasSupport( PrinterSupport::SetOrientation );

    aPrinter.getArray()[4].Name = "IsBusy";
    aPrinter.getArray()[4].Value <<= pPrinter->IsPrinting();

    aPrinter.getArray()[3].Name = "PaperSize";
    awt::Size aSize = impl_Size_Object2Struct(pPrinter->GetPaperSize() );
    aPrinter.getArray()[3].Value <<= aSize;

    aPrinter.getArray()[2].Name = "PaperFormat";
    view::PaperFormat eFormat = convertToPaperFormat(pPrinter->GetPaper());
    aPrinter.getArray()[2].Value <<= eFormat;

    aPrinter.getArray()[1].Name = "PaperOrientation";
    view::PaperOrientation eOrient = static_cast<view::PaperOrientation>(pPrinter->GetOrientation());
    aPrinter.getArray()[1].Value <<= eOrient;

    aPrinter.getArray()[0].Name = "Name";
    OUString sStringTemp = pPrinter->GetName() ;
    aPrinter.getArray()[0].Value <<= sStringTemp;

    return aPrinter;
}


//  XPrintable


void SfxPrintHelper::impl_setPrinter(const uno::Sequence< beans::PropertyValue >& rPrinter,
                                     VclPtr<SfxPrinter>& pPrinter,
                                     SfxPrinterChangeFlags& nChangeFlags,
                                     SfxViewShell*& pViewSh)

{
    // Get old Printer
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.is() ?
                                SfxViewFrame::GetFirst( m_pData->m_pObjectShell.get(), false ) : nullptr;
    if ( !pViewFrm )
        return;

    pViewSh = pViewFrm->GetViewShell();
    pPrinter = pViewSh->GetPrinter(true);
    if ( !pPrinter )
        return;

    // new Printer-Name available?
    nChangeFlags = SfxPrinterChangeFlags::NONE;
    sal_Int32 lDummy = 0;
    for ( int n = 0; n < rPrinter.getLength(); ++n )
    {
        // get Property-Value from printer description
        const beans::PropertyValue &rProp = rPrinter.getConstArray()[n];

        // Name-Property?
        if ( rProp.Name == "Name" )
        {
            OUString aPrinterName;
            if ( ! ( rProp.Value >>= aPrinterName ) )
                throw css::lang::IllegalArgumentException();

            if ( aPrinterName != pPrinter->GetName() )
            {
                pPrinter = VclPtr<SfxPrinter>::Create( pPrinter->GetOptions().Clone(), aPrinterName );
                nChangeFlags = SfxPrinterChangeFlags::PRINTER;
            }
            break;
        }
    }

    Size aSetPaperSize( 0, 0);
    view::PaperFormat nPaperFormat = view::PaperFormat_USER;

    // other properties
    for ( int i = 0; i < rPrinter.getLength(); ++i )
    {
        // get Property-Value from printer description
        const beans::PropertyValue &rProp = rPrinter.getConstArray()[i];

        // PaperOrientation-Property?
        if ( rProp.Name == "PaperOrientation" )
        {
            view::PaperOrientation eOrient;
            if ( !( rProp.Value >>= eOrient ) )
            {
                if ( !( rProp.Value >>= lDummy ) )
                    throw css::lang::IllegalArgumentException();
                eOrient = static_cast<view::PaperOrientation>(lDummy);
            }

            if ( static_cast<Orientation>(eOrient) != pPrinter->GetOrientation() )
            {
                pPrinter->SetOrientation( static_cast<Orientation>(eOrient) );
                nChangeFlags |= SfxPrinterChangeFlags::CHG_ORIENTATION;
            }
        }

        // PaperFormat-Property?
        else if ( rProp.Name == "PaperFormat" )
        {
            if ( !( rProp.Value >>= nPaperFormat ) )
            {
                if ( !( rProp.Value >>= lDummy ) )
                    throw css::lang::IllegalArgumentException();
                nPaperFormat = static_cast<view::PaperFormat>(lDummy);
            }

            if ( convertToPaper(nPaperFormat) != pPrinter->GetPaper() )
            {
                pPrinter->SetPaper( convertToPaper(nPaperFormat) );
                nChangeFlags |= SfxPrinterChangeFlags::CHG_SIZE;
            }
        }

        // PaperSize-Property?
        else if ( rProp.Name == "PaperSize" )
        {
            awt::Size aTempSize ;
            if ( !( rProp.Value >>= aTempSize ) )
            {
                throw css::lang::IllegalArgumentException();
            }
            aSetPaperSize = impl_Size_Struct2Object(aTempSize);
        }

        // PrinterTray-Property
        else if ( rProp.Name == "PrinterPaperTray" )
        {
            OUString aTmp;
            if ( !( rProp.Value >>= aTmp ) )
                throw css::lang::IllegalArgumentException();
            const sal_uInt16 nCount = pPrinter->GetPaperBinCount();
            for (sal_uInt16 nBin=0; nBin<nCount; nBin++)
            {
                OUString aName( pPrinter->GetPaperBinName(nBin) );
                if ( aName == aTmp )
                {
                    pPrinter->SetPaperBin(nBin);
                    break;
                }
            }
        }
    }

    // The PaperSize may be set only when actually PAPER_USER
    // applies, otherwise the driver could choose a invalid format.
    if(nPaperFormat == view::PaperFormat_USER && aSetPaperSize.Width())
    {
        // Bug 56929 - MapMode of 100mm which recalculated when
        // the device is set. Additionally only set if they were really changed.
        aSetPaperSize = pPrinter->LogicToPixel(aSetPaperSize, MapMode(MapUnit::Map100thMM));
        if( aSetPaperSize != pPrinter->GetPaperSizePixel() )
        {
            pPrinter->SetPaperSizeUser( pPrinter->PixelToLogic( aSetPaperSize ) );
            nChangeFlags |= SfxPrinterChangeFlags::CHG_SIZE;
        }
    }

    //wait until printing is done
    SfxPrinter* pDocPrinter = pViewSh->GetPrinter();
    while ( pDocPrinter->IsPrinting() )
        Application::Yield();
}

void SAL_CALL SfxPrintHelper::setPrinter(const uno::Sequence< beans::PropertyValue >& rPrinter)
{
    // object already disposed?
    SolarMutexGuard aGuard;

    SfxViewShell* pViewSh = nullptr;
    VclPtr<SfxPrinter> pPrinter;
    SfxPrinterChangeFlags nChangeFlags = SfxPrinterChangeFlags::NONE;
    impl_setPrinter(rPrinter,pPrinter,nChangeFlags,pViewSh);
    // set new printer
    if ( pViewSh && pPrinter )
        pViewSh->SetPrinter( pPrinter, nChangeFlags );
}


//  ImplPrintWatch thread for asynchronous printing with moving temp. file to ucb location


/* This implements a thread which will be started to wait for asynchronous
   print jobs to temp. locally files. If they finish we move the temp. files
   to their right locations by using the ucb.
 */
class ImplUCBPrintWatcher : public ::osl::Thread
{
    private:
        /// of course we must know the printer which execute the job
        VclPtr<SfxPrinter> m_pPrinter;
        /// this describes the target location for the printed temp file
        OUString const m_sTargetURL;
        /// it holds the temp file alive, till the print job will finish and remove it from disk automatically if the object die
        ::utl::TempFile* m_pTempFile;

    public:
        /* initialize this watcher but don't start it */
        ImplUCBPrintWatcher( SfxPrinter* pPrinter, ::utl::TempFile* pTempFile, const OUString& sTargetURL )
                : m_pPrinter  ( pPrinter   )
                , m_sTargetURL( sTargetURL )
                , m_pTempFile ( pTempFile  )
        {}

        /* waits for finishing of the print job and moves the temp file afterwards
           Note: Starting of the job is done outside this thread!
           But we have to free some of the given resources on heap!
         */
        void SAL_CALL run() override
        {
            osl_setThreadName("ImplUCBPrintWatcher");

            /* SAFE { */
            {
                SolarMutexGuard aGuard;
                while( m_pPrinter->IsPrinting() )
                    Application::Yield();
                m_pPrinter.clear(); // don't delete it! It's borrowed only :-)
            }
            /* } SAFE */

            // lock for further using of our member isn't necessary - because
            // we run alone by definition. Nobody join for us nor use us...
            moveAndDeleteTemp(&m_pTempFile,m_sTargetURL);

            // finishing of this run() method will call onTerminate() automatically
            // kill this thread there!
        }

        /* nobody wait for this thread. We must kill ourself ...
         */
        void SAL_CALL onTerminated() override
        {
            delete this;
        }

        /* static helper to move the temp. file to the target location by using the ucb
           It's static to be usable from outside too. So it's not really necessary to start
           the thread, if finishing of the job was detected outside this thread.
           But it must be called without using a corresponding thread for the given parameter!
         */
        static void moveAndDeleteTemp( ::utl::TempFile** ppTempFile, const OUString& sTargetURL )
        {
            // move the file
            try
            {
                INetURLObject aSplitter(sTargetURL);
                OUString        sFileName = aSplitter.getName(
                                            INetURLObject::LAST_SEGMENT,
                                            true,
                                            INetURLObject::DecodeMechanism::WithCharset);
                if (aSplitter.removeSegment() && !sFileName.isEmpty())
                {
                    ::ucbhelper::Content aSource(
                            (*ppTempFile)->GetURL(),
                            css::uno::Reference< css::ucb::XCommandEnvironment >(),
                            comphelper::getProcessComponentContext());

                    ::ucbhelper::Content aTarget(
                            aSplitter.GetMainURL(INetURLObject::DecodeMechanism::NONE),
                            css::uno::Reference< css::ucb::XCommandEnvironment >(),
                            comphelper::getProcessComponentContext());

                    aTarget.transferContent(
                            aSource,
                            ::ucbhelper::InsertOperation::Copy,
                            sFileName,
                            css::ucb::NameClash::OVERWRITE);
                }
            }
            catch (const css::ucb::ContentCreationException&)
            {
                OSL_FAIL("content create exception");
            }
            catch (const css::ucb::CommandAbortedException&)
            {
                OSL_FAIL("command abort exception");
            }
            catch (const css::uno::RuntimeException&)
            {
                OSL_FAIL("runtime exception");
            }
            catch (const css::uno::Exception&)
            {
                OSL_FAIL("unknown exception");
            }

            // kill the temp file!
            delete *ppTempFile;
            *ppTempFile = nullptr;
        }
};


//  XPrintable

void SAL_CALL SfxPrintHelper::print(const uno::Sequence< beans::PropertyValue >& rOptions)
{
    if( Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
        return;

    // object already disposed?
    // object already disposed?
    SolarMutexGuard aGuard;

    // get view for sfx printing capabilities
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.is() ?
                                SfxViewFrame::GetFirst( m_pData->m_pObjectShell.get(), false ) : nullptr;
    if ( !pViewFrm )
        return;
    SfxViewShell* pView = pViewFrm->GetViewShell();
    if ( !pView )
        return;
    bool bMonitor = false;
    // We need this information at the end of this method, if we start the vcl printer
    // by executing the slot. Because if it is a ucb relevant URL we must wait for
    // finishing the print job and move the temporary local file by using the ucb
    // to the right location. But in case of no file name is given or it is already
    // a local one we can suppress this special handling. Because then vcl makes all
    // right for us.
    OUString sUcbUrl;
    ::utl::TempFile* pUCBPrintTempFile = nullptr;

    uno::Sequence < beans::PropertyValue > aCheckedArgs( rOptions.getLength() );
    sal_Int32 nProps = 0;
    bool  bWaitUntilEnd = false;
    sal_Int16 nDuplexMode = css::view::DuplexMode::UNKNOWN;
    for ( int n = 0; n < rOptions.getLength(); ++n )
    {
        // get Property-Value from options
        const beans::PropertyValue &rProp = rOptions.getConstArray()[n];

        // FileName-Property?
        if ( rProp.Name == "FileName" )
        {
            // unpack th URL and check for a valid and well known protocol
            OUString sTemp;
            if (
                ( rProp.Value.getValueType()!=cppu::UnoType<OUString>::get())  ||
                (!(rProp.Value>>=sTemp))
               )
            {
                throw css::lang::IllegalArgumentException();
            }

            OUString      sPath;
            OUString      sURL  (sTemp);
            INetURLObject aCheck(sURL );
            if (aCheck.GetProtocol()==INetProtocol::NotValid)
            {
                // OK - it's not a valid URL. But may it's a simple
                // system path directly. It will be supported for historical
                // reasons. Otherwise we break to much external code ...
                // We try to convert it to a file URL. If its possible
                // we put the system path to the item set and let vcl work with it.
                // No ucb or thread will be necessary then. In case it couldn't be
                // converted it's not an URL nor a system path. Then we can't accept
                // this parameter and have to throw an exception.
                const OUString& sSystemPath(sTemp);
                OUString sFileURL;
                if (::osl::FileBase::getFileURLFromSystemPath(sSystemPath,sFileURL)!=::osl::FileBase::E_None)
                    throw css::lang::IllegalArgumentException();
                aCheckedArgs[nProps].Name = rProp.Name;
                aCheckedArgs[nProps++].Value <<= sFileURL;
                // and append the local filename
                aCheckedArgs.realloc( aCheckedArgs.getLength()+1 );
                aCheckedArgs[nProps].Name = "LocalFileName";
                aCheckedArgs[nProps++].Value <<= sTemp;
            }
            else
            // It's a valid URL. but now we must know, if it is a local one or not.
            // It's a question of using ucb or not!
            if (osl::FileBase::getSystemPathFromFileURL(sURL, sPath) == osl::FileBase::E_None)
            {
                // it's a local file, we can use vcl without special handling
                // And we have to use the system notation of the incoming URL.
                // But it into the descriptor and let the slot be executed at
                // the end of this method.
                aCheckedArgs[nProps].Name = rProp.Name;
                aCheckedArgs[nProps++].Value <<= sTemp;
                // and append the local filename
                aCheckedArgs.realloc( aCheckedArgs.getLength()+1 );
                aCheckedArgs[nProps].Name = "LocalFileName";
                aCheckedArgs[nProps++].Value <<= sPath;
            }
            else
            {
                // it's an ucb target. So we must use a temp. file for vcl
                // and move it after printing by using the ucb.
                // Create a temp file on the heap (because it must delete the
                // real file on disk automatically if it die - bt we have to share it with
                // some other sources ... e.g. the ImplUCBPrintWatcher).
                // And we put the name of this temp file to the descriptor instead
                // of the URL. The URL we save for later using separately.
                // Execution of the print job will be done later by executing
                // a slot ...
                if(!pUCBPrintTempFile)
                    pUCBPrintTempFile = new ::utl::TempFile();
                pUCBPrintTempFile->EnableKillingFile();

                //FIXME: does it work?
                aCheckedArgs[nProps].Name = "LocalFileName";
                aCheckedArgs[nProps++].Value <<= pUCBPrintTempFile->GetFileName();
                sUcbUrl = sURL;
            }
        }

        // CopyCount-Property
        else if ( rProp.Name == "CopyCount" )
        {
            sal_Int32 nCopies = 0;
            if ( !( rProp.Value >>= nCopies ) )
                throw css::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= nCopies;
        }

        // Collate-Property
        // Sort-Property (deprecated)
        else if ( rProp.Name == "Collate" || rProp.Name == "Sort" )
        {
            bool bTemp;
            if ( !(rProp.Value >>= bTemp) )
                throw css::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = "Collate";
            aCheckedArgs[nProps++].Value <<= bTemp;
        }

        // Pages-Property
        else if ( rProp.Name == "Pages" )
        {
            OUString sTemp;
            if( !(rProp.Value >>= sTemp) )
                throw css::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= sTemp;
        }

        // MonitorVisible
        else if ( rProp.Name == "MonitorVisible" )
        {
            if( !(rProp.Value >>= bMonitor) )
                throw css::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= bMonitor;
        }

        // Wait
        else if ( rProp.Name == "Wait" )
        {
            if ( !(rProp.Value >>= bWaitUntilEnd) )
                throw css::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= bWaitUntilEnd;
        }

        else if ( rProp.Name == "DuplexMode" )
        {
            if ( !(rProp.Value >>= nDuplexMode ) )
                throw css::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= nDuplexMode;
        }
    }

    if ( nProps != aCheckedArgs.getLength() )
        aCheckedArgs.realloc(nProps);

    // Execute the print request every time.
    // It doesn't matter if it is a real printer used or we print to a local file
    // nor if we print to a temp file and move it afterwards by using the ucb.
    // That will be handled later. see pUCBPrintFile below!
    pView->ExecPrint( aCheckedArgs, true, false );

    // Ok - may be execution before has finished (or started!) printing.
    // And may it was a printing to a file.
    // Now we have to check if we can move the file (if necessary) via ucb to his right location.
    // Cases:
    //  a) printing finished                        => move the file directly and forget the watcher thread
    //  b) printing is asynchron and runs currently => start watcher thread and exit this method
    //                                                 This thread make all necessary things by itself.
    if (!pUCBPrintTempFile)
        return;

    // a)
    SfxPrinter* pPrinter = pView->GetPrinter();
    if ( ! pPrinter->IsPrinting() )
        ImplUCBPrintWatcher::moveAndDeleteTemp(&pUCBPrintTempFile,sUcbUrl);
    // b)
    else
    {
        // Note: we create(d) some resource on the heap (thread and temp file).
        // They will be deleted by the thread automatically if he finish his run() method.
        ImplUCBPrintWatcher* pWatcher = new ImplUCBPrintWatcher( pPrinter, pUCBPrintTempFile, sUcbUrl );
        pWatcher->create();
    }
}

void IMPL_PrintListener_DataContainer::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxPrintingHint* pPrintHint = dynamic_cast<const SfxPrintingHint*>(&rHint);
    if ( &rBC != m_pObjectShell.get()
        || !pPrintHint
        || pPrintHint->GetWhich() == SFX_PRINTABLESTATE_CANCELJOB )
        return;

    if ( pPrintHint->GetWhich() == css::view::PrintableState_JOB_STARTED )
    {
        if ( !m_xPrintJob.is() )
            m_xPrintJob = new SfxPrintJob_Impl( this );
        m_aPrintOptions = pPrintHint->GetOptions();
    }

    ::cppu::OInterfaceContainerHelper* pContainer = m_aInterfaceContainer.getContainer(
        cppu::UnoType<view::XPrintJobListener>::get());
    if ( !pContainer )
        return;

    view::PrintJobEvent aEvent;
    aEvent.Source = m_xPrintJob;
    aEvent.State = pPrintHint->GetWhich();

    ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
    while (pIterator.hasMoreElements())
        static_cast<view::XPrintJobListener*>(pIterator.next())->printJobEvent( aEvent );
}

void SAL_CALL SfxPrintHelper::addPrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener )
{
    SolarMutexGuard aGuard;
    m_pData->m_aInterfaceContainer.addInterface( cppu::UnoType<view::XPrintJobListener>::get(), xListener );
}

void SAL_CALL SfxPrintHelper::removePrintJobListener( const css::uno::Reference< css::view::XPrintJobListener >& xListener )
{
    SolarMutexGuard aGuard;
    m_pData->m_aInterfaceContainer.removeInterface( cppu::UnoType<view::XPrintJobListener>::get(), xListener );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

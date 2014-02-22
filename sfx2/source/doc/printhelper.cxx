/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "printhelper.hxx"

#include <com/sun/star/view/XPrintJob.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/view/PaperFormat.hpp>
#include <com/sun/star/view/PaperOrientation.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
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
#include <unotools/localfilehelper.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vcl/settings.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/event.hxx>

#define SFX_PRINTABLESTATE_CANCELJOB    -2

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

struct IMPL_PrintListener_DataContainer : public SfxListener
{
    SfxObjectShellRef                               m_pObjectShell;
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aInterfaceContainer;
    uno::Reference< com::sun::star::view::XPrintJob>     m_xPrintJob;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aPrintOptions;

    IMPL_PrintListener_DataContainer( ::osl::Mutex& aMutex)
            :   m_pObjectShell          ( 0 )
            ,   m_aInterfaceContainer   ( aMutex )
    {
    }


    void Notify(            SfxBroadcaster& aBC     ,
                    const   SfxHint&        aHint   ) ;
};

awt::Size impl_Size_Object2Struct( const Size& aSize )
{
    awt::Size aReturnValue;
    aReturnValue.Width  = aSize.Width()  ;
    aReturnValue.Height = aSize.Height() ;
    return aReturnValue ;
}

Size impl_Size_Struct2Object( const awt::Size& aSize )
{
    Size aReturnValue;
    aReturnValue.Width()  = aSize.Width  ;
    aReturnValue.Height() = aSize.Height ;
    return aReturnValue ;
}

class SfxPrintJob_Impl : public cppu::WeakImplHelper1
<
    com::sun::star::view::XPrintJob
>
{
        IMPL_PrintListener_DataContainer* m_pData;

public:
        SfxPrintJob_Impl( IMPL_PrintListener_DataContainer* pData );
        virtual Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPrintOptions(  ) throw (RuntimeException);
        virtual Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPrinter(  ) throw (RuntimeException);
        virtual Reference< ::com::sun::star::view::XPrintable > SAL_CALL getPrintable(  ) throw (RuntimeException);
        virtual void SAL_CALL cancelJob() throw (RuntimeException);
};

SfxPrintJob_Impl::SfxPrintJob_Impl( IMPL_PrintListener_DataContainer* pData )
    : m_pData( pData )
{
}

Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SfxPrintJob_Impl::getPrintOptions() throw (RuntimeException)
{
    return m_pData->m_aPrintOptions;
}

Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL SfxPrintJob_Impl::getPrinter() throw (RuntimeException)
{
    if( m_pData->m_pObjectShell.Is() )
    {
        Reference < view::XPrintable > xPrintable( m_pData->m_pObjectShell->GetModel(), UNO_QUERY );
        if ( xPrintable.is() )
            return xPrintable->getPrinter();
    }
    return Sequence< ::com::sun::star::beans::PropertyValue >();
}

Reference< ::com::sun::star::view::XPrintable > SAL_CALL SfxPrintJob_Impl::getPrintable() throw (RuntimeException)
{
    Reference < view::XPrintable > xPrintable( m_pData->m_pObjectShell.Is() ? m_pData->m_pObjectShell->GetModel() : NULL, UNO_QUERY );
    return xPrintable;
}

void SAL_CALL SfxPrintJob_Impl::cancelJob() throw (RuntimeException)
{
    
    if( m_pData->m_pObjectShell.Is() )
        m_pData->m_pObjectShell->Broadcast( SfxPrintingHint( SFX_PRINTABLESTATE_CANCELJOB ) );
}

SfxPrintHelper::SfxPrintHelper()
{
    m_pData = new IMPL_PrintListener_DataContainer(m_aMutex);
}

void SAL_CALL SfxPrintHelper::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    if ( aArguments.getLength() )
    {
        com::sun::star::uno::Reference < com::sun::star::frame::XModel > xModel;
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
}

SfxPrintHelper::~SfxPrintHelper()
{
    delete m_pData;
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
            case view::PaperFormat_MAKE_FIXED_SIZE:
                break;
            
        }
        return eRet;
    }
}





uno::Sequence< beans::PropertyValue > SAL_CALL SfxPrintHelper::getPrinter() throw(::com::sun::star::uno::RuntimeException)
{
    
    SolarMutexGuard aGuard;

    
    const Printer *pPrinter = NULL;
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.Is() ? SfxViewFrame::GetFirst( m_pData->m_pObjectShell, sal_False ) : 0;
    SfxViewFrame* pFirst = pViewFrm;
    while ( pViewFrm && !pPrinter )
    {
        pPrinter = pViewFrm->GetViewShell()->GetActivePrinter();
        pViewFrm = SfxViewFrame::GetNext( *pViewFrm, m_pData->m_pObjectShell, sal_False );
    }

    
    if ( !pPrinter && pFirst )
        pPrinter = pFirst->GetViewShell()->GetPrinter(sal_True);

    if ( !pPrinter )
        return uno::Sequence< beans::PropertyValue >();

    uno::Sequence< beans::PropertyValue > aPrinter(8);

    aPrinter.getArray()[7].Name = "CanSetPaperSize";
    aPrinter.getArray()[7].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_PAPERSIZE ) );

    aPrinter.getArray()[6].Name = "CanSetPaperFormat";
    aPrinter.getArray()[6].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_PAPER ) );

    aPrinter.getArray()[5].Name = "CanSetPaperOrientation";
    aPrinter.getArray()[5].Value <<= ( pPrinter->HasSupport( SUPPORT_SET_ORIENTATION ) );

    aPrinter.getArray()[4].Name = "IsBusy";
    aPrinter.getArray()[4].Value <<= ( pPrinter->IsPrinting() );

    aPrinter.getArray()[3].Name = "PaperSize";
    awt::Size aSize = impl_Size_Object2Struct(pPrinter->GetPaperSize() );
    aPrinter.getArray()[3].Value <<= aSize;

    aPrinter.getArray()[2].Name = "PaperFormat";
    view::PaperFormat eFormat = convertToPaperFormat(pPrinter->GetPaper());
    aPrinter.getArray()[2].Value <<= eFormat;

    aPrinter.getArray()[1].Name = "PaperOrientation";
    view::PaperOrientation eOrient = (view::PaperOrientation)pPrinter->GetOrientation();
    aPrinter.getArray()[1].Value <<= eOrient;

    aPrinter.getArray()[0].Name = "Name";
    OUString sStringTemp = pPrinter->GetName() ;
    aPrinter.getArray()[0].Value <<= sStringTemp;

    return aPrinter;
}





void SfxPrintHelper::impl_setPrinter(const uno::Sequence< beans::PropertyValue >& rPrinter,SfxPrinter*& pPrinter,sal_uInt16& nChangeFlags,SfxViewShell*& pViewSh)

{
    
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.Is() ?
                                SfxViewFrame::GetFirst( m_pData->m_pObjectShell, sal_False ) : 0;
    if ( !pViewFrm )
        return;

    pViewSh = pViewFrm->GetViewShell();
    pPrinter = pViewSh->GetPrinter(sal_True);
    if ( !pPrinter )
        return;

    
    nChangeFlags = 0;
    sal_Int32 lDummy = 0;
    for ( int n = 0; n < rPrinter.getLength(); ++n )
    {
        
        const beans::PropertyValue &rProp = rPrinter.getConstArray()[n];

        
        if ( rProp.Name.equalsAscii( "Name" ) )
        {
            OUString aPrinterName;
            if ( ! ( rProp.Value >>= aPrinterName ) )
                throw ::com::sun::star::lang::IllegalArgumentException();

            if ( aPrinterName != pPrinter->GetName() )
            {
                pPrinter = new SfxPrinter( pPrinter->GetOptions().Clone(), aPrinterName );
                nChangeFlags = SFX_PRINTER_PRINTER;
            }
            break;
        }
    }

    Size aSetPaperSize( 0, 0);
    view::PaperFormat nPaperFormat = view::PaperFormat_USER;

    
    for ( int i = 0; i < rPrinter.getLength(); ++i )
    {
        
        const beans::PropertyValue &rProp = rPrinter.getConstArray()[i];

        
        if ( rProp.Name.equalsAscii( "PaperOrientation" ) )
        {
            view::PaperOrientation eOrient;
            if ( !( rProp.Value >>= eOrient ) )
            {
                if ( !( rProp.Value >>= lDummy ) )
                    throw ::com::sun::star::lang::IllegalArgumentException();
                eOrient = ( view::PaperOrientation) lDummy;
            }

            if ( (Orientation) eOrient != pPrinter->GetOrientation() )
            {
                pPrinter->SetOrientation( (Orientation) eOrient );
                nChangeFlags |= SFX_PRINTER_CHG_ORIENTATION;
            }
        }

        
        else if ( rProp.Name.equalsAscii( "PaperFormat" ) )
        {
            if ( !( rProp.Value >>= nPaperFormat ) )
            {
                if ( !( rProp.Value >>= lDummy ) )
                    throw ::com::sun::star::lang::IllegalArgumentException();
                nPaperFormat = ( view::PaperFormat ) lDummy;
            }

            if ( convertToPaper(nPaperFormat) != pPrinter->GetPaper() )
            {
                pPrinter->SetPaper( convertToPaper(nPaperFormat) );
                nChangeFlags |= SFX_PRINTER_CHG_SIZE;
            }
        }

        
        else if ( rProp.Name.equalsAscii( "PaperSize" ) )
        {
            awt::Size aTempSize ;
            if ( !( rProp.Value >>= aTempSize ) )
            {
                throw ::com::sun::star::lang::IllegalArgumentException();
            }
            else
            {
                aSetPaperSize = impl_Size_Struct2Object(aTempSize);
            }
        }

        
        else if ( rProp.Name.equalsAscii( "PrinterPaperTray" ) )
        {
            OUString aTmp;
            if ( !( rProp.Value >>= aTmp ) )
                throw ::com::sun::star::lang::IllegalArgumentException();
            sal_uInt16 nCount = pPrinter->GetPaperBinCount();
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

    
    
    if(nPaperFormat == view::PaperFormat_USER && aSetPaperSize.Width())
    {
        
        
        aSetPaperSize = pPrinter->LogicToPixel( aSetPaperSize, MAP_100TH_MM );
        if( aSetPaperSize != pPrinter->GetPaperSizePixel() )
        {
            pPrinter->SetPaperSizeUser( pPrinter->PixelToLogic( aSetPaperSize ) );
            nChangeFlags |= SFX_PRINTER_CHG_SIZE;
        }
    }

    
    SfxPrinter* pDocPrinter = pViewSh->GetPrinter();
    while ( pDocPrinter->IsPrinting() )
        Application::Yield();
}

void SAL_CALL SfxPrintHelper::setPrinter(const uno::Sequence< beans::PropertyValue >& rPrinter)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    
    SolarMutexGuard aGuard;

    SfxViewShell* pViewSh = NULL;
    SfxPrinter* pPrinter = NULL;
    sal_uInt16 nChangeFlags = 0;
    impl_setPrinter(rPrinter,pPrinter,nChangeFlags,pViewSh);
    
    if ( pViewSh && pPrinter )
        pViewSh->SetPrinter( pPrinter, nChangeFlags, false );
}





/* This implements a thread which will be started to wait for asynchronous
   print jobs to temp. localy files. If they finish we move the temp. files
   to her right locations by using the ucb.
 */
class ImplUCBPrintWatcher : public ::osl::Thread
{
    private:
        
        SfxPrinter* m_pPrinter;
        
        OUString m_sTargetURL;
        
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
        void SAL_CALL run()
        {
            /* SAFE { */
            {
                SolarMutexGuard aGuard;
                while( m_pPrinter->IsPrinting() )
                    Application::Yield();
                m_pPrinter = NULL; 
            }
            /* } SAFE */

            
            
            moveAndDeleteTemp(&m_pTempFile,m_sTargetURL);

            
            
        }

        /* nobody wait for this thread. We must kill ourself ...
         */
        void SAL_CALL onTerminated()
        {
            delete this;
        }

        /* static helper to move the temp. file to the target location by using the ucb
           It's static to be useable from outside too. So it's not really necessary to start
           the thread, if finishing of the job was detected outside this thread.
           But it must be called without using a corresponding thread for the given parameter!
         */
        static void moveAndDeleteTemp( ::utl::TempFile** ppTempFile, const OUString& sTargetURL )
        {
            
            try
            {
                INetURLObject aSplitter(sTargetURL);
                OUString        sFileName = aSplitter.getName(
                                            INetURLObject::LAST_SEGMENT,
                                            true,
                                            INetURLObject::DECODE_WITH_CHARSET);
                if (aSplitter.removeSegment() && !sFileName.isEmpty())
                {
                    ::ucbhelper::Content aSource(
                            OUString((*ppTempFile)->GetURL()),
                            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                            comphelper::getProcessComponentContext());

                    ::ucbhelper::Content aTarget(
                            OUString(aSplitter.GetMainURL(INetURLObject::NO_DECODE)),
                            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                            comphelper::getProcessComponentContext());

                    aTarget.transferContent(
                            aSource,
                            ::ucbhelper::InsertOperation_COPY,
                            OUString(sFileName),
                            ::com::sun::star::ucb::NameClash::OVERWRITE);
                }
            }
            catch (const ::com::sun::star::ucb::ContentCreationException&)
            {
                OSL_FAIL("content create exception");
            }
            catch (const ::com::sun::star::ucb::CommandAbortedException&)
            {
                OSL_FAIL("command abort exception");
            }
            catch (const ::com::sun::star::uno::RuntimeException&)
            {
                OSL_FAIL("runtime exception");
            }
            catch (const ::com::sun::star::uno::Exception&)
            {
                OSL_FAIL("unknown exception");
            }

            
            delete *ppTempFile;
            *ppTempFile = NULL;
        }
};






void SAL_CALL SfxPrintHelper::print(const uno::Sequence< beans::PropertyValue >& rOptions)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    if( Application::GetSettings().GetMiscSettings().GetDisablePrinting() )
        return;

    
    
    SolarMutexGuard aGuard;

    
    SfxViewFrame *pViewFrm = m_pData->m_pObjectShell.Is() ?
                                SfxViewFrame::GetFirst( m_pData->m_pObjectShell, sal_False ) : 0;
    if ( !pViewFrm )
        return;
    SfxViewShell* pView = pViewFrm->GetViewShell();
    if ( !pView )
        return;
    sal_Bool bMonitor = sal_False;
    
    
    
    
    
    
    OUString sUcbUrl;
    ::utl::TempFile* pUCBPrintTempFile = NULL;

    uno::Sequence < beans::PropertyValue > aCheckedArgs( rOptions.getLength() );
    sal_Int32 nProps = 0;
    sal_Bool  bWaitUntilEnd = sal_False;
    sal_Int16 nDuplexMode = ::com::sun::star::view::DuplexMode::UNKNOWN;
    for ( int n = 0; n < rOptions.getLength(); ++n )
    {
        
        const beans::PropertyValue &rProp = rOptions.getConstArray()[n];

        
        if ( rProp.Name.equalsAscii( "FileName" ) )
        {
            
            OUString sTemp;
            if (
                ( rProp.Value.getValueType()!=::getCppuType((const OUString*)0))  ||
                (!(rProp.Value>>=sTemp))
               )
            {
                throw ::com::sun::star::lang::IllegalArgumentException();
            }

            OUString      sPath        ;
            OUString      sURL  (sTemp);
            INetURLObject aCheck(sURL );
            if (aCheck.GetProtocol()==INET_PROT_NOT_VALID)
            {
                
                
                
                
                
                
                
                
                OUString sSystemPath(sTemp);
                OUString sFileURL;
                if (::osl::FileBase::getFileURLFromSystemPath(sSystemPath,sFileURL)!=::osl::FileBase::E_None)
                    throw ::com::sun::star::lang::IllegalArgumentException();
                aCheckedArgs[nProps].Name = rProp.Name;
                aCheckedArgs[nProps++].Value <<= sFileURL;
                
                aCheckedArgs.realloc( aCheckedArgs.getLength()+1 );
                aCheckedArgs[nProps].Name = "LocalFileName";
                aCheckedArgs[nProps++].Value <<= OUString( sTemp );
            }
            else
            
            
            if (::utl::LocalFileHelper::ConvertURLToSystemPath(sURL,sPath))
            {
                
                
                
                
                aCheckedArgs[nProps].Name = rProp.Name;
                aCheckedArgs[nProps++].Value <<= sTemp;
                
                aCheckedArgs.realloc( aCheckedArgs.getLength()+1 );
                aCheckedArgs[nProps].Name = "LocalFileName";
                aCheckedArgs[nProps++].Value <<= sPath;
            }
            else
            {
                
                
                
                
                
                
                
                
                
                if(!pUCBPrintTempFile)
                    pUCBPrintTempFile = new ::utl::TempFile();
                pUCBPrintTempFile->EnableKillingFile();

                
                aCheckedArgs[nProps].Name = "LocalFileName";
                aCheckedArgs[nProps++].Value <<= OUString( pUCBPrintTempFile->GetFileName() );
                sUcbUrl = sURL;
            }
        }

        
        else if ( rProp.Name.equalsAscii( "CopyCount" ) )
        {
            sal_Int32 nCopies = 0;
            if ( !( rProp.Value >>= nCopies ) )
                throw ::com::sun::star::lang::IllegalArgumentException();

            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= nCopies;
        }

        
        
        else if ( rProp.Name.equalsAscii( "Collate" ) ||
                  rProp.Name.equalsAscii( "Sort" ) )
        {
            sal_Bool bTemp = sal_Bool();
            if ( rProp.Value >>= bTemp )
            {
                aCheckedArgs[nProps].Name = "Collate";
                aCheckedArgs[nProps++].Value <<= bTemp;
            }
            else
                throw ::com::sun::star::lang::IllegalArgumentException();
        }

        
        else if ( rProp.Name.equalsAscii( "Pages" ) )
        {
            OUString sTemp;
            if( rProp.Value >>= sTemp )
            {
                aCheckedArgs[nProps].Name = rProp.Name;
                aCheckedArgs[nProps++].Value <<= sTemp;
            }
            else
                throw ::com::sun::star::lang::IllegalArgumentException();
        }

        
        else if ( rProp.Name.equalsAscii( "MonitorVisible" ) )
        {
            if( !(rProp.Value >>= bMonitor) )
                throw ::com::sun::star::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= bMonitor;
        }

        
        else if ( rProp.Name.equalsAscii( "Wait" ) )
        {
            if ( !(rProp.Value >>= bWaitUntilEnd) )
                throw ::com::sun::star::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= bWaitUntilEnd;
        }

        else if ( rProp.Name.equalsAscii( "DuplexMode" ) )
        {
            if ( !(rProp.Value >>= nDuplexMode ) )
                throw ::com::sun::star::lang::IllegalArgumentException();
            aCheckedArgs[nProps].Name = rProp.Name;
            aCheckedArgs[nProps++].Value <<= nDuplexMode;
        }
    }

    if ( nProps != aCheckedArgs.getLength() )
        aCheckedArgs.realloc(nProps);

    
    
    
    
    pView->ExecPrint( aCheckedArgs, sal_True, sal_False );

    
    
    
    
    
    
    
    if (pUCBPrintTempFile)
    {
        
        SfxPrinter* pPrinter = pView->GetPrinter();
        if ( ! pPrinter->IsPrinting() )
            ImplUCBPrintWatcher::moveAndDeleteTemp(&pUCBPrintTempFile,sUcbUrl);
        
        else
        {
            
            
            ImplUCBPrintWatcher* pWatcher = new ImplUCBPrintWatcher( pPrinter, pUCBPrintTempFile, sUcbUrl );
            pWatcher->create();
        }
    }
}

void IMPL_PrintListener_DataContainer::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    SfxPrintingHint* pPrintHint = PTR_CAST( SfxPrintingHint, &rHint );
    if ( &rBC != m_pObjectShell
        || !pPrintHint
        || pPrintHint->GetWhich() == SFX_PRINTABLESTATE_CANCELJOB )
        return;

    if ( pPrintHint->GetWhich() == com::sun::star::view::PrintableState_JOB_STARTED )
    {
        if ( !m_xPrintJob.is() )
            m_xPrintJob = new SfxPrintJob_Impl( this );
        m_aPrintOptions = pPrintHint->GetOptions();
    }

    ::cppu::OInterfaceContainerHelper* pContainer = m_aInterfaceContainer.getContainer(
        ::getCppuType( ( const uno::Reference< view::XPrintJobListener >*) NULL ) );
    if ( !pContainer )
        return;

    view::PrintJobEvent aEvent;
    aEvent.Source = m_xPrintJob;
    aEvent.State = (com::sun::star::view::PrintableState) pPrintHint->GetWhich();

    ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
    while (pIterator.hasMoreElements())
        ((view::XPrintJobListener*)pIterator.next())->printJobEvent( aEvent );
}

void SAL_CALL SfxPrintHelper::addPrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pData->m_aInterfaceContainer.addInterface( ::getCppuType((const uno::Reference < view::XPrintJobListener>*)0), xListener );
}

void SAL_CALL SfxPrintHelper::removePrintJobListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XPrintJobListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pData->m_aInterfaceContainer.removeInterface( ::getCppuType((const uno::Reference < view::XPrintJobListener>*)0), xListener );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

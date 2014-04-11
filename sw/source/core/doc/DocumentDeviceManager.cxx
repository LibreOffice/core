#include <DocumentDeviceManager.hxx>

#include <IDocumentDeviceAccess.hxx>
#include <doc.hxx>
#include <sfx2/printer.hxx>
#include <vcl/virdev.hxx>
#include <vcl/outdev.hxx>
#include <vcl/jobset.hxx>
#include <printdata.hxx>
#include <vcl/mapmod.hxx>
#include <svl/itemset.hxx>
#include <svx/svdmodel.hxx>
#include <cmdid.h>
#include <wdocsh.hxx>
#include <prtopt.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
#include <viewopt.hxx>
#include <swwait.hxx>
#include <fntcache.hxx>


class SwDocShell;
class SwWait;


namespace sw {

DocumentDeviceManager::DocumentDeviceManager( SwDoc& i_rSwdoc ) : m_rSwdoc( i_rSwdoc ), mpPrt(0), mpVirDev(0), mpPrtData(0) {}

SfxPrinter* DocumentDeviceManager::getPrinter(/*[in]*/ bool bCreate ) const
{
    SfxPrinter* pRet = 0;
    if ( !bCreate ||  mpPrt )
        pRet =  mpPrt;
    else
        pRet = &CreatePrinter_();

    return pRet;
}

void DocumentDeviceManager::setPrinter(/*[in]*/ SfxPrinter *pP,/*[in]*/ bool bDeleteOld,/*[in]*/ bool bCallPrtDataChanged )
{
    if ( pP !=  mpPrt )
    {
        if ( bDeleteOld )
            delete  mpPrt;
         mpPrt = pP;

        // our printer should always use TWIP. Don't rely on this being set in SwViewShell::InitPrt, there
        // are situations where this isn't called.
        // #i108712# / 2010-02-26 / frank.schoenheit@sun.com
        if ( mpPrt )
        {
            MapMode aMapMode( mpPrt->GetMapMode() );
            aMapMode.SetMapUnit( MAP_TWIP );
             mpPrt->SetMapMode( aMapMode );
        }

        if ( m_rSwdoc.GetDrawModel() && !m_rSwdoc.get( IDocumentSettingAccess::USE_VIRTUAL_DEVICE ) )
            m_rSwdoc.GetDrawModel()->SetRefDevice( mpPrt );
    }

    if ( bCallPrtDataChanged &&
         // #i41075# Do not call PrtDataChanged() if we do not
         // use the printer for formatting:
         !m_rSwdoc.get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
        PrtDataChanged();
}

VirtualDevice* DocumentDeviceManager::getVirtualDevice(/*[in]*/ bool bCreate ) const
{
    VirtualDevice* pRet = 0;
    if ( !bCreate || mpVirDev )
        pRet = mpVirDev;
    else
        pRet = &CreateVirtualDevice_();

    return pRet;
}

void DocumentDeviceManager::setVirtualDevice(/*[in]*/ VirtualDevice* pVd,/*[in]*/ bool bDeleteOld, /*[in]*/ bool )
{
    if ( mpVirDev != pVd )
    {
        if ( bDeleteOld )
            delete mpVirDev;
        mpVirDev = pVd;

        if ( m_rSwdoc.GetDrawModel() && m_rSwdoc.get( IDocumentSettingAccess::USE_VIRTUAL_DEVICE ) )
            m_rSwdoc.GetDrawModel()->SetRefDevice( mpVirDev );
    }
}

OutputDevice* DocumentDeviceManager::getReferenceDevice(/*[in]*/ bool bCreate ) const
{
    OutputDevice* pRet = 0;
    if ( !m_rSwdoc.get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
    {
        pRet = getPrinter( bCreate );

        if ( bCreate && !mpPrt->IsValid() )
        {
            pRet = getVirtualDevice( true );
        }
    }
    else
    {
        pRet = getVirtualDevice( bCreate );
    }

    return pRet;
}

void DocumentDeviceManager::setReferenceDeviceType(/*[in]*/ bool bNewVirtual, /*[in]*/ bool bNewHiRes )
{
    if ( m_rSwdoc.get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) != bNewVirtual ||
         m_rSwdoc.get(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE) != bNewHiRes )
    {
        if ( bNewVirtual )
        {
            VirtualDevice* pMyVirDev = getVirtualDevice( true );
            if ( !bNewHiRes )
                pMyVirDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE06 );
            else
                pMyVirDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );

            if( m_rSwdoc.GetDrawModel() )
                m_rSwdoc.GetDrawModel()->SetRefDevice( pMyVirDev );
        }
        else
        {
            // #i41075#
            // We have to take care that a printer exists before calling
            // PrtDataChanged() in order to prevent that PrtDataChanged()
            // triggers this funny situation:
            // getReferenceDevice()->getPrinter()->CreatePrinter_()
            // ->setPrinter()-> PrtDataChanged()
            SfxPrinter* pPrinter = getPrinter( true );
            if( m_rSwdoc.GetDrawModel() )
                m_rSwdoc.GetDrawModel()->SetRefDevice( pPrinter );
        }

        m_rSwdoc.set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, bNewVirtual );
        m_rSwdoc.set(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE, bNewHiRes );
        PrtDataChanged();
        m_rSwdoc.SetModified();
    }
}

const JobSetup* DocumentDeviceManager::getJobsetup() const
{
    return mpPrt ? &mpPrt->GetJobSetup() : 0;
}

void DocumentDeviceManager::setJobsetup(/*[in]*/ const JobSetup &rJobSetup )
{
    bool bCheckPageDescs = 0 == mpPrt;
    bool bDataChanged = false;

    if ( mpPrt )
    {
        if ( mpPrt->GetName() == rJobSetup.GetPrinterName() )
        {
            if ( mpPrt->GetJobSetup() != rJobSetup )
            {
                mpPrt->SetJobSetup( rJobSetup );
                bDataChanged = true;
            }
        }
        else
            delete mpPrt, mpPrt = 0;
    }

    if( !mpPrt )
    {
        //The ItemSet is deleted by Sfx!
        SfxItemSet *pSet = new SfxItemSet( m_rSwdoc.GetAttrPool(),
                        FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                        SID_HTML_MODE,  SID_HTML_MODE,
                        SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                        0 );
        SfxPrinter *p = new SfxPrinter( pSet, rJobSetup );
        if ( bCheckPageDescs )
            setPrinter( p, true, true );
        else
        {
            mpPrt = p;
            bDataChanged = true;
        }
    }
    if ( bDataChanged && !m_rSwdoc.get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
        PrtDataChanged();
}

const SwPrintData & DocumentDeviceManager::getPrintData() const
{
    if(!mpPrtData)
    {
        DocumentDeviceManager * pThis = const_cast< DocumentDeviceManager * >(this);
        pThis->mpPrtData = new SwPrintData;

        // SwPrintData should be initialized from the configuration,
        // the respective config item is implememted by SwPrintOptions which
        // is also derived from SwPrintData
        const SwDocShell *pDocSh = m_rSwdoc.GetDocShell();
        OSL_ENSURE( pDocSh, "pDocSh is 0, can't determine if this is a WebDoc or not" );
        bool bWeb = 0 != dynamic_cast< const SwWebDocShell * >(pDocSh);
        SwPrintOptions aPrintOptions( bWeb );
        *pThis->mpPrtData = aPrintOptions;
    }
    return *mpPrtData;
}

void DocumentDeviceManager::setPrintData(/*[in]*/ const SwPrintData& rPrtData )
{
    if(!mpPrtData)
        mpPrtData = new SwPrintData;
    *mpPrtData = rPrtData;
}

DocumentDeviceManager::~DocumentDeviceManager()
{
    delete mpPrtData;
    delete mpVirDev;
    DELETEZ( mpPrt );
}

VirtualDevice& DocumentDeviceManager::CreateVirtualDevice_() const
{
#ifdef IOS
    VirtualDevice* pNewVir = new VirtualDevice( 8 );
#else
    VirtualDevice* pNewVir = new VirtualDevice( 1 );
#endif

    pNewVir->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );

    // #i60945# External leading compatibility for unix systems.
    if ( m_rSwdoc.get(IDocumentSettingAccess::UNIX_FORCE_ZERO_EXT_LEADING ) )
        pNewVir->Compat_ZeroExtleadBug();

    MapMode aMapMode( pNewVir->GetMapMode() );
    aMapMode.SetMapUnit( MAP_TWIP );
    pNewVir->SetMapMode( aMapMode );

    const_cast<DocumentDeviceManager*>(this)->setVirtualDevice( pNewVir, true, true );
    return *mpVirDev;
}

SfxPrinter& DocumentDeviceManager::CreatePrinter_() const
{
    OSL_ENSURE( ! mpPrt, "Do not call CreatePrinter_(), call getPrinter() instead" );

#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "Printer will be created!" );
#endif

    // We create a default SfxPrinter.
    // The ItemSet is deleted by Sfx!
    SfxItemSet *pSet = new SfxItemSet( m_rSwdoc.GetAttrPool(),
                    FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                    SID_HTML_MODE,  SID_HTML_MODE,
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );

    SfxPrinter* pNewPrt = new SfxPrinter( pSet );
    const_cast<DocumentDeviceManager*>(this)->setPrinter( pNewPrt, true, true );
    return *mpPrt;
}

void DocumentDeviceManager::PrtDataChanged()
{
// If you change this, also modify InJobSetup in Sw3io if appropriate.

    // #i41075#
    OSL_ENSURE( m_rSwdoc.get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) ||
            0 != getPrinter( false ), "PrtDataChanged will be called recursively!" );
    SwRootFrm* pTmpRoot = m_rSwdoc.GetCurrentLayout();
    boost::scoped_ptr<SwWait> pWait;
    bool bEndAction = false;

    if( m_rSwdoc.GetDocShell() )
        m_rSwdoc.GetDocShell()->UpdateFontList();

    bool bDraw = true;
    if ( pTmpRoot )
    {
        SwViewShell *pSh = m_rSwdoc.GetCurrentViewShell();
        if( pSh &&
            (!pSh->GetViewOptions()->getBrowseMode() ||
             pSh->GetViewOptions()->IsPrtFormat()) )
        {
            if ( m_rSwdoc.GetDocShell() )
                pWait.reset(new SwWait( *m_rSwdoc.GetDocShell(), true ));

            pTmpRoot->StartAllAction();
            bEndAction = true;

            bDraw = false;
            if( m_rSwdoc.GetDrawModel() )
            {
                m_rSwdoc.GetDrawModel()->SetAddExtLeading( m_rSwdoc.get(IDocumentSettingAccess::ADD_EXT_LEADING) );
                m_rSwdoc.GetDrawModel()->SetRefDevice( getReferenceDevice( false ) );
            }

            pFntCache->Flush();

            std::set<SwRootFrm*> aAllLayouts = m_rSwdoc.GetAllLayouts();
            std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_SIZE));

            do
            {
                pSh->InitPrt( getPrinter(false) );
                pSh = (SwViewShell*)pSh->GetNext();
            }
            while ( pSh != m_rSwdoc.GetCurrentViewShell() );
        }
    }
    if ( bDraw && m_rSwdoc.GetDrawModel() )
    {
        const bool bTmpAddExtLeading = m_rSwdoc.get(IDocumentSettingAccess::ADD_EXT_LEADING);
        if ( bTmpAddExtLeading != m_rSwdoc.GetDrawModel()->IsAddExtLeading() )
            m_rSwdoc.GetDrawModel()->SetAddExtLeading( bTmpAddExtLeading );

        OutputDevice* pOutDev = getReferenceDevice( false );
        if ( pOutDev != m_rSwdoc.GetDrawModel()->GetRefDevice() )
            m_rSwdoc.GetDrawModel()->SetRefDevice( pOutDev );
    }

    m_rSwdoc.PrtOLENotify( sal_True );

    if ( bEndAction )
        pTmpRoot->EndAllAction();
}

}


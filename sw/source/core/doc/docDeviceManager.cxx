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


namespace sw {

DocumentDeviceManager::DocumentDeviceManager( SwDoc& i_rSwdoc ) : m_rSwdoc( i_rSwdoc ), mpPrt(0), mpVirDec(0), mpPrtData(0) {}

SfxPrinter* DocumentDeviceManger::getPrinter(/*[in]*/ bool bCreate ) const
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

        if ( i_rSwdoc.get_mpDrawModel() && !get( IDocumentSettingAccess::USE_VIRTUAL_DEVICE ) )
            i_rSwdoc.get_mpDrawModel().SetRefDevice( mpPrt );
    }

    if ( bCallPrtDataChanged &&
         // #i41075# Do not call PrtDataChanged() if we do not
         // use the printer for formatting:
         !get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
        i_rSwdoc.PrtDataChanged();
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

        if ( i_rSwdoc.get_mpDrawModel() && get( IDocumentSettingAccess::USE_VIRTUAL_DEVICE ) )
            i_rSwdoc.get_mpDrawModel()->SetRefDevice( mpVirDev );
    }
}

OutputDevice* DocumentDeviceManager::getReferenceDevice(/*[in]*/ bool bCreate ) const
{
    OutputDevice* pRet = 0;
    if ( !get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
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
    if ( get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) != bNewVirtual ||
         get(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE) != bNewHiRes )
    {
        if ( bNewVirtual )
        {
            VirtualDevice* pMyVirDev = getVirtualDevice( true );
            if ( !bNewHiRes )
                pMyVirDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE06 );
            else
                pMyVirDev->SetReferenceDevice( VirtualDevice::REFDEV_MODE_MSO1 );

            if( m_rSwdoc.get_mpDrawModel() )
                m_rSwdoc.get_mpDrawModel()->SetRefDevice( pMyVirDev );
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
            if( m_rSwdoc.get_mpDrawModel() )
                m_rSwdoc.get_mpDrawModel()->SetRefDevice( pPrinter );
        }

        set(IDocumentSettingAccess::USE_VIRTUAL_DEVICE, bNewVirtual );
        set(IDocumentSettingAccess::USE_HIRES_VIRTUAL_DEVICE, bNewHiRes );
        m_rSwdoc.PrtDataChanged();
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
        SfxItemSet *pSet = new SfxItemSet( GetAttrPool(),
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
    if ( bDataChanged && !get(IDocumentSettingAccess::USE_VIRTUAL_DEVICE) )
        m_rSwdoc.PrtDataChanged();
}

const SwPrintData & DocumentDeviceManager::getPrintData() const
{
    if(!mpPrtData)
    {
        SwDoc * pThis = const_cast< SwDoc * >(this);
        mpPrtData = new SwPrintData;

        // SwPrintData should be initialized from the configuration,
        // the respective config item is implememted by SwPrintOptions which
        // is also derived from SwPrintData
        const SwDocShell *pDocSh = m_rSwdoc.GetDocShell();
        OSL_ENSURE( pDocSh, "pDocSh is 0, can't determine if this is a WebDoc or not" );
        bool bWeb = 0 != dynamic_cast< const SwWebDocShell * >(pDocSh);
        SwPrintOptions aPrintOptions( bWeb );
        mpPrtData = aPrintOptions;
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
    if ( get(IDocumentSettingAccess::UNIX_FORCE_ZERO_EXT_LEADING ) )
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
    SfxItemSet *pSet = new SfxItemSet( ((SwDoc*)this)->GetAttrPool(),
                    FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                    SID_HTML_MODE,  SID_HTML_MODE,
                    SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                    SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                    0 );

    SfxPrinter* pNewPrt = new SfxPrinter( pSet );
    const_cast<DocumentDeviceManager*>(this)->setPrinter( pNewPrt, true, true );
    return *mpPrt;
}


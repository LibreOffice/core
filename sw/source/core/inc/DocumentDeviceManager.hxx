#ifndef  INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDEVICEMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDEVICEMANAGER_HXX

#include <IDocumentDeviceAccess.hxx>


class SwDoc;
class SfxPrinter;
class VirtualDevice;
class OutputDevice;
class JobSetup;
class PrintData;

namespace sw {

class DocumentDeviceManager : public IDocumentDeviceAccess {

public:

    DocumentDeviceManager( SwDoc& i_rSwdoc );

    SfxPrinter* getPrinter(/*[in]*/ bool bCreate ) const;

    void setPrinter(/*[in]*/ SfxPrinter *pP,/*[in]*/ bool bDeleteOld,/*[in]*/ bool bCallPrtDataChanged );

    VirtualDevice* getVirtualDevice(/*[in]*/ bool bCreate ) const;

    void setVirtualDevice(/*[in]*/ VirtualDevice* pVd,/*[in]*/ bool bDeleteOld, /*[in]*/ bool );

    OutputDevice* getReferenceDevice(/*[in]*/ bool bCreate ) const;

    void setReferenceDeviceType(/*[in]*/ bool bNewVirtual, /*[in]*/ bool bNewHiRes );

    const JobSetup* getJobsetup() const;

    void setJobsetup(/*[in]*/ const JobSetup &rJobSetup );

    const SwPrintData & getPrintData() const;

    void setPrintData(/*[in]*/ const SwPrintData& rPrtData );

    ~DocumentDeviceManager();

private:

    VirtualDevice& CreateVirtualDevice_() const;
    SfxPrinter& CreatePrinter_() const;

    SwDoc& m_rSwdoc;
    SfxPrinter* mpPrt;
    VirtualDevice* mpVirDev;
    SwPrintData* mpPrtData;
};

}
#endif


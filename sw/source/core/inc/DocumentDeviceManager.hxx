#ifndef  INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDEVICEMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DOCUMENTDEVICEMANAGER_HXX

#include <IDocumentDeviceAccess.hxx>
#include <boost/utility.hpp>
#include <sal/types.h>


class SwDoc;
class SfxPrinter;
class VirtualDevice;
class OutputDevice;
class JobSetup;
class SwPrintData;

namespace sw {

class DocumentDeviceManager : public IDocumentDeviceAccess,
                              public ::boost::noncopyable {

public:

    DocumentDeviceManager( SwDoc& i_rSwdoc );

    SfxPrinter* getPrinter(/*[in]*/ bool bCreate ) const SAL_OVERRIDE;

    void setPrinter(/*[in]*/ SfxPrinter *pP,/*[in]*/ bool bDeleteOld,/*[in]*/ bool bCallPrtDataChanged ) SAL_OVERRIDE;

    VirtualDevice* getVirtualDevice(/*[in]*/ bool bCreate ) const SAL_OVERRIDE;

    void setVirtualDevice(/*[in]*/ VirtualDevice* pVd,/*[in]*/ bool bDeleteOld, /*[in]*/ bool ) SAL_OVERRIDE;

    OutputDevice* getReferenceDevice(/*[in]*/ bool bCreate ) const SAL_OVERRIDE;

    void setReferenceDeviceType(/*[in]*/ bool bNewVirtual, /*[in]*/ bool bNewHiRes ) SAL_OVERRIDE;

    const JobSetup* getJobsetup() const SAL_OVERRIDE;

    void setJobsetup(/*[in]*/ const JobSetup &rJobSetup ) SAL_OVERRIDE;

    const SwPrintData & getPrintData() const SAL_OVERRIDE;

    void setPrintData(/*[in]*/ const SwPrintData& rPrtData ) SAL_OVERRIDE;

    virtual ~DocumentDeviceManager() SAL_OVERRIDE;

private:

    VirtualDevice& CreateVirtualDevice_() const;
    SfxPrinter& CreatePrinter_() const;
    void PrtDataChanged(); /**< Printer or JobSetup altered.
                                Care has to be taken of the necessary
                                invalidations and notifications. */

    SwDoc& m_rSwdoc;
    SfxPrinter* mpPrt;
    VirtualDevice* mpVirDev;
    SwPrintData* mpPrtData;
};

}
#endif


#ifndef _SD_UNOPRESVIEW_HXX
#define _SD_UNOPRESVIEW_HXX

#include "SdUnoDrawView.hxx"

/**
 * This class implements the view component for a SdPresViewShell or SdPreviewViewShell
 */
class SdUnoPresView :   public SdUnoDrawView
{
public:
    SdUnoPresView(SdView* pSdView, SdDrawViewShell* pSdViewSh) throw();
    virtual ~SdUnoPresView() throw();

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

protected:
    /**
     * This method must return the name to index table. This table contains all property
     * names and types of this object.
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
};

#endif

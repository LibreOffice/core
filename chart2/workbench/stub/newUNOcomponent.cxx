//replace XXXX and YYYY and ...
#ifndef _CHART2_XXXX_HXX
#include "XXXX.hxx"
#endif
#ifndef _CHART2_SERVICENAMES_CHARTTYPES_HXX
#include "servicenames_ ... .hxx"
#endif

//.............................................................................
namespace chart
{
//.............................................................................

XXXX::XXXX(
        uno::Reference<uno::XComponentContext> const & xContext)
{
    m_xMCF = xContext->getServiceManager();
}

XXXX::~XXXX()
{
}

//-----------------------------------------------------------------
// lang::XServiceInfo
//-----------------------------------------------------------------

APPHELPER_XSERVICEINFO_IMPL(XXXX,CHART2_VIEW_XXXX_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > XXXX
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART2_VIEW_XXXX_SERVICE_NAME;
    return aSNS;
}

//-----------------------------------------------------------------
// chart2::YYYY
//-----------------------------------------------------------------


//.............................................................................
} //namespace chart
//.............................................................................

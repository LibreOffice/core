//replace XXXX and YYYY
#ifndef _CHART2_XXXX_HXX
#define _CHART2_XXXX_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
/*
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
*/

//----
#include "ServiceMacros.hxx"

//---- chart2
#ifndef _COM_SUN_STAR_CHART2_YYYY_HPP_
#include <drafts/com/sun/star/chart2/YYYY.hpp>
#endif

//----
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class XXXX : public ::cppu::WeakImplHelper2<
        ::drafts::com::sun::star::chart2::YYYY
        , ::com::sun::star::lang::XServiceInfo
        //::com::sun::star::lang::XComponent ???
        //::com::sun::star::uno::XWeak          // implemented by WeakImplHelper(optional interface)
        //::com::sun::star::uno::XInterface     // implemented by WeakImplHelper(optional interface)
        //::com::sun::star::lang::XTypeProvider // implemented by WeakImplHelper
        >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiComponentFactory > m_xMCF;

    //no default constructor
    XXXX(){};
public:
    XXXX(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~XXXX();

    //-------------------------------------------------------------------------
    // ::com::sun::star::lang::XServiceInfo
    //-------------------------------------------------------------------------

    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(XXXX)

    //-------------------------------------------------------------------------
    // chart2::YYYY
    //-------------------------------------------------------------------------
};
//.............................................................................
} //namespace chart
//.............................................................................
#endif

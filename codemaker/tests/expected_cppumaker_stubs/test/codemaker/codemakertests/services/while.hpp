#pragma once

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppu/unotype.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#if defined ANDROID || defined IOS //TODO
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/detail/component-defines.h>
#endif

#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_services_dot_while && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_services_dot_while) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_services_dot_while
extern "C" ::css::uno::XInterface * SAL_CALL LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_services_dot_while(::css::uno::XComponentContext *, ::css::uno::Sequence< ::css::uno::Any > const &);
#endif

namespace test { namespace codemaker { namespace codemakertests { namespace services {

class service_while {
public:
    static ::css::uno::Reference< ::css::uno::XInterface > method_while(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context, ::sal_Int32 param_while) {
        assert(the_context.is());
        ::css::uno::Sequence< ::css::uno::Any > the_arguments(1);
        ::css::uno::Any* the_arguments_array = the_arguments.getArray();
        the_arguments_array[0] <<= param_while;
        ::css::uno::Reference< ::css::uno::XInterface > the_instance;
        try {
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_services_dot_while && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_services_dot_while) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_services_dot_while
            the_instance = ::css::uno::Reference< ::css::uno::XInterface >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_services_dot_while)(the_context.get(), the_arguments)), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
            ::css::uno::Reference< ::css::lang::XInitialization > init(the_instance, ::css::uno::UNO_QUERY);
            if (init.is()) {
                init->initialize(the_arguments);
            }
#else
            the_instance = ::css::uno::Reference< ::css::uno::XInterface >(the_context->getServiceManager()->createInstanceWithArgumentsAndContext( "test.codemaker.codemakertests.services.while", the_arguments, the_context), ::css::uno::UNO_QUERY);
#endif
        } catch (const ::css::uno::RuntimeException &) {
            throw;
        } catch (const ::css::uno::Exception & the_exception) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.services.while" + " of type " + "com.sun.star.uno.XInterface" + ": " + the_exception.Message, the_context);
        }
        if (!the_instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString("component context fails to supply service ") + "test.codemaker.codemakertests.services.while" + " of type " + "com.sun.star.uno.XInterface", the_context);
        }
        return the_instance;
    }

private:
    service_while(); // not implemented
    service_while(service_while &); // not implemented
    ~service_while(); // not implemented
    void operator =(service_while); // not implemented
};

} } } }

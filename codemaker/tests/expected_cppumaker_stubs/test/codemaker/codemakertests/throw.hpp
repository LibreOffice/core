#pragma once

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XNamingService.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "cppu/unotype.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#if defined ANDROID || defined IOS //TODO
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/detail/component-defines.h>
#endif

#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_throw && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_throw) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_throw
extern "C" ::css::uno::XInterface * SAL_CALL LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_throw(::css::uno::XComponentContext *, ::css::uno::Sequence< ::css::uno::Any > const &);
#endif

namespace test { namespace codemaker { namespace codemakertests {

class singleton_throw {
public:
    static ::css::uno::Reference< ::css::uno::XNamingService > get(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context) {
        assert(the_context.is());
        ::css::uno::Reference< ::css::uno::XNamingService > instance;
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_throw && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_throw) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_throw
        instance = ::css::uno::Reference< ::css::uno::XNamingService >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_throw)(the_context.get(), ::css::uno::Sequence< ::css::uno::Any >())), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
#else
        the_context->getValueByName(::rtl::OUString( "/singletons/test.codemaker.codemakertests.throw" )) >>= instance;
#endif
        if (!instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString( "component context fails to supply singleton test.codemaker.codemakertests.throw of type com.sun.star.uno.XNamingService" ), the_context);
        }
        return instance;
    }

private:
    singleton_throw(); // not implemented
    singleton_throw(singleton_throw &); // not implemented
    ~singleton_throw(); // not implemented
    void operator =(singleton_throw); // not implemented
};

} } }

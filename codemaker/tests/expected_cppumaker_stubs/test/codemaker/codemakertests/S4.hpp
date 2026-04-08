#pragma once

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "test/codemaker/codemakertests/XTest.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "cppu/unotype.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#if defined ANDROID || defined IOS //TODO
#include <com/sun/star/lang/XInitialization.hpp>
#include <osl/detail/component-defines.h>
#endif

#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S4 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S4) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S4
extern "C" ::css::uno::XInterface * SAL_CALL LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S4(::css::uno::XComponentContext *, ::css::uno::Sequence< ::css::uno::Any > const &);
#endif

namespace test { namespace codemaker { namespace codemakertests {

class S4 {
public:
    static ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > get(::css::uno::Reference< ::css::uno::XComponentContext > const & the_context) {
        assert(the_context.is());
        ::css::uno::Reference< ::test::codemaker::codemakertests::XTest > instance;
#if defined LO_URE_CURRENT_ENV && defined LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S4 && (LO_URE_CURRENT_ENV) == (LO_URE_CTOR_ENV_test_dot_codemaker_dot_codemakertests_dot_S4) && defined LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S4
        instance = ::css::uno::Reference< ::test::codemaker::codemakertests::XTest >(::css::uno::Reference< ::css::uno::XInterface >(static_cast< ::css::uno::XInterface * >((*LO_URE_CTOR_FUN_test_dot_codemaker_dot_codemakertests_dot_S4)(the_context.get(), ::css::uno::Sequence< ::css::uno::Any >())), ::SAL_NO_ACQUIRE), ::css::uno::UNO_QUERY);
#else
        the_context->getValueByName(::rtl::OUString( "/singletons/test.codemaker.codemakertests.S4" )) >>= instance;
#endif
        if (!instance.is()) {
            throw ::css::uno::DeploymentException(::rtl::OUString( "component context fails to supply singleton test.codemaker.codemakertests.S4 of type test.codemaker.codemakertests.XTest" ), the_context);
        }
        return instance;
    }

private:
    S4(); // not implemented
    S4(S4 &); // not implemented
    ~S4(); // not implemented
    void operator =(S4); // not implemented
};

} } }

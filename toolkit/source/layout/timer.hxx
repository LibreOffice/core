#ifndef CORE_TIMER_HXX
#define CORE_TIMER_HXX

#include <com/sun/star/awt/XLayoutUnit.hpp>
#include <cppuhelper/implbase1.hxx>

namespace layoutimpl
{

typedef ::cppu::WeakImplHelper1< com::sun::star::awt::XLayoutUnit > LayoutUnit_Base;

class LayoutUnit : public LayoutUnit_Base
{
public:
    LayoutUnit();
    void SAL_CALL queueResize( const com::sun::star::uno::Reference< com::sun::star::awt::XLayoutContainer > &xContainer )
        throw( com::sun::star::uno::RuntimeException );
};

}

#endif /*CORE_TIMER_HXX*/

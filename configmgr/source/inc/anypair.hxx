#ifndef CFGMGR_ANYPAIR_HXX
#define CFGMGR_ANYPAIR_HXX

#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

namespace configmgr
{
    namespace css  = com::sun::star;
    namespace uno  = css::uno;
    namespace lang = css::lang;

    //==========================================================================
    //= AnyPair
    //==========================================================================
    // this AnyPair holds 2 Any's with has always the same type.
    // to safe some mem we have decide to create this construct.


    struct cfgmgr_AnyPair
    {
        typelib_TypeDescriptionReference * m_pType;
        void*     m_pFirst;
        void*     m_pSecond;
    };

// -----------------------------------------------------------------------------
    class AnyPair
    {
        cfgmgr_AnyPair m_aAnyPair;

    public:
        // ctors
        AnyPair();
        explicit AnyPair(uno::Type const& _aType); // one Type, any's are null
        explicit AnyPair(uno::Any const& _aAny); // one any

        explicit AnyPair(uno::Any const& _aAny, uno::Any const& _aAny2) SAL_THROW((lang::IllegalArgumentException));

        // copy-ctor
        AnyPair(AnyPair const& _aAny);

        // assign operator
        AnyPair& operator=(AnyPair const& _aAny);

        // d-tor
        ~AnyPair();

        // set-types
        void setType(uno::Type const& _aType);

        void setFirst(uno::Any const& _aAny);
        void setSecond(uno::Any const& _aAny);


        uno::Any getFirst() const;
        uno::Any getSecond() const;
        uno::Type getValueType() const;

        bool hasFirst() const {return m_aAnyPair.m_pFirst  ? true : false;}
        bool hasSecond() const {return m_aAnyPair.m_pSecond ? true : false;}
        bool isNull() const   {return m_aAnyPair.m_pFirst == NULL && m_aAnyPair.m_pSecond == 0;}

        void check_init();
        void init();
    };




} // namespace

#endif

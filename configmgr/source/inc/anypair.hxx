#ifndef CFGMGR_ANYPAIR_HXX
#define CFGMGR_ANYPAIR_HXX

#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
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
    //= Basic (POD) data structure for representing a single value in an AnyPair
    //==========================================================================

    typedef uno_Any cfgmgr_AnyPair_Data;

    inline bool cfgmgr_AnyPair_Data_hasValue(cfgmgr_AnyPair_Data const* _pData)
    { return (typelib_TypeClass_VOID != _pData->pType->eTypeClass); }
    //==========================================================================
    //= cfgmgr_AnyPair Basic (POD) data structure for an AnyPair
    //==========================================================================

    struct cfgmgr_AnyPair
    {
        typelib_TypeDescriptionReference *  pType;
        cfgmgr_AnyPair_Data                 m_first;
        cfgmgr_AnyPair_Data                 m_second;
    };

    inline bool cfgmgr_AnyPair_isEmpty(cfgmgr_AnyPair const* _pPair)
    { return (typelib_TypeClass_VOID == _pPair->pType->eTypeClass); }

// -----------------------------------------------------------------------------
    //==========================================================================
    //= AnyPair
    //==========================================================================
    // this AnyPair holds 2 nullable Any's which have to have the same type.

    class AnyPair
    {
        cfgmgr_AnyPair m_aAnyPair;

    public:
        // ctors
        AnyPair();
        explicit AnyPair(uno::Type const& _aType); // one Type, any's are null
        explicit AnyPair(uno::Any const& _aAny); // one any - to first

        explicit AnyPair(uno::Any const& _aAny, uno::Any const& _aAny2) SAL_THROW((lang::IllegalArgumentException));

        // copy-ctor
        AnyPair(AnyPair const& _aAny);

        // assign operator
        AnyPair& operator=(AnyPair const& _aAny);

        // d-tor
        ~AnyPair();

        // set-types
        sal_Bool setFirst(uno::Any const& _aAny);
        sal_Bool setSecond(uno::Any const& _aAny);


        uno::Any getFirst() const;
        uno::Any getSecond() const;
        uno::Type getValueType() const;

        bool hasFirst()  const { return cfgmgr_AnyPair_Data_hasValue(&m_aAnyPair.m_first);}
        bool hasSecond() const { return cfgmgr_AnyPair_Data_hasValue(&m_aAnyPair.m_second);}
        bool hasValue()  const { return hasFirst() || hasSecond(); }
        bool isNull  ()  const { return ! hasValue(); }
        bool isEmpty()   const { return cfgmgr_AnyPair_isEmpty(&m_aAnyPair); }

        void check_init() {};
        void init() {};
    };




} // namespace

#endif

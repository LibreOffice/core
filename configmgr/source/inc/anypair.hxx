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

    typedef const void * cfgmgr_AnyPair_Data;

    //==========================================================================
    //= flags for handling the state of an Anypair
    //==========================================================================
    enum {
        cfgmgr_SELECT_FIRST  = 0x01,
        cfgmgr_SELECT_SECOND = 0x02,
        cfgmgr_SELECT_BOTH = cfgmgr_SELECT_FIRST | cfgmgr_SELECT_SECOND
    };
    typedef sal_uInt8 cfgmgr_SelectorType;

    //==========================================================================
    //= data structure for descriptive data for an AnyPair
    //==========================================================================
    struct cfgmgr_AnyPair_Desc
    {
        typelib_TypeDescriptionReference *  pType;
        cfgmgr_SelectorType nState;
    };

    inline bool cfgmgr_AnyPair_isNull(cfgmgr_AnyPair_Desc const* _pDesc, cfgmgr_SelectorType nSelect)
    { return (_pDesc->nState & nSelect) == 0; }

    inline bool cfgmgr_AnyPair_isEmpty(cfgmgr_AnyPair_Desc const* _pDesc)
    { return (typelib_TypeClass_VOID == _pDesc->pType->eTypeClass); }

    //==========================================================================
    //= cfgmgr_AnyPair Basic (POD) data structure for a nullable pair of Anys
    //==========================================================================

    struct cfgmgr_AnyPair
    {
        typedef cfgmgr_AnyPair_Data  Data;
        typedef cfgmgr_AnyPair_Desc  Desc;

        Desc                desc;
        cfgmgr_AnyPair_Data first;
        cfgmgr_AnyPair_Data second;
    };

// -----------------------------------------------------------------------------
    //==========================================================================
    //= AnyPair
    //==========================================================================
    // this AnyPair holds 2 nullable Any's which have to have the same type.

    class AnyPair
    {
        cfgmgr_AnyPair m_aAnyPair;

    public:
        enum SelectMember
        {
           SELECT_FIRST   = cfgmgr_SELECT_FIRST,
           SELECT_SECOND  = cfgmgr_SELECT_SECOND,
           SELECT_BOTH    = cfgmgr_SELECT_BOTH
        };
    public:
        // ctors
        AnyPair();
        explicit AnyPair(uno::Type const& _aType); // one Type, any's are null
        explicit AnyPair(uno::Any const& _aAny, SelectMember _select); // one selected any

        explicit AnyPair(uno::Any const& _aAny, uno::Any const& _aAny2) SAL_THROW((lang::IllegalArgumentException));

        // copy
        AnyPair(AnyPair const& _aAny);
        AnyPair& operator=(AnyPair const& _aAny);

        // d-tor
        ~AnyPair();

        // elementwise setters
        sal_Bool setFirst(uno::Any const& _aAny);
        sal_Bool setSecond(uno::Any const& _aAny);
        sal_Bool setValue(uno::Any const& _aAny, SelectMember _select);

        // clear data (but not type)
        void clear(SelectMember _select = SELECT_BOTH);


        // checking state and availablity of values
        bool isEmpty()   const { return cfgmgr_AnyPair_isEmpty(&m_aAnyPair.desc); }

        bool isNull  ()  const { return ! hasValue(); }

        bool hasValue(SelectMember _select = SELECT_BOTH)  const
        {
            return !cfgmgr_AnyPair_isNull(&m_aAnyPair.desc, (cfgmgr_SelectorType)_select);
        }
        bool hasFirst()  const
        {
            return hasValue(SELECT_FIRST);
        }
        bool hasSecond() const
        {
            return hasValue(SELECT_SECOND);
        }

        // elementwise getters
        uno::Type getValueType() const;
        uno::Any getFirst() const;
        uno::Any getSecond() const;
        uno::Any getValue(SelectMember _select) const;

    };




} // namespace

#endif

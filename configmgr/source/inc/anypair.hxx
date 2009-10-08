#ifndef CFGMGR_ANYPAIR_HXX
#define CFGMGR_ANYPAIR_HXX

#include <uno/any2.h>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace configmgr
{
    namespace css  = com::sun::star;
    namespace uno  = css::uno;
    namespace lang = css::lang;

    //==========================================================================
    //= flags for handling the state of an Anypair
    //==========================================================================
    enum {
        cfgmgr_SELECT_FIRST  = 0x01,
        cfgmgr_SELECT_SECOND = 0x02,
        cfgmgr_SELECT_BOTH = cfgmgr_SELECT_FIRST | cfgmgr_SELECT_SECOND
    };

    //==========================================================================
    //= data structure for descriptive data for an AnyPair
    //==========================================================================
    struct cfgmgr_AnyPair_Desc
    {
        typelib_TypeDescriptionReference *  pType;
        sal_uInt8 nState;
    };

    inline bool cfgmgr_AnyPair_isNull(cfgmgr_AnyPair_Desc const* _pDesc, sal_uInt8 nSelect)
    { return (_pDesc->nState & nSelect) == 0; }

    inline bool cfgmgr_AnyPair_isEmpty(cfgmgr_AnyPair_Desc const* _pDesc)
    { return (typelib_TypeClass_VOID == _pDesc->pType->eTypeClass); }

    //==========================================================================
    //= cfgmgr_AnyPair Basic (POD) data structure for a nullable pair of Anys
    //==========================================================================

    struct cfgmgr_AnyPair
    {
        cfgmgr_AnyPair_Desc                desc;
        const void * first;
        const void * second;
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

        // clear data (but not type)
        void clear(SelectMember _select = SELECT_BOTH);


        // checking state and availablity of values
        bool isEmpty()   const { return cfgmgr_AnyPair_isEmpty(&m_aAnyPair.desc); }

        bool isNull  ()  const { return ! hasValue(); }

        bool hasValue(SelectMember _select = SELECT_BOTH)  const
        {
            return !cfgmgr_AnyPair_isNull(&m_aAnyPair.desc, (sal_uInt8)_select);
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

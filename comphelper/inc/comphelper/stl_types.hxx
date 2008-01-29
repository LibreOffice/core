/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stl_types.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 17:13:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _COMPHELPER_STLTYPES_HXX_
#define _COMPHELPER_STLTYPES_HXX_

#if !defined(__SGI_STL_VECTOR_H) || !defined(__SGI_STL_MAP_H) || !defined(__SGI_STL_MULTIMAP_H)

#include <vector>
#include <map>

#include <stack>
#include <set>

#ifdef _MSC_VER
# ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES // needed by Visual C++ for math constants
# endif
#endif

#include <math.h> // prevent conflict between exception and std::exception
#include <functional>


#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

//... namespace comphelper ................................................
namespace comphelper
{
//.........................................................................

//========================================================================
// comparisation functions

//------------------------------------------------------------------------
    struct UStringLess : public ::std::binary_function< ::rtl::OUString, ::rtl::OUString, bool>
{
    bool operator() (const ::rtl::OUString& x, const ::rtl::OUString& y) const { return x < y ? true : false;}      // construct prevents a MSVC6 warning
};
//------------------------------------------------------------------------
struct UStringMixLess : public ::std::binary_function< ::rtl::OUString, ::rtl::OUString, bool>
{
    bool m_bCaseSensitive;
public:
    UStringMixLess(bool bCaseSensitive = true):m_bCaseSensitive(bCaseSensitive){}
    bool operator() (const ::rtl::OUString& x, const ::rtl::OUString& y) const
    {
        if (m_bCaseSensitive)
            return rtl_ustr_compare(x.getStr(), y.getStr()) < 0 ? true : false;
        else
            return rtl_ustr_compareIgnoreAsciiCase(x.getStr(), y.getStr()) < 0 ? true : false;
    }

    bool isCaseSensitive() const {return m_bCaseSensitive;}
};
//------------------------------------------------------------------------
struct UStringEqual
{
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const { return lhs.equals( rhs );}
};

//------------------------------------------------------------------------
struct UStringIEqual
{
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const { return lhs.equalsIgnoreAsciiCase( rhs );}
};

//------------------------------------------------------------------------
struct UStringHash
{
    size_t operator() (const ::rtl::OUString& rStr) const {return rStr.hashCode();}
};

//------------------------------------------------------------------------
class UStringMixEqual
{
    sal_Bool m_bCaseSensitive;

public:
    UStringMixEqual(sal_Bool bCaseSensitive = sal_True):m_bCaseSensitive(bCaseSensitive){}
    sal_Bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const
    {
        return m_bCaseSensitive ? lhs.equals( rhs ) : lhs.equalsIgnoreAsciiCase( rhs );
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};
//------------------------------------------------------------------------
class TStringMixEqualFunctor : public ::std::binary_function< ::rtl::OUString,::rtl::OUString,bool>
{
    sal_Bool m_bCaseSensitive;

public:
    TStringMixEqualFunctor(sal_Bool bCaseSensitive = sal_True)
        :m_bCaseSensitive(bCaseSensitive)
    {}
    bool operator() (const ::rtl::OUString& lhs, const ::rtl::OUString& rhs) const
    {
        return !!(m_bCaseSensitive ? lhs.equals( rhs ) : lhs.equalsIgnoreAsciiCase( rhs ));
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};
//------------------------------------------------------------------------
class TPropertyValueEqualFunctor : public ::std::binary_function< ::com::sun::star::beans::PropertyValue,::rtl::OUString,bool>
{
public:
    TPropertyValueEqualFunctor()
    {}
    bool operator() (const ::com::sun::star::beans::PropertyValue& lhs, const ::rtl::OUString& rhs) const
    {
        return !!(lhs.Name == rhs);
    }
};
//------------------------------------------------------------------------
class UStringMixHash
{
    sal_Bool m_bCaseSensitive;

public:
    UStringMixHash(sal_Bool bCaseSensitive = sal_True):m_bCaseSensitive(bCaseSensitive){}
    size_t operator() (const ::rtl::OUString& rStr) const
    {
        return m_bCaseSensitive ? rStr.hashCode() : rStr.toAsciiUpperCase().hashCode();
    }
    sal_Bool isCaseSensitive() const {return m_bCaseSensitive;}
};

//=====================================================================
//= OInterfaceCompare
//=====================================================================
/** is stl-compliant structure for comparing Reference&lt; &lt;iface&gt; &gt; instances
*/
template < class IAFCE >
struct OInterfaceCompare
    :public ::std::binary_function  <   ::com::sun::star::uno::Reference< IAFCE >
                                    ,   ::com::sun::star::uno::Reference< IAFCE >
                                    ,   bool
                                    >
{
    bool operator() (const ::com::sun::star::uno::Reference< IAFCE >& lhs, const ::com::sun::star::uno::Reference< IAFCE >& rhs) const
    {
        return lhs.get() < rhs.get();
            // this does not make any sense if you see the semantics of the pointer returned by get:
            // It's a pointer to a point in memory where an interface implementation lies.
            // But for our purpose (provide a reliable less-operator which can be used with the STL), this is
            // sufficient ....
    }
};

template <class _Tp, class _Arg>
class mem_fun1_t : public ::std::binary_function<_Tp*,_Arg,void>
{
    typedef void (_Tp::*_fun_type)(_Arg);
public:
    explicit mem_fun1_t(_fun_type __pf) : _M_f(__pf) {}
    void operator()(_Tp* __p, _Arg __x) const { (__p->*_M_f)(__x); }
private:
    _fun_type _M_f;
};

template <class _Tp, class _Arg>
inline mem_fun1_t<_Tp,_Arg> mem_fun(void (_Tp::*__f)(_Arg))
{
    return mem_fun1_t<_Tp,_Arg>(__f);
}

//.........................................................................
}
//... namespace comphelper ................................................

//==================================================================
// consistently defining stl-types
//==================================================================

#define DECLARE_STL_ITERATORS(classname)                            \
    typedef classname::iterator         classname##Iterator;        \
    typedef classname::const_iterator   Const##classname##Iterator  \

#define DECLARE_STL_MAP(keytype, valuetype, comparefct, classname)  \
    typedef std::map< keytype, valuetype, comparefct >  classname;  \
    DECLARE_STL_ITERATORS(classname)                                \

#define DECLARE_STL_STDKEY_MAP(keytype, valuetype, classname)               \
    DECLARE_STL_MAP(keytype, valuetype, std::less< keytype >, classname)    \

#define DECLARE_STL_VECTOR(valuetyp, classname)     \
    typedef std::vector< valuetyp >     classname;  \
    DECLARE_STL_ITERATORS(classname)                \

#define DECLARE_STL_USTRINGACCESS_MAP(valuetype, classname)                 \
    DECLARE_STL_MAP(::rtl::OUString, valuetype, ::comphelper::UStringLess, classname)   \

#define DECLARE_STL_STDKEY_SET(valuetype, classname)    \
    typedef ::std::set< valuetype > classname;          \
    DECLARE_STL_ITERATORS(classname)                    \

#define DECLARE_STL_SET(valuetype, comparefct, classname)               \
    typedef ::std::set< valuetype, comparefct > classname;  \
    DECLARE_STL_ITERATORS(classname)                        \

#endif

#endif  // _COMPHELPER_STLTYPES_HXX_


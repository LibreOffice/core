/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef CSV_TPL_FUNCALL_HXX
#define CSV_TPL_FUNCALL_HXX

// BASE CLASSES
#include <algorithm>




namespace csv
{
namespace func
{


/** @concept "csv:: Function Objects"

    A set of function objects that can be generated from any kind of
    function or member function with none or one parameter by the
    helper function ->make_func().

    Naming Scheme
    =============

    The naming scheme consists of three variables
      f - the kind of function
      p - the parameter of the function
      c - call operator() of the function object with these arguments

    Each of those may have the following values:
      f:
        f - free, no owning class
        c - const member function of a class
        m - modifying member function of a class
      p:
        n - no parameter
        c - const parameter by reference
        m - modifyable parameter by reference,
        v - parameter by value
      c:
        n - none
        o - the owning object on which the function shall be called
        a - the argument of the function
        b - both, the object on which the function shall be called
            and the argument of the function

    Which gives the following 35 possible combinations:
    ff_pn_cn
    ff_pc_cn
    ff_pc_ca
    ff_pm_cn
    ff_pm_ca
    ff_pv_cn
    ff_pv_ca

    fc_pn_cn
    fc_pn_co
    fc_pc_cn
    fc_pc_co
    fc_pc_ca
    fc_pc_cb
    fc_pm_cn
    fc_pm_co
    fc_pm_ca
    fc_pm_cb
    fc_pv_cn
    fc_pv_co
    fc_pv_ca
    fc_pv_cb

    fm_pn_cn
    fm_pn_co
    fm_pc_cn
    fm_pc_co
    fm_pc_ca
    fm_pc_cb
    fm_pm_cn
    fm_pm_co
    fm_pm_ca
    fm_pm_cb
    fm_pv_cn
    fm_pv_co
    fm_pv_ca
    fm_pv_cb

    These function objects are complicate to handle, so they can be created
    with the overloaded function
        <function_object> csv::make_func(<function_type>, <argument_types>);

    For the rare, but possible case that the owning class and the function
    argument have the same type, these clarifying variations to make_func()
    can be used:
        make_func_callwith_obj(), make_func_callwith_arg().
*/


/** Function object.

    @concept ->"csv::func Function Objects"
    @see    csv::make_func()
*/
template <class R>
struct ff_pn_cn
{
    typedef R           result_type;
    typedef R (*        function_type )();

    R                   operator()() const
                            { return (*f)(); }

                        ff_pn_cn(
                            function_type       i_f)
                            :   f(i_f)  {}
  private:
    function_type       f;
};


/** Function object.

    @concept ->"csv::func Function Objects"
    @see    csv::make_func()
*/
template <class R, class C>
struct fc_pn_co
{
    typedef R           result_type;
    typedef R (C::*     function_type )() const;

    R                   operator()(
                            const C &           i_c ) const
                            { return (i_c.*f)(); }

                        fc_pn_co(
                            function_type       i_f)
                            :   f(i_f)  {}
  private:
    function_type       f;
};



/** Function object.

    @concept ->"csv::func Function Objects"
    @see    csv::make_func()
*/
template <class R, class C, class P>
struct fc_pm_co
{
    typedef R           result_type;
    typedef R (C::*     function_type )(P&) const;

    R                   operator()(
                            const C &           i_c ) const
                            { return (i_c.*f)(p); }

                        fc_pm_co(
                            function_type       i_f,
                            P &                 i_p)
                            :   f(i_f), p(i_p) {}
  private:
    function_type       f;
    P &                 p;
};







}   // namespace func


/** Creates a function object of type ff_pn_cn.
    @concept ->"csv::func Function Objects"
*/
template <class R>
inline func::ff_pn_cn<R>
make_func( R(*i_f)() )
{
    return func::ff_pn_cn<R>(i_f);
}

///** Creates a function object of type ff_py_cn.
//    @concept ->"csv::func Function Objects"
//*/
//template <class R, class P>
//inline func::ff_py_cn<R,P>
//make_func( R(*i_f)(P), P i_p )
//{
//    return func::ff_py_cn<R,A>(i_f, i_p);
//}
//
///** Creates a function object of type ff_py_ca.
//    @concept ->"csv::func Function Objects"
//*/
//template <class R, class P>
//inline func::ff_py_ca<R,P>
//make_func( R(*i_f)(P) )
//{
//    return func::ff_py_ca<R,P>(i_f);
//}


/** Creates a function object of type fc_pn_co.
    @concept ->"csv::func Function Objects"
*/
template <class R, class C>
inline func::fc_pn_co<R,C>
make_func( R(C::*i_f)() const )
{
    return func::fc_pn_co<R,C>(i_f);
}



/** Creates a function object of type fc_pm_co.
    @concept ->"csv::func Function Objects"
*/
template <class R, class C, class P>
inline func::fc_pm_co<R,C,P>
make_func( R(C::*i_f)(P &) const, P & i_p)
{
    return func::fc_pm_co<R,C,P>(i_f, i_p);
}



/* Because std::for_each is defined as a non-modifying algorithm
   it is redefined here. It is also provided for containers.
*/

template <class I, class F>
F
for_each(I i_itBegin, I i_itEnd, F io_functionToBeCalled)
{
    for (I it = i_itBegin; it != i_itEnd; ++it)
    {
        io_functionToBeCalled(*it);
    }
    return io_functionToBeCalled;
}

template <class C, class F>
F
for_each_in(const C & i_container, F io_functionToBeCalled)
{
    typename C::const_iterator const
        itEnd = i_container.end();
    for ( typename C::const_iterator it = i_container.begin();
          it != itEnd;
          ++it )
    {
        io_functionToBeCalled(*it);
    }
    return io_functionToBeCalled;
}

template <class C, class F>
F
for_each_in(C & i_container, F io_functionToBeCalled)
{
    typename C::iterator const
        itEnd = i_container.end();
    for ( typename C::iterator it = i_container.begin();
          it != itEnd;
          ++it )
    {
        io_functionToBeCalled(*it);
    }
    return io_functionToBeCalled;
}




}   // namespace csv
#endif

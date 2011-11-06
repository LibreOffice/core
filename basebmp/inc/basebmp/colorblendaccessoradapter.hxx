/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef INCLUDED_BASEBMP_COLORBLENDACCESSORADAPTER_HXX
#define INCLUDED_BASEBMP_COLORBLENDACCESSORADAPTER_HXX

#include <basebmp/colortraits.hxx>

namespace basebmp
{

/** Accessor adapter that blends input value against fixed color value

    Used to blend an alpha mask 'through' a fixed color value into the
    destination.

    The getter functors return a constant value (usually the zero of
    the value type, this preserves the original destination content
    when blitting through a mask) - there really isn't no other
    sensible default behaviour for these methods.
 */
template< class    WrappedAccessor,
          typename AlphaType,
          bool     polarity > class ConstantColorBlendSetterAccessorAdapter
{
public:
    typedef AlphaType                            alpha_type;
    typedef AlphaType                            value_type;
    typedef typename WrappedAccessor::value_type color_type;

private:
    typename ColorTraits< color_type >::
             template blend_functor<alpha_type,polarity>::type   maFunctor;
    WrappedAccessor                                              maWrappee;
    color_type                                                   maBlendColor;
    value_type                                                   maGetterValue;

public:
    ConstantColorBlendSetterAccessorAdapter() :
        maFunctor(),
        maWrappee(),
        maBlendColor(),
        maGetterValue()
    {}

    template< class T > explicit ConstantColorBlendSetterAccessorAdapter( T acc ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor(),
        maGetterValue()
    {}

    template< class T > ConstantColorBlendSetterAccessorAdapter( T          acc,
                                                                 color_type col ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor(col),
        maGetterValue()
    {}

    template< class T > ConstantColorBlendSetterAccessorAdapter( T          acc,
                                                                 color_type col,
                                                                 value_type val ) :
        maFunctor(),
        maWrappee(acc),
        maBlendColor(col),
        maGetterValue(val)
    {}

    // -------------------------------------------------------

    void        setColor( color_type col ) { maBlendColor=col; }
    color_type  getColor() { return maBlendColor; }
    void        setGetterValue( value_type val ) { maGetterValue=val; }
    value_type  getGetterValue() { return maGetterValue; }

    // -------------------------------------------------------

    WrappedAccessor const& getWrappedAccessor() const { return maWrappee; }
    WrappedAccessor&       getWrappedAccessor() { return maWrappee; }

    // -------------------------------------------------------

    /// @return constant value, regardless of iterator content
    template< typename IteratorType > value_type operator()(IteratorType const& ) const
    {
        return maGetterValue;
    }
    /// @return constant value, regardless of iterator content
    template< typename IteratorType, class Difference >
    value_type operator()(IteratorType const& , Difference const& ) const
    {
        return maGetterValue;
    }

    // -------------------------------------------------------

    template< typename V, typename IteratorType >
    void set(V const& value, IteratorType const& i) const
    {
        maWrappee.set(
            maFunctor(
                vigra::detail::RequiresExplicitCast<alpha_type>::cast(value),
                maWrappee(i),
                maBlendColor),
            i );
    }

    template< typename V, typename IteratorType, class Difference >
    void set(V const& value, IteratorType const& i, Difference const& diff) const
    {
        maWrappee.set(
            maFunctor(
                vigra::detail::RequiresExplicitCast<alpha_type>::cast(value),
                maWrappee(i,diff),
                maBlendColor),
            i,
            diff );
    }
};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_COLORBLENDACCESSORADAPTER_HXX */

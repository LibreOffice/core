/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_O3TL_LAZY_UPDATE_HXX
#define INCLUDED_O3TL_LAZY_UPDATE_HXX

#include <sal/types.h>
#include <boost/function.hpp>

namespace o3tl
{
    /** Update output object lazily

        This template collects data in input type, and updates the
        output type with the given update functor, but only if the
        output is requested. Useful if updating is expensive, or input
        changes frequently, but output is only comparatively seldom
        used.

        @example
        <pre>
LazyUpdate<InType,OutType,LAZYUPDATE_DIRECT_TAG> myValue;
*myValue = newInput;
myValue->updateInput( this, that, those );

output( *myValue );
        </pre>
        or
        <pre>
output( myValue.getOutValue() );
        </pre>
        if the compiler does not recognize the const context.
     */
    template< typename InputType, typename OutputType, typename Tag > class LazyUpdate;

    /// LazyUpdate specialization takes boost::function argument
    struct LAZYUPDATE_FUNCTOR_TAG       {};
    /// LazyUpdate specialization takes OutputType (*FunctionType)( InputType const& ) argument
    struct LAZYUPDATE_FUNCTION_TAG      {};
    /// LazyUpdate specialization can directly convert, OutputType ctor must take InputType argument
    struct LAZYUPDATE_DIRECT_TAG  {};

    // -----------------------------------------------------------------------------------------------------

    namespace detail
    {
        /// @internal
        template< typename InputType, typename OutputType, typename Functor > class LazyUpdateImpl : private Functor
        {
        public:
            typedef OutputType output_type;
            typedef InputType  input_type;

            LazyUpdateImpl() :
                m_aInput()
            {}

            template< typename ParamType > explicit LazyUpdateImpl( ParamType const& rParm ) :
                Functor(rParm),
                m_aInput()
            {}

            enum UnaryConstructorTag{ UNARY_CONSTRUCTOR_TAG };
            LazyUpdateImpl( const input_type& rInput, UnaryConstructorTag ) :
                m_aInput(rInput)
            {}

            template< typename ParamType > LazyUpdateImpl( ParamType const&  rParm,
                                                           const input_type& rInput ) :
                Functor(rParm),
                m_aInput(rInput)
            {}

            // default copy ctor/assignment operator are ok
            // LazyUpdate( const LazyUpdate& );
            // LazyUpdate& operator=( const LazyUpdate& );

            void               setInValue( input_type const& rIn ) { Functor::m_bCacheDirty = true; m_aInput = rIn; }
            input_type const&  getInValue()  const                 { return m_aInput; }
            output_type const& getOutValue() const                 { return this->implUpdateValue(m_aInput); }

            input_type& operator*()  { Functor::m_bCacheDirty = true; return m_aInput;  }
            input_type* operator->() { Functor::m_bCacheDirty = true; return &m_aInput; }

            output_type const& operator*() const  { return this->implUpdateValue(m_aInput);  }
            output_type const* operator->() const { return &(this->implUpdateValue(m_aInput)); }

        private:
            input_type m_aInput;
        };

        template< typename InputType, typename OutputType > struct DefaultFunctor
        {
        protected:
            typedef OutputType output_type;
            typedef InputType  input_type;

            DefaultFunctor() :
                m_aOutput(),
                m_bCacheDirty(true)
            {}

            OutputType const& implUpdateValue( input_type const& rIn ) const
            {
                if( m_bCacheDirty )
                {
                    m_aOutput = output_type( rIn );
                    m_bCacheDirty = false;
                }

                return m_aOutput;
            }

            mutable output_type m_aOutput;
            mutable bool        m_bCacheDirty; // when true, m_aOutput needs update
        };

        template< typename InputType, typename OutputType, typename FunctionType > struct FunctionPointer
        {
        protected:
            typedef OutputType   output_type;
            typedef InputType    input_type;
            typedef FunctionType function_type;

            FunctionPointer() :
                m_pFunc(),
                m_aOutput(),
                m_bCacheDirty(true)

            {}

            explicit FunctionPointer( function_type const& pFunc ) :
                m_pFunc(pFunc),
                m_aOutput(),
                m_bCacheDirty(true)

            {}

            output_type const& implUpdateValue( input_type const& rIn ) const
            {
                if( m_bCacheDirty )
                {
                    m_aOutput = m_pFunc( rIn );
                    m_bCacheDirty = false;
                }

                return m_aOutput;
            }

            function_type       m_pFunc;
            mutable output_type m_aOutput;
            mutable bool        m_bCacheDirty; // when true, m_aOutput needs update
        };
    }

    // -----------------------------------------------------------------------------------------------------

    // partial specializations for the three LAZYUPDATE_* tags

    template< typename InputType, typename OutputType > class LazyUpdate<InputType,
                                                                         OutputType,
                                                                         LAZYUPDATE_DIRECT_TAG> :
        public detail::LazyUpdateImpl<InputType,
                                      OutputType,
                                      detail::DefaultFunctor<InputType, OutputType> >
    {
    public:
        LazyUpdate() {}
        explicit LazyUpdate( InputType const& rIn ) :
            detail::LazyUpdateImpl<InputType,
                                   OutputType,
                                   detail::DefaultFunctor<InputType, OutputType> >(
                                       rIn,
                                       detail::LazyUpdateImpl<
                                            InputType,
                                            OutputType,
                                            detail::DefaultFunctor<InputType, OutputType> >::UNARY_CONSTRUCTOR_TAG )
        {}
    };

    // -----------------------------------------------------------------------------------------------------

    template< typename InputType, typename OutputType > class LazyUpdate<InputType,
                                                                         OutputType,
                                                                         LAZYUPDATE_FUNCTION_TAG> :
        public detail::LazyUpdateImpl<InputType,
                                      OutputType,
                                      detail::FunctionPointer<
                                          InputType,
                                          OutputType,
                                          OutputType (*)( InputType const& ) > >
    {
    public:
        explicit LazyUpdate( OutputType (*pFunc)( InputType const& ) ) :
            detail::LazyUpdateImpl<InputType,
                                   OutputType,
                                   detail::FunctionPointer<
                                       InputType,
                                       OutputType,
                                       OutputType (*)( InputType const& )> >(pFunc)
        {}
        LazyUpdate( OutputType (*pFunc)( InputType const& ),
                    InputType const& rIn ) :
            detail::LazyUpdateImpl<InputType,
                                   OutputType,
                                   detail::FunctionPointer<
                                       InputType,
                                       OutputType,
                                       OutputType (*)( InputType const& )> >(pFunc,rIn)
        {}
    };

    // -----------------------------------------------------------------------------------------------------

    template< typename InputType, typename OutputType > class LazyUpdate<InputType,
                                                                         OutputType,
                                                                         LAZYUPDATE_FUNCTOR_TAG> :
        public detail::LazyUpdateImpl<InputType,
                                      OutputType,
                                      detail::FunctionPointer<
                                          InputType,
                                          OutputType,
                                          boost::function1<OutputType,InputType> > >
    {
    public:
        explicit LazyUpdate( boost::function1<OutputType,InputType> const& rFunc ) :
            detail::LazyUpdateImpl<InputType,
                                   OutputType,
                                   detail::FunctionPointer<
                                       InputType,
                                       OutputType,
                                       boost::function1<OutputType,InputType> > >(rFunc)
        {}
        LazyUpdate( boost::function1<OutputType,InputType> const& rFunc,
                    InputType const& rIn ) :
            detail::LazyUpdateImpl<InputType,
                                   OutputType,
                                   detail::FunctionPointer<
                                       InputType,
                                       OutputType,
                                       boost::function1<OutputType,InputType> > >(rFunc,rIn)
        {}
    };

}

#endif /* INCLUDED_O3TL_LAZY_UPDATE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

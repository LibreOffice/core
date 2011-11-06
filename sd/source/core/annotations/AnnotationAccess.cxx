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

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/office/XAnnotationAccess.hpp"

namespace css = ::com::sun::star;

class AnnotationAccess:
    public ::cppu::WeakImplHelper1<
        css::office::XAnnotationAccess>
{
public:
    explicit AnnotationAccess(css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::office::XAnnotationAccess:
    virtual css::uno::Reference< css::office::XAnnotation > SAL_CALL createAndInsertAnnotation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeAnnotation(const css::uno::Reference< css::office::XAnnotation > & annotation) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual css::uno::Reference< css::office::XAnnotationEnumeration > SAL_CALL createAnnotationEnumeration() throw (css::uno::RuntimeException);

private:
    AnnotationAccess(const AnnotationAccess &); // not defined
    AnnotationAccess& operator=(const AnnotationAccess &); // not defined

    // destructor is private and will be called indirectly by the release call    virtual ~AnnotationAccess() {}

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

AnnotationAccess::AnnotationAccess(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{}

// ::com::sun::star::office::XAnnotationAccess:
css::uno::Reference< css::office::XAnnotation > SAL_CALL AnnotationAccess::createAndInsertAnnotation() throw (css::uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "createAndInsertAnnotation" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return css::uno::Reference< css::office::XAnnotation >();
}

void SAL_CALL AnnotationAccess::removeAnnotation(const css::uno::Reference< css::office::XAnnotation > & annotation) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    // TODO: Insert your implementation for "removeAnnotation" here.
}

css::uno::Reference< css::office::XAnnotationEnumeration > SAL_CALL AnnotationAccess::createAnnotationEnumeration() throw (css::uno::RuntimeException)
{
    // TODO: Exchange the default return implementation for "createAnnotationEnumeration" !!!
    // Exchange the default return implementation.
    // NOTE: Default initialized polymorphic structs can cause problems because of
    // missing default initialization of primitive types of some C++ compilers or
    // different Any initialization in Java and C++ polymorphic structs.
    return css::uno::Reference< css::office::XAnnotationEnumeration >();
}



/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "sal/config.h"

#include "cppuhelper/implbase1.hxx"

#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/office/XAnnotationEnumeration.hpp"

#include "sdpage.hxx"
namespace css = ::com::sun::star;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace sd {

class AnnotationEnumeration: public ::cppu::WeakImplHelper1< css::office::XAnnotationEnumeration >
{
public:
    AnnotationEnumeration( const AnnotationVector& rAnnotations );

    // ::com::sun::star::office::XAnnotationEnumeration:
    virtual ::sal_Bool SAL_CALL hasMoreElements() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::office::XAnnotation > SAL_CALL nextElement() throw (css::uno::RuntimeException, css::container::NoSuchElementException);

private:
    AnnotationEnumeration(const AnnotationEnumeration &); // not defined
    AnnotationEnumeration& operator=(const AnnotationEnumeration &); // not defined

    // destructor is private and will be called indirectly by the release call    virtual ~AnnotationEnumeration() {}

    AnnotationVector maAnnotations;
    AnnotationVector::iterator maIter;
};

Reference< XAnnotationEnumeration > createAnnotationEnumeration( const sd::AnnotationVector& rAnnotations )
{
    return new AnnotationEnumeration( rAnnotations );
}

AnnotationEnumeration::AnnotationEnumeration( const AnnotationVector& rAnnotations )
: maAnnotations(rAnnotations)
{
    maIter = maAnnotations.begin();
}

// ::com::sun::star::office::XAnnotationEnumeration:
::sal_Bool SAL_CALL AnnotationEnumeration::hasMoreElements() throw (css::uno::RuntimeException)
{
    return maIter != maAnnotations.end() ? sal_True : sal_False;
}

css::uno::Reference< css::office::XAnnotation > SAL_CALL AnnotationEnumeration::nextElement() throw (css::uno::RuntimeException, css::container::NoSuchElementException)
{
    if( maIter == maAnnotations.end() )
        throw css::container::NoSuchElementException();

    return (*maIter++);
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

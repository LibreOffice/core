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

#include "sal/config.h"

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include "com/sun/star/office/XAnnotationEnumeration.hpp"

#include "AnnotationEnumeration.hxx"
#include "sdpage.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::office;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

namespace sd {

class AnnotationEnumeration: public ::cppu::WeakImplHelper< css::office::XAnnotationEnumeration >
{
public:
    explicit AnnotationEnumeration( const AnnotationVector& rAnnotations );
    AnnotationEnumeration(const AnnotationEnumeration&) = delete;
    AnnotationEnumeration& operator=(const AnnotationEnumeration&) = delete;

    // css::office::XAnnotationEnumeration:
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Reference< css::office::XAnnotation > SAL_CALL nextElement() override;

private:
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

// css::office::XAnnotationEnumeration:
sal_Bool SAL_CALL AnnotationEnumeration::hasMoreElements()
{
    return maIter != maAnnotations.end();
}

css::uno::Reference< css::office::XAnnotation > SAL_CALL AnnotationEnumeration::nextElement()
{
    if( maIter == maAnnotations.end() )
        throw css::container::NoSuchElementException();

    return (*maIter++);
}

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

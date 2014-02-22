/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/office/XAnnotationAccess.hpp"

class AnnotationAccess:
    public ::cppu::WeakImplHelper1<
        css::office::XAnnotationAccess>
{
public:
    explicit AnnotationAccess(css::uno::Reference< css::uno::XComponentContext > const & context);

    
    virtual css::uno::Reference< css::office::XAnnotation > SAL_CALL createAndInsertAnnotation() throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeAnnotation(const css::uno::Reference< css::office::XAnnotation > & annotation) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual css::uno::Reference< css::office::XAnnotationEnumeration > SAL_CALL createAnnotationEnumeration() throw (css::uno::RuntimeException);

private:
    AnnotationAccess(const AnnotationAccess &); 
    AnnotationAccess& operator=(const AnnotationAccess &); 

    

    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};

AnnotationAccess::AnnotationAccess(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{}


css::uno::Reference< css::office::XAnnotation > SAL_CALL AnnotationAccess::createAndInsertAnnotation() throw (css::uno::RuntimeException)
{
    
    
    
    
    
    return css::uno::Reference< css::office::XAnnotation >();
}

void SAL_CALL AnnotationAccess::removeAnnotation(const css::uno::Reference< css::office::XAnnotation > & annotation) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    
}

css::uno::Reference< css::office::XAnnotationEnumeration > SAL_CALL AnnotationAccess::createAnnotationEnumeration() throw (css::uno::RuntimeException)
{
    
    
    
    
    
    return css::uno::Reference< css::office::XAnnotationEnumeration >();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

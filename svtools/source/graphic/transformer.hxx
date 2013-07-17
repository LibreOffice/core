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

#ifndef _GOODIES_GRAPHICTRANSFORMER_HXX
#define _GOODIES_GRAPHICTRANSFORMER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphicTransformer.hpp>

namespace unographic {

// ----------------------
// - GraphicTransformer -
// ----------------------

typedef ::cppu::WeakAggImplHelper1<
    ::com::sun::star::graphic::XGraphicTransformer
    > GraphicTransformer_UnoImplHelper1;
class GraphicTransformer : public GraphicTransformer_UnoImplHelper1
{
    public:

    GraphicTransformer();
    ~GraphicTransformer();

    // XGraphicTransformer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL colorChange(
        const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rGraphic,
            sal_Int32 nColorFrom, sal_Int8 nTolerance, sal_Int32 nColorTo, sal_Int8 nAlphaTo )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic > SAL_CALL applyDuotone(
        const ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >& rGraphic,
            sal_Int32 nColorOne, sal_Int32 nColorTwo )
                throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

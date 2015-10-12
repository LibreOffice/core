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
#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_LINEAR3DTRANSFORMATION_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_LINEAR3DTRANSFORMATION_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/chart2/XTransformation.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>

namespace chart
{

class Linear3DTransformation : public ::cppu::WeakImplHelper<
    ::com::sun::star::chart2::XTransformation
    >
{
public:
    Linear3DTransformation( const ::com::sun::star::drawing::HomogenMatrix& rHomMatrix, bool bSwapXAndY  );
    virtual ~Linear3DTransformation();

    // ____ XTransformation ____
    /// @see ::com::sun::star::chart2::XTransformation
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL transform(
        const ::com::sun::star::uno::Sequence< double >& rSourceValues )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException, std::exception) override;
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getSourceDimension()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getTargetDimension()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    ::com::sun::star::drawing::HomogenMatrix    m_Matrix;
    bool                                        m_bSwapXAndY;
};

}  // namespace chart

// INCLUDED_CHART2_SOURCE_VIEW_INC_LINEAR3DTRANSFORMATION_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

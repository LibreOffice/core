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
#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/chart2/XTransformation.hpp>
#include <com/sun/star/drawing/HomogenMatrix.hpp>

namespace chart
{

class Linear3DTransformation : public ::cppu::WeakImplHelper<
    css::chart2::XTransformation
    >
{
public:
    Linear3DTransformation( const css::drawing::HomogenMatrix& rHomMatrix, bool bSwapXAndY  );
    virtual ~Linear3DTransformation() override;

    // ____ XTransformation ____
    /// @see css::chart2::XTransformation
    virtual css::uno::Sequence< double > SAL_CALL transform(
        const css::uno::Sequence< double >& rSourceValues ) override;
    /// @see css::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getSourceDimension() override;
    /// @see css::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getTargetDimension() override;

private:
    css::drawing::HomogenMatrix    m_Matrix;
    bool                           m_bSwapXAndY;
};

}  // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

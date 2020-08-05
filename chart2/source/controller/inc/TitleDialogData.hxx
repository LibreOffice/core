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

#include <ReferenceSizeProvider.hxx>
#include <memory>
#include <com/sun/star/uno/Sequence.hxx>

namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::uno { class XComponentContext; }

namespace chart
{

struct TitleDialogData
{
    css::uno::Sequence< sal_Bool > aPossibilityList;
    css::uno::Sequence< sal_Bool > aExistenceList;
    css::uno::Sequence< OUString > aTextList;
    std::unique_ptr< ReferenceSizeProvider > apReferenceSizeProvider;

    TitleDialogData(std::unique_ptr<ReferenceSizeProvider> pReferenzeSizeProvider = nullptr);

    void readFromModel( const css::uno::Reference< css::frame::XModel >& xChartModel );
    /* return true if anything has changed;
    when pOldState is NULL then all data are written to the model
    */
    bool writeDifferenceToModel( const css::uno::Reference< css::frame::XModel >& xChartModel
                        , const css::uno::Reference< css::uno::XComponentContext >& xContext
                        , TitleDialogData* pOldState=nullptr );
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

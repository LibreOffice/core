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
#ifndef _CHART2_TITLEDIALOGDATA_HXX
#define _CHART2_TITLEDIALOGDATA_HXX

#include "ReferenceSizeProvider.hxx"
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <memory>

namespace chart
{

/**
*/

struct TitleDialogData
{
    ::com::sun::star::uno::Sequence< sal_Bool > aPossibilityList;
    ::com::sun::star::uno::Sequence< sal_Bool > aExistenceList;
    ::com::sun::star::uno::Sequence< OUString > aTextList;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< ReferenceSizeProvider >    apReferenceSizeProvider;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH

    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    TitleDialogData( ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProvider =
                     ::std::auto_ptr< ReferenceSizeProvider >( 0 ));
    SAL_WNODEPRECATED_DECLARATIONS_POP

    void readFromModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    /* return true if anything has changed;
    when pOldState is NULL then all data are written to the model
    */
    bool writeDifferenceToModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel
                        , const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext
                        , TitleDialogData* pOldState=NULL );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

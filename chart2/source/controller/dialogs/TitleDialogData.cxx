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

#include <sal/config.h>

#include <TitleDialogData.hxx>
#include <TitleHelper.hxx>
#include <ChartModelHelper.hxx>
#include <Diagram.hxx>
#include <AxisHelper.hxx>
#include <ChartModel.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

TitleDialogData::TitleDialogData( std::unique_ptr<ReferenceSizeProvider> pRefSizeProvider )
        : aPossibilityList{ true, true, true, true, true, true, true }
        , aExistenceList{ false, false, false, false, false, false, false }
        , aTextList(7)
        , apReferenceSizeProvider( std::move(pRefSizeProvider) )
{
}

void TitleDialogData::readFromModel( const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    rtl::Reference< Diagram > xDiagram = xChartModel->getFirstChartDiagram();

    //get possibilities
    uno::Sequence< sal_Bool > aAxisPossibilityList;
    AxisHelper::getAxisOrGridPossibilities( aAxisPossibilityList, xDiagram );
    sal_Bool* pPossibilityList = aPossibilityList.getArray();
    pPossibilityList[2]=aAxisPossibilityList[0];//x axis title
    pPossibilityList[3]=aAxisPossibilityList[1];//y axis title
    pPossibilityList[4]=aAxisPossibilityList[2];//z axis title
    pPossibilityList[5]=aAxisPossibilityList[3];//secondary x axis title
    pPossibilityList[6]=aAxisPossibilityList[4];//secondary y axis title

    sal_Bool* pExistenceList = aExistenceList.getArray();
    auto pTextList = aTextList.getArray();
    //find out which title exists and get their text
    //main title:
    for( auto nTitleIndex = +TitleHelper::TITLE_BEGIN;
         nTitleIndex < +TitleHelper::NORMAL_TITLE_END;
         nTitleIndex++)
    {
        rtl::Reference< Title > xTitle =  TitleHelper::getTitle(
            static_cast< TitleHelper::eTitleType >( nTitleIndex ), xChartModel );
        pExistenceList[nTitleIndex] = xTitle.is();
        pTextList[nTitleIndex]=TitleHelper::getCompleteString( xTitle );
    }
}

bool TitleDialogData::writeDifferenceToModel(
                          const rtl::Reference<::chart::ChartModel>& xChartModel
                        , const uno::Reference< uno::XComponentContext >& xContext
                        , const TitleDialogData* pOldState )
{
    bool bChanged = false;
    for( auto nN = +TitleHelper::TITLE_BEGIN;
         nN < +TitleHelper::NORMAL_TITLE_END;
         nN++)
    {
        if( !pOldState || ( pOldState->aExistenceList[nN] != aExistenceList[nN] ) )
        {
            if(aExistenceList[nN])
            {
                TitleHelper::createTitle(
                    static_cast< TitleHelper::eTitleType >( nN ), aTextList[nN], xChartModel, xContext,
                    apReferenceSizeProvider.get() );
                bChanged = true;
            }
            else
            {
                TitleHelper::removeTitle( static_cast< TitleHelper::eTitleType >( nN ), xChartModel );
                bChanged = true;
            }
        }
        else if( !pOldState || ( pOldState->aTextList[nN] != aTextList[nN] ) )
        {
            //change content
            rtl::Reference< Title > xTitle(
                TitleHelper::getTitle( static_cast< TitleHelper::eTitleType >( nN ), xChartModel ) );
            if(xTitle.is())
            {
                TitleHelper::setCompleteString( aTextList[nN], xTitle, xContext, nullptr, true );
                bChanged = true;
            }
        }
    }
    return bChanged;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

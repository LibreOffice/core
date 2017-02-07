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

#include "TitleDialogData.hxx"
#include "TitleHelper.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

TitleDialogData::TitleDialogData( ReferenceSizeProvider* pRefSizeProvider )
        : aPossibilityList(7)
        , aExistenceList(7)
        , aTextList(7)
        , apReferenceSizeProvider( pRefSizeProvider )
{
    for (sal_Int32 i = 0; i < 7; i++)
    {
        aPossibilityList[i] = true;
        aExistenceList[i] = false;
    }
}

void TitleDialogData::readFromModel( const uno::Reference< frame::XModel>& xChartModel )
{
    uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram(xChartModel);

    //get possibilities
    uno::Sequence< sal_Bool > aAxisPossibilityList;
    AxisHelper::getAxisOrGridPossibilities( aAxisPossibilityList, xDiagram );
    this->aPossibilityList[2]=aAxisPossibilityList[0];//x axis title
    this->aPossibilityList[3]=aAxisPossibilityList[1];//y axis title
    this->aPossibilityList[4]=aAxisPossibilityList[2];//z axis title
    this->aPossibilityList[5]=aAxisPossibilityList[3];//secondary x axis title
    this->aPossibilityList[6]=aAxisPossibilityList[4];//secondary y axis title

    //find out which title exists and get their text
    //main title:
    for( sal_Int32 nTitleIndex = static_cast< sal_Int32 >( TitleHelper::TITLE_BEGIN);
         nTitleIndex < static_cast< sal_Int32 >( TitleHelper::NORMAL_TITLE_END );
         nTitleIndex++)
    {
        uno::Reference< XTitle > xTitle =  TitleHelper::getTitle(
            static_cast< TitleHelper::eTitleType >( nTitleIndex ), xChartModel );
        this->aExistenceList[nTitleIndex] = xTitle.is();
        this->aTextList[nTitleIndex]=TitleHelper::getCompleteString( xTitle );
    }
}

bool TitleDialogData::writeDifferenceToModel(
                          const uno::Reference< frame::XModel >& xChartModel
                        , const uno::Reference< uno::XComponentContext >& xContext
                        , TitleDialogData* pOldState )
{
    bool bChanged = false;
    for( sal_Int32 nN = static_cast< sal_Int32 >( TitleHelper::TITLE_BEGIN );
         nN < static_cast< sal_Int32 >( TitleHelper::NORMAL_TITLE_END );
         nN++)
    {
        if( !pOldState || ( pOldState->aExistenceList[nN] != this->aExistenceList[nN] ) )
        {
            if(this->aExistenceList[nN])
            {
                TitleHelper::createTitle(
                    static_cast< TitleHelper::eTitleType >( nN ), this->aTextList[nN], xChartModel, xContext,
                    apReferenceSizeProvider.get() );
                bChanged = true;
            }
            else
            {
                TitleHelper::removeTitle( static_cast< TitleHelper::eTitleType >( nN ), xChartModel );
                bChanged = true;
            }
        }
        else if( !pOldState || ( pOldState->aTextList[nN] != this->aTextList[nN] ) )
        {
            //change content
            uno::Reference< XTitle > xTitle(
                TitleHelper::getTitle( static_cast< TitleHelper::eTitleType >( nN ), xChartModel ) );
            if(xTitle.is())
            {
                TitleHelper::setCompleteString( this->aTextList[nN], xTitle, xContext );
                bChanged = true;
            }
        }
    }
    return bChanged;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

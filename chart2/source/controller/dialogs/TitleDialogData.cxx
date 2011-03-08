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
#include "precompiled_chart2.hxx"

#include "TitleDialogData.hxx"
#include "TitleHelper.hxx"
#include "ChartModelHelper.hxx"
#include "AxisHelper.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

TitleDialogData::TitleDialogData( ::std::auto_ptr< ReferenceSizeProvider > apRefSizeProvider )
        : aPossibilityList(7)
        , aExistenceList(7)
        , aTextList(7)
        , apReferenceSizeProvider( apRefSizeProvider )
{
    sal_Int32 nN = 0;
    for(nN=7;nN--;)
        aPossibilityList[nN]=sal_True;
    for(nN=7;nN--;)
        aExistenceList[nN]=sal_False;
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

    //find out which title exsist and get their text
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

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

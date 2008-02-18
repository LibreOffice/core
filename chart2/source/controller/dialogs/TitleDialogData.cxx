/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TitleDialogData.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:42:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

/*************************************************************************
 *
 *  $RCSfile: TitleHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 12:49:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "TitleHelper.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "ContextHelper.hxx"

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

rtl::OUString TitleHelper::getIdentifierForTitle( TitleHelper::eTitleType nTitleIndex )
{
    switch( nTitleIndex )
    {
        case MAIN_TITLE:
        {
            static rtl::OUString m_aIdentifier( C2U( "@main-title" ) );
            return m_aIdentifier;
        }
        case SUB_TITLE:
        {
            static rtl::OUString m_aIdentifier( C2U( "@sub-title" ) );
            return m_aIdentifier;
        }
        case X_AXIS_TITLE:
        {
            static rtl::OUString m_aIdentifier( C2U( "@xaxis-title" ) );
            return m_aIdentifier;
        }
        case Y_AXIS_TITLE:
        {
            static rtl::OUString m_aIdentifier( C2U( "@yaxis-title" ) );
            return m_aIdentifier;
        }
        case Z_AXIS_TITLE:
        {
            static rtl::OUString m_aIdentifier( C2U( "@zaxis-title" ) );
            return m_aIdentifier;
        }
        default:
            OSL_ENSURE( false, "Unsupported Title-Type requested" );
            return ::rtl::OUString();
    }
}

uno::Reference< XTitled > lcl_getTitleParent( TitleHelper::eTitleType nTitleIndex
                                              , const uno::Reference< frame::XModel >& xModel )
{
    switch( nTitleIndex )
    {
        case TitleHelper::MAIN_TITLE:
            return uno::Reference< XTitled >( xModel, uno::UNO_QUERY );
        case TitleHelper::SUB_TITLE:
        {
            uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
            if( !xChartDoc.is())
                return NULL;
            return uno::Reference< XTitled >( xChartDoc->getDiagram(), uno::UNO_QUERY );
        }
        case TitleHelper::X_AXIS_TITLE:
            //@todo
            break;
        case TitleHelper::Y_AXIS_TITLE:
            //@todo
            break;
        case TitleHelper::Z_AXIS_TITLE:
            //@todo
            break;
        default:
            OSL_ENSURE( false, "Unsupported Title-Type requested" );
            break;
    }

    return uno::Reference< XTitled >();
}

uno::Reference< XTitle > TitleHelper::getTitle( TitleHelper::eTitleType nTitleIndex
                            , const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xModel ) );
    if( xTitled.is())
        return xTitled->getTitle();
    return NULL;
}

uno::Reference< XTitle > TitleHelper::createTitle(
      TitleHelper::eTitleType nTitleIndex
    , const rtl::OUString& rTitleText
    , const uno::Reference< frame::XModel >& xModel
    , const uno::Reference< uno::XComponentContext > & xContext )
{
    if( !rTitleText.getLength() )
        return NULL;

    uno::Reference< XTitle > xTitle(NULL);
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xModel ) );

    ContextHelper::tContextEntryMapType aContextValues(
            ContextHelper::MakeContextEntryMap( C2U( "Identifier" )
            , uno::makeAny( TitleHelper::getIdentifierForTitle(nTitleIndex) )) );
    uno::Reference< uno::XComponentContext > xNewContext(
        ContextHelper::createContext( aContextValues, xContext ) );

    if(xNewContext.is() && xTitled.is())
    {
        xTitle.set( xContext->getServiceManager()->createInstanceWithContext(
                C2U( "drafts.com.sun.star.chart2.Title" ),
                    xNewContext ), uno::UNO_QUERY );

        if(xTitle.is())
        {
            setCompleteString( rTitleText, xTitle, xNewContext );
            xTitled->setTitle( xTitle );
        }
    }
    return xTitle;
}

rtl::OUString TitleHelper::getCompleteString( const uno::Reference< XTitle >& xTitle )
{
    rtl::OUString aRet;
    if(!xTitle.is())
        return aRet;
    uno::Sequence< uno::Reference< XFormattedString > > aStringList = xTitle->getText();
    for( sal_Int32 nN=0; nN<aStringList.getLength();nN++ )
        aRet += aStringList[nN]->getString();
    return aRet;
}

void TitleHelper::setCompleteString( const rtl::OUString& rNewText
                    , const uno::Reference< XTitle >& xTitle
                    , const uno::Reference< uno::XComponentContext > & xContext )
{
    //the format of the first old text portion will be maintained if there is any
    if(!xTitle.is())
        return;

    uno::Sequence< uno::Reference< XFormattedString > > aNewStringList(1);

    uno::Sequence< uno::Reference< XFormattedString > >  aOldStringList = xTitle->getText();
    if( aOldStringList.getLength() )
    {
        aNewStringList[0].set( aOldStringList[0] );
        aNewStringList[0]->setString( rNewText );
    }
    else
    {
        uno::Reference< uno::XInterface > xI(
            xContext->getServiceManager()->createInstanceWithContext(
            C2U( "drafts.com.sun.star.chart2.FormattedString" ), xContext ) );
        uno::Reference< XFormattedString > xFormattedString( xI, uno::UNO_QUERY );

        if(xFormattedString.is())
        {
            xFormattedString->setString( rNewText );
            aNewStringList[0].set( xFormattedString );
        }
    }
    xTitle->setText( aNewStringList );
}

void TitleHelper::removeTitle( TitleHelper::eTitleType nTitleIndex
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xModel )
{
    uno::Reference< XTitled > xTitled( lcl_getTitleParent( nTitleIndex, xModel ) );
    if( xTitled.is())
    {
        xTitled->setTitle(NULL);
    }
}

//.............................................................................
} //namespace chart
//.............................................................................


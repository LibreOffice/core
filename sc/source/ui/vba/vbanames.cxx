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
#include <vbahelper/helperdecl.hxx>

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>

#include "vbanames.hxx"
#include "vbaname.hxx"
#include "vbarange.hxx"
#include "vbaglobals.hxx"
#include <vector>
#include <vcl/msgbox.hxx>
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "compiler.hxx"
#include "tokenarray.hxx"
#include "cellsuno.hxx"

#include <memory>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class NamesEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > m_xModel;
    uno::WeakReference< XHelperInterface > m_xParent;
    uno::Reference< sheet::XNamedRanges > m_xNames;
public:
    NamesEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel , const uno::Reference< sheet::XNamedRanges >& xNames ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_xModel( xModel ), m_xParent( xParent ), m_xNames( xNames ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        uno::Reference< sheet::XNamedRange > xNamed( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return uno::makeAny( uno::Reference< excel::XName > ( new ScVbaName( m_xParent, m_xContext, xNamed ,m_xNames , m_xModel ) ) );
    }

};

ScVbaNames::ScVbaNames(const css::uno::Reference< ov::XHelperInterface >& xParent,
            const css::uno::Reference< css::uno::XComponentContext >& xContext,
            const css::uno::Reference< css::sheet::XNamedRanges >& xNames,
            const css::uno::Reference< css::frame::XModel >& xModel ):
            ScVbaNames_BASE(  xParent , xContext , uno::Reference< container::XIndexAccess >( xNames, uno::UNO_QUERY ) ),
            mxModel( xModel ),
            mxNames( xNames )
{
    m_xNameAccess.set( xNames, uno::UNO_QUERY_THROW );
}

ScVbaNames::~ScVbaNames()
{
}

ScDocument *
ScVbaNames::getScDocument()
{
    uno::Reference< frame::XModel > xModel( getModel() , uno::UNO_QUERY_THROW );
    ScTabViewShell * pTabViewShell = excel::getBestViewShell( xModel );
    if ( !pTabViewShell )
        throw uno::RuntimeException( "No ViewShell available" );
    ScViewData& rViewData = pTabViewShell->GetViewData();
    return rViewData.GetDocument();
}

css::uno::Any
ScVbaNames::Add( const css::uno::Any& Name ,
                                        const css::uno::Any& RefersTo,
                                        const css::uno::Any& /*Visible*/,
                                        const css::uno::Any& /*MacroType*/,
                                        const css::uno::Any& /*ShoutcutKey*/,
                                        const css::uno::Any& /*Category*/,
                                        const css::uno::Any& NameLocal,
                                        const css::uno::Any& /*RefersToLocal*/,
                                        const css::uno::Any& /*CategoryLocal*/,
                                        const css::uno::Any& RefersToR1C1,
                                        const css::uno::Any& RefersToR1C1Local )
    throw (css::uno::RuntimeException, std::exception)
{
    OUString sName;
    uno::Reference< excel::XRange > xRange;
    if ( Name.hasValue() )
        Name >>= sName;
    else if ( NameLocal.hasValue() )
        NameLocal >>= sName;
    if ( !sName.isEmpty() )
    {
        if ( !ScRangeData::IsNameValid( sName , getScDocument() ) )
        {
            OUString sResult ;
            sal_Int32 nToken = 0;
            sal_Int32 nIndex = 0;
            sResult = sName.getToken( nToken , '!' , nIndex );
            if ( -1 == nIndex )
                sResult = sName;
            else
                sResult = sName.copy( nIndex );
            sName = sResult ;
            if ( !ScRangeData::IsNameValid( sName , getScDocument() ) )
                throw uno::RuntimeException( "This Name is not valid ." );
        }
    }
    uno::Reference< table::XCellRange > xUnoRange;
    if ( RefersTo.hasValue() || RefersToR1C1.hasValue() || RefersToR1C1Local.hasValue() )
    {
        OUString sFormula;

        formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_NATIVE_XL_A1;
        if ( RefersTo.hasValue() )
        {
            if ( RefersTo.getValueTypeClass() == uno::TypeClass_STRING )
                RefersTo >>= sFormula;
            else
                RefersTo >>= xRange;
        }
        if ( RefersToR1C1.hasValue() )
        {
            if ( RefersToR1C1.getValueTypeClass() == uno::TypeClass_STRING )
            {
                RefersToR1C1 >>= sFormula;
                eGram = formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1;
            }
            else
                RefersToR1C1 >>= xRange;
        }
        if ( RefersToR1C1Local.hasValue() )
        {
            if ( RefersToR1C1Local.getValueTypeClass() == uno::TypeClass_STRING )
            {
                RefersToR1C1Local >>= sFormula;
                eGram = formula::FormulaGrammar::GRAM_NATIVE_XL_R1C1;
            }
            else
                RefersToR1C1Local >>= xRange;
        }
        if ( !xRange.is() && !sFormula.isEmpty() )
        {
            ScAddress aBlank;
            ScCompiler aComp( getScDocument(), aBlank );
            aComp.SetGrammar( eGram );
            std::unique_ptr<ScTokenArray> pTokens(aComp.CompileString(sFormula));
            if ( pTokens )
            {
                ScRange aRange;
                ScDocShell* pDocSh = excel::getDocShell(getModel());
                if (pTokens->IsValidReference(aRange, aBlank))
                    xUnoRange =  new ScCellRangeObj( pDocSh, aRange );
                else
                {
                    // assume it's an address try strip the '=' if it's there
                    // and try and create a range ( must be a better way )
                    if ( sFormula.startsWith("=") )
                        sFormula = sFormula.copy(1);
                    ScRangeList aCellRanges;
                    sal_uInt16 nFlags = 0;

                    formula::FormulaGrammar::AddressConvention eConv = ( eGram == formula::FormulaGrammar::GRAM_NATIVE_XL_A1 ) ? formula::FormulaGrammar::CONV_XL_A1 : formula::FormulaGrammar::CONV_XL_R1C1;
                    if ( ScVbaRange::getCellRangesForAddress( nFlags, sFormula, pDocSh, aCellRanges, eConv , ',' ) )
                    {
                        if ( aCellRanges.size() == 1 )
                            xUnoRange =  new ScCellRangeObj( pDocSh, *aCellRanges.front() );
                        else
                        {
                            uno::Reference< sheet::XSheetCellRangeContainer > xRanges( new ScCellRangesObj( pDocSh, aCellRanges ) );
                            xRange = new ScVbaRange( mxParent, mxContext, xRanges );
                        }
                    }

                }
            }
        }
    }

    if ( xRange.is() || xUnoRange.is() )
    {
        if ( !xRange.is() )
            xRange = new ScVbaRange( mxParent, mxContext, xUnoRange );

        uno::Reference< excel::XRange > xArea( xRange->Areas( uno::makeAny( sal_Int32(1) ) ), uno::UNO_QUERY );

        uno::Any xAny = xArea->getCellRange() ;

        uno::Reference< sheet::XCellRangeAddressable > thisRangeAdd( xAny, ::uno::UNO_QUERY_THROW);

        table::CellRangeAddress aAddr = thisRangeAdd->getRangeAddress();
        ScAddress aPos( static_cast< SCCOL >( aAddr.StartColumn ) , static_cast< SCROW >( aAddr.StartRow ) , static_cast< SCTAB >(aAddr.Sheet ) );
        uno::Any xAny2 ;
        if ( mxNames.is() )
        {
            sal_Int32 nUnoType = 0;
            table::CellAddress aCellAddr( aAddr.Sheet , aAddr.StartColumn , aAddr.StartRow );
            if ( mxNames->hasByName( sName ) )
                mxNames->removeByName(sName);
            OUString sTmp = "$";
            uno::Reference< ov::XCollection > xCol( xRange->Areas( uno::Any() ), uno::UNO_QUERY );
            for ( sal_Int32 nArea = 1; nArea <= xCol->getCount(); ++nArea )
            {
                xArea.set( xRange->Areas( uno::makeAny( nArea ) ), uno::UNO_QUERY_THROW );

                OUString sRangeAdd = xArea->Address( xAny2, xAny2 , xAny2 , xAny2, xAny2 );
                if ( nArea > 1 )
                    sTmp += ",";
                sTmp = sTmp + "'" + xRange->getWorksheet()->getName() + "'." + sRangeAdd;
            }
            mxNames->addNewByName( sName , sTmp , aCellAddr , nUnoType);
            return Item( uno::makeAny( sName ), uno::Any() );
        }
    }
    return css::uno::Any();
}

// XEnumerationAccess
css::uno::Type
ScVbaNames::getElementType() throw( css::uno::RuntimeException )
{
    return cppu::UnoType<ov::excel::XName>::get();
}

uno::Reference< container::XEnumeration >
ScVbaNames::createEnumeration() throw (uno::RuntimeException)
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( mxNames, uno::UNO_QUERY_THROW );
    return new NamesEnumeration( getParent(), mxContext, xEnumAccess->createEnumeration(), mxModel , mxNames );
}

uno::Any
ScVbaNames::createCollectionObject( const uno::Any& aSource )
{
    uno::Reference< sheet::XNamedRange > xName( aSource, uno::UNO_QUERY );
    return uno::makeAny( uno::Reference< excel::XName > ( new ScVbaName( getParent(), mxContext, xName, mxNames , mxModel ) ) );
}

OUString
ScVbaNames::getServiceImplName()
{
    return OUString( "ScVbaNames" );
}

css::uno::Sequence<OUString>
ScVbaNames::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.NamedRanges";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

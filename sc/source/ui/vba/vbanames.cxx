/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include <vbahelper/helperdecl.hxx>

#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>

#include "vbanames.hxx"
#include "vbaname.hxx"
#include "vbarange.hxx"
#include "vbaglobals.hxx"
#include <vector>
#include <rangenam.hxx>
#include <vcl/msgbox.hxx>
#include "tabvwsh.hxx"
#include "viewdata.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

class NamesEnumeration : public EnumerationHelperImpl
{
    uno::Reference< frame::XModel > m_xModel;
    uno::WeakReference< XHelperInterface > m_xParent;
    uno::Reference< sheet::XNamedRanges > m_xNames;
public:
    NamesEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration,  const uno::Reference< frame::XModel >& xModel , const uno::Reference< sheet::XNamedRanges >& xNames ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_xModel( xModel ), m_xParent( xParent ), m_xNames( xNames ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
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
        throw uno::RuntimeException( rtl::OUString::createFromAscii("No ViewShell available"), uno::Reference< uno::XInterface >() );
    ScViewData* pViewData = pTabViewShell->GetViewData();
    if ( !pViewData )
        throw uno::RuntimeException( rtl::OUString::createFromAscii("No ViewData available"), uno::Reference< uno::XInterface >() );
    return pViewData->GetDocument();
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
                                        const css::uno::Any& RefersToR1C1Local ) throw (css::uno::RuntimeException)
{
    rtl::OUString sName;
    uno::Reference< excel::XRange > xRange;
    if ( Name.hasValue() )
        Name >>= sName;
    else if ( NameLocal.hasValue() )
        NameLocal >>= sName;
    if ( sName.getLength() != 0 )
    {
        if ( !ScRangeData::IsNameValid( sName , getScDocument() ) )
        {
            ::rtl::OUString sResult ;
            sal_Int32 nToken = 0;
            sal_Int32 nIndex = 0;
            sResult = sName.getToken( nToken , '!' , nIndex );
            if ( -1 == nIndex )
                sResult = sName;
            else
                sResult = sName.copy( nIndex );
            sName = sResult ;
            if ( !ScRangeData::IsNameValid( sName , getScDocument() ) )
                throw uno::RuntimeException( rtl::OUString::createFromAscii("This Name is a valid ."), uno::Reference< uno::XInterface >() );
        }
    }
    if ( RefersTo.hasValue() || RefersToR1C1.hasValue() || RefersToR1C1Local.hasValue() )
    {
        if ( RefersTo.hasValue() )
            RefersTo >>= xRange;
        if ( RefersToR1C1.hasValue() )
            RefersToR1C1 >>= xRange;
        if ( RefersToR1C1Local.hasValue() )
            RefersToR1C1Local >>= xRange;
    }

    if ( xRange.is() )
    {
        ScVbaRange* pRange = dynamic_cast< ScVbaRange* >( xRange.get() );
        uno::Reference< table::XCellRange > thisRange ;
        uno::Any xAny = pRange->getCellRange() ;
        if ( xAny.hasValue() )
            xAny >>= thisRange;
        uno::Reference< sheet::XCellRangeAddressable > thisRangeAdd( thisRange, ::uno::UNO_QUERY_THROW);
        table::CellRangeAddress aAddr = thisRangeAdd->getRangeAddress();
        ScAddress aPos( static_cast< SCCOL >( aAddr.StartColumn ) , static_cast< SCROW >( aAddr.StartRow ) , static_cast< SCTAB >(aAddr.Sheet ) );
        uno::Any xAny2 ;
        String sRangeAdd = xRange->Address( xAny2, xAny2 , xAny2 , xAny2, xAny2 );
        String sTmp;
        sTmp += String::CreateFromAscii("$");
        sTmp += UniString(xRange->getWorksheet()->getName());
        sTmp += String::CreateFromAscii(".");
        sTmp += sRangeAdd;
        if ( mxNames.is() )
        {
            RangeType nType = RT_NAME;
            table::CellAddress aCellAddr( aAddr.Sheet , aAddr.StartColumn , aAddr.StartRow );
            if ( mxNames->hasByName( sName ) )
                mxNames->removeByName(sName);
            mxNames->addNewByName( sName , rtl::OUString(sTmp) , aCellAddr , (sal_Int32)nType);
            uno::Reference< sheet::XNamedRange > xName( mxNames->getByName( sName ), uno::UNO_QUERY_THROW );
            return uno::Any( uno::Reference< excel::XName >( new ScVbaName( getParent(), mxContext, xName, mxNames, mxModel ) ) );
        }
    }
    return css::uno::Any();
}

// XEnumerationAccess
css::uno::Type
ScVbaNames::getElementType() throw( css::uno::RuntimeException )
{
    return ov::excel::XName::static_type(0);
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

rtl::OUString&
ScVbaNames::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaNames") );
    return sImplName;
}

css::uno::Sequence<rtl::OUString>
ScVbaNames::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.NamedRanges" ) );
    }
    return aServiceNames;
}



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
#include "vbapagebreaks.hxx"
#include "vbapagebreak.hxx"
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XWorksheet.hpp>
using namespace ::com::sun::star;
using namespace ::ooo::vba;

class RangePageBreaks : public ::cppu::WeakImplHelper<container::XIndexAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< sheet::XSheetPageBreak > mxSheetPageBreak;
    bool m_bColumn;

public:
    RangePageBreaks( const uno::Reference< XHelperInterface >& xParent,
                     const uno::Reference< uno::XComponentContext >& xContext,
                     uno::Reference< sheet::XSheetPageBreak >& xSheetPageBreak,
                     bool bColumn ) : mxParent( xParent ), mxContext( xContext ), mxSheetPageBreak( xSheetPageBreak ), m_bColumn( bColumn )
    {
    }

    sal_Int32 getAPIStartofRange( const uno::Reference< excel::XRange >& xRange ) throw (css::uno::RuntimeException)
    {
        if( m_bColumn )
            return xRange->getColumn() - 1;
        return xRange->getRow() - 1;
    }

    sal_Int32 getAPIEndIndexofRange( const uno::Reference< excel::XRange >& xRange, sal_Int32 nUsedStart ) throw (uno::RuntimeException)
    {
        if( m_bColumn )
            return nUsedStart + xRange->Columns( uno::Any() )->getCount() - 1;
        return nUsedStart + xRange->Rows( uno::Any() )->getCount();
    }

    uno::Sequence<sheet::TablePageBreakData> getAllPageBreaks() throw (uno::RuntimeException)
    {
        if( m_bColumn )
            return mxSheetPageBreak->getColumnPageBreaks();
        return mxSheetPageBreak->getRowPageBreaks();
    }

    uno::Reference<container::XIndexAccess> getRowColContainer() throw (uno::RuntimeException)
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange( mxSheetPageBreak, uno::UNO_QUERY_THROW );
        uno::Reference<container::XIndexAccess> xIndexAccess;
        if( m_bColumn )
            xIndexAccess.set( xColumnRowRange->getColumns(), uno::UNO_QUERY_THROW );
        else
            xIndexAccess.set( xColumnRowRange->getRows(), uno::UNO_QUERY_THROW );
        return xIndexAccess;
    }

    sheet::TablePageBreakData getTablePageBreakData( sal_Int32 nAPIItemIndex ) throw (uno::RuntimeException);
    uno::Any Add( const css::uno::Any& Before ) throw ( css::script::BasicErrorException, css::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException, std::exception) override;
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override;
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException, std::exception) override
    {
        if( m_bColumn )
             return cppu::UnoType<excel::XVPageBreak>::get();
        return  cppu::UnoType<excel::XHPageBreak>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return true;
    }
};

/** @TODO Unlike MS Excel this method only considers the pagebreaks that intersect the used range
*  To become completely compatible the print area has to be considered. As far as I found out this printarea
*  also considers the position and sizes of shapes and manually inserted page breaks
*  Note: In MS  there is a limit of 1026 horizontal page breaks per sheet.
*/
sal_Int32 SAL_CALL RangePageBreaks::getCount(  ) throw (uno::RuntimeException, std::exception)
{
    sal_Int32 nCount = 0;
    uno::Reference< excel::XWorksheet > xWorksheet( mxParent, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XRange > xRange = xWorksheet->getUsedRange();
    sal_Int32 nUsedStart = getAPIStartofRange( xRange );
    sal_Int32 nUsedEnd = getAPIEndIndexofRange( xRange, nUsedStart );
    uno::Sequence<sheet::TablePageBreakData> aTablePageBreakData = getAllPageBreaks();

    sal_Int32 nLength = aTablePageBreakData.getLength();
    for( sal_Int32 i=0; i<nLength; i++ )
    {
        sal_Int32 nPos = aTablePageBreakData[i].Position;
        if( nPos > nUsedEnd + 1)
            return nCount;
        nCount++;
    }

    return nCount;
}

uno::Any SAL_CALL RangePageBreaks::getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if( (Index < getCount()) && ( Index >= 0 ))
    {
        sheet::TablePageBreakData aTablePageBreakData = getTablePageBreakData( Index );
        uno::Reference< container::XIndexAccess > xIndexAccess = getRowColContainer();
        sal_Int32 nPos = aTablePageBreakData.Position;
        if( (nPos < xIndexAccess->getCount()) && (nPos > -1) )
        {
            uno::Reference< beans::XPropertySet > xRowColPropertySet( xIndexAccess->getByIndex(nPos), uno::UNO_QUERY_THROW );
            if( m_bColumn )
                return uno::makeAny( uno::Reference< excel::XVPageBreak >( new ScVbaVPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
            return uno::makeAny( uno::Reference< excel::XHPageBreak >( new ScVbaHPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
        }
    }
    throw lang::IndexOutOfBoundsException();
}

sheet::TablePageBreakData RangePageBreaks::getTablePageBreakData( sal_Int32 nAPIItemIndex ) throw (uno::RuntimeException)
{
    sal_Int32 index = -1;
    sheet::TablePageBreakData aTablePageBreakData;
    uno::Reference< excel::XWorksheet > xWorksheet( mxParent, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XRange > xRange = xWorksheet->getUsedRange();
    sal_Int32 nUsedStart = getAPIStartofRange( xRange );
    sal_Int32 nUsedEnd = getAPIEndIndexofRange( xRange, nUsedStart );
    uno::Sequence<sheet::TablePageBreakData> aTablePageBreakDataList = getAllPageBreaks();

    sal_Int32 nLength = aTablePageBreakDataList.getLength();
    for( sal_Int32 i=0; i<nLength; i++ )
    {
        aTablePageBreakData = aTablePageBreakDataList[i];
        sal_Int32 nPos = aTablePageBreakData.Position;
        if( nPos > nUsedEnd + 1 )
            DebugHelper::runtimeexception(ERRCODE_BASIC_METHOD_FAILED, OUString());
        index++;
        if( index == nAPIItemIndex )
            return aTablePageBreakData;
    }

    return aTablePageBreakData;
}

uno::Any RangePageBreaks::Add( const css::uno::Any& Before ) throw ( css::script::BasicErrorException, css::uno::RuntimeException)
{
    uno::Reference< excel::XRange > xRange;
    Before >>= xRange;
    if( !xRange.is() )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, OUString());
    }

    sal_Int32 nAPIRowColIndex = getAPIStartofRange( xRange );
    uno::Reference< container::XIndexAccess > xIndexAccess = getRowColContainer();
    uno::Reference< beans::XPropertySet > xRowColPropertySet( xIndexAccess->getByIndex(nAPIRowColIndex), uno::UNO_QUERY_THROW );
    xRowColPropertySet->setPropertyValue("IsStartOfNewPage", uno::makeAny(true));
    sheet::TablePageBreakData aTablePageBreakData;
    aTablePageBreakData.ManualBreak = true;
    aTablePageBreakData.Position = nAPIRowColIndex;
    if( m_bColumn )
        return uno::makeAny( uno::Reference< excel::XVPageBreak >( new ScVbaVPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
    return uno::makeAny( uno::Reference< excel::XHPageBreak >( new ScVbaHPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
}

class RangePageBreaksEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    sal_Int32 nIndex;
public:
    explicit RangePageBreaksEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : m_xIndexAccess( xIndexAccess ), nIndex( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( nIndex < m_xIndexAccess->getCount() )
            return m_xIndexAccess->getByIndex( nIndex++ );
        throw container::NoSuchElementException();
    }
};

ScVbaHPageBreaks::ScVbaHPageBreaks( const uno::Reference< XHelperInterface >& xParent,
                                    const uno::Reference< uno::XComponentContext >& xContext,
                                    uno::Reference< sheet::XSheetPageBreak >& xSheetPageBreak) throw (uno::RuntimeException):
                          ScVbaHPageBreaks_BASE( xParent,xContext, new RangePageBreaks( xParent, xContext, xSheetPageBreak, false ))
{
}

uno::Any SAL_CALL ScVbaHPageBreaks::Add( const uno::Any& Before) throw ( script::BasicErrorException, uno::RuntimeException, std::exception)
{
    RangePageBreaks* pPageBreaks = dynamic_cast< RangePageBreaks* >( m_xIndexAccess.get() );
    if( pPageBreaks )
    {
        return pPageBreaks->Add( Before );
    }
    return uno::Any();
}

uno::Reference< container::XEnumeration >
ScVbaHPageBreaks::createEnumeration() throw (uno::RuntimeException)
{
    return new RangePageBreaksEnumWrapper( m_xIndexAccess );
}

uno::Any
ScVbaHPageBreaks::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource; // its already a pagebreak object
}

uno::Type
ScVbaHPageBreaks::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<excel::XHPageBreak>::get();
}

OUString
ScVbaHPageBreaks::getServiceImplName()
{
    return OUString("ScVbaHPageBreaks");
}

uno::Sequence< OUString >
ScVbaHPageBreaks::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.HPageBreaks";
    }
    return aServiceNames;
}

//VPageBreak
ScVbaVPageBreaks::ScVbaVPageBreaks( const uno::Reference< XHelperInterface >& xParent,
                                    const uno::Reference< uno::XComponentContext >& xContext,
                                    uno::Reference< sheet::XSheetPageBreak >& xSheetPageBreak ) throw ( uno::RuntimeException )
:   ScVbaVPageBreaks_BASE( xParent, xContext, new RangePageBreaks( xParent, xContext, xSheetPageBreak, true ) )
{
}

ScVbaVPageBreaks::~ScVbaVPageBreaks()
{
}

uno::Any SAL_CALL
ScVbaVPageBreaks::Add( const uno::Any& Before ) throw ( script::BasicErrorException, uno::RuntimeException, std::exception )
{
    RangePageBreaks* pPageBreaks = dynamic_cast< RangePageBreaks* >( m_xIndexAccess.get() );
    if( pPageBreaks )
    {
        return pPageBreaks->Add( Before );
    }
    return uno::Any();
}

uno::Reference< container::XEnumeration >
ScVbaVPageBreaks::createEnumeration() throw ( uno::RuntimeException )
{
    return new RangePageBreaksEnumWrapper( m_xIndexAccess );
}

uno::Any
ScVbaVPageBreaks::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource; // its already a pagebreak object
}

uno::Type
ScVbaVPageBreaks::getElementType() throw ( uno::RuntimeException )
{
    return cppu::UnoType<excel::XVPageBreak>::get();
}

OUString
ScVbaVPageBreaks::getServiceImplName()
{
    return OUString("ScVbaVPageBreaks");
}

uno::Sequence< OUString >
ScVbaVPageBreaks::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.VPageBreaks";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

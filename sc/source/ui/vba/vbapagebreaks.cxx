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
#include <basic/sberrors.hxx>
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XWorksheet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sheet/XSheetPageBreak.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <utility>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

namespace {

class RangePageBreaks : public ::cppu::WeakImplHelper<container::XIndexAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< sheet::XSheetPageBreak > mxSheetPageBreak;
    bool m_bColumn;

public:
    RangePageBreaks( uno::Reference< XHelperInterface > xParent,
                     uno::Reference< uno::XComponentContext > xContext,
                     uno::Reference< sheet::XSheetPageBreak > xSheetPageBreak,
                     bool bColumn ) : mxParent(std::move( xParent )), mxContext(std::move( xContext )), mxSheetPageBreak(std::move( xSheetPageBreak )), m_bColumn( bColumn )
    {
    }

    /// @throws css::uno::RuntimeException
    sal_Int32 getAPIStartofRange( const uno::Reference< excel::XRange >& xRange )
    {
        if( m_bColumn )
            return xRange->getColumn() - 1;
        return xRange->getRow() - 1;
    }

    /// @throws uno::RuntimeException
    sal_Int32 getAPIEndIndexofRange( const uno::Reference< excel::XRange >& xRange, sal_Int32 nUsedStart )
    {
        if( m_bColumn )
            return nUsedStart + xRange->Columns( uno::Any() )->getCount() - 1;
        return nUsedStart + xRange->Rows( uno::Any() )->getCount();
    }

    /// @throws uno::RuntimeException
    uno::Sequence<sheet::TablePageBreakData> getAllPageBreaks()
    {
        if( m_bColumn )
            return mxSheetPageBreak->getColumnPageBreaks();
        return mxSheetPageBreak->getRowPageBreaks();
    }

    /// @throws uno::RuntimeException
    uno::Reference<container::XIndexAccess> getRowColContainer() const
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange( mxSheetPageBreak, uno::UNO_QUERY_THROW );
        uno::Reference<container::XIndexAccess> xIndexAccess;
        if( m_bColumn )
            xIndexAccess.set( xColumnRowRange->getColumns(), uno::UNO_QUERY_THROW );
        else
            xIndexAccess.set( xColumnRowRange->getRows(), uno::UNO_QUERY_THROW );
        return xIndexAccess;
    }

    /// @throws uno::RuntimeException
    sheet::TablePageBreakData getTablePageBreakData( sal_Int32 nAPIItemIndex );
    /// @throws css::script::BasicErrorException
    /// @throws css::uno::RuntimeException
    uno::Any Add( const css::uno::Any& Before );

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) override;
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        if( m_bColumn )
             return cppu::UnoType<excel::XVPageBreak>::get();
        return  cppu::UnoType<excel::XHPageBreak>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
};

}

/** @TODO Unlike MS Excel this method only considers the pagebreaks that intersect the used range
*  To become completely compatible the print area has to be considered. As far as I found out this printarea
*  also considers the position and sizes of shapes and manually inserted page breaks
*  Note: In MS  there is a limit of 1026 horizontal page breaks per sheet.
*/
sal_Int32 SAL_CALL RangePageBreaks::getCount(  )
{
    uno::Reference< excel::XWorksheet > xWorksheet( mxParent, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XRange > xRange = xWorksheet->getUsedRange();
    sal_Int32 nUsedStart = getAPIStartofRange( xRange );
    sal_Int32 nUsedEnd = getAPIEndIndexofRange( xRange, nUsedStart );
    const uno::Sequence<sheet::TablePageBreakData> aTablePageBreakData = getAllPageBreaks();

    auto pPageBreak = std::find_if(aTablePageBreakData.begin(), aTablePageBreakData.end(),
        [nUsedEnd](const sheet::TablePageBreakData& rPageBreak) { return rPageBreak.Position > nUsedEnd + 1; });

    return static_cast<sal_Int32>(std::distance(aTablePageBreakData.begin(), pPageBreak));
}

uno::Any SAL_CALL RangePageBreaks::getByIndex( sal_Int32 Index )
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
                return uno::Any( uno::Reference< excel::XVPageBreak >( new ScVbaVPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
            return uno::Any( uno::Reference< excel::XHPageBreak >( new ScVbaHPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
        }
    }
    throw lang::IndexOutOfBoundsException();
}

sheet::TablePageBreakData RangePageBreaks::getTablePageBreakData( sal_Int32 nAPIItemIndex )
{
    sal_Int32 index = -1;
    sheet::TablePageBreakData aTablePageBreakData;
    uno::Reference< excel::XWorksheet > xWorksheet( mxParent, uno::UNO_QUERY_THROW );
    uno::Reference< excel::XRange > xRange = xWorksheet->getUsedRange();
    sal_Int32 nUsedStart = getAPIStartofRange( xRange );
    sal_Int32 nUsedEnd = getAPIEndIndexofRange( xRange, nUsedStart );
    const uno::Sequence<sheet::TablePageBreakData> aTablePageBreakDataList = getAllPageBreaks();

    for( const auto& rTablePageBreakData : aTablePageBreakDataList )
    {
        aTablePageBreakData = rTablePageBreakData;
        sal_Int32 nPos = aTablePageBreakData.Position;
        if( nPos > nUsedEnd + 1 )
            DebugHelper::runtimeexception(ERRCODE_BASIC_METHOD_FAILED);
        index++;
        if( index == nAPIItemIndex )
            return aTablePageBreakData;
    }

    return aTablePageBreakData;
}

uno::Any RangePageBreaks::Add( const css::uno::Any& Before )
{
    uno::Reference< excel::XRange > xRange;
    Before >>= xRange;
    if( !xRange.is() )
    {
        DebugHelper::basicexception(ERRCODE_BASIC_BAD_ARGUMENT, {});
    }

    sal_Int32 nAPIRowColIndex = getAPIStartofRange( xRange );
    uno::Reference< container::XIndexAccess > xIndexAccess = getRowColContainer();
    uno::Reference< beans::XPropertySet > xRowColPropertySet( xIndexAccess->getByIndex(nAPIRowColIndex), uno::UNO_QUERY_THROW );
    xRowColPropertySet->setPropertyValue(u"IsStartOfNewPage"_ustr, uno::Any(true));
    sheet::TablePageBreakData aTablePageBreakData;
    aTablePageBreakData.ManualBreak = true;
    aTablePageBreakData.Position = nAPIRowColIndex;
    if( m_bColumn )
        return uno::Any( uno::Reference< excel::XVPageBreak >( new ScVbaVPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
    return uno::Any( uno::Reference< excel::XHPageBreak >( new ScVbaHPageBreak( mxParent, mxContext, xRowColPropertySet, aTablePageBreakData) ));
}

namespace {

class RangePageBreaksEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference<container::XIndexAccess > m_xIndexAccess;
    sal_Int32 nIndex;
public:
    explicit RangePageBreaksEnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess ) : m_xIndexAccess(std::move( xIndexAccess )), nIndex( 0 ) {}
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( nIndex < m_xIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( nIndex < m_xIndexAccess->getCount() )
            return m_xIndexAccess->getByIndex( nIndex++ );
        throw container::NoSuchElementException();
    }
};

}

ScVbaHPageBreaks::ScVbaHPageBreaks( const uno::Reference< XHelperInterface >& xParent,
                                    const uno::Reference< uno::XComponentContext >& xContext,
                                    const uno::Reference< sheet::XSheetPageBreak >& xSheetPageBreak):
                          ScVbaHPageBreaks_BASE( xParent,xContext, new RangePageBreaks( xParent, xContext, xSheetPageBreak, false ))
{
}

uno::Any SAL_CALL ScVbaHPageBreaks::Add( const uno::Any& Before)
{
    RangePageBreaks* pPageBreaks = dynamic_cast< RangePageBreaks* >( m_xIndexAccess.get() );
    if( pPageBreaks )
    {
        return pPageBreaks->Add( Before );
    }
    return uno::Any();
}

uno::Reference< container::XEnumeration >
ScVbaHPageBreaks::createEnumeration()
{
    return new RangePageBreaksEnumWrapper( m_xIndexAccess );
}

uno::Any
ScVbaHPageBreaks::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource; // it's already a pagebreak object
}

uno::Type
ScVbaHPageBreaks::getElementType()
{
    return cppu::UnoType<excel::XHPageBreak>::get();
}

OUString
ScVbaHPageBreaks::getServiceImplName()
{
    return u"ScVbaHPageBreaks"_ustr;
}

uno::Sequence< OUString >
ScVbaHPageBreaks::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.HPageBreaks"_ustr
    };
    return aServiceNames;
}

//VPageBreak
ScVbaVPageBreaks::ScVbaVPageBreaks( const uno::Reference< XHelperInterface >& xParent,
                                    const uno::Reference< uno::XComponentContext >& xContext,
                                    const uno::Reference< sheet::XSheetPageBreak >& xSheetPageBreak )
:   ScVbaVPageBreaks_BASE( xParent, xContext, new RangePageBreaks( xParent, xContext, xSheetPageBreak, true ) )
{
}

ScVbaVPageBreaks::~ScVbaVPageBreaks()
{
}

uno::Any SAL_CALL
ScVbaVPageBreaks::Add( const uno::Any& Before )
{
    RangePageBreaks* pPageBreaks = dynamic_cast< RangePageBreaks* >( m_xIndexAccess.get() );
    if( pPageBreaks )
    {
        return pPageBreaks->Add( Before );
    }
    return uno::Any();
}

uno::Reference< container::XEnumeration >
ScVbaVPageBreaks::createEnumeration()
{
    return new RangePageBreaksEnumWrapper( m_xIndexAccess );
}

uno::Any
ScVbaVPageBreaks::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource; // it's already a pagebreak object
}

uno::Type
ScVbaVPageBreaks::getElementType()
{
    return cppu::UnoType<excel::XVPageBreak>::get();
}

OUString
ScVbaVPageBreaks::getServiceImplName()
{
    return u"ScVbaVPageBreaks"_ustr;
}

uno::Sequence< OUString >
ScVbaVPageBreaks::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.VPageBreaks"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

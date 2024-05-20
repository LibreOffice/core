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
#include "vbaglobals.hxx"

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include "vbaapplication.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::ooo::vba;

// ScVbaGlobals

//ScVbaGlobals::ScVbaGlobals(  css::uno::Reference< css::uno::XComponentContext >const& rxContext,  ) : ScVbaGlobals_BASE( uno::Reference< XHelperInterface >(), rxContext )

ScVbaGlobals::ScVbaGlobals( uno::Sequence< uno::Any > const& aArgs, uno::Reference< uno::XComponentContext >const& rxContext ) : ScVbaGlobals_BASE( uno::Reference< XHelperInterface >(), rxContext, "ExcelDocumentContext" )
{
    uno::Sequence< beans::PropertyValue > aInitArgs( aArgs.hasElements() ? 2 : 1 );
    auto pInitArgs = aInitArgs.getArray();
    pInitArgs[ 0 ].Name = "Application";
    pInitArgs[ 0 ].Value <<= getApplication();
    if ( aArgs.hasElements() )
    {
        pInitArgs[ 1 ].Name = "ExcelDocumentContext";
        pInitArgs[ 1 ].Value <<= getXSomethingFromArgs< frame::XModel >( aArgs, 0 );
    }
    init( aInitArgs );
}

ScVbaGlobals::~ScVbaGlobals()
{
}

// XGlobals

uno::Reference<excel::XApplication > const &
ScVbaGlobals::getApplication()
{
    if ( !mxApplication.is() )
        mxApplication.set( new ScVbaApplication( mxContext) );
    return mxApplication;
}

uno::Reference<excel::XApplication > SAL_CALL
ScVbaGlobals::getExcel()
{
       return getApplication();
}

uno::Reference< excel::XWorkbook > SAL_CALL
ScVbaGlobals::getActiveWorkbook()
{
    uno::Reference< excel::XWorkbook > xWorkbook( getApplication()->getActiveWorkbook(), uno::UNO_SET_THROW);
    return xWorkbook;
}

uno::Reference< excel::XWindow > SAL_CALL
ScVbaGlobals::getActiveWindow()
{
    return getApplication()->getActiveWindow();
}

uno::Reference< excel::XWorksheet > SAL_CALL
ScVbaGlobals::getActiveSheet()
{
    return getApplication()->getActiveSheet();
}

uno::Any SAL_CALL
ScVbaGlobals::WorkBooks( const uno::Any& aIndex )
{
    return getApplication()->Workbooks(aIndex);
}

uno::Any SAL_CALL
ScVbaGlobals::WorkSheets(const uno::Any& aIndex)
{
    return getApplication()->Worksheets( aIndex );
}
uno::Any SAL_CALL
ScVbaGlobals::Sheets( const uno::Any& aIndex )
{
    return WorkSheets( aIndex );
}

uno::Any SAL_CALL
ScVbaGlobals::Range( const uno::Any& Cell1, const uno::Any& Cell2 )
{
    return getApplication()->Range( Cell1, Cell2 );
}

uno::Any SAL_CALL
ScVbaGlobals::Names( const css::uno::Any& aIndex )
{
    return getApplication()->Names( aIndex );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaGlobals::getActiveCell()
{
    return getApplication()->getActiveCell();
}

uno::Reference< XAssistant > SAL_CALL
ScVbaGlobals::getAssistant()
{
    return getApplication()->getAssistant();
}

uno::Any SAL_CALL
ScVbaGlobals::getSelection()
{
    return getApplication()->getSelection();
}

uno::Reference< excel::XWorkbook > SAL_CALL
ScVbaGlobals::getThisWorkbook()
{
    return getApplication()->getThisWorkbook();
}
void SAL_CALL
ScVbaGlobals::Calculate()
{
    return getApplication()->Calculate();
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaGlobals::Cells( const uno::Any& RowIndex, const uno::Any& ColumnIndex )
{
    return getApplication()->getActiveSheet()->Cells( RowIndex, ColumnIndex );
}
uno::Reference< excel::XRange > SAL_CALL
ScVbaGlobals::Columns( const uno::Any& aIndex )
{
    return getApplication()->getActiveSheet()->Columns( aIndex );
}

uno::Any SAL_CALL
ScVbaGlobals::CommandBars( const uno::Any& aIndex )
{
    uno::Reference< XApplicationBase > xBase( getApplication(), uno::UNO_QUERY_THROW );
    return xBase->CommandBars( aIndex );
}

css::uno::Reference< ov::excel::XRange > SAL_CALL
ScVbaGlobals::Union( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 )
{
    return getApplication()->Union(  Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11, Arg12, Arg13, Arg14, Arg15, Arg16, Arg17, Arg18, Arg19, Arg20, Arg21, Arg22, Arg23, Arg24, Arg25, Arg26, Arg27, Arg28, Arg29, Arg30 );
}
css::uno::Reference< ov::excel::XRange > SAL_CALL
ScVbaGlobals::Intersect( const css::uno::Reference< ov::excel::XRange >& Arg1, const css::uno::Reference< ov::excel::XRange >& Arg2, const css::uno::Any& Arg3, const css::uno::Any& Arg4, const css::uno::Any& Arg5, const css::uno::Any& Arg6, const css::uno::Any& Arg7, const css::uno::Any& Arg8, const css::uno::Any& Arg9, const css::uno::Any& Arg10, const css::uno::Any& Arg11, const css::uno::Any& Arg12, const css::uno::Any& Arg13, const css::uno::Any& Arg14, const css::uno::Any& Arg15, const css::uno::Any& Arg16, const css::uno::Any& Arg17, const css::uno::Any& Arg18, const css::uno::Any& Arg19, const css::uno::Any& Arg20, const css::uno::Any& Arg21, const css::uno::Any& Arg22, const css::uno::Any& Arg23, const css::uno::Any& Arg24, const css::uno::Any& Arg25, const css::uno::Any& Arg26, const css::uno::Any& Arg27, const css::uno::Any& Arg28, const css::uno::Any& Arg29, const css::uno::Any& Arg30 )
{
    return getApplication()->Intersect(  Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8, Arg9, Arg10, Arg11, Arg12, Arg13, Arg14, Arg15, Arg16, Arg17, Arg18, Arg19, Arg20, Arg21, Arg22, Arg23, Arg24, Arg25, Arg26, Arg27, Arg28, Arg29, Arg30 );
}

uno::Any SAL_CALL
ScVbaGlobals::Evaluate( const OUString& Name )
{
    return getApplication()->Evaluate( Name );
}

css::uno::Any SAL_CALL
ScVbaGlobals::WorksheetFunction(  )
{
    return getApplication()->WorksheetFunction();
}

uno::Any SAL_CALL
ScVbaGlobals::Windows( const uno::Any& aIndex )
{
    return getApplication()->Windows( aIndex );
}

uno::Reference< excel::XRange > SAL_CALL
ScVbaGlobals::Rows( const uno::Any& aIndex )
{
    return getApplication()->getActiveSheet()->Rows( aIndex );

}

uno::Any SAL_CALL
ScVbaGlobals::getDebug()
{
    try // return empty object on error
    {
        uno::Reference< lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_SET_THROW );
        uno::Reference< uno::XInterface > xVBADebug = xServiceManager->createInstanceWithContext(
            u"ooo.vba.Debug"_ustr, mxContext );
        return uno::Any( xVBADebug );
    }
    catch( uno::Exception& )
    {
    }
    return uno::Any();
}

uno::Any SAL_CALL
ScVbaGlobals::MenuBars( const uno::Any& aIndex )
{
    return getApplication()->MenuBars(aIndex);
}

uno::Sequence< OUString > SAL_CALL
ScVbaGlobals::getAvailableServiceNames(  )
{
    static const uno::Sequence< OUString > serviceNames = comphelper::concatSequences(
        ScVbaGlobals_BASE::getAvailableServiceNames(),
        uno::Sequence< OUString >
        {
            u"ooo.vba.excel.Range"_ustr,
            u"ooo.vba.excel.Workbook"_ustr,
            u"ooo.vba.excel.Window"_ustr,
            u"ooo.vba.excel.Worksheet"_ustr,
            u"ooo.vba.excel.Application"_ustr,
            u"ooo.vba.excel.Hyperlink"_ustr,
            u"com.sun.star.script.vba.VBASpreadsheetEventProcessor"_ustr
        } );
    return serviceNames;
}

OUString
ScVbaGlobals::getServiceImplName()
{
    return u"ScVbaGlobals"_ustr;
}

uno::Sequence< OUString >
ScVbaGlobals::getServiceNames()
{
        static uno::Sequence< OUString > aServiceNames
        {
            u"ooo.vba.excel.Globals"_ustr
        };
        return aServiceNames;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
ScVbaGlobals_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new ScVbaGlobals(arguments, context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

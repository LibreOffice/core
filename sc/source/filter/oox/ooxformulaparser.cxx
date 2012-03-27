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

#include <boost/shared_ptr.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XFilterFormulaParser.hpp>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include "formulaparser.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

class OOXMLFormulaParserImpl : private FormulaFinalizer
{
public:
    explicit            OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxModelFactory );

    Sequence< FormulaToken > parseFormula( const OUString& rFormula, const CellAddress& rReferencePos );

protected:
    virtual const FunctionInfo* resolveBadFuncName( const OUString& rTokenData ) const;

private:
    ApiParserWrapper    maApiParser;
};

// ----------------------------------------------------------------------------

OOXMLFormulaParserImpl::OOXMLFormulaParserImpl( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    FormulaFinalizer( OpCodeProvider( rxModelFactory, FILTER_OOXML, BIFF_UNKNOWN, true ) ),
    maApiParser( rxModelFactory, *this )
{
}

Sequence< FormulaToken > OOXMLFormulaParserImpl::parseFormula( const OUString& rFormula, const CellAddress& rReferencePos )
{
    return finalizeTokenArray( maApiParser.parseFormula( rFormula, rReferencePos ) );
}

const FunctionInfo* OOXMLFormulaParserImpl::resolveBadFuncName( const OUString& rTokenData ) const
{
    /*  Try to parse calls to library functions. The format of such a function
        call is assumed to be
            "'<path-to-office-install>\Library\<libname>'!<funcname>". */

    // the string has to start with an apostroph (followed by the library URL)
    if( (rTokenData.getLength() >= 6) && (rTokenData[ 0 ] == '\'') )
    {
        // library URL and function name are separated by an exclamation mark
        sal_Int32 nExclamPos = rTokenData.lastIndexOf( '!' );
        if( (1 < nExclamPos) && (nExclamPos + 1 < rTokenData.getLength()) && (rTokenData[ nExclamPos - 1 ] == '\'') )
        {
            // find the last backslash that separates library path and name
            sal_Int32 nFileSep = rTokenData.lastIndexOf( '\\', nExclamPos - 2 );
            if( nFileSep > 1 )
            {
                // find preceding backslash that separates the last directory name
                sal_Int32 nDirSep = rTokenData.lastIndexOf( '\\', nFileSep - 1 );
                // function library is located in a directory called 'library'
                if( (nDirSep > 0) && rTokenData.matchIgnoreAsciiCaseAsciiL( RTL_CONSTASCII_STRINGPARAM( "\\LIBRARY\\" ), nDirSep ) )
                {
                    // try to find a function info for the function name
                    OUString aFuncName = rTokenData.copy( nExclamPos + 1 ).toAsciiUpperCase();
                    const FunctionInfo* pFuncInfo = getFuncInfoFromOoxFuncName( aFuncName );
                    if( pFuncInfo && (pFuncInfo->meFuncLibType != FUNCLIB_UNKNOWN) )
                    {
                        // check that the name of the library matches
                        OUString aLibName = rTokenData.copy( nFileSep + 1, nExclamPos - nFileSep - 2 );
                        if( pFuncInfo->meFuncLibType == getFuncLibTypeFromLibraryName( aLibName ) )
                            return pFuncInfo;
                    }
                }
            }
        }
    }
    return 0;
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

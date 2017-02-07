/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "interpre.hxx"
#include "jumpmatrix.hxx"
#include "formulacell.hxx"
#include "scmatrix.hxx"
#include <rtl/strbuf.hxx>
#include <formula/errorcodes.hxx>
#include <svtools/miscopt.hxx>

#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include "libxml/xpath.h"
#include <datastreamgettime.hxx>
#include <dpobject.hxx>
#include <document.hxx>

#include <cstring>
#include <memory>

using namespace com::sun::star;

// TODO: Add new methods for ScInterpreter here.

void ScInterpreter::ScFilterXML()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 2 ) )
    {
        SCSIZE nMatCols = 1, nMatRows = 1, nNode = 0;
        const ScMatrix* pPathMatrix = nullptr;
        // In array/matrix context node elements' results are to be
        // subsequently stored. Check this before obtaining any argument from
        // the stack so the stack type can be used.
        if (pJumpMatrix || bMatrixFormula || pCur->IsInForceArray())
        {
            if (pJumpMatrix)
            {
                // Single result, GetString() will retrieve the corresponding
                // argument and JumpMatrix() will store it at the proper
                // position. Note that nMatCols and nMatRows are still 1.
                SCSIZE nCurCol = 0, nCurRow = 0;
                pJumpMatrix->GetPos( nCurCol, nCurRow);
                nNode = nCurRow;
            }
            else if (bMatrixFormula)
            {
                // If there is no formula cell then continue with a single
                // result.
                if (pMyFormulaCell)
                {
                    SCCOL nCols;
                    SCROW nRows;
                    pMyFormulaCell->GetMatColsRows( nCols, nRows);
                    nMatCols = nCols;
                    nMatRows = nRows;
                }
            }
            else if (GetStackType() == formula::svMatrix)
            {
                pPathMatrix = pStack[sp-1]->GetMatrix();
                if (!pPathMatrix)
                {
                    PushIllegalParameter();
                    return;
                }
                pPathMatrix->GetDimensions( nMatCols, nMatRows);

                /* TODO: it is unclear what should happen if there are
                 * different path arguments in matrix elements. We may have to
                 * evaluate each, and for repeated identical paths use
                 * subsequent nodes. As is, the path at 0,0 is used as obtained
                 * by GetString(). */

            }
        }

        OUString aXPathExpression = GetString().getString();
        OUString aString = GetString().getString();
        if(aString.isEmpty() || aXPathExpression.isEmpty())
        {
            PushError( FormulaError::NoValue );
            return;
        }

        OString aOXPathExpression = OUStringToOString( aXPathExpression, RTL_TEXTENCODING_UTF8 );
        const char* pXPathExpr = aOXPathExpression.getStr();
        OString aOString = OUStringToOString( aString, RTL_TEXTENCODING_UTF8 );
        const char* pXML = aOString.getStr();

        std::shared_ptr<xmlParserCtxt> pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt );

        std::shared_ptr<xmlDoc> pDoc( xmlParseMemory( pXML, aOString.getLength() ),
                xmlFreeDoc );

        if(!pDoc)
        {
            PushError( FormulaError::NoValue );
            return;
        }

        std::shared_ptr<xmlXPathContext> pXPathCtx( xmlXPathNewContext(pDoc.get()),
                xmlXPathFreeContext );

        std::shared_ptr<xmlXPathObject> pXPathObj( xmlXPathEvalExpression(BAD_CAST(pXPathExpr), pXPathCtx.get()),
                xmlXPathFreeObject );

        if(!pXPathObj)
        {
            PushError( FormulaError::NoValue );
            return;
        }

        switch(pXPathObj->type)
        {
            case XPATH_UNDEFINED:
                break;
            case XPATH_NODESET:
                {
                    xmlNodeSetPtr pNodeSet = pXPathObj->nodesetval;
                    if(!pNodeSet)
                    {
                        PushError( FormulaError::NoValue );
                        return;
                    }

                    const size_t nSize = pNodeSet->nodeNr;
                    if (nNode >= nSize)
                    {
                        // For pJumpMatrix
                        PushError( FormulaError::NotAvailable);
                        return;
                    }

                    /* TODO: for nMatCols>1 IF stack type is svMatrix, i.e.
                     * pPathMatrix!=nullptr, we may want a result matrix with
                     * nMatCols columns as well, but clarify first how to treat
                     * differing path elements. */

                    ScMatrixRef xResMat;
                    if (nMatRows > 1)
                    {
                        xResMat = GetNewMat( 1, nMatRows, true);
                        if (!xResMat)
                        {
                            PushError( FormulaError::CodeOverflow);
                            return;
                        }
                    }

                    for ( ; nNode < nMatRows; ++nNode)
                    {
                        if( nSize > nNode )
                        {
                            rtl::OUString aResult;
                            if(pNodeSet->nodeTab[nNode]->type == XML_NAMESPACE_DECL)
                            {
                                xmlNsPtr ns = reinterpret_cast<xmlNsPtr>(pNodeSet->nodeTab[nNode]);
                                xmlNodePtr cur = reinterpret_cast<xmlNodePtr>(ns->next);
                                std::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                                aResult = OStringToOUString(OString(reinterpret_cast<char*>(pChar2.get())), RTL_TEXTENCODING_UTF8);
                            }
                            else
                            {
                                xmlNodePtr cur = pNodeSet->nodeTab[nNode];
                                std::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                                aResult = OStringToOUString(OString(reinterpret_cast<char*>(pChar2.get())), RTL_TEXTENCODING_UTF8);
                            }
                            if (xResMat)
                                xResMat->PutString( mrStrPool.intern( aResult), 0, nNode);
                            else
                                PushString(aResult);
                        }
                        else
                        {
                            if (xResMat)
                                xResMat->PutError( FormulaError::NotAvailable, 0, nNode);
                            else
                                PushError( FormulaError::NotAvailable );
                        }
                    }
                    if (xResMat)
                        PushMatrix( xResMat);
                }
                break;
            case XPATH_BOOLEAN:
                {
                    bool bVal = pXPathObj->boolval != 0;
                    PushDouble(double(bVal));
                }
                break;
            case XPATH_NUMBER:
                {
                    double fVal = pXPathObj->floatval;
                    PushDouble(fVal);
                }
                break;
            case XPATH_STRING:
                PushString(OUString::createFromAscii(reinterpret_cast<char*>(pXPathObj->stringval)));
                break;
            case XPATH_POINT:
                PushNoValue();
                break;
            case XPATH_RANGE:
                PushNoValue();
                break;
            case XPATH_LOCATIONSET:
                PushNoValue();
                break;
            case XPATH_USERS:
                PushNoValue();
                break;
            case XPATH_XSLT_TREE:
                PushNoValue();
                break;

        }
    }
}

void ScInterpreter::ScWebservice()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        OUString aURI = GetString().getString();

        if(aURI.isEmpty())
        {
            PushError( FormulaError::NoValue );
            return;
        }

        uno::Reference< ucb::XSimpleFileAccess3 > xFileAccess( ucb::SimpleFileAccess::create( comphelper::getProcessComponentContext() ), uno::UNO_QUERY );
        if(!xFileAccess.is())
        {
            PushError( FormulaError::NoValue );
            return;
        }

        uno::Reference< io::XInputStream > xStream;
        try {
            xStream = xFileAccess->openFileRead( aURI );
        }
        catch (...)
        {
            // don't let any exceptions pass
            PushError( FormulaError::NoValue );
            return;
        }
        if ( !xStream.is() )
        {
            PushError( FormulaError::NoValue );
            return;
        }

        const sal_Int32 BUF_LEN = 8000;
        uno::Sequence< sal_Int8 > buffer( BUF_LEN );
        OStringBuffer aBuffer( 64000 );

        sal_Int32 nRead = 0;
        while ( ( nRead = xStream->readBytes( buffer, BUF_LEN ) ) == BUF_LEN )
        {
            aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
        }

        if ( nRead > 0 )
        {
            aBuffer.append( reinterpret_cast< const char* >( buffer.getConstArray() ), nRead );
        }

        xStream->closeInput();

        OUString aContent = OStringToOUString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 );
        PushString( aContent );
    }
}

/**
 Returns a string in which all non-alphanumeric characters except stroke and
 underscore (-_) have been replaced with a percent (%) sign
 followed by hex digits.
 It is encoded the same way that the posted data from a WWW form is encoded,
 that is the same way as in application/x-www-form-urlencoded media type and
 as per RFC 3986.

 @see fdo#76870
*/
void ScInterpreter::ScEncodeURL()
{
    sal_uInt8 nParamCount = GetByte();
    if ( MustHaveParamCount( nParamCount, 1 ) )
    {
        OUString aStr = GetString().getString();
        if ( aStr.isEmpty() )
        {
            PushError( FormulaError::NoValue );
            return;
        }

        OString aUtf8Str( aStr.toUtf8());
        const sal_Int32 nLen = aUtf8Str.getLength();
        OStringBuffer aUrlBuf( nLen );
        for ( int i = 0; i < nLen; i++ )
        {
            sal_Char c = aUtf8Str[ i ];
            if ( rtl::isAsciiAlphanumeric( static_cast<unsigned char>( c ) ) || c == '-' || c == '_' )
                aUrlBuf.append( c );
            else
            {
                aUrlBuf.append( '%' );
                aUrlBuf.append( OString::number( static_cast<unsigned char>( c ), 16 ).toAsciiUpperCase() );
            }
        }
        PushString( OUString::fromUtf8( aUrlBuf.makeStringAndClear() ) );
    }
}

void ScInterpreter::ScDebugVar()
{
    // This is to be used by developers only!  Never document this for end
    // users.  This is a convenient way to extract arbitrary internal state to
    // a cell for easier debugging.

    SvtMiscOptions aMiscOptions;
    if (!aMiscOptions.IsExperimentalMode())
    {
        PushError(FormulaError::NoName);
        return;
    }

    if (!MustHaveParamCount(GetByte(), 1))
    {
        PushIllegalParameter();
        return;
    }

    rtl_uString* p = GetString().getDataIgnoreCase();
    if (!p)
    {
        PushIllegalParameter();
        return;
    }

    OUString aStrUpper(p);

    if (aStrUpper == "PIVOTCOUNT")
    {
        // Set the number of pivot tables in the document.

        double fVal = 0.0;
        if (pDok->HasPivotTable())
        {
            const ScDPCollection* pDPs = pDok->GetDPCollection();
            fVal = pDPs->GetCount();
        }
        PushDouble(fVal);
    }
    else if (aStrUpper == "DATASTREAM_IMPORT")
        PushDouble( sc::datastream_get_time( sc::DebugTime::Import ) );
    else if (aStrUpper == "DATASTREAM_RECALC")
        PushDouble( sc::datastream_get_time( sc::DebugTime::Recalc ) );
    else if (aStrUpper == "DATASTREAM_RENDER")
        PushDouble( sc::datastream_get_time( sc::DebugTime::Render ) );
    else
        PushIllegalParameter();
}

void ScInterpreter::ScErf()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        double x = GetDouble();
        PushDouble( ::rtl::math::erf( x ) );
    }
}

void ScInterpreter::ScErfc()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        double x = GetDouble();
        PushDouble( ::rtl::math::erfc( x ) );
    }
}

void ScInterpreter::ScColor()
{
    sal_uInt8 nParamCount = GetByte();
    if(MustHaveParamCount(nParamCount, 3, 4))
    {
        double nAlpha = 0;
        if(nParamCount == 4)
            nAlpha = rtl::math::approxFloor(GetDouble());

        if(nAlpha < 0 || nAlpha > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nBlue = rtl::math::approxFloor(GetDouble());

        if(nBlue < 0 || nBlue > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nGreen = rtl::math::approxFloor(GetDouble());

        if(nGreen < 0 || nGreen > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nRed = rtl::math::approxFloor(GetDouble());

        if(nRed < 0 || nRed > 255)
        {
            PushIllegalArgument();
            return;
        }

        double nVal = 256*256*256*nAlpha + 256*256*nRed + 256*nGreen + nBlue;
        PushDouble(nVal);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

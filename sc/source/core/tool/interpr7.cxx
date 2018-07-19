/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <interpre.hxx>
#include <jumpmatrix.hxx>
#include <formulacell.hxx>
#include <scmatrix.hxx>
#include <rtl/strbuf.hxx>
#include <formula/errorcodes.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/miscopt.hxx>
#include <tools/urlobj.hxx>

#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include <libxml/xpath.h>
#include <datastreamgettime.hxx>
#include <dpobject.hxx>
#include <document.hxx>
#include <tokenarray.hxx>
#include <webservicelink.hxx>

#include <sc.hrc>

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
        if (!nMatCols || !nMatRows)
        {
            PushNoValue();
            return;
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
                PushNoValue();
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

static ScWebServiceLink* lcl_GetWebServiceLink(const sfx2::LinkManager* pLinkMgr, const OUString& rURL)
{
    size_t nCount = pLinkMgr->GetLinks().size();
    for (size_t i=0; i<nCount; ++i)
    {
        ::sfx2::SvBaseLink* pBase = pLinkMgr->GetLinks()[i].get();
        if (ScWebServiceLink* pLink = dynamic_cast<ScWebServiceLink*>(pBase))
        {
            if (pLink->GetURL() == rURL)
                return pLink;
        }
    }

    return nullptr;
}

static bool lcl_FunctionAccessLoadWebServiceLink( OUString& rResult, ScDocument* pDoc, const OUString& rURI )
{
    // For FunctionAccess service always force a changed data update.
    ScWebServiceLink aLink( pDoc, rURI);
    if (aLink.DataChanged( OUString(), css::uno::Any()) != sfx2::SvBaseLink::UpdateResult::SUCCESS)
        return false;

    if (!aLink.HasResult())
        return false;

    rResult = aLink.GetResult();

    return true;
}

void ScInterpreter::ScWebservice()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        OUString aURI = GetString().getString();

        if (aURI.isEmpty())
        {
            PushError( FormulaError::NoValue );
            return;
        }

        INetURLObject aObj(aURI, INetProtocol::File);
        INetProtocol eProtocol = aObj.GetProtocol();
        if (eProtocol != INetProtocol::Http && eProtocol != INetProtocol::Https)
        {
            PushError(FormulaError::NoValue);
            return;
        }

        if (!mpLinkManager)
        {
            if (!pDok->IsFunctionAccess() || pDok->HasLinkFormulaNeedingCheck())
            {
                PushError( FormulaError::NoValue);
            }
            else
            {
                OUString aResult;
                if (lcl_FunctionAccessLoadWebServiceLink( aResult, pDok, aURI))
                    PushString( aResult);
                else
                    PushError( FormulaError::NoValue);
            }
            return;
        }

        // Need to reinterpret after loading (build links)
        rArr.AddRecalcMode( ScRecalcMode::ONLOAD_LENIENT );

        //  while the link is not evaluated, idle must be disabled (to avoid circular references)
        bool bOldEnabled = pDok->IsIdleEnabled();
        pDok->EnableIdle(false);

        // Get/ Create link object
        ScWebServiceLink* pLink = lcl_GetWebServiceLink(mpLinkManager, aURI);

        bool bWasError = (pMyFormulaCell && pMyFormulaCell->GetRawError() != FormulaError::NONE);

        if (!pLink)
        {
            pLink = new ScWebServiceLink(pDok, aURI);
            mpLinkManager->InsertFileLink(*pLink, OBJECT_CLIENT_FILE, aURI);
            if ( mpLinkManager->GetLinks().size() == 1 )                    // the first one?
            {
                SfxBindings* pBindings = pDok->GetViewBindings();
                if (pBindings)
                    pBindings->Invalidate( SID_LINKS );             // Link-Manager enabled
            }

            //if the document was just loaded, but the ScDdeLink entry was missing, then
            //don't update this link until the links are updated in response to the users
            //decision
            if (!pDok->HasLinkFormulaNeedingCheck())
            {
                pLink->Update();
            }

            if (pMyFormulaCell)
            {
                // StartListening after the Update to avoid circular references
                pMyFormulaCell->StartListening(*pLink);
            }
        }
        else
        {
            if (pMyFormulaCell)
                pMyFormulaCell->StartListening(*pLink);
        }

        //  If an new Error from Reschedule appears when the link is executed then reset the errorflag
        if (pMyFormulaCell && pMyFormulaCell->GetRawError() != FormulaError::NONE && !bWasError)
            pMyFormulaCell->SetErrCode(FormulaError::NONE);

        //  check the value
        if (pLink->HasResult())
            PushString(pLink->GetResult());
        else if (pDok->HasLinkFormulaNeedingCheck())
        {
            // If this formula cell is recalculated just after load and the
            // expression is exactly WEBSERVICE("literal_URI") (i.e. no other
            // calculation involved, not even a cell reference) and a cached
            // result is set as hybrid string then use that as result value to
            // prevent a #VALUE! result due to the "Automatic update of
            // external links has been disabled."
            // This will work only once, as the new formula cell result won't
            // be a hybrid anymore.
            /* TODO: the FormulaError::LinkFormulaNeedingCheck could be used as
             * a signal for the formula cell to keep the hybrid string as
             * result of the overall formula *iff* no higher prioritized
             * ScRecalcMode than ONLOAD_LENIENT is present in the entire
             * document (i.e. the formula result could not be influenced by an
             * ONLOAD_MUST or ALWAYS recalc, necessary as we don't track
             * interim results of subexpressions that could be compared), which
             * also means to track setting ScRecalcMode somehow.. note this is
             * just a vague idea so far and might or might not work. */
            if (pMyFormulaCell && pMyFormulaCell->HasHybridStringResult())
            {
                if (pMyFormulaCell->GetCode()->GetCodeLen() == 2)
                {
                    formula::FormulaToken const * const * pRPN = pMyFormulaCell->GetCode()->GetCode();
                    if (pRPN[0]->GetType() == formula::svString && pRPN[1]->GetOpCode() == ocWebservice)
                        PushString( pMyFormulaCell->GetResultString());
                    else
                        PushError(FormulaError::LinkFormulaNeedingCheck);
                }
                else
                    PushError(FormulaError::LinkFormulaNeedingCheck);
            }
            else
                PushError(FormulaError::LinkFormulaNeedingCheck);
        }
        else
            PushError(FormulaError::NoValue);

        pDok->EnableIdle(bOldEnabled);
        mpLinkManager->CloseCachedComps();
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
                OString convertedChar = OString::number( static_cast<unsigned char>( c ), 16 ).toAsciiUpperCase();
                // RFC 3986 indicates:
                // "A percent-encoded octet is encoded as a character triplet,
                // consisting of the percent character "%" followed by the two hexadecimal digits
                // representing that octet's numeric value"
                if (convertedChar.getLength() == 1)
                    aUrlBuf.append("0");
                aUrlBuf.append(convertedChar);
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
        return;

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

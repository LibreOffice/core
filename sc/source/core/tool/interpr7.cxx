/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "interpre.hxx"
#include <rtl/strbuf.hxx>
#include <formula/errorcodes.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/linkmgr.hxx>
#include <svtools/miscopt.hxx>
#include <tools/urlobj.hxx>

#include <com/sun/star/ucb/XSimpleFileAccess3.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#include "libxml/xpath.h"
#include <datastreamgettime.hxx>
#include <dpobject.hxx>
#include <document.hxx>
#include <tokenarray.hxx>
#include <webservicelink.hxx>
#include "formulacell.hxx"

#include <sc.hrc>

#include <boost/shared_ptr.hpp>
#include <cstring>

using namespace com::sun::star;

// TODO: Add new methods for ScInterpreter here.

void ScInterpreter::ScFilterXML()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 2 ) )
    {
        OUString aXPathExpression = GetString().getString();
        OUString aString = GetString().getString();
        if(aString.isEmpty() || aXPathExpression.isEmpty())
        {
            PushError( errNoValue );
            return;
        }

        OString aOXPathExpression = OUStringToOString( aXPathExpression, RTL_TEXTENCODING_UTF8 );
        const char* pXPathExpr = aOXPathExpression.getStr();
        OString aOString = OUStringToOString( aString, RTL_TEXTENCODING_UTF8 );
        const char* pXML = aOString.getStr();

        boost::shared_ptr<xmlParserCtxt> pContext(
                xmlNewParserCtxt(), xmlFreeParserCtxt );

        boost::shared_ptr<xmlDoc> pDoc( xmlParseMemory( pXML, aOString.getLength() ),
                xmlFreeDoc );

        if(!pDoc)
        {
            PushError( errNoValue );
            return;
        }

        boost::shared_ptr<xmlXPathContext> pXPathCtx( xmlXPathNewContext(pDoc.get()),
                xmlXPathFreeContext );

        boost::shared_ptr<xmlXPathObject> pXPathObj( xmlXPathEvalExpression(BAD_CAST(pXPathExpr), pXPathCtx.get()),
                xmlXPathFreeObject );

        if(!pXPathObj)
        {
            PushError( errNoValue );
            return;
        }

        rtl::OUString aResult;

        switch(pXPathObj->type)
        {
            case XPATH_UNDEFINED:
                break;
            case XPATH_NODESET:
                {
                    xmlNodeSetPtr pNodeSet = pXPathObj->nodesetval;
                    if(!pNodeSet)
                    {
                        PushError( errNoValue );
                        return;
                    }

                    size_t nSize = pNodeSet->nodeNr;
                    if( nSize >= 1 )
                    {
                        if(pNodeSet->nodeTab[0]->type == XML_NAMESPACE_DECL)
                        {
                            xmlNsPtr ns = reinterpret_cast<xmlNsPtr>(pNodeSet->nodeTab[0]);
                            xmlNodePtr cur = reinterpret_cast<xmlNodePtr>(ns->next);
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            aResult = OStringToOUString(OString(reinterpret_cast<char*>(pChar2.get())), RTL_TEXTENCODING_UTF8);
                        }
                        else
                        {
                            xmlNodePtr cur = pNodeSet->nodeTab[0];
                            boost::shared_ptr<xmlChar> pChar2(xmlNodeGetContent(cur), xmlFree);
                            aResult = OStringToOUString(OString(reinterpret_cast<char*>(pChar2.get())), RTL_TEXTENCODING_UTF8);
                        }
                    }
                    else
                    {
                        PushError( errNoValue );
                        return;
                    }
                }
                PushString(aResult);
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
        ::sfx2::SvBaseLink* pBase = *pLinkMgr->GetLinks()[i];
        if (ScWebServiceLink* pLink = dynamic_cast<ScWebServiceLink*>(pBase))
        {
            if (pLink->GetURL() == rURL)
                return pLink;
        }
    }

    return nullptr;
}

void ScInterpreter::ScWebservice()
{
    sal_uInt8 nParamCount = GetByte();
    if (MustHaveParamCount( nParamCount, 1 ) )
    {
        OUString aURI = GetString().getString();

        if (aURI.isEmpty())
        {
            PushError( errNoValue );
            return;
        }

        INetURLObject aObj(aURI, INetProtocol::File);
        INetProtocol eProtocol = aObj.GetProtocol();
        if (eProtocol != INetProtocol::Http && eProtocol != INetProtocol::Https)
        {
            PushError(errNoValue);
            return;
        }

        sfx2::LinkManager* pLinkMgr = pDok->GetLinkManager();
        if (!pLinkMgr)
        {
            PushError( errNoValue );
            return;
        }

        // Need to reinterpret after loading (build links)
        if (rArr.IsRecalcModeNormal())
            rArr.SetExclusiveRecalcModeOnLoad();

        //  while the link is not evaluated, idle must be disabled (to avoid circular references)
        bool bOldEnabled = pDok->IsIdleEnabled();
        pDok->EnableIdle(false);

        // Get/ Create link object
        ScWebServiceLink* pLink = lcl_GetWebServiceLink(pLinkMgr, aURI);

        bool bWasError = (pMyFormulaCell && pMyFormulaCell->GetRawError() != 0);

        if (!pLink)
        {
            pLink = new ScWebServiceLink(pDok, aURI);
            PushError( errNoValue );
            if ( pLinkMgr->GetLinks().size() == 1 )                    // the first one?
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
            PushError( errNoValue );
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
        if (pMyFormulaCell && pMyFormulaCell->GetRawError() != 0 && !bWasError)
            pMyFormulaCell->SetErrCode(0);

        //  check the value
        if (pLink->HasResult())
            PushString(pLink->GetResult());
        else
            PushError(errNoValue);

        pDok->EnableIdle(bOldEnabled);
        pLinkMgr->CloseCachedComps();
    }
}

/**
 Returns a string in which all non-alphanumeric characters except stroke and
 underscore (-_) have been replaced with a percent (%) sign
 followed by hex digits.
 It is encoded the same way that the posted data from a WWW form is encoded,
 that is the same way as in application/x-www-form-urlencoded media type and
 as pwer RFC 3986.

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
            PushError( errNoValue );
            return;
        }

        OString aUtf8Str( aStr.toUtf8());
        const sal_Int32 nLen = aUtf8Str.getLength();
        OStringBuffer aUrlBuf( nLen );;
        for ( int i = 0; i < nLen; i++ )
        {
            sal_Char c = aUtf8Str[ i ];
            if ( rtl::isAsciiAlphanumeric( static_cast<sal_uChar>( c ) ) || c == '-' || c == '_' )
                aUrlBuf.append( c );
            else
            {
                aUrlBuf.append( '%' );
                aUrlBuf.append( OString::number( static_cast<sal_uChar>( c ), 16 ).toAsciiUpperCase() );
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
        PushError(ScErrorCodes::errNoName);
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
        PushDouble( sc::datastream_get_time( 0 ) );
    else if (aStrUpper == "DATASTREAM_RECALC")
        PushDouble( sc::datastream_get_time( 1 ) );
    else if (aStrUpper == "DATASTREAM_RENDER")
        PushDouble( sc::datastream_get_time( 2 ) );
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

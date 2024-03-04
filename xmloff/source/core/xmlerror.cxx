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

#include <utility>
#include <xmloff/xmlerror.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <com/sun/star/xml/sax/XLocator.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XLocator;
using ::com::sun::star::xml::sax::SAXParseException;


/// ErrorRecord: contains all information for one error


class ErrorRecord
{
public:

    ErrorRecord( sal_Int32 nId,
                 const Sequence<OUString>& rParams,
                 OUString aExceptionMessage,
                 sal_Int32 nRow,
                 sal_Int32 nColumn,
                 OUString aPublicId,
                 OUString aSystemId);

    sal_Int32 nId;  /// error ID

    OUString sExceptionMessage;/// message of original exception (if available)

    // XLocator information:
    sal_Int32 nRow;     /// row number where error occurred (or -1 for unknown)
    sal_Int32 nColumn;  /// column number where error occurred (or -1)
    OUString sPublicId; /// public identifier
    OUString sSystemId; /// public identifier

    /// message Parameters
    Sequence<OUString> aParams;
};


ErrorRecord::ErrorRecord( sal_Int32 nID, const Sequence<OUString>& rParams,
    OUString aExceptionMessage, sal_Int32 nRowNumber, sal_Int32 nCol,
    OUString aPublicId, OUString aSystemId) :
        nId(nID),
        sExceptionMessage(std::move(aExceptionMessage)),
        nRow(nRowNumber),
        nColumn(nCol),
        sPublicId(std::move(aPublicId)),
        sSystemId(std::move(aSystemId)),
        aParams(rParams)
{
}

XMLErrors::XMLErrors()
{
}

XMLErrors::~XMLErrors()
{
}

void XMLErrors::AddRecord(
    sal_Int32 nId,
    const Sequence<OUString> & rParams,
    const OUString& rExceptionMessage,
    sal_Int32 nRow,
    sal_Int32 nColumn,
    const OUString& rPublicId,
    const OUString& rSystemId )
{
    m_aErrors.emplace_back( nId, rParams, rExceptionMessage,
                                    nRow, nColumn, rPublicId, rSystemId );

#ifdef DBG_UTIL

    // give detailed assertion on this message

    OUStringBuffer sMessage( "An error or a warning has occurred during XML import/export!\n" );

    // ID & flags
    sMessage.append(
        "Error-Id: 0x"
        + OUString::number( nId, 16 )
        + "\n    Flags: " );
    sal_Int32 nFlags = (nId & XMLERROR_MASK_FLAG);
    sMessage.append( nFlags >> 28, 16 );
    if( (nFlags & XMLERROR_FLAG_WARNING) != 0 )
        sMessage.append( " WARNING" );
    if( (nFlags & XMLERROR_FLAG_ERROR) != 0 )
        sMessage.append( " ERROR" );
    if( (nFlags & XMLERROR_FLAG_SEVERE) != 0 )
        sMessage.append( " SEVERE" );
    sMessage.append( "\n    Class: " );
    sal_Int32 nClass = (nId & XMLERROR_MASK_CLASS);
    sMessage.append( nClass >> 16, 16 );
    if( (nClass & XMLERROR_CLASS_IO) != 0 )
        sMessage.append( " IO" );
    if( (nClass & XMLERROR_CLASS_FORMAT) != 0 )
        sMessage.append( " FORMAT" );
    if( (nClass & XMLERROR_CLASS_API) != 0 )
        sMessage.append( " API" );
    if( (nClass & XMLERROR_CLASS_OTHER) != 0 )
        sMessage.append( " OTHER" );
    sMessage.append( "\n    Number: " );
    sal_Int32 nNumber = (nId & XMLERROR_MASK_NUMBER);
    sMessage.append( OUString::number(nNumber, 16)
        + "\n" );

    // the parameters
    sMessage.append( "Parameters:\n" );
    sal_Int32 nLength = rParams.getLength();
    const OUString* pParams = rParams.getConstArray();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        sMessage.append( "    " + OUString::number(i) + ": " + pParams[i] + "\n" );
    }

    // the exception message
    sMessage.append( "Exception-Message: " + rExceptionMessage + "\n" );

    // position (if given)
    if( (nRow != -1) || (nColumn != -1) )
    {
        sMessage.append( "Position:\n    Public Identifier: "
            + rPublicId
            + "\n    System Identifier: "
            + rSystemId
            + "\n    Row, Column: "
            + OUString::number( nRow )
            + ","
            + OUString::number( nColumn )
            + "\n" );
    }

    SAL_WARN( "xmloff", sMessage.makeStringAndClear() );
#endif
}

void XMLErrors::AddRecord(
    sal_Int32 nId,
    const Sequence<OUString> & rParams,
    const OUString& rExceptionMessage,
    const Reference<XLocator> & rLocator)
{
    if ( rLocator.is() )
    {
        AddRecord( nId, rParams, rExceptionMessage,
                   rLocator->getLineNumber(), rLocator->getColumnNumber(),
                   rLocator->getPublicId(), rLocator->getSystemId() );
    }
    else
    {
        AddRecord( nId, rParams, rExceptionMessage,
                   -1, -1, "", "" );
    }
}

void XMLErrors::ThrowErrorAsSAXException(sal_Int32 nIdMask)
{
    // search first error/warning that matches the nIdMask
    for( const auto& rError : m_aErrors )
    {
        if ( (rError.nId & nIdMask) != 0 )
        {
            // we throw the error
            ErrorRecord& rErr = m_aErrors[0];
            throw SAXParseException(
                rErr.sExceptionMessage, nullptr, Any(rErr.aParams),
                rErr.sPublicId, rErr.sSystemId, rErr.nRow, rErr.nColumn );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

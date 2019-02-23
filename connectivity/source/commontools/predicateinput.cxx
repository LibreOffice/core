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


#include <connectivity/predicateinput.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/i18n/LocaleData.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <osl/diagnose.h>
#include <connectivity/sqlnode.hxx>
#include <connectivity/PColumn.hxx>
#include <comphelper/numbers.hxx>

#include <memory>


namespace dbtools
{


    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::util::NumberFormatter;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::i18n::LocaleData;
    using ::com::sun::star::i18n::LocaleDataItem;
    using ::com::sun::star::uno::Any;

    using namespace ::com::sun::star::sdbc;
    using namespace ::connectivity;

    using ::connectivity::OSQLParseNode;


    static sal_Unicode lcl_getSeparatorChar( const OUString& _rSeparator, sal_Unicode _nFallback )
    {
        OSL_ENSURE( !_rSeparator.isEmpty(), "::lcl_getSeparatorChar: invalid separator string!" );

        sal_Unicode nReturn( _nFallback );
        if ( !_rSeparator.isEmpty() )
            nReturn = _rSeparator[0];
        return nReturn;
    }

    bool OPredicateInputController::getSeparatorChars( const Locale& _rLocale, sal_Unicode& _rDecSep, sal_Unicode& _rThdSep ) const
    {
        _rDecSep = '.';
        _rThdSep = ',';
        try
        {
            LocaleDataItem aLocaleData;
            if ( m_xLocaleData.is() )
            {
                aLocaleData = m_xLocaleData->getLocaleItem( _rLocale );
                _rDecSep = lcl_getSeparatorChar( aLocaleData.decimalSeparator, _rDecSep );
                _rThdSep = lcl_getSeparatorChar( aLocaleData.thousandSeparator, _rThdSep );
                return true;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OPredicateInputController::getSeparatorChars: caught an exception!" );
        }
        return false;
    }


    OPredicateInputController::OPredicateInputController(
        const Reference< XComponentContext >& rxContext, const Reference< XConnection >& _rxConnection, const IParseContext* _pParseContext )
        : m_xConnection( _rxConnection )
        ,m_aParser( rxContext, _pParseContext )
    {
        try
        {
            // create a number formatter / number formats supplier pair
            OSL_ENSURE( rxContext.is(), "OPredicateInputController::OPredicateInputController: need a service factory!" );
            if ( rxContext.is() )
            {
                m_xFormatter.set( NumberFormatter::create(rxContext), UNO_QUERY_THROW );
            }

            Reference< XNumberFormatsSupplier >  xNumberFormats = ::dbtools::getNumberFormats( m_xConnection, true );
            if ( !xNumberFormats.is() )
                ::comphelper::disposeComponent( m_xFormatter );
            else
                m_xFormatter->attachNumberFormatsSupplier( xNumberFormats );

            // create the locale data
            if ( rxContext.is() )
            {
                m_xLocaleData = LocaleData::create( rxContext );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OPredicateInputController::OPredicateInputController: caught an exception!" );
        }
    }


    std::unique_ptr<OSQLParseNode> OPredicateInputController::implPredicateTree(OUString& _rErrorMessage, const OUString& _rStatement, const Reference< XPropertySet > & _rxField) const
    {
        std::unique_ptr<OSQLParseNode> pReturn = const_cast< OSQLParser& >( m_aParser ).predicateTree( _rErrorMessage, _rStatement, m_xFormatter, _rxField );
        if ( !pReturn )
        {   // is it a text field ?
            sal_Int32 nType = DataType::OTHER;
            _rxField->getPropertyValue("Type") >>= nType;

            if  (   ( DataType::CHAR        == nType )
                ||  ( DataType::VARCHAR     == nType )
                ||  ( DataType::LONGVARCHAR == nType )
                ||  ( DataType::CLOB        == nType )
                )
            {   // yes -> force a quoted text and try again
                OUString sQuoted( _rStatement );
                if  (   !sQuoted.isEmpty()
                    &&  (   !sQuoted.startsWith("'")
                        ||  !sQuoted.endsWith("'")
                        )
                    )
                {
                    static const char sSingleQuote[] = "'";

                    sal_Int32 nIndex = -1;
                    sal_Int32 nTemp = 0;
                    while ( -1 != ( nIndex = sQuoted.indexOf( '\'',nTemp ) ) )
                    {
                        sQuoted = sQuoted.replaceAt( nIndex, 1, "''" );
                        nTemp = nIndex+2;
                    }

                    sQuoted = sSingleQuote + sQuoted + sSingleQuote;
                }
                pReturn = const_cast< OSQLParser& >( m_aParser ).predicateTree( _rErrorMessage, sQuoted, m_xFormatter, _rxField );
            }

            // one more fallback: for numeric fields, and value strings containing a decimal/thousands separator
            // problem which is to be solved with this:
            // * a system locale "german"
            // * a column formatted with an english number format
            // => the output is german (as we use the system locale for this), i.e. "3,4"
            // => the input does not recognize the german text, as predicateTree uses the number format
            //    of the column to determine the main locale - the locale on the context is only a fallback
            if  (   ( DataType::FLOAT == nType )
                ||  ( DataType::REAL == nType )
                ||  ( DataType::DOUBLE == nType )
                ||  ( DataType::NUMERIC == nType )
                ||  ( DataType::DECIMAL == nType )
                )
            {
                const IParseContext& rParseContext = m_aParser.getContext();
                // get the separators for the locale of our parse context
                sal_Unicode nCtxDecSep;
                sal_Unicode nCtxThdSep;
                getSeparatorChars( rParseContext.getPreferredLocale(), nCtxDecSep, nCtxThdSep );

                // determine the locale of the column we're building a predicate string for
                sal_Unicode nFmtDecSep( nCtxDecSep );
                sal_Unicode nFmtThdSep( nCtxThdSep );
                try
                {
                    Reference< XPropertySetInfo > xPSI( _rxField->getPropertySetInfo() );
                    if ( xPSI.is() && xPSI->hasPropertyByName("FormatKey") )
                    {
                        sal_Int32 nFormatKey = 0;
                        _rxField->getPropertyValue("FormatKey") >>= nFormatKey;
                        if ( nFormatKey && m_xFormatter.is() )
                        {
                            Locale aFormatLocale;
                            ::comphelper::getNumberFormatProperty(
                                m_xFormatter,
                                nFormatKey,
                                "Locale"
                            ) >>= aFormatLocale;

                            // valid locale
                            if ( !aFormatLocale.Language.isEmpty() )
                            {
                                getSeparatorChars( aFormatLocale, nFmtDecSep, nCtxThdSep );
                            }
                        }
                    }
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "OPredicateInputController::implPredicateTree: caught an exception while dealing with the formats!" );
                }

                bool bDecDiffers = ( nCtxDecSep != nFmtDecSep );
                bool bFmtDiffers = ( nCtxThdSep != nFmtThdSep );
                if ( bDecDiffers || bFmtDiffers )
                {   // okay, at least one differs
                    // "translate" the value into the "format locale"
                    OUString sTranslated( _rStatement );
                    const sal_Unicode nIntermediate( '_' );
                    sTranslated = sTranslated.replace( nCtxDecSep,  nIntermediate );
                    sTranslated = sTranslated.replace( nCtxThdSep,  nFmtThdSep );
                    sTranslated = sTranslated.replace( nIntermediate, nFmtDecSep );

                    pReturn = const_cast< OSQLParser& >( m_aParser ).predicateTree( _rErrorMessage, sTranslated, m_xFormatter, _rxField );
                }
            }
        }
        return pReturn;
    }


    bool OPredicateInputController::normalizePredicateString(
        OUString& _rPredicateValue, const Reference< XPropertySet > & _rxField, OUString* _pErrorMessage ) const
    {
        OSL_ENSURE( m_xConnection.is() && m_xFormatter.is() && _rxField.is(),
            "OPredicateInputController::normalizePredicateString: invalid state or params!" );

        bool bSuccess = false;
        if ( m_xConnection.is() && m_xFormatter.is() && _rxField.is() )
        {
            // parse the string
            OUString sError;
            OUString sTransformedText( _rPredicateValue );
            std::unique_ptr<OSQLParseNode> pParseNode = implPredicateTree( sError, sTransformedText, _rxField );
            if ( _pErrorMessage ) *_pErrorMessage = sError;

            if ( pParseNode )
            {
                const IParseContext& rParseContext = m_aParser.getContext();
                sal_Unicode nDecSeparator, nThousandSeparator;
                getSeparatorChars( rParseContext.getPreferredLocale(), nDecSeparator, nThousandSeparator );

                // translate it back into a string
                sTransformedText.clear();
                pParseNode->parseNodeToPredicateStr(
                    sTransformedText, m_xConnection, m_xFormatter, _rxField, OUString(),
                    rParseContext.getPreferredLocale(), static_cast<sal_Char>(nDecSeparator), &rParseContext
                );
                _rPredicateValue = sTransformedText;

                bSuccess = true;
            }
        }

        return bSuccess;
    }


    OUString OPredicateInputController::getPredicateValueStr(
        const OUString& _rPredicateValue, const Reference< XPropertySet > & _rxField ) const
    {
        OSL_ENSURE( _rxField.is(), "OPredicateInputController::getPredicateValue: invalid params!" );
        OUString sReturn;
        if ( _rxField.is() )
        {
            // The following is mostly stolen from the former implementation in the parameter dialog
            // (dbaccess/source/ui/dlg/paramdialog.cxx). I do not fully understand this .....

            OUString sError;
            std::unique_ptr<OSQLParseNode> pParseNode = implPredicateTree( sError, _rPredicateValue, _rxField );

            implParseNode(std::move(pParseNode), true) >>= sReturn;
        }

        return sReturn;
    }

    OUString OPredicateInputController::getPredicateValueStr(
        const OUString& _sField, const OUString& _rPredicateValue ) const
    {
        OUString sReturn = _rPredicateValue;
        OUString sError;
        sal_Int32 nIndex = 0;
        OUString sField = _sField.getToken(0, '(', nIndex);
        if(nIndex == -1)
            sField = _sField;
        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionReturnType(sField,&m_aParser.getContext());
        if ( nType == DataType::OTHER || sField.isEmpty() )
        {
            // first try the international version
            OUString sSql = "SELECT * FROM x WHERE " + sField + _rPredicateValue;
            std::unique_ptr<OSQLParseNode> pParseNode( const_cast< OSQLParser& >( m_aParser ).parseTree( sError, sSql, true ) );
            nType = DataType::DOUBLE;
        }

        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        parse::OParseColumn* pColumn = new parse::OParseColumn( sField,
                                                                OUString(),
                                                                OUString(),
                                                                OUString(),
                                                                ColumnValue::NULLABLE_UNKNOWN,
                                                                0,
                                                                0,
                                                                nType,
                                                                false,
                                                                false,
                                                                xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers(),
                                                                OUString(),
                                                                OUString(),
                                                                OUString());
        Reference<XPropertySet> xColumn = pColumn;
        pColumn->setFunction(true);
        pColumn->setRealName(sField);

        std::unique_ptr<OSQLParseNode> pParseNode = implPredicateTree( sError, _rPredicateValue, xColumn );
        if(pParseNode)
        {
            implParseNode(std::move(pParseNode), true) >>= sReturn;
        }
        return sReturn;
    }

    Any OPredicateInputController::getPredicateValue(
        const OUString& _rPredicateValue, const Reference< XPropertySet > & _rxField ) const
    {
        OSL_ENSURE( _rxField.is(), "OPredicateInputController::getPredicateValue: invalid params!" );

        if ( _rxField.is() )
        {
            // The following is mostly stolen from the former implementation in the parameter dialog
            // (dbaccess/source/ui/dlg/paramdialog.cxx). I do not fully understand this .....

            OUString sError;
            std::unique_ptr<OSQLParseNode> pParseNode = implPredicateTree( sError, _rPredicateValue, _rxField );

            return implParseNode(std::move(pParseNode), false);
        }

        return Any();
    }

    Any OPredicateInputController::implParseNode(std::unique_ptr<OSQLParseNode> pParseNode, bool _bForStatementUse) const
    {
        if ( ! pParseNode )
            return Any();
        else
        {
            OUString sReturn;
            OSQLParseNode* pOdbcSpec = pParseNode->getByRule( OSQLParseNode::odbc_fct_spec );
            if ( pOdbcSpec )
            {
                if ( _bForStatementUse )
                {
                    OSQLParseNode* pFuncSpecParent = pOdbcSpec->getParent();
                    OSL_ENSURE( pFuncSpecParent, "OPredicateInputController::getPredicateValue: an ODBC func spec node without parent?" );
                    if ( pFuncSpecParent )
                        pFuncSpecParent->parseNodeToStr(sReturn, m_xConnection, &m_aParser.getContext());
                }
                else
                {
                    OSQLParseNode* pValueNode = pOdbcSpec->getChild(1);
                    if ( SQLNodeType::String == pValueNode->getNodeType() )
                        sReturn = pValueNode->getTokenValue();
                    else
                        pValueNode->parseNodeToStr(sReturn, m_xConnection, &m_aParser.getContext());
                }
            }
            else
            {
                if (pParseNode->getKnownRuleID() == OSQLParseNode::test_for_null )
                {
                    assert(pParseNode->count() == 2);
                    return Any();
                }
                // LEM this seems overly permissive as test...
                else if (pParseNode->count() >= 3)
                {
                    OSQLParseNode* pValueNode = pParseNode->getChild(2);
                    assert(pValueNode && "OPredicateInputController::getPredicateValue: invalid node child!");
                    if ( !_bForStatementUse )
                    {
                        if ( SQLNodeType::String == pValueNode->getNodeType() )
                            sReturn = pValueNode->getTokenValue();
                        else
                            pValueNode->parseNodeToStr(
                                sReturn, m_xConnection, &m_aParser.getContext()
                            );
                    }
                    else
                        pValueNode->parseNodeToStr(
                            sReturn, m_xConnection, &m_aParser.getContext()
                        );
                }
                else
                {
                    OSL_FAIL( "OPredicateInputController::getPredicateValue: unknown/invalid structure (noodbc)!" );
                    return Any();
                }
            }
            return Any(sReturn);
        }
    }

}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

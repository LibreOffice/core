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
#include <com/sun/star/util/NumberFormatter.hpp>
#include <osl/diagnose.h>
#include <connectivity/sqlnode.hxx>
#include <connectivity/PColumn.hxx>
#include <comphelper/numbers.hxx>

#include <boost/shared_ptr.hpp>

//.........................................................................
namespace dbtools
{
//.........................................................................

    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::util::NumberFormatter;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::i18n::LocaleData;
    using ::com::sun::star::i18n::XLocaleData;
    using ::com::sun::star::i18n::LocaleDataItem;

    using namespace ::com::sun::star::sdbc;
    using namespace ::connectivity;

    using ::connectivity::OSQLParseNode;

    #define Reference ::com::sun::star::uno::Reference

    //=====================================================================
    //---------------------------------------------------------------------
    static sal_Unicode lcl_getSeparatorChar( const ::rtl::OUString& _rSeparator, sal_Unicode _nFallback )
    {
        OSL_ENSURE( !_rSeparator.isEmpty(), "::lcl_getSeparatorChar: invalid separator string!" );

        sal_Unicode nReturn( _nFallback );
        if ( !_rSeparator.isEmpty() )
            nReturn = static_cast< sal_Char >( _rSeparator.getStr()[0] );
        return nReturn;
    }

    //=====================================================================
    //= OPredicateInputController
    //=====================================================================
    //---------------------------------------------------------------------
    sal_Bool OPredicateInputController::getSeparatorChars( const Locale& _rLocale, sal_Unicode& _rDecSep, sal_Unicode& _rThdSep ) const
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
                _rThdSep = lcl_getSeparatorChar( aLocaleData.decimalSeparator, _rThdSep );
                return sal_True;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OPredicateInputController::getSeparatorChars: caught an exception!" );
        }
        return sal_False;
    }

    //---------------------------------------------------------------------
    OPredicateInputController::OPredicateInputController(
        const Reference< XMultiServiceFactory >& _rxORB, const Reference< XConnection >& _rxConnection, const IParseContext* _pParseContext )
        :m_xORB( _rxORB )
        ,m_xConnection( _rxConnection )
        ,m_aParser( m_xORB, _pParseContext )
    {
        try
        {
            // create a number formatter / number formats supplier pair
            OSL_ENSURE( m_xORB.is(), "OPredicateInputController::OPredicateInputController: need a service factory!" );
            if ( m_xORB.is() )
            {
                m_xFormatter = Reference< XNumberFormatter >(
                    NumberFormatter::create(comphelper::ComponentContext(m_xORB).getUNOContext()),
                    UNO_QUERY_THROW
                );
            }

            Reference< XNumberFormatsSupplier >  xNumberFormats = ::dbtools::getNumberFormats( m_xConnection, sal_True );
            if ( !xNumberFormats.is() )
                ::comphelper::disposeComponent( m_xFormatter );
            else
                m_xFormatter->attachNumberFormatsSupplier( xNumberFormats );

            // create the locale data
            if ( m_xORB.is() )
            {
                m_xLocaleData = LocaleData::create( comphelper::ComponentContext(m_xORB).getUNOContext() );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OPredicateInputController::OPredicateInputController: caught an exception!" );
        }
    }

    //---------------------------------------------------------------------
    OSQLParseNode* OPredicateInputController::implPredicateTree(::rtl::OUString& _rErrorMessage, const ::rtl::OUString& _rStatement, const Reference< XPropertySet > & _rxField) const
    {
        OSQLParseNode* pReturn = const_cast< OSQLParser& >( m_aParser ).predicateTree( _rErrorMessage, _rStatement, m_xFormatter, _rxField );
        if ( !pReturn )
        {   // is it a text field ?
            sal_Int32 nType = DataType::OTHER;
            _rxField->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Type" )) ) >>= nType;

            if  (   ( DataType::CHAR        == nType )
                ||  ( DataType::VARCHAR     == nType )
                ||  ( DataType::LONGVARCHAR == nType )
                ||  ( DataType::CLOB        == nType )
                )
            {   // yes -> force a quoted text and try again
                ::rtl::OUString sQuoted( _rStatement );
                if  (   !sQuoted.isEmpty()
                    &&  (   (sQuoted.getStr()[0] != '\'')
                        ||  (sQuoted.getStr()[ sQuoted.getLength() - 1 ] != '\'' )
                        )
                    )
                {
                    static const ::rtl::OUString sSingleQuote( RTL_CONSTASCII_USTRINGPARAM( "'" ) );
                    static const ::rtl::OUString sDoubleQuote( RTL_CONSTASCII_USTRINGPARAM( "''" ) );

                    sal_Int32 nIndex = -1;
                    sal_Int32 nTemp = 0;
                    while ( -1 != ( nIndex = sQuoted.indexOf( '\'',nTemp ) ) )
                    {
                        sQuoted = sQuoted.replaceAt( nIndex, 1, sDoubleQuote );
                        nTemp = nIndex+2;
                    }

                    ::rtl::OUString sTemp( sSingleQuote );
                    ( sTemp += sQuoted ) += sSingleQuote;
                    sQuoted = sTemp;
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
                    if ( xPSI.is() && xPSI->hasPropertyByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FormatKey" )) ) )
                    {
                        sal_Int32 nFormatKey = 0;
                        _rxField->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FormatKey" )) ) >>= nFormatKey;
                        if ( nFormatKey && m_xFormatter.is() )
                        {
                            Locale aFormatLocale;
                            ::comphelper::getNumberFormatProperty(
                                m_xFormatter,
                                nFormatKey,
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Locale" ) )
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

                sal_Bool bDecDiffers = ( nCtxDecSep != nFmtDecSep );
                sal_Bool bFmtDiffers = ( nCtxThdSep != nFmtThdSep );
                if ( bDecDiffers || bFmtDiffers )
                {   // okay, at least one differs
                    // "translate" the value into the "format locale"
                    ::rtl::OUString sTranslated( _rStatement );
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

    //---------------------------------------------------------------------
    sal_Bool OPredicateInputController::normalizePredicateString(
        ::rtl::OUString& _rPredicateValue, const Reference< XPropertySet > & _rxField, ::rtl::OUString* _pErrorMessage ) const
    {
        OSL_ENSURE( m_xConnection.is() && m_xFormatter.is() && _rxField.is(),
            "OPredicateInputController::normalizePredicateString: invalid state or params!" );

        sal_Bool bSuccess = sal_False;
        if ( m_xConnection.is() && m_xFormatter.is() && _rxField.is() )
        {
            // parse the string
            ::rtl::OUString sError;
            ::rtl::OUString sTransformedText( _rPredicateValue );
            OSQLParseNode* pParseNode = implPredicateTree( sError, sTransformedText, _rxField );
            if ( _pErrorMessage ) *_pErrorMessage = sError;

            if ( pParseNode )
            {
                const IParseContext& rParseContext = m_aParser.getContext();
                sal_Unicode nDecSeparator, nThousandSeparator;
                getSeparatorChars( rParseContext.getPreferredLocale(), nDecSeparator, nThousandSeparator );

                // translate it back into a string
                sTransformedText = ::rtl::OUString();
                pParseNode->parseNodeToPredicateStr(
                    sTransformedText, m_xConnection, m_xFormatter, _rxField,
                    rParseContext.getPreferredLocale(), (sal_Char)nDecSeparator, &rParseContext
                );
                _rPredicateValue = sTransformedText;
                delete pParseNode;

                bSuccess = sal_True;
            }
        }

        return bSuccess;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OPredicateInputController::getPredicateValue(
        const ::rtl::OUString& _rPredicateValue, const Reference< XPropertySet > & _rxField,
        sal_Bool _bForStatementUse, ::rtl::OUString* _pErrorMessage ) const
    {
        OSL_ENSURE( _rxField.is(), "OPredicateInputController::getPredicateValue: invalid params!" );
        ::rtl::OUString sReturn;
        if ( _rxField.is() )
        {
            ::rtl::OUString sValue( _rPredicateValue );

            // a little problem : if the field is a text field, the normalizePredicateString added two
            // '-characters to the text. If we would give this to predicateTree this would add
            // two  additional '-characters which we don't want. So check the field format.
            // FS - 06.01.00 - 71532
            sal_Bool bValidQuotedText = ( sValue.getLength() >= 2 )
                                    &&  ( sValue.getStr()[0] == '\'' )
                                    &&  ( sValue.getStr()[ sValue.getLength() - 1 ] == '\'' );
                // again : as normalizePredicateString always did a conversion on the value text,
                // bValidQuotedText == sal_True implies that we have a text field, as no other field
                // values will be formatted with the quote characters
            if ( bValidQuotedText )
            {
                sValue = sValue.copy( 1, sValue.getLength() - 2 );
                static const ::rtl::OUString sSingleQuote( RTL_CONSTASCII_USTRINGPARAM( "'" ) );
                static const ::rtl::OUString sDoubleQuote( RTL_CONSTASCII_USTRINGPARAM( "''" ) );

                sal_Int32 nIndex = -1;
                sal_Int32 nTemp = 0;
                while ( -1 != ( nIndex = sValue.indexOf( sDoubleQuote,nTemp ) ) )
                {
                    sValue = sValue.replaceAt( nIndex, 2, sSingleQuote );
                    nTemp = nIndex+2;
                }
            }

            // The following is mostly stolen from the former implementation in the parameter dialog
            // (dbaccess/source/ui/dlg/paramdialog.cxx). I do not fully understand this .....

            ::rtl::OUString sError;
            OSQLParseNode* pParseNode = implPredicateTree( sError, sValue, _rxField );
            if ( _pErrorMessage )
                *_pErrorMessage = sError;

            sReturn = implParseNode(pParseNode,_bForStatementUse);
        }

        return sReturn;
    }

    ::rtl::OUString OPredicateInputController::getPredicateValue(
        const ::rtl::OUString& _sField, const ::rtl::OUString& _rPredicateValue, sal_Bool _bForStatementUse, ::rtl::OUString* _pErrorMessage ) const
    {
        ::rtl::OUString sReturn = _rPredicateValue;
        ::rtl::OUString sError;
        ::rtl::OUString sField = _sField;
        sal_Int32 nIndex = 0;
        sField = sField.getToken(0,'(',nIndex);
        if(nIndex == -1)
            sField = _sField;
        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionReturnType(sField,&m_aParser.getContext());
        if ( nType == DataType::OTHER || sField.isEmpty() )
        {
            // first try the international version
            ::rtl::OUString sSql;
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT * "));
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM x WHERE "));
            sSql += sField;
            sSql += _rPredicateValue;
            ::std::auto_ptr<OSQLParseNode> pParseNode( const_cast< OSQLParser& >( m_aParser ).parseTree( sError, sSql, sal_True ) );
            nType = DataType::DOUBLE;
            if ( pParseNode.get() )
            {
                OSQLParseNode* pColumnRef = pParseNode->getByRule(OSQLParseNode::column_ref);
                if ( pColumnRef )
                {
                }
            }
        }

        Reference<XDatabaseMetaData> xMeta = m_xConnection->getMetaData();
        parse::OParseColumn* pColumn = new parse::OParseColumn( sField,
                                                                ::rtl::OUString(),
                                                                ::rtl::OUString(),
                                                                ::rtl::OUString(),
                                                                ColumnValue::NULLABLE_UNKNOWN,
                                                                0,
                                                                0,
                                                                nType,
                                                                sal_False,
                                                                sal_False,
                                                                xMeta.is() && xMeta->supportsMixedCaseQuotedIdentifiers());
        Reference<XPropertySet> xColumn = pColumn;
        pColumn->setFunction(sal_True);
        pColumn->setRealName(sField);

        OSQLParseNode* pParseNode = implPredicateTree( sError, _rPredicateValue, xColumn );
        if ( _pErrorMessage )
            *_pErrorMessage = sError;
        return pParseNode ? implParseNode(pParseNode,_bForStatementUse) : sReturn;
    }

    ::rtl::OUString OPredicateInputController::implParseNode(OSQLParseNode* pParseNode,sal_Bool _bForStatementUse) const
    {
        ::rtl::OUString sReturn;
        if ( pParseNode )
        {
            boost::shared_ptr<OSQLParseNode> xTakeOwnership(pParseNode);
            OSQLParseNode* pOdbcSpec = pParseNode->getByRule( OSQLParseNode::odbc_fct_spec );
            if ( pOdbcSpec )
            {
                if ( _bForStatementUse )
                {
                    OSQLParseNode* pFuncSpecParent = pOdbcSpec->getParent();
                    OSL_ENSURE( pFuncSpecParent, "OPredicateInputController::getPredicateValue: an ODBC func spec node without parent?" );
                    if ( pFuncSpecParent )
                        pFuncSpecParent->parseNodeToStr(sReturn, m_xConnection, &m_aParser.getContext(), sal_False, sal_True);
                }
                else
                {
                    OSQLParseNode* pValueNode = pOdbcSpec->getChild(1);
                    if ( SQL_NODE_STRING == pValueNode->getNodeType() )
                        sReturn = pValueNode->getTokenValue();
                    else
                        pValueNode->parseNodeToStr(sReturn, m_xConnection, &m_aParser.getContext(), sal_False, sal_True);
                }
            }
            else
            {
                if  ( pParseNode->count() >= 3 )
                {
                    OSQLParseNode* pValueNode = pParseNode->getChild(2);
                    OSL_ENSURE( pValueNode, "OPredicateInputController::getPredicateValue: invalid node child!" );
                    if ( !_bForStatementUse )
                    {
                        if ( SQL_NODE_STRING == pValueNode->getNodeType() )
                            sReturn = pValueNode->getTokenValue();
                        else
                            pValueNode->parseNodeToStr(
                                sReturn, m_xConnection, &m_aParser.getContext(), sal_False, sal_True
                            );
                    }
                    else
                        pValueNode->parseNodeToStr(
                            sReturn, m_xConnection, &m_aParser.getContext(), sal_False, sal_True
                        );
                }
                else
                    OSL_FAIL( "OPredicateInputController::getPredicateValue: unknown/invalid structure (noodbc)!" );
            }
        }
        return sReturn;
    }
//.........................................................................
}   // namespace dbtools
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: predicateinput.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONNECTIVITY_PREDICATEINPUT_HXX
#include <connectivity/predicateinput.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................

    using ::com::sun::star::sdbc::XConnection;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::util::XNumberFormatsSupplier;
    using ::com::sun::star::util::XNumberFormatter;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::uno::Exception;
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
        OSL_ENSURE( 0 < _rSeparator.getLength(), "::lcl_getSeparatorChar: invalid separator string!" );

        sal_Unicode nReturn( _nFallback );
        if ( _rSeparator.getLength() )
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
            OSL_ENSURE( sal_False, "OPredicateInputController::getSeparatorChars: caught an exception!" );
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
                m_xFormatter = Reference< XNumberFormatter >( m_xORB->createInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.NumberFormatter" ) ) ),
                    UNO_QUERY
                );
            }

            Reference< XNumberFormatsSupplier >  xNumberFormats = ::dbtools::getNumberFormats( m_xConnection, sal_True );
            if ( !xNumberFormats.is() )
                ::comphelper::disposeComponent( m_xFormatter );
            else if ( m_xFormatter.is() )
                m_xFormatter->attachNumberFormatsSupplier( xNumberFormats );

            // create the locale data
            if ( m_xORB.is() )
            {
                m_xLocaleData = m_xLocaleData.query( m_xORB->createInstance(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.i18n.LocaleData" ) ) )
                );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OPredicateInputController::OPredicateInputController: caught an exception!" );
        }
    }

    //---------------------------------------------------------------------
    OSQLParseNode* OPredicateInputController::implPredicateTree(::rtl::OUString& _rErrorMessage, const ::rtl::OUString& _rStatement, const Reference< XPropertySet > & _rxField) const
    {
        OSQLParseNode* pReturn = const_cast< OSQLParser& >( m_aParser ).predicateTree( _rErrorMessage, _rStatement, m_xFormatter, _rxField );
        if ( !pReturn )
        {   // is it a text field ?
            sal_Int32 nType = DataType::OTHER;
            _rxField->getPropertyValue( ::rtl::OUString::createFromAscii( "Type" ) ) >>= nType;

            if  (   ( DataType::CHAR == nType )
                ||  ( DataType::VARCHAR == nType )
                ||  ( DataType::LONGVARCHAR == nType )
                )
            {   // yes -> force a quoted text and try again
                ::rtl::OUString sQuoted( _rStatement );
                if  (   sQuoted.getLength()
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
                    if ( xPSI.is() && xPSI->hasPropertyByName( ::rtl::OUString::createFromAscii( "FormatKey" ) ) )
                    {
                        sal_Int32 nFormatKey = 0;
                        _rxField->getPropertyValue( ::rtl::OUString::createFromAscii( "FormatKey" ) ) >>= nFormatKey;
                        if ( nFormatKey && m_xFormatter.is() )
                        {
                            Locale aFormatLocale;
                            ::comphelper::getNumberFormatProperty(
                                m_xFormatter,
                                nFormatKey,
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Locale" ) )
                            ) >>= aFormatLocale;

                            // valid locale
                            if ( aFormatLocale.Language.getLength() )
                            {
                                getSeparatorChars( aFormatLocale, nFmtDecSep, nCtxThdSep );
                            }
                        }
                    }
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "OPredicateInputController::implPredicateTree: caught an exception while dealing with the formats!" );
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
                    sTransformedText, m_xConnection->getMetaData(), m_xFormatter, _rxField,
                    rParseContext.getPreferredLocale(), nDecSeparator
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
            if ( _pErrorMessage ) *_pErrorMessage = sError;

            if ( pParseNode )
            {
                OSQLParseNode* pOdbcSpec = pParseNode->getByRule( OSQLParseNode::odbc_fct_spec );
                if ( pOdbcSpec )
                {
                    if ( !_bForStatementUse )
                    {
                        if  (   ( pOdbcSpec->count() >= 2 )
                            &&  ( SQL_NODE_STRING == pOdbcSpec->getChild(1)->getNodeType() )
                            )
                        {

                            sReturn = pOdbcSpec->getChild(1)->getTokenValue();
                        }
                        else
                            OSL_ENSURE( sal_False, "OPredicateInputController::getPredicateValue: unknown/invalid structure (odbc + param use)!" );
                    }
                    else
                    {
                        OSQLParseNode* pFuncSpecParent = pOdbcSpec->getParent();
                        OSL_ENSURE( pFuncSpecParent, "OPredicateInputController::getPredicateValue: an ODBC func spec node without parent?" );
                        if ( pFuncSpecParent )
                            pFuncSpecParent->parseNodeToStr(
                                sReturn, m_xConnection->getMetaData(), &m_aParser.getContext(), sal_False, sal_True
                            );
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
                                    sReturn, m_xConnection->getMetaData(), &m_aParser.getContext(), sal_False, sal_True
                                );
                        }
                        else
                            pValueNode->parseNodeToStr(
                                sReturn, m_xConnection->getMetaData(), &m_aParser.getContext(), sal_False, sal_True
                            );
                    }
                    else
                        OSL_ENSURE( sal_False, "OPredicateInputController::getPredicateValue: unknown/invalid structure (noodbc)!" );
                }

                delete pParseNode;
            }
        }

        return sReturn;
    }
//.........................................................................
}   // namespace dbtools
//.........................................................................



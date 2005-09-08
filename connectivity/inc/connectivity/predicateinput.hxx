/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: predicateinput.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 05:01:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONNECTIVITY_PREDICATEINPUT_HXX
#define CONNECTIVITY_PREDICATEINPUT_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XLOCALEDATA_HPP_
#include <com/sun/star/i18n/XLocaleData.hpp>
#endif

#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=====================================================================
    //= OPredicateInputController
    //=====================================================================
    /** A class which allows input of an SQL predicate for a row set column
        into a edit field.
    */
    class OPredicateInputController
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                m_xFormatter;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData >
                m_xLocaleData;

        ::connectivity::OSQLParser
                m_aParser;

    public:
        OPredicateInputController(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::connectivity::IParseContext* _pParseContext = NULL
        );

        /** transforms a "raw" predicate value (usually obtained from a user input) into a valid predicate for the given column
        @param _rPredicateValue
            The text to normalize.
        @param _rxField
            The field for which the text should be a predicate value.
        @param _pErrorMessage
            If not <NULL/>, and a parsing error occurs, the error message will be copied to the string the argument
            points to.
        */
        sal_Bool        normalizePredicateString(
            ::rtl::OUString& _rPredicateValue,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
            ::rtl::OUString* _pErrorMessage = NULL
        ) const;

        /** get's a value of the predicate which can be used in a WHERE clause.
        @param _rPredicateValue
            the value which has been normalized using normalizePredicateString
        @param _rxField
            is the field for which a predicate is to be entered
        @param _bForStatementUse
            If <TRUE/>, the returned value can be used in an SQL statement. If <FALSE/>, it can be used
            for instance for setting parameter values.
        @param _pErrorMessage
            If not <NULL/>, and a parsing error occurs, the error message will be copied to the string the argument
            points to.
        @see normalizePredicateString
        */
        ::rtl::OUString getPredicateValue(
            const ::rtl::OUString& _rPredicateValue,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField,
            sal_Bool _bForStatementUse,
            ::rtl::OUString* _pErrorMessage = NULL
        ) const;

    private:
        ::connectivity::OSQLParseNode* implPredicateTree(
            ::rtl::OUString& _rErrorMessage,
            const ::rtl::OUString& _rStatement,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _rxField
        ) const;

        sal_Bool getSeparatorChars(
            const ::com::sun::star::lang::Locale& _rLocale,
            sal_Unicode& _rDecSep,
            sal_Unicode& _rThdSep
        ) const;
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // CONNECTIVITY_PREDICATEINPUT_HXX


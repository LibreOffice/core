/*************************************************************************
 *
 *  $RCSfile: sqlparse.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 10:30:58 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#define _CONNECTIVITY_SQLPARSE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _OSL_MUTEX_HXX_ //autogen wg. Mutex
#include <osl/mutex.hxx>
#endif
#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif
#ifndef YYBISON
#ifndef FLEX_SCANNER
#include <connectivity/sqlbison.hxx>
#endif
#endif
#ifndef _COM_SUN_STAR_I18N_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XLOCALEDATA_HPP_
#include <com/sun/star/i18n/XLocaleData.hpp>
#endif

// forward declarations
namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace beans
            {
                class XPropertySet;
            }
            namespace util
            {
                class XNumberFormatter;
            }
            namespace lang
            {
                struct Locale;
            }
        }
    }
}
namespace connectivity
{
    class OSQLScanner;

    //==========================================================================
    //= OParseContext
    //==========================================================================
    class OParseContext
    {
    public:
        enum    ErrorCode
        {
            ERROR_NONE      = 0,
            ERROR_GENERAL,                  // "Syntax error in SQL expression"
            ERROR_GENERAL_HINT,             // "before \"#\" expression.", uses 1 parameter
            ERROR_VALUE_NO_LIKE,            // "The value # can not be used with LIKE!", uses 1 parameter
            ERROR_FIELD_NO_LIKE,            // "LIKE can not be used with this field!"
            ERROR_INVALID_COMPARE,          // "The entered criterion can not be compared with this field!";
            ERROR_INVALID_INT_COMPARE,      // "The field can not be compared with a number!"
            ERROR_INVALID_STRING_COMPARE,   // "The field can not be compared with a string!"
            ERROR_INVALID_DATE_COMPARE,     // "The field can not be compared with a date!"
            ERROR_INVALID_REAL_COMPARE      // "The field can not be compared with a floating point number!"
        };

        enum    InternationalKeyCode
        {
            KEY_NONE = 0,
            KEY_LIKE = SQL_TOKEN_LIKE,
            KEY_NOT = SQL_TOKEN_NOT,
            KEY_NULL = SQL_TOKEN_NULL,
            KEY_TRUE = SQL_TOKEN_TRUE,
            KEY_FALSE = SQL_TOKEN_FALSE,
            KEY_IS = SQL_TOKEN_IS,
            KEY_BETWEEN = SQL_TOKEN_BETWEEN,
            KEY_OR = SQL_TOKEN_OR,
            KEY_AND = SQL_TOKEN_AND,
            KEY_AVG = SQL_TOKEN_AVG,
            KEY_COUNT = SQL_TOKEN_COUNT,
            KEY_MAX = SQL_TOKEN_MAX,
            KEY_MIN = SQL_TOKEN_MIN,
            KEY_SUM = SQL_TOKEN_SUM
        };

    public:
        OParseContext();

        virtual ~OParseContext();
        // retrieves language specific error messages
        virtual ::rtl::OUString getErrorMessage(ErrorCode _eCodes) const;

        // retrieves language specific keyword strings (only ASCII allowed)
        virtual ::rtl::OString getIntlKeywordAscii(InternationalKeyCode _eKey) const;

        // finds out, if we have an international keyword (only ASCII allowed)
        virtual InternationalKeyCode getIntlKeyCode(const ::rtl::OString& rToken) const;

        // determines the default international setting
        static const ::com::sun::star::lang::Locale& getDefaultLocale();
    };

    //==========================================================================
    //= OSQLParser
    //==========================================================================
    /** Parser for SQL92
    */
    class OSQLParser
    {
        friend class OSQLParseNode;
        friend class OSQLInternalNode;
        friend struct OSQLParseNode::SQLParseNodeParameter;

    private:
    //  static parts for parsers
        static sal_uInt32           s_nRuleIDs[OSQLParseNode::rule_count + 1];
        static OParseContext        s_aDefaultContext;

    //  parts controled by mutex
        static ::osl::Mutex         s_aMutex;
        static OSQLScanner*         s_pScanner;
        static OSQLParseNodes*      s_pGarbageCollector;
        static sal_Int32            s_nRefCount;

    // informations on the current parse action
        const OParseContext*        m_pContext;
        OSQLParseNode*              m_pParseTree;   // result from parsing
        ::com::sun::star::lang::Locale* m_pLocale;      // current locale settings for parsing
        ::rtl::OUString                     m_sFieldName;   // current field name for a predicate
        ::rtl::OUString                     m_sErrorMessage;// current error msg
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xField;       // current field
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                                    m_xFormatter;   // current number formatter
        sal_Int32                   m_nFormatKey;   // numberformat, which should be used
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification> m_xCharClass;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData>              m_xLocaleData;

        // convert a string into double trim it to scale of _nscale and than transform it back to string
        ::rtl::OUString stringToDouble(const ::rtl::OUString& _rValue,sal_Int16 _nScale);

    public:
        // if NULL, a default context will be used
        // the context must live as long as the parser
        OSQLParser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory,const OParseContext* _pContext = NULL);
        ~OSQLParser();

        // Parsing an SQLStatement
        OSQLParseNode* parseTree(::rtl::OUString& rErrorMessage,
                       const ::rtl::OUString& rStatement,
                       sal_Bool bInternational = sal_False);

        // Check a Predicate
        OSQLParseNode* predicateTree(::rtl::OUString& rErrorMessage, const ::rtl::OUString& rStatement,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                       const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xField);

        // Access to the context
        const OParseContext& getContext() const {return *m_pContext;}

        // TokenIDToStr: Token-Name zu einer Token-Nr.
        static ::rtl::OString TokenIDToStr(sal_uInt32 nTokenID, const OParseContext* pContext = NULL);

        // StrToTokenID: Token-Nr. zu einem Token-Namen.
        // static sal_uInt32 StrToTokenID(const ::rtl::OString & rName);

        // RuleIDToStr gibt den zu einer RuleID gehoerenden ::rtl::OUString zurueck
        // (Leerstring, falls nicht gefunden)
        static ::rtl::OUString RuleIDToStr(sal_uInt32 nRuleID);

        // StrToRuleID berechnet zu einem ::rtl::OUString die RuleID (d.h. ::com::sun::star::sdbcx::Index in yytname)
        // (0, falls nicht gefunden). Die Suche nach der ID aufgrund eines Strings ist
        // extrem ineffizient (sequentielle Suche nach ::rtl::OUString)!
        static sal_uInt32 StrToRuleID(const ::rtl::OString & rValue);

        // RuleId mit enum, wesentlich effizienter
        static sal_uInt32 RuleID(OSQLParseNode::Rule eRule);



        void error(sal_Char *fmt);
        int SQLlex();
#ifdef YYBISON
        void setParseTree(OSQLParseNode * pNewParseTree);

        // Is the parse in a special mode?
        // Predicate chack is used to check a condition for a field
        sal_Bool inPredicateCheck() const {return m_xField.is();}
        const ::rtl::OUString& getFieldName() const {return m_sFieldName;}

        void reduceLiteral(OSQLParseNode*& pLiteral, sal_Bool bAppendBlank);
         // does not change the pLiteral argument
        sal_Int16 buildNode(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare);
        // makes a string out of a number, pLiteral will be deleted
        sal_Int16 buildNode_STR_NUM(OSQLParseNode*& pAppend,OSQLParseNode*& pLiteral,OSQLParseNode*& pCompare);
        sal_Int16 buildNode_Date(const double& fValue, sal_Int16 nType, OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare);

        sal_Int16 buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral);
        // pCompre will be deleted if it is not used
        sal_Int16 buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare);

        sal_Int16 buildLikeRule(OSQLParseNode*& pAppend,OSQLParseNode*& pLiteral,const OSQLParseNode* pEscape);
        sal_Int16 buildStringNodes(OSQLParseNode*& pLiteral);
#else
#endif
    };
}


#endif //_CONNECTIVITY_SQLPARSE_HXX

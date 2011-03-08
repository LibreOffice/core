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
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#define _CONNECTIVITY_SQLPARSE_HXX

#include <com/sun/star/uno/Reference.h>
#include <osl/mutex.hxx>
#include <connectivity/sqlnode.hxx>
#ifndef YYBISON
#ifndef FLEX_SCANNER
#ifndef BISON_INCLUDED
#define BISON_INCLUDED
#include "sqlbison.hxx"
#endif
#endif
#endif
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include "connectivity/IParseContext.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include <salhelper/singletonref.hxx>
#include <osl/mutex.hxx>

#include <map>

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
    class SQLError;

    //==========================================================================
    //= OParseContext
    //==========================================================================
    class OOO_DLLPUBLIC_DBTOOLS OParseContext : public IParseContext
    {
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

        /** set's the default locale which should be used when analyzing strings
            <p>If no locale is set, and any method which needs a locale is called, a default
            (en-US) is used.</p>
            <p>If, while parsing, the locale can be obtained from other sources (such as the number format
            set for a table column), the preferred locale is ignored.</p>
        */
        static void setDefaultLocale( const ::com::sun::star::lang::Locale& _rLocale );

        /** get's a locale instance which should be used when parsing in the context specified by this instance
            <p>if this is not overridden by derived classes, it returns the static default locale.</p>
        */
        virtual ::com::sun::star::lang::Locale getPreferredLocale( ) const;
    };

    //==========================================================================
    // OSQLParseNodesContainer
    // grabage collection of nodes
    //==========================================================================
    class OSQLParseNodesContainer
    {
        ::osl::Mutex m_aMutex;
        ::std::vector< OSQLParseNode* > m_aNodes;
    public:
        OSQLParseNodesContainer();
        ~OSQLParseNodesContainer();

        void push_back(OSQLParseNode* _pNode);
        void erase(OSQLParseNode* _pNode);
        bool empty() const;
        void clear();
        void clearAndDelete();
    };

    typedef salhelper::SingletonRef<OSQLParseNodesContainer> OSQLParseNodesGarbageCollector;

    //==========================================================================
    //= OSQLParser
    //==========================================================================
    struct OSQLParser_Data;
    /** Parser for SQL92
    */
    class OOO_DLLPUBLIC_DBTOOLS OSQLParser
    {
        friend class OSQLParseNode;
        friend class OSQLInternalNode;
        friend struct SQLParseNodeParameter;

    private:
        typedef ::std::map< sal_uInt32, OSQLParseNode::Rule >   RuleIDMap;
    //  static parts for parsers
        static sal_uInt32           s_nRuleIDs[OSQLParseNode::rule_count + 1];
        static RuleIDMap            s_aReverseRuleIDLookup;
        static OParseContext        s_aDefaultContext;

        static OSQLScanner*                     s_pScanner;
        static OSQLParseNodesGarbageCollector*  s_pGarbageCollector;
        static sal_Int32                        s_nRefCount;

    // informations on the current parse action
        const IParseContext*        m_pContext;
        OSQLParseNode*              m_pParseTree;   // result from parsing
        ::std::auto_ptr< OSQLParser_Data >
                                    m_pData;
        ::rtl::OUString                     m_sFieldName;   // current field name for a predicate
        ::rtl::OUString                     m_sErrorMessage;// current error msg

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xField;       // current field
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >
                                    m_xFormatter;   // current number formatter
        sal_Int32                   m_nFormatKey;   // numberformat, which should be used
        sal_Int32                   m_nDateFormatKey;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification> m_xCharClass;
        static ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData>       s_xLocaleData;
        ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XLocaleData>      xDummy; // can be deleted after 627

        // convert a string into double trim it to scale of _nscale and than transform it back to string
        ::rtl::OUString stringToDouble(const ::rtl::OUString& _rValue,sal_Int16 _nScale);
        OSQLParseNode*  buildDate(sal_Int32 _nType,OSQLParseNode*& pLiteral);
        bool            extractDate(OSQLParseNode* pLiteral,double& _rfValue);
        void            killThousandSeparator(OSQLParseNode* pLiteral);
        OSQLParseNode*  convertNode(sal_Int32 nType,OSQLParseNode*& pLiteral);
        // makes a string out of a number, pLiteral will be deleted
        OSQLParseNode*  buildNode_STR_NUM(OSQLParseNode*& pLiteral);
        OSQLParseNode*  buildNode_Date(const double& fValue, sal_Int32 nType);

        static ::osl::Mutex& getMutex();

    public:
        // if NULL, a default context will be used
        // the context must live as long as the parser
        OSQLParser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory,const IParseContext* _pContext = NULL);
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
        const IParseContext& getContext() const {return *m_pContext;}

        /// access to the SQLError instance owned by this parser
        const SQLError& getErrorHelper() const;

        // TokenIDToStr: token name belonging to a token number.
        static ::rtl::OString TokenIDToStr(sal_uInt32 nTokenID, const IParseContext* pContext = NULL);

        // StrToTokenID: token number belonging to a token name.
        // static sal_uInt32 StrToTokenID(const ::rtl::OString & rName);

        // (empty string if not found)
        static ::rtl::OUString RuleIDToStr(sal_uInt32 nRuleID);

        // StrToRuleID calculates the RuleID for a ::rtl::OUString (that is, ::com::sun::star::sdbcx::Index in yytname)
        // (0 if not found). The search for an ID based on a String is
        // extremely inefficient (sequential search for ::rtl::OUString)!
        static sal_uInt32 StrToRuleID(const ::rtl::OString & rValue);

        static OSQLParseNode::Rule RuleIDToRule( sal_uInt32 _nRule );

        // RuleId with enum, far more efficient
        static sal_uInt32 RuleID(OSQLParseNode::Rule eRule);
        // compares the _sFunctionName with all known function names and return the DataType of the return value
        static sal_Int32 getFunctionReturnType(const ::rtl::OUString& _sFunctionName, const IParseContext* pContext = NULL);

        // returns the type for a parameter in a given function name
        static sal_Int32 getFunctionParameterType(sal_uInt32 _nTokenId,sal_uInt32 _nPos);

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
        sal_Int16 buildNode(OSQLParseNode*& pAppend,OSQLParseNode* pCompare,OSQLParseNode* pLiteral,OSQLParseNode* pLiteral2);

        sal_Int16 buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral);
        // pCompre will be deleted if it is not used
        sal_Int16 buildPredicateRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare,OSQLParseNode* pLiteral2 = NULL);

        sal_Int16 buildLikeRule(OSQLParseNode*& pAppend,OSQLParseNode*& pLiteral,const OSQLParseNode* pEscape);
        sal_Int16 buildStringNodes(OSQLParseNode*& pLiteral);
#else
#endif
    };
}


#endif //_CONNECTIVITY_SQLPARSE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_CONNECTIVITY_SQLPARSE_HXX
#define INCLUDED_CONNECTIVITY_SQLPARSE_HXX

#include <memory>

#include <com/sun/star/uno/Reference.h>
#include <osl/mutex.hxx>
#include <connectivity/sqlnode.hxx>
#include <connectivity/IParseContext.hxx>
#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/sqlerror.hxx>
#include <salhelper/singletonref.hxx>

#include <map>

namespace com::sun::star::i18n { class XCharacterClassification; }
namespace com::sun::star::i18n { class XLocaleData4; }

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
        }
    }
}

namespace connectivity
{
    class OSQLScanner;

    //= OParseContext

    class OOO_DLLPUBLIC_DBTOOLS OParseContext : public IParseContext
    {
    public:
        OParseContext();

        virtual ~OParseContext();
        // retrieves language specific error messages
        virtual OUString getErrorMessage(ErrorCode _eCodes) const override;

        // retrieves language specific keyword strings (only ASCII allowed)
        virtual OString getIntlKeywordAscii(InternationalKeyCode _eKey) const override;

        // finds out, if we have an international keyword (only ASCII allowed)
        virtual InternationalKeyCode getIntlKeyCode(const OString& rToken) const override;

        // determines the default international setting
        static const css::lang::Locale& getDefaultLocale();

        /** gets a locale instance which should be used when parsing in the context specified by this instance
            <p>if this is not overridden by derived classes, it returns the static default locale.</p>
        */
        virtual css::lang::Locale getPreferredLocale( ) const override;
    };

    // OSQLParseNodesContainer
    // garbage collection of nodes

    class OSQLParseNodesContainer
    {
        ::osl::Mutex m_aMutex;
        ::std::vector< OSQLParseNode* > m_aNodes;
    public:
        OSQLParseNodesContainer();
        ~OSQLParseNodesContainer();

        void push_back(OSQLParseNode* _pNode);
        void erase(OSQLParseNode* _pNode);
        void clear();
        void clearAndDelete();
    };

    typedef salhelper::SingletonRef<OSQLParseNodesContainer> OSQLParseNodesGarbageCollector;

    //= OSQLParser

    struct OSQLParser_Data
    {
        css::lang::Locale               aLocale;
        ::connectivity::SQLError        aErrors;
    };

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

    // information on the current parse action
        const IParseContext*        m_pContext;
        std::unique_ptr<OSQLParseNode> m_pParseTree;   // result from parsing
        ::std::unique_ptr< OSQLParser_Data >
                                    m_pData;
        OUString                     m_sFieldName;   // current field name for a predicate
        OUString                     m_sErrorMessage;// current error msg

        css::uno::Reference< css::beans::XPropertySet >
                                    m_xField;       // current field
        css::uno::Reference< css::util::XNumberFormatter >
                                    m_xFormatter;   // current number formatter
        sal_Int32                   m_nFormatKey;   // numberformat, which should be used
        sal_Int32                   m_nDateFormatKey;
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        css::uno::Reference< css::i18n::XCharacterClassification> m_xCharClass;
        static css::uno::Reference< css::i18n::XLocaleData4>       s_xLocaleData;

        // convert a string into double trim it to scale of _nscale and than transform it back to string
        OUString stringToDouble(const OUString& _rValue,sal_Int16 _nScale);
        OSQLParseNode*  buildDate(sal_Int32 _nType,OSQLParseNode*& pLiteral);
        bool            extractDate(OSQLParseNode const * pLiteral,double& _rfValue);
        void            killThousandSeparator(OSQLParseNode* pLiteral);
        OSQLParseNode*  convertNode(sal_Int32 nType, OSQLParseNode* pLiteral);
        // makes a string out of a number, pLiteral will be deleted
        OSQLParseNode*  buildNode_STR_NUM(OSQLParseNode*& pLiteral);
        OSQLParseNode*  buildNode_Date(const double& fValue, sal_Int32 nType);

        static ::osl::Mutex& getMutex();

    public:
        // if NULL, a default context will be used
        // the context must live as long as the parser
        OSQLParser(const css::uno::Reference< css::uno::XComponentContext >& rxContext, const IParseContext* _pContext = nullptr);
        ~OSQLParser();

        // Parsing an SQLStatement
        std::unique_ptr<OSQLParseNode> parseTree(OUString& rErrorMessage,
                       const OUString& rStatement,
                       bool bInternational = false);

        // Check a Predicate
        // set bUseRealName to false if you pass a xField that comes from where you got that field,
        // as opposed from to from yourself.
        std::unique_ptr<OSQLParseNode> predicateTree(OUString& rErrorMessage, const OUString& rStatement,
                       const css::uno::Reference< css::util::XNumberFormatter > & xFormatter,
                       const css::uno::Reference< css::beans::XPropertySet > & xField,
                       bool bUseRealName = true);

        // Access to the context
        const IParseContext& getContext() const {return *m_pContext;}

        /// access to the SQLError instance owned by this parser
        const SQLError& getErrorHelper() const;

        // TokenIDToStr: token name belonging to a token number.
        static OString TokenIDToStr(sal_uInt32 nTokenID, const IParseContext* pContext = nullptr);

#if OSL_DEBUG_LEVEL > 0
        // (empty string if not found)
        static OUString RuleIDToStr(sal_uInt32 nRuleID);
#endif

        // StrToRuleID calculates the RuleID for a OUString (that is, css::sdbcx::Index in yytname)
        // (0 if not found). The search for an ID based on a String is
        // extremely inefficient (sequential search for OUString)!
        static sal_uInt32 StrToRuleID(const OString & rValue);

        static OSQLParseNode::Rule RuleIDToRule( sal_uInt32 _nRule );

        // RuleId with enum, far more efficient
        static sal_uInt32 RuleID(OSQLParseNode::Rule eRule);
        // compares the _sFunctionName with all known function names and return the DataType of the return value
        static sal_Int32 getFunctionReturnType(const OUString& _sFunctionName, const IParseContext* pContext);

        // returns the type for a parameter in a given function name
        static sal_Int32 getFunctionParameterType(sal_uInt32 _nTokenId,sal_uInt32 _nPos);

        void error(const sal_Char *fmt);
        static int SQLlex();
#ifdef YYBISON
        void setParseTree(OSQLParseNode * pNewParseTree);

        // Is the parse in a special mode?
        // Predicate check is used to check a condition for a field
        bool inPredicateCheck() const {return m_xField.is();}
        const OUString& getFieldName() const {return m_sFieldName;}

        static void reduceLiteral(OSQLParseNode*& pLiteral, bool bAppendBlank);
         // does not change the pLiteral argument
        sal_Int16 buildNode(OSQLParseNode*& pAppend,OSQLParseNode* pCompare,OSQLParseNode* pLiteral,OSQLParseNode* pLiteral2);

        sal_Int16 buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral);
        // pCompre will be deleted if it is not used
        sal_Int16 buildPredicateRule(OSQLParseNode*& pAppend,OSQLParseNode* const pLiteral,OSQLParseNode* pCompare,OSQLParseNode* pLiteral2 = nullptr);

        sal_Int16 buildLikeRule(OSQLParseNode* pAppend, OSQLParseNode*& pLiteral, const OSQLParseNode* pEscape);
        sal_Int16 buildStringNodes(OSQLParseNode*& pLiteral);
#endif
    };
}

#endif // INCLUDED_CONNECTIVITY_SQLPARSE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

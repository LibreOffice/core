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

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_XMLCONVERTER_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_XMLCONVERTER_HXX

#include "global.hxx"
#include "detfunc.hxx"
#include "detdata.hxx"
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/sheet/ConditionOperator.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>
#include <com/sun/star/util/DateTime.hpp>

class ScDocument;
class DateTime;

class ScXMLConverter
{
public:
    inline              ScXMLConverter()    {}
    inline              ~ScXMLConverter()   {}

// helper methods
    static ScDocument*  GetScDocument(
                            css::uno::Reference< css::frame::XModel > xModel );

// IMPORT: GeneralFunction / ScSubTotalFunc
    static css::sheet::GeneralFunction
                        GetFunctionFromString(
                            const OUString& rString );
    static ScSubTotalFunc GetSubTotalFuncFromString(
                            const OUString& rString );

// EXPORT: GeneralFunction / ScSubTotalFunc
    static void         GetStringFromFunction(
                            OUString& rString,
                            const css::sheet::GeneralFunction eFunction,
                            bool bAppendStr = false );
    static void         GetStringFromFunction(
                            OUString& rString,
                            const ScSubTotalFunc eFunction,
                            bool bAppendStr = false );

// IMPORT: DataPilotFieldOrientation
    static css::sheet::DataPilotFieldOrientation
                        GetOrientationFromString(
                            const OUString& rString );

// EXPORT: DataPilotFieldOrientation
    static void         GetStringFromOrientation(
                            OUString& rString,
                            const css::sheet::DataPilotFieldOrientation eOrientation,
                            bool bAppendStr = false );

// IMPORT: Detective
    static ScDetectiveObjType
                        GetDetObjTypeFromString(
                            const OUString& rString );
    static bool         GetDetOpTypeFromString(
                            ScDetOpType& rDetOpType,
                            const OUString& rString );

// EXPORT: Detective
    static void         GetStringFromDetObjType(
                            OUString& rString,
                            const ScDetectiveObjType eObjType,
                            bool bAppendStr = false );
    static void         GetStringFromDetOpType(
                            OUString& rString,
                            const ScDetOpType eOpType,
                            bool bAppendStr = false );

// IMPORT: Formulas
    static void         ConvertCellRangeAddress(
                            OUString& sFormula,
                            const bool bIsFormula = true);
// EXPORT: Core Date Time
    static void         ConvertDateTimeToString(const DateTime& aDateTime, OUStringBuffer& sDate);

};

enum ScXMLConditionToken
{
    XML_COND_INVALID,                       /// Token not recognized.
    XML_COND_AND,                           /// The 'and' token.
    XML_COND_CELLCONTENT,                   /// The 'cell-content' token.
    XML_COND_ISBETWEEN,                     /// The 'cell-content-is-between' token.
    XML_COND_ISNOTBETWEEN,                  /// The 'cell-content-is-not-between' token.
    XML_COND_ISWHOLENUMBER,                 /// The 'cell-content-is-whole-number' token.
    XML_COND_ISDECIMALNUMBER,               /// The 'cell-content-is-decimal-number' token.
    XML_COND_ISDATE,                        /// The 'cell-content-is-date' token.
    XML_COND_ISTIME,                        /// The 'cell-content-is-time' token.
    XML_COND_ISINLIST,                      /// The 'cell-content-is-in-list' token.
    XML_COND_TEXTLENGTH,                    /// The 'cell-content-text-length' token.
    XML_COND_TEXTLENGTH_ISBETWEEN,          /// The 'cell-content-text-length-is-between' token.
    XML_COND_TEXTLENGTH_ISNOTBETWEEN,       /// The 'cell-content-text-length-is-not-between' token.
    XML_COND_ISTRUEFORMULA                  /// The 'is-true-formula' token.
};

/** Result of an attempt to parse a single condition in a 'condition' attribute
    value of e.g. conditional formatting or data validation.
 */
struct ScXMLConditionParseResult
{
    ScXMLConditionToken meToken;            /// The leading condition token.
    css::sheet::ValidationType
                        meValidation;       /// A data validation type if existing.
    css::sheet::ConditionOperator
                        meOperator;         /// A comparison operator if existing.
    OUString            maOperand1;         /// First operand of the token or comparison value.
    OUString            maOperand2;         /// Second operand of 'between' conditions.
    sal_Int32           mnEndIndex;         /// Index of first character following the condition.
};

namespace ScXMLConditionHelper
{
    /** Parses the next condition in a 'condition' attribute value of e.g.
        conditional formatting or data validation.
     */
    void         parseCondition(
                            ScXMLConditionParseResult& rParseResult,
                            const OUString& rAttribute,
                            sal_Int32 nStartIndex );

    OUString getExpression(const sal_Unicode*& rpcString, const sal_Unicode* pcEnd, sal_Unicode cEndChar );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

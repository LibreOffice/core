/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_XMLCONVERTER_HXX
#define SC_XMLCONVERTER_HXX

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
class SvXMLUnitConverter;


//___________________________________________________________________

class ScXMLConverter
{
public:
    inline              ScXMLConverter()    {}
    inline              ~ScXMLConverter()   {}

// helper methods
    static ScDocument*  GetScDocument(
                            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel );

// IMPORT: GeneralFunction / ScSubTotalFunc
    static ::com::sun::star::sheet::GeneralFunction
                        GetFunctionFromString(
                            const ::rtl::OUString& rString );
    static ScSubTotalFunc GetSubTotalFuncFromString(
                            const ::rtl::OUString& rString );

// EXPORT: GeneralFunction / ScSubTotalFunc
    static void         GetStringFromFunction(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::sheet::GeneralFunction eFunction,
                            sal_Bool bAppendStr = sal_False );
    static void         GetStringFromFunction(
                            ::rtl::OUString& rString,
                            const ScSubTotalFunc eFunction,
                            sal_Bool bAppendStr = sal_False );

// IMPORT: DataPilotFieldOrientation
    static ::com::sun::star::sheet::DataPilotFieldOrientation
                        GetOrientationFromString(
                            const ::rtl::OUString& rString );

// EXPORT: DataPilotFieldOrientation
    static void         GetStringFromOrientation(
                            ::rtl::OUString& rString,
                            const ::com::sun::star::sheet::DataPilotFieldOrientation eOrientation,
                            sal_Bool bAppendStr = sal_False );

// IMPORT: Detective
    static ScDetectiveObjType
                        GetDetObjTypeFromString(
                            const ::rtl::OUString& rString );
    static sal_Bool     GetDetOpTypeFromString(
                            ScDetOpType& rDetOpType,
                            const ::rtl::OUString& rString );

// EXPORT: Detective
    static void         GetStringFromDetObjType(
                            ::rtl::OUString& rString,
                            const ScDetectiveObjType eObjType,
                            sal_Bool bAppendStr = sal_False );
    static void         GetStringFromDetOpType(
                            ::rtl::OUString& rString,
                            const ScDetOpType eOpType,
                            sal_Bool bAppendStr = sal_False );

// IMPORT: Formulas
    static void         ParseFormula(
                            ::rtl::OUString& sFormula,
                            const sal_Bool bIsFormula = sal_True);
// EXPORT: Core Date Time
    static void         ConvertDateTimeToString(const DateTime& aDateTime, rtl::OUStringBuffer& sDate);
//UNUSED2008-05  // IMPORT: Core Date Time
//UNUSED2008-05      static void         ConvertStringToDateTime(const rtl::OUString& sDate, DateTime& aDateTime, SvXMLUnitConverter* pUnitConverter);

    static void         ConvertCoreToAPIDateTime(const DateTime& aDateTime, com::sun::star::util::DateTime& rDateTime);

    static void         ConvertAPIToCoreDateTime(const com::sun::star::util::DateTime& aDateTime, DateTime& rDateTime);
};

// ============================================================================

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

// ----------------------------------------------------------------------------

/** Result of an attempt to parse a single condition in a 'condition' attribute
    value of e.g. conditional formatting or data validation.
 */
struct ScXMLConditionParseResult
{
    ScXMLConditionToken meToken;            /// The leading condition token.
    ::com::sun::star::sheet::ValidationType
                        meValidation;       /// A data validation type if existing.
    ::com::sun::star::sheet::ConditionOperator
                        meOperator;         /// A comparison operator if existing.
    ::rtl::OUString     maOperand1;         /// First operand of the token or comparison value.
    ::rtl::OUString     maOperand2;         /// Second operand of 'between' conditions.
    sal_Int32           mnEndIndex;         /// Index of first character following the condition.
};

// ----------------------------------------------------------------------------

class ScXMLConditionHelper
{
public:
    /** Parses the next condition in a 'condition' attribute value of e.g.
        conditional formatting or data validation.
     */
    static void         parseCondition(
                            ScXMLConditionParseResult& rParseResult,
                            const ::rtl::OUString& rAttribute,
                            sal_Int32 nStartIndex );

private:
                        ScXMLConditionHelper();
                        ~ScXMLConditionHelper();
};

// ============================================================================

#endif


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

/** @#file
 *
 *  XML import of all variable related text fields plus database display field
 */

#ifndef INCLUDED_XMLOFF_INC_TXTVFLDI_HXX
#define INCLUDED_XMLOFF_INC_TXTVFLDI_HXX

#include "txtfldi.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>


/** helper class: parses value-type and associated value attributes */
class XMLValueImportHelper
{

    const OUString sPropertyContent;
    const OUString sPropertyValue;
    const OUString sPropertyNumberFormat;
    const OUString sPropertyIsFixedLanguage;

    SvXMLImport& rImport;
    XMLTextImportHelper& rHelper;

    OUString sValue;     /// string value (only valid if bStringValueOK)
    double fValue;              /// double value (only valid if bFloatValueOK)
    sal_Int32 nFormatKey;       /// format key (only valid of bFormatOK)
    OUString sFormula;   /// formula string
    OUString sDefault;   /// default (see bStringDefault/bFormulaDef.)
    bool bIsDefaultLanguage;/// format (of nFormatKey) has system language?

    bool bStringType;       /// is this a string (or a float) type?
    bool bFormatOK;         /// have we read a style:data-style-name attr.?
    bool bTypeOK;           /// have we read a value-type attribute?
    bool bStringValueOK;    /// have we read a string-value attr.?
    bool bFloatValueOK;     /// have we read any of the float attr.s?
    bool bFormulaOK;        /// have we read the formula attribute?

    const bool bSetType;    /// should PrepareField set the SetExp subtype?
    const bool bSetValue;   /// should PrepareField set content/value?
    const bool bSetStyle;   /// should PrepareField set NumberFormat?
    const bool bSetFormula; /// should PrepareField set Formula?

    const bool bStringDefault;  /// default: string-value = content
    const bool bFormulaDefault; /// default: formula = content

public:
    XMLValueImportHelper(
        SvXMLImport& rImprt,                    /// XML Import
        XMLTextImportHelper& rHlp,              /// text import helper
        bool bType,                         /// process type (PrepareField)
        bool bStyle,                        /// process data style (P.F.)
        bool bValue,                        /// process value (Prep.Field)
        bool bFormula);                     /// process formula (Prep.F.)

    virtual ~XMLValueImportHelper();

    /// process attribute values
    void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);

    /// is value a string (rather than double)?
    inline bool IsStringValue() { return bStringType; }

    /// has format been read?
    inline bool IsFormatOK() { return bFormatOK; }

    inline void SetDefault(const OUString& sStr) { sDefault = sStr; }
};



/**
 * abstract parent class for all variable related fields
 * - variable-set/get/decl      (not -decls),
 * - user-field-get/decl        (not -decls),
 * - sequence/-decl             (not -decls),
 * - expression,
 * - text-input
 *
 * Processes the following attributes:
 * - name
 * - formula
 * - display
 * - value, value-type, data-style-name (via XMLValueImportHelper)
 * - description.
 *
 * Each attribute has a corresponding member, a bool variable to indicate
 * whether it was set or not, and a bool variable whether it should be set
 * using the standard property name.
 *
 * bValid is set true, when name is found!
 * (Most variable related fields are valid, if a name is
 * found. However, some are always valid. In this case, setting bValid
 * does not matter.)
 */
class XMLVarFieldImportContext : public XMLTextFieldImportContext
{
protected:
    const OUString sPropertyContent;
    const OUString sPropertyHint;
    const OUString sPropertyHelp;
    const OUString sPropertyTooltip;
    const OUString sPropertyIsVisible;
    const OUString sPropertyIsDisplayFormula;
    const OUString sPropertyCurrentPresentation;

private:
    OUString sName;              /// name attribute
    OUString sFormula;           /// formula attribute
    OUString sDescription;       /// description
    OUString sHelp;              /// help text
    OUString sHint;              /// hint
    XMLValueImportHelper aValueHelper;  /// value, value-type, and style
    bool bDisplayFormula;           /// display formula?(rather than value)
    bool bDisplayNone;              /// hide field?

    bool bNameOK;                   /// sName was set
    bool bFormulaOK;                /// sFormula was set
    bool bDescriptionOK;            /// sDescription was set
    bool bHelpOK;                   /// sHelp was set
    bool bHintOK;                   /// sHint was set
    bool bDisplayOK;                /// sDisplayFormula/-None were set

    bool bSetFormula;               /// set Formula property
    bool bSetFormulaDefault;        /// use content as default for formula
    bool bSetDescription;           /// set sDescription with Hint-property
    bool bSetHelp;
    bool bSetHint;
    bool bSetVisible;               /// set IsVisible
    bool bSetDisplayFormula;        /// set DisplayFormula (sub type???)
    bool bSetPresentation;          /// set presentation frm elem. content?

public:


    XMLVarFieldImportContext(
        // for XMLTextFieldImportContext:
        SvXMLImport& rImport,           /// XML Import
        XMLTextImportHelper& rHlp,      /// text import helper
        const sal_Char* pServiceName,   /// name of SO API service
        sal_uInt16 nPrfx,               /// namespace prefix
        const OUString& rLocalName,  /// element name w/o prefix
        // config variables for PrepareField behavior:
        bool bFormula,              /// set Formula property
        bool bFormulaDefault,       /// use content as default for formula
        bool bDescription,          /// set sDescription with Hint-property
        bool bHelp,
        bool bHint,
        bool bVisible,              /// set IsVisible (display attr)
        bool bDisplayFormula,       /// set ??? (display attr.)
        bool bType,                 /// set value type with ???-property
        bool bStyle,                /// set data style (NumberFormat-Prop.)
        bool bValue,                /// set value with Content/Value-Prop.
        bool bPresentation);        /// set presentation from elem. content

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;

    // various accessor methods:
    inline OUString GetName()    { return sName; }
    inline bool IsStringValue()     { return aValueHelper.IsStringValue();}
};



/** import variable get fields (<text:variable-get>) */
class XMLVariableGetFieldImportContext : public XMLVarFieldImportContext
{
public:


    XMLVariableGetFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix


protected:
    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};



/** import expression fields (<text:expression>) */
class XMLExpressionFieldImportContext : public XMLVarFieldImportContext
{
    const OUString sPropertySubType;

public:


    XMLExpressionFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

protected:
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};

/*** import text input fields (<text:text-input>) */
class XMLTextInputFieldImportContext : public XMLVarFieldImportContext
{
    const OUString sPropertyContent;

public:


    XMLTextInputFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

protected:
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};






/**
 * uperclass for variable/user-set, var/user-input, and sequence fields
 * inds field master of appropriate type and attaches field to it.
 */
class XMLSetVarFieldImportContext : public XMLVarFieldImportContext
{
    const VarType eFieldType;

public:


    XMLSetVarFieldImportContext(
        // for XMLTextFieldImportContext:
        SvXMLImport& rImport,           /// see XMLTextFieldImportContext
        XMLTextImportHelper& rHlp,      /// see XMLTextFieldImportContext
        const sal_Char* pServiceName,   /// see XMLTextFieldImportContext
        sal_uInt16 nPrfx,               /// see XMLTextFieldImportContext
        const OUString& rLocalName, /// see XMLTextFieldImportContext
        // for finding appropriate field master (see EndElement())
        VarType eVarType,               /// variable type
        // config variables:
        bool bFormula,              /// see XMLTextFieldImportContext
        bool bFormulaDefault,       /// see XMLTextFieldImportContext
        bool bDescription,          /// see XMLTextFieldImportContext
        bool bHelp,                 /// see XMLTextFieldImportContext
        bool bHint,                 /// see XMLTextFieldImportContext
        bool bVisible,              /// see XMLTextFieldImportContext
        bool bDisplayFormula,       /// see XMLTextFieldImportContext
        bool bType,                 /// see XMLTextFieldImportContext
        bool bStyle,                /// see XMLTextFieldImportContext
        bool bValue,                /// see XMLTextFieldImportContext
        bool bPresentation);        /// see XMLTextFieldImportContext

protected:

    /// create XTextField, attach master and insert into document;
    /// also calls PrepareTextField
    virtual void EndElement() override;

    /// find appropriate field master
    bool FindFieldMaster(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xMaster);
};



/** import variable set fields (<text:variable-set>) */
class XMLVariableSetFieldImportContext : public XMLSetVarFieldImportContext
{
    const OUString sPropertySubType;

public:


    XMLVariableSetFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix

protected:
    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};



/** variable input fields (<text:variable-input>) */
class XMLVariableInputFieldImportContext : public XMLSetVarFieldImportContext
{
    const OUString sPropertySubType;
    const OUString sPropertyIsInput;

public:


    XMLVariableInputFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix

protected:

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};



/** user fields (<text:user-field-get>) */
class XMLUserFieldImportContext : public XMLSetVarFieldImportContext
{

public:


    XMLUserFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix
};

/** user input fields (<text:user-field-input>) */
class XMLUserFieldInputImportContext : public XMLVarFieldImportContext
{

public:


    XMLUserFieldInputImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix

    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};



/** sequence fields (<text:sequence>) */
class XMLSequenceFieldImportContext : public XMLSetVarFieldImportContext
{
    const OUString sPropertyNumberFormat;
    const OUString sPropertySequenceValue;
    OUString sNumFormat;
    OUString sNumFormatSync;
    OUString sRefName;

    bool bRefNameOK;

public:


    XMLSequenceFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};






/**
 * variable declaration container for all variable fields
 *      (variable-decls, user-field-decls, sequence-decls)
 */
class XMLVariableDeclsImportContext : public SvXMLImportContext
{
    enum VarType eVarDeclsContextType;
    XMLTextImportHelper& rImportHelper;

public:


    XMLVariableDeclsImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName,      /// element name w/o prefix
        enum VarType eVarType);                 /// variable type

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) override;
};

/**
 * variable field declarations
 *      (variable-decl, user-field-decl, sequence-decl)
 */
class XMLVariableDeclImportContext : public SvXMLImportContext
{
    const OUString sPropertySubType;
    const OUString sPropertyNumberingLevel;
    const OUString sPropertyNumberingSeparator;
    const OUString sPropertyIsExpression;

    OUString sName;
    XMLValueImportHelper aValueHelper;
    sal_Int8 nNumLevel;
    sal_Unicode cSeparationChar;

public:


    XMLVariableDeclImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName,      /// element name w/o prefix
        const ::com::sun::star::uno::Reference< /// list of element attributes
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList,
        enum VarType eVarType);                 /// variable type

    /// get field master for name and rename if appropriate
    static bool FindFieldMaster(::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet> & xMaster,
                                    SvXMLImport& rImport,
                                    XMLTextImportHelper& rHelper,
                                    const OUString& sVarName,
                                    enum VarType eVarType);
};



/** import table formula fields (deprecated; for Writer 2.0 compatibility) */
class XMLTableFormulaImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyIsShowFormula;
    const OUString sPropertyCurrentPresentation;

    XMLValueImportHelper aValueHelper;

    bool bIsShowFormula;

public:


    XMLTableFormulaImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix
    virtual ~XMLTableFormulaImportContext();

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) override;
};



/** import database display fields (<text:database-display>) */
class XMLDatabaseDisplayImportContext : public XMLDatabaseFieldImportContext
{
    const OUString sPropertyColumnName;
    const OUString sPropertyDatabaseFormat;
    const OUString sPropertyCurrentPresentation;
    const OUString sPropertyIsVisible;

    XMLValueImportHelper aValueHelper;

    OUString sColumnName;
    bool bColumnOK;

    bool bDisplay;
    bool bDisplayOK;

public:


    XMLDatabaseDisplayImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) override;

    /// create, prepare and insert database field master and database field
    virtual void EndElement() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

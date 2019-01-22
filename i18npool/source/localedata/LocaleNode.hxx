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
#ifndef INCLUDED_I18NPOOL_SOURCE_LOCALEDATA_LOCALENODE_HXX
#define INCLUDED_I18NPOOL_SOURCE_LOCALEDATA_LOCALENODE_HXX

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <string>
#include <vector>
#include <memory>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

class OFileWriter
{
public:
    OFileWriter(const char *pcFile, const char *locale );
    ~OFileWriter();
    void  writeStringCharacters(const OUString& str) const;
    void  writeAsciiString(const char *str)const ;
    void  writeInt(sal_Int16 nb) const;
    void  writeFunction(const char *func, const char *count, const char *array) const;
    void  writeRefFunction(const char *func, const OUString& useLocale) const;
    void  writeFunction(const char *func, const char *count, const char *array, const char *from, const char *to) const;
    void  writeRefFunction(const char *func, const OUString& useLocale, const char *to) const;
    void  writeFunction2(const char *func, const char *style, const char* attr, const char *array) const;
    void  writeRefFunction2(const char *func, const OUString& useLocale) const;
    void  writeFunction3(const char *func, const char *style, const char* levels, const char* attr, const char *array) const;
    void  writeRefFunction3(const char *func, const OUString& useLocale) const;
    void  writeIntParameter(const sal_Char* pAsciiStr, const sal_Int16 count, sal_Int16 val) const;
    bool  writeDefaultParameter(const sal_Char* pAsciiStr, const OUString& str, sal_Int16 count) const;
    void  writeParameter(const sal_Char* pAsciiStr, const OUString& aChars) const;
    void  writeParameter(const sal_Char* pAsciiStr, const OUString& aChars, sal_Int16 count) const;
    void  writeParameter(const sal_Char* pAsciiStr, const OUString& aChars, sal_Int16 count0, sal_Int16 count1) const;
    void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const OUString& aChars, const sal_Int16 count) const;
    void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const OUString& aChars, sal_Int16 count0, sal_Int16 count1) const;
    void  closeOutput() const;
    /// Return the locale string, something like en_US or de_DE
    const char * getLocale() const { return theLocale.c_str(); }
private:
    std::string theLocale;
    FILE *m_f;
};

class Attr {
    Sequence <OUString> name;
    Sequence <OUString> value;

public:
    explicit Attr (const Reference< XAttributeList > & attr);
    OUString getValueByName (const sal_Char *str) const;
    const OUString& getValueByIndex (sal_Int32 idx) const ;
};

class LocaleNode
{
    OUString aName;
    OUString aValue;
    Attr aAttribs;
    LocaleNode * parent;
    std::vector<std::unique_ptr<LocaleNode>> children;

protected:
    mutable int nError;

public:
    LocaleNode (const OUString& name, const Reference< XAttributeList > & attr);
    void setValue(const OUString &oValue) { aValue += oValue; };
    const OUString& getName() const { return aName; };
    const OUString& getValue() const { return aValue; };
    const Attr& getAttr() const { return aAttribs; };
    sal_Int32 getNumberOfChildren () const { return sal_Int32(children.size()); };
    LocaleNode * getChildAt (sal_Int32 idx) const { return children[idx].get(); };
    const LocaleNode * findNode ( const sal_Char *name) const;
    virtual ~LocaleNode();
    void addChild (  LocaleNode * node);
    const LocaleNode* getRoot() const;
    int getError() const;
    virtual void generateCode (const OFileWriter &of) const;
    // MUST >= nMinLen
    // nMinLen <= 0 : no error
    // nMinLen >  0 : error if less than nMinLen characters
    // SHOULD NOT > nMaxLen
    // nMaxLen <  0 : any length
    // nMaxLen >= 0 : warning if more than nMaxLen characters
    OUString writeParameterCheckLen( const OFileWriter &of, const char* pParameterName, const LocaleNode* pNode, sal_Int32 nMinLen, sal_Int32 nMaxLen ) const;
    OUString writeParameterCheckLen( const OFileWriter &of, const char* pNodeName, const char* pParameterName, sal_Int32 nMinLen, sal_Int32 nMaxLen ) const;
    // ++nError with output to stderr
    void incError( const char* pStr ) const;
    // ++nError with output to stderr
    void incError( const OUString& rStr ) const;
    // ++nError with output to stderr, pStr should contain "%d"
    void incErrorInt( const char* pStr, int nVal ) const;
    // ++nError with output to stderr, pStr should contain "%s"
    void incErrorStr( const char* pStr, const OUString& rVal ) const;
    // ++nError with output to stderr, pStr should contain "%s %s"
    void incErrorStrStr( const char* pStr, const OUString& rVal1, const OUString& rVal2 ) const;
    static LocaleNode* createNode (const OUString& name,const Reference< XAttributeList > & attr);
};

class LCInfoNode : public LocaleNode {
public:
    LCInfoNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };
    virtual void generateCode (const OFileWriter &of) const override;
};


class LCCTYPENode : public LocaleNode {
public:
    LCCTYPENode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCFormatNode : public LocaleNode {
    static sal_Int16 mnSection;
    static sal_Int16 mnFormats;
public:
    LCFormatNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCCollationNode : public LocaleNode {
public:
    LCCollationNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCIndexNode : public LocaleNode {
public:
    LCIndexNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCSearchNode : public LocaleNode {
public:
    LCSearchNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCCalendarNode : public LocaleNode {
public:
    LCCalendarNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCCurrencyNode : public LocaleNode {
public:
    LCCurrencyNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCTransliterationNode : public LocaleNode {
public:
    LCTransliterationNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCMiscNode : public LocaleNode {
public:
    LCMiscNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCNumberingLevelNode : public LocaleNode {
public:
    LCNumberingLevelNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

class LCOutlineNumberingLevelNode : public LocaleNode {
public:
    LCOutlineNumberingLevelNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

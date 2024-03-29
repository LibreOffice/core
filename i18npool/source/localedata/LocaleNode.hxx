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
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include <com/sun/star/uno/Sequence.hxx>

namespace com::sun::star::xml::sax { class XAttributeList; }

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

class OFileWriter
{
public:
    OFileWriter(const char *pcFile, const char *locale );
    ~OFileWriter();
    void  writeStringCharacters(std::u16string_view str) const;
    void  writeOUStringLiteralCharacters(std::u16string_view str) const;
    void  writeAsciiString(const char *str)const ;
    void  writeInt(sal_Int16 nb) const;
    void  writeHexInt(sal_Int16 nb) const;
    void  writeOUStringRefFunction(const char *func, std::u16string_view useLocale) const;
    void  writeOUStringRefFunction(const char *func, std::u16string_view useLocale, const char *to) const;
    void  writeOUStringFunction(const char *func, const char *count, const char *array) const;
    void  writeOUStringFunction(const char *func, const char *count, const char *array, const char *from, const char *to) const;
    void  writeOUStringFunction2(const char *func, const char *style, const char* attr, const char *array) const;
    void  writeOUStringRefFunction2(const char *func, std::u16string_view useLocale) const;
    void  writeOUStringFunction3(const char *func, const char *style, const char* levels, const char* attr, const char *array) const;
    void  writeOUStringRefFunction3(const char *func, std::u16string_view useLocale) const;
    void  writeOUStringLiteralIntParameter(const char* pAsciiStr, const sal_Int16 count, sal_Int16 val) const;
    bool  writeOUStringLiteralDefaultParameter(const char* pAsciiStr, std::u16string_view str, sal_Int16 count) const;
    void  writeOUStringLiteralParameter(const char* pAsciiStr, std::u16string_view aChars) const;
    void  writeParameter(const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count) const;
    void  writeOUStringLiteralParameter(const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count) const;
    void  writeOUStringLiteralParameter(const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count0, sal_Int16 count1) const;
    void  writeOUStringLiteralParameter(const char* pTagStr, const char* pAsciiStr, std::u16string_view aChars, const sal_Int16 count) const;
    void  writeOUStringLiteralParameter(const char* pTagStr, const char* pAsciiStr, std::u16string_view aChars, sal_Int16 count0, sal_Int16 count1) const;
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
    OUString getValueByName (const char *str) const;
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
    LocaleNode (OUString name, const Reference< XAttributeList > & attr);
    void setValue(std::u16string_view oValue) { aValue += oValue; };
    const OUString& getName() const { return aName; };
    const OUString& getValue() const { return aValue; };
    const Attr& getAttr() const { return aAttribs; };
    sal_Int32 getNumberOfChildren () const { return sal_Int32(children.size()); };
    LocaleNode * getChildAt (sal_Int32 idx) const { return children[idx].get(); };
    const LocaleNode * findNode ( const char *name) const;
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
    OUString writeOUStringLiteralParameterCheckLen( const OFileWriter &of, const char* pParameterName, const LocaleNode* pNode, sal_Int32 nMinLen, sal_Int32 nMaxLen ) const;
    OUString writeOUStringLiteralParameterCheckLen( const OFileWriter &of, const char* pNodeName, const char* pParameterName, sal_Int32 nMinLen, sal_Int32 nMaxLen ) const;
    // ++nError with output to stderr
    void incError( const char* pStr ) const;
    // ++nError with output to stderr
    void incError( std::u16string_view rStr ) const;
    // ++nError with output to stderr, pStr should contain "%d"
    void incErrorInt( const char* pStr, int nVal ) const;
    // ++nError with output to stderr, pStr should contain "%s"
    void incErrorStr( const char* pStr, std::u16string_view rVal ) const;
    // ++nError with output to stderr, pStr should contain "%s %s"
    void incErrorStrStr( const char* pStr, std::u16string_view rVal1, std::u16string_view rVal2 )
        const;
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

    bool expectedCalendarElement( std::u16string_view rName,
            const LocaleNode* pNode, sal_Int16 nChild, std::u16string_view rCalendarID ) const;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

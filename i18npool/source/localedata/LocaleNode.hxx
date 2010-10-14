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
#ifndef _LOCALE_NODE_
#define _LOCALE_NODE_
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <vector>

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star::xml::sax;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

class OFileWriter
{
public:

OFileWriter(const char *pcFile, const char *locale );
virtual ~OFileWriter();
    virtual void  writeStringCharacters(const ::rtl::OUString& str) const;
    virtual void  writeAsciiString(const char *str)const ;
    virtual void  writeInt(sal_Int16 nb) const;
    virtual void  writeFunction(const char *func, const char *count, const char *array) const;
    virtual void  writeRefFunction(const char *func, const ::rtl::OUString& useLocale) const;
    virtual void  writeFunction(const char *func, const char *count, const char *array, const char *from, const char *to) const;
    virtual void  writeRefFunction(const char *func, const ::rtl::OUString& useLocale, const char *to) const;
    virtual void  writeFunction2(const char *func, const char *style, const char* attr, const char *array) const;
    virtual void  writeRefFunction2(const char *func, const ::rtl::OUString& useLocale) const;
    virtual void  writeFunction3(const char *func, const char *style, const char* levels, const char* attr, const char *array) const;
    virtual void  writeRefFunction3(const char *func, const ::rtl::OUString& useLocale) const;
    virtual void  writeIntParameter(const sal_Char* pAsciiStr, const sal_Int16 count, sal_Int16 val) const;
    virtual bool  writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str, sal_Int16 count) const;
    virtual bool  writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str) const;
    virtual void  writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const;
    virtual void  writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count) const;
    virtual void  writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const;
    virtual void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, const sal_Int16 count) const;
    virtual void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const;
    virtual void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const;
    virtual void  flush(void) const ;
    virtual void  closeOutput(void) const;
    /// Return the locale string, something like en_US or de_DE
    const char * getLocale() const { return theLocale; }
private:
    char m_pcFile[1024];
    char theLocale[50];
    FILE *m_f;
};

class Attr {
    Sequence <OUString> name;
    Sequence <OUString> value;

public:
    Attr (const Reference< XAttributeList > & attr);
    const OUString& getValueByName (const sal_Char *str) const;
    sal_Int32 getLength() const;
    const OUString& getTypeByIndex (sal_Int32 idx) const;
    const OUString& getValueByIndex (sal_Int32 idx) const ;
};

class LocaleNode
{
    OUString aName;
    OUString aValue;
    Attr * xAttribs;
    LocaleNode * parent;
    LocaleNode* * children;
    sal_Int32 nChildren;
    sal_Int32 childArrSize;

    void setParent ( LocaleNode*  node);

protected:
    mutable int nError;

public:
    LocaleNode (const OUString& name, const Reference< XAttributeList > & attr);
    inline void setValue(const OUString &oValue) { aValue += oValue; };
    inline const OUString& getName() const { return aName; };
    inline const OUString& getValue() const { return aValue; };
    inline const Attr* getAttr() const { return xAttribs; };
    inline sal_Int32 getNumberOfChildren () const { return nChildren; };
    inline  LocaleNode * getChildAt (sal_Int32 idx) const { return children[idx] ; };
    const LocaleNode * findNode ( const sal_Char *name) const;
    void print () const;
    void printR () const;
    virtual ~LocaleNode();
    void addChild (  LocaleNode * node);
    const LocaleNode* getParent() const { return parent; };
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
    void incError( const ::rtl::OUString& rStr ) const;
    // ++nError with output to stderr, pStr should contain "%d", otherwise appended
    void incErrorInt( const char* pStr, int nVal ) const;
    // ++nError with output to stderr, pStr should contain "%s", otherwise appended
    void incErrorStr( const char* pStr, const ::rtl::OUString& rVal ) const;
    // used by incError...(), returns a pointer to a static buffer,
    // pDefaultConversion is appended if pFormat doesn't contain a %
    // specification and should be something like ": %d" or ": %s" or similar.
    char* prepareErrorFormat( const char* pFormat, const char* pDefaultConversion ) const;
    static LocaleNode* createNode (const OUString& name,const Reference< XAttributeList > & attr);
};

class LCInfoNode : public LocaleNode {
public:
    inline LCInfoNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };
    virtual void generateCode (const OFileWriter &of) const;
};


class LCCTYPENode : public LocaleNode {
public:
    inline LCCTYPENode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCFormatNode : public LocaleNode {
    static sal_Int16 mnSection;
    static sal_Int16 mnFormats;
public:
    inline LCFormatNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCCollationNode : public LocaleNode {
public:
    inline LCCollationNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCIndexNode : public LocaleNode {
public:
    inline LCIndexNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCSearchNode : public LocaleNode {
public:
    inline LCSearchNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCCalendarNode : public LocaleNode {
public:
    inline LCCalendarNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCCurrencyNode : public LocaleNode {
public:
    inline LCCurrencyNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCTransliterationNode : public LocaleNode {
public:
    inline LCTransliterationNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCMiscNode : public LocaleNode {
public:
    inline LCMiscNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCNumberingLevelNode : public LocaleNode {
public:
    inline LCNumberingLevelNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

class LCOutlineNumberingLevelNode : public LocaleNode {
public:
    inline LCOutlineNumberingLevelNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

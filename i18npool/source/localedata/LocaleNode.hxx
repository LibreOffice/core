/*************************************************************************
 *
 *  $RCSfile: LocaleNode.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: khong $ $Date: 2002-07-11 17:24:36 $
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
#ifndef _LOCALE_NODE_
#define _LOCALE_NODE_
#include <tools/string.hxx>
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
~OFileWriter();
    virtual void  writeStringCharacters(const ::rtl::OUString& str) const;
    virtual void  writeAsciiString(const char *str)const ;
    virtual void  writeInt(sal_Int16 nb) const;
    virtual void  writeFunction(const char *func, const char *count, const char *array) const;
    virtual void  writeUseFunction(const char *func, const ::rtl::OUString& useLocale) const;
    virtual void  writeFunction(const char *func, const char *count, const char *array, const char *from, const char *to) const;
    virtual void  writeUseFunction(const char *func, const ::rtl::OUString& useLocale, const char *from, const char *to) const;
    virtual void  writeFunction2(const char *func, const char *style, const char* attr, const char *array) const;
    virtual void  writeUseFunction2(const char *func, const ::rtl::OUString& useLocale) const;
    virtual void  writeFunction3(const char *func, const char *style, const char* levels, const char* attr, const char *array) const;
    virtual void  writeUseFunction3(const char *func, const ::rtl::OUString& useLocale) const;
    virtual void  writeIntParameter(const sal_Char* pAsciiStr, const sal_Int16 count, sal_Int16 val) const;
    virtual void  writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str, sal_Int16 count) const;
    virtual void  writeDefaultParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& str) const;
    virtual void  writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const;
    virtual void  writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count) const;
    virtual void  writeParameter(const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const;
    virtual void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, const sal_Int16 count) const;
    virtual void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars) const;
    virtual void  writeParameter(const sal_Char* pTagStr, const sal_Char* pAsciiStr, const ::rtl::OUString& aChars, sal_Int16 count0, sal_Int16 count1) const;
    virtual void  flush(void) const ;
    virtual void  closeOutput(void) const;
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
    OUString getValueByName (const sal_Char *str) const;
    sal_Int32 getLength() const;
    OUString getTypeByIndex (sal_Int32 idx) const;
    OUString getValueByIndex (sal_Int32 idx) const ;
};

class LocaleNode{
    OUString aName;
    Attr * xAttribs;
    OUString aValue;
    LocaleNode * parent;
    LocaleNode* * children;
    void setParent ( LocaleNode*  node);
    sal_Int32 nChildren;
    sal_Int32 childArrSize;
    //inline LocaleNode() { ; }
public:
    LocaleNode (const OUString& name, const Reference< XAttributeList > & attr);
    inline void setValue(const OUString &oValue) { aValue = oValue; };
    inline const OUString getName() { return aName; };
    inline const OUString getValue() { return aValue; };
    inline const Attr* getAttr() { return xAttribs; };
    inline const sal_Int32 getNumberOfChildren () { return nChildren; };
    inline  LocaleNode * getChildAt (sal_Int32 idx) { return children[idx] ; };
    LocaleNode * findNode ( const sal_Char *name);
    void print () ;
    void printR () ;
    ~LocaleNode();
    void addChild (  LocaleNode * node);
    virtual void generateCode (const OFileWriter &of);
    static LocaleNode* createNode (const OUString& name,const Reference< XAttributeList > & attr);
};

class LCInfoNode : public LocaleNode {
public:
    inline LCInfoNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };
    virtual void generateCode (const OFileWriter &of);
};


class LCCTYPENode : public LocaleNode {
public:
    inline LCCTYPENode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCFormatNode : public LocaleNode {
public:
    inline LCFormatNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCCollationNode : public LocaleNode {
public:
    inline LCCollationNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCSearchNode : public LocaleNode {
public:
    inline LCSearchNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCCalendarNode : public LocaleNode {
public:
    inline LCCalendarNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCCurrencyNode : public LocaleNode {
public:
    inline LCCurrencyNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCTransliterationNode : public LocaleNode {
public:
    inline LCTransliterationNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCMiscNode : public LocaleNode {
public:
    inline LCMiscNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCNumberingLevelNode : public LocaleNode {
public:
    inline LCNumberingLevelNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

class LCOutlineNumberingLevelNode : public LocaleNode {
public:
    inline LCOutlineNumberingLevelNode (const OUString& name,
                const Reference< XAttributeList > & attr) : LocaleNode (name, attr) { ; };

    virtual void generateCode (const OFileWriter &of);
};

#endif

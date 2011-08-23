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

#ifndef INCLUDED_TAG_LOGGER_HXX
#define INCLUDED_TAG_LOGGER_HXX

#ifdef DEBUG

#include <rtl/ustring.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <string>
#include <vector>
#include <stack>
#include <hash_map>
#include <boost/shared_ptr.hpp>
#include <iostream>

namespace writerfilter
{
    using namespace::std;
    
    struct XMLAttribute
    {
        string mName;
        string mValue;
    public:
        XMLAttribute(string sName, string sValue) 
        : mName(sName), mValue(sValue) 
        {
        }        
    };
    
    class WRITERFILTER_DLLPUBLIC XMLTag
    {
    public:
        enum eMode { START, END, COMPLETE };
        typedef boost::shared_ptr<XMLTag> Pointer_t;
        static Pointer_t NIL;
        
    private:
        string mTag;
        string mChars;
        
        typedef vector<XMLAttribute> XMLAttributes_t;
        XMLAttributes_t mAttrs;
        typedef vector<XMLTag::Pointer_t> XMLTags_t;
        XMLTags_t mTags;
        eMode mMode;
        
    public:
        XMLTag(string sTag, eMode mode = COMPLETE) : mTag(sTag), mMode(mode) {}
        
        void addAttr(string name, string value);
        void addAttr(string name, const ::rtl::OUString & value);
        void addAttr(string name, sal_uInt32 nValue);
        void addAttr(string name, uno::Any rAny);
        void addTag(Pointer_t pTag);
        void chars(const string & rChars);
        void chars(const ::rtl::OUString & rChars);
        const string & getTag() const;
        string toString() const; 
        
        ostream & output(ostream & o) const;
    };

    class WRITERFILTER_DLLPUBLIC TagLogger
    {
    public:
        typedef boost::shared_ptr<TagLogger> Pointer_t;
        
    private:
        stack<XMLTag::Pointer_t> mTags;
        XMLTag::Pointer_t currentTag() const;
        XMLTag::Pointer_t mpRoot;
        string mFileName;

        TagLogger();
        
    public:
        ~TagLogger();
        
        static Pointer_t getInstance(const char * name);
        
        void setFileName(const string & rName);
        
        void startDocument();
        void element(const string & name);
        void startElement(const string & name);
        void attribute(const string & name, const string & value);
        void attribute(const string & name, const ::rtl::OUString & value);
        void attribute(const string & name, sal_uInt32 value);
        void attribute(const string & name, const uno::Any aAny);
        void addTag(XMLTag::Pointer_t pTag);
        void chars(const string & chars);
        void chars(const ::rtl::OUString & chars);
        void endElement(const string & name);
        void endDocument();
        
        ostream & output(ostream & o) const;
        static void dump(const char * name);
    };
    
    class IdToString
    {
    public:
        typedef boost::shared_ptr<IdToString> Pointer_t;
        virtual string toString(const Id & id) const = 0;
    };
    
    class WRITERFILTER_DLLPUBLIC PropertySetToTagHandler : public Properties
    {
        XMLTag::Pointer_t mpTag;
        IdToString::Pointer_t mpIdToString;
        
    public:
        PropertySetToTagHandler(IdToString::Pointer_t pIdToString) 
        : mpTag(new XMLTag("propertyset")), mpIdToString(pIdToString) {}
        virtual ~PropertySetToTagHandler();
        
        XMLTag::Pointer_t getTag() const { return mpTag; }
        
        void resolve(XMLTag & rTag,
                     writerfilter::Reference<Properties>::Pointer_t props);
        
        virtual void attribute(Id name, Value & val);
        virtual void sprm(Sprm & sprm);
    };

WRITERFILTER_DLLPUBLIC XMLTag::Pointer_t unoPropertySetToTag(uno::Reference<beans::XPropertySet> rPropSet);
}

#endif // DEBUG
#endif // INCLUDED_TAG_LOGGER_HXX

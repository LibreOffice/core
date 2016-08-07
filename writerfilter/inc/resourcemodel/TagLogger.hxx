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



#ifndef INCLUDED_TAG_LOGGER_HXX
#define INCLUDED_TAG_LOGGER_HXX

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
        string toTree(const string & sIndent = "") const;

        ostream & output(ostream & o, const string & sIndent = "") const;
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
        PropertySetToTagHandler(IdToString::Pointer_t pIdToString);
        virtual ~PropertySetToTagHandler();

        XMLTag::Pointer_t getTag() const { return mpTag; }

        void resolve(XMLTag & rTag,
                     writerfilter::Reference<Properties>::Pointer_t props);

        virtual void attribute(Id name, Value & val);
        virtual void sprm(Sprm & sprm);
    };

WRITERFILTER_DLLPUBLIC XMLTag::Pointer_t unoPropertySetToTag(uno::Reference<beans::XPropertySet> rPropSet);
}

#endif // INCLUDED_TAG_LOGGER_HXX

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



#ifndef INCLUDED_LOGGED_RESOURCES_HXX
#define INCLUDED_LOGGED_RESOURCES_HXX

#include "WW8ResourceModel.hxx"
#include "TagLogger.hxx"

namespace writerfilter
{

class LoggedResourcesHelper
{
public:
    explicit LoggedResourcesHelper(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedResourcesHelper();

    void startElement(const string & sElement);
    void endElement(const string & sElement);
    void chars(const ::rtl::OUString & rChars);
    void chars(const string & rChars);
    void attribute(const string & rName, const string & rValue);
    void attribute(const string & rName, sal_uInt32 nValue);

    void setPrefix(const string & rPrefix);

private:
    TagLogger::Pointer_t mpLogger;
    string msPrefix;
};

class LoggedStream : public Stream
{
public:
    explicit LoggedStream(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedStream();

    void startSectionGroup();
    void endSectionGroup();
    void startParagraphGroup();
    void endParagraphGroup();
    void startCharacterGroup();
    void endCharacterGroup();
    void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    void endShape();
    void text(const sal_uInt8 * data, size_t len);
    void utext(const sal_uInt8 * data, size_t len);
    void props(writerfilter::Reference<Properties>::Pointer_t ref);
    void table(Id name, writerfilter::Reference<Table>::Pointer_t ref);
    void substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref);
    void info(const string & info);

protected:
    virtual void lcl_startSectionGroup() = 0;
    virtual void lcl_endSectionGroup() = 0;
    virtual void lcl_startParagraphGroup() = 0;
    virtual void lcl_endParagraphGroup() = 0;
    virtual void lcl_startCharacterGroup() = 0;
    virtual void lcl_endCharacterGroup() = 0;
    virtual void lcl_startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape )  = 0;
    virtual void lcl_endShape() = 0;
    virtual void lcl_text(const sal_uInt8 * data, size_t len) = 0;
    virtual void lcl_utext(const sal_uInt8 * data, size_t len) = 0;
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref) = 0;
    virtual void lcl_table(Id name, writerfilter::Reference<Table>::Pointer_t ref) = 0;
    virtual void lcl_substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref) = 0;
    virtual void lcl_info(const string & info) = 0;

    LoggedResourcesHelper mHelper;
};

class LoggedProperties : public Properties
{
public:
    explicit LoggedProperties(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedProperties();

    void attribute(Id name, Value & val);
    void sprm(Sprm & sprm);

protected:
    virtual void lcl_attribute(Id name, Value & val) = 0;
    virtual void lcl_sprm(Sprm & sprm) = 0;

    LoggedResourcesHelper mHelper;
};

class LoggedTable : public Table
{
public:
    explicit LoggedTable(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedTable();

    void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

protected:
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) = 0;

    LoggedResourcesHelper mHelper;
};

}
#endif // INCLUDED_LOGGED_RESOURCES_HXX

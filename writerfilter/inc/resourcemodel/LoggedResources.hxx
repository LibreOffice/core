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

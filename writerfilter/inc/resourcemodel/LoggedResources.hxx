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

#ifndef INCLUDED_LOGGED_RESOURCES_HXX
#define INCLUDED_LOGGED_RESOURCES_HXX

#include "WW8ResourceModel.hxx"
#include "TagLogger.hxx"

#include <WriterFilterDllApi.hxx>

namespace writerfilter
{

#ifdef DEBUG_LOGGING
class LoggedResourcesHelper
{
public:
    explicit LoggedResourcesHelper(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedResourcesHelper();

    void startElement(const string & sElement);
    void endElement(const string & sElement);
    void chars(const OUString & rChars);
    void chars(const string & rChars);
    void attribute(const string & rName, const string & rValue);
    void attribute(const string & rName, sal_uInt32 nValue);

private:
    TagLogger::Pointer_t mpLogger;
    string msPrefix;
};
#endif

class LoggedStream : public Stream
{
public:
    explicit LoggedStream(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedStream();

    void startSectionGroup() SAL_OVERRIDE;
    void endSectionGroup() SAL_OVERRIDE;
    void startParagraphGroup() SAL_OVERRIDE;
    void endParagraphGroup() SAL_OVERRIDE;
    void startCharacterGroup() SAL_OVERRIDE;
    void endCharacterGroup() SAL_OVERRIDE;
    void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape ) SAL_OVERRIDE;
    void endShape() SAL_OVERRIDE;
    void text(const sal_uInt8 * data, size_t len) SAL_OVERRIDE;
    void utext(const sal_uInt8 * data, size_t len) SAL_OVERRIDE;
    void positivePercentage(const OUString& rText) SAL_OVERRIDE;
    void props(writerfilter::Reference<Properties>::Pointer_t ref) SAL_OVERRIDE;
    void table(Id name, writerfilter::Reference<Table>::Pointer_t ref) SAL_OVERRIDE;
    void substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref) SAL_OVERRIDE;
    void info(const string & info) SAL_OVERRIDE;

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
    virtual void lcl_positivePercentage(const OUString& /*rText*/) { }
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref) = 0;
    virtual void lcl_table(Id name, writerfilter::Reference<Table>::Pointer_t ref) = 0;
    virtual void lcl_substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref) = 0;
    virtual void lcl_info(const string & info) = 0;

#ifdef DEBUG_LOGGING
    LoggedResourcesHelper mHelper;
#endif
};

class LoggedProperties : public Properties
{
public:
    explicit LoggedProperties(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedProperties();

    void attribute(Id name, Value & val) SAL_OVERRIDE;
    void sprm(Sprm & sprm) SAL_OVERRIDE;

protected:
    virtual void lcl_attribute(Id name, Value & val) = 0;
    virtual void lcl_sprm(Sprm & sprm) = 0;

#ifdef DEBUG_LOGGING
    LoggedResourcesHelper mHelper;
#endif
};

class LoggedTable : public Table
{
public:
    explicit LoggedTable(TagLogger::Pointer_t pLogger, const string & sPrefix);
    virtual ~LoggedTable();

    void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) SAL_OVERRIDE;

protected:
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) = 0;

#ifdef DEBUG_LOGGING
    LoggedResourcesHelper mHelper;
#endif
};

}
#endif // INCLUDED_LOGGED_RESOURCES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

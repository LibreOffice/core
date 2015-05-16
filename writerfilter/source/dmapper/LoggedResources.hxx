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

#ifndef INCLUDED_WRITERFILTER_SOURCE_DMAPPER_LOGGEDRESOURCES_HXX
#define INCLUDED_WRITERFILTER_SOURCE_DMAPPER_LOGGEDRESOURCES_HXX

#include <dmapper/resourcemodel.hxx>
#include "TagLogger.hxx"


namespace writerfilter
{

#ifdef DEBUG_WRITERFILTER
class LoggedResourcesHelper
{
public:
    explicit LoggedResourcesHelper(const std::string & sPrefix);
    virtual ~LoggedResourcesHelper();

    void startElement(const std::string & sElement);
    void endElement(const std::string & sElement);
    void chars(const OUString & rChars);
    void chars(const std::string & rChars);
    void attribute(const std::string & rName, const std::string & rValue);
    void attribute(const std::string & rName, sal_uInt32 nValue);

private:
    std::string msPrefix;
};
#endif

class LoggedStream : public Stream
{
public:
    explicit LoggedStream(const std::string & sPrefix);
    virtual ~LoggedStream();

    void startSectionGroup() SAL_OVERRIDE;
    void endSectionGroup() SAL_OVERRIDE;
    void startParagraphGroup() SAL_OVERRIDE;
    void endParagraphGroup() SAL_OVERRIDE;
    void startCharacterGroup() SAL_OVERRIDE;
    void endCharacterGroup() SAL_OVERRIDE;
    void startShape(css::uno::Reference<css::drawing::XShape> const& xShape) SAL_OVERRIDE;
    void endShape() SAL_OVERRIDE;
    void text(const sal_uInt8 * data, size_t len) SAL_OVERRIDE;
    void utext(const sal_uInt8 * data, size_t len) SAL_OVERRIDE;
    void positionOffset(const OUString& rText, bool bVertical) SAL_OVERRIDE;
    void align(const OUString& rText, bool bVertical) SAL_OVERRIDE;
    void positivePercentage(const OUString& rText) SAL_OVERRIDE;
    void props(writerfilter::Reference<Properties>::Pointer_t ref) SAL_OVERRIDE;
    void table(Id name, writerfilter::Reference<Table>::Pointer_t ref) SAL_OVERRIDE;
    void substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref) SAL_OVERRIDE;
    void info(const std::string & info) SAL_OVERRIDE;

protected:
    virtual void lcl_startSectionGroup() = 0;
    virtual void lcl_endSectionGroup() = 0;
    virtual void lcl_startParagraphGroup() = 0;
    virtual void lcl_endParagraphGroup() = 0;
    virtual void lcl_startCharacterGroup() = 0;
    virtual void lcl_endCharacterGroup() = 0;
    virtual void lcl_startShape(css::uno::Reference<css::drawing::XShape> const& xShape) = 0;
    virtual void lcl_endShape() = 0;
    virtual void lcl_text(const sal_uInt8 * data, size_t len) = 0;
    virtual void lcl_utext(const sal_uInt8 * data, size_t len) = 0;
    virtual void lcl_positionOffset(const OUString& /*rText*/, bool /*bVertical*/) { }
    virtual css::awt::Point getPositionOffset() SAL_OVERRIDE { return css::awt::Point(); }
    virtual void lcl_align(const OUString& /*rText*/, bool /*bVertical*/) { }
    virtual void lcl_positivePercentage(const OUString& /*rText*/) { }
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref) = 0;
    virtual void lcl_table(Id name, writerfilter::Reference<Table>::Pointer_t ref) = 0;
    virtual void lcl_substream(Id name, writerfilter::Reference<Stream>::Pointer_t ref) = 0;
    virtual void lcl_info(const std::string & info) = 0;

#ifdef DEBUG_WRITERFILTER
    LoggedResourcesHelper mHelper;
#endif
};

class LoggedProperties : public Properties
{
public:
    explicit LoggedProperties(const std::string & sPrefix);
    virtual ~LoggedProperties();

    void attribute(Id name, Value & val) SAL_OVERRIDE;
    void sprm(Sprm & sprm) SAL_OVERRIDE;

protected:
    virtual void lcl_attribute(Id name, Value & val) = 0;
    virtual void lcl_sprm(Sprm & sprm) = 0;

#ifdef DEBUG_WRITERFILTER
    LoggedResourcesHelper mHelper;
#endif
};

class LoggedTable : public Table
{
public:
    explicit LoggedTable(const std::string & sPrefix);
    virtual ~LoggedTable();

    void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) SAL_OVERRIDE;

protected:
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref) = 0;

#ifdef DEBUG_WRITERFILTER
    LoggedResourcesHelper mHelper;
#endif
};

}
#endif // INCLUDED_WRITERFILTER_SOURCE_DMAPPER_LOGGEDRESOURCES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

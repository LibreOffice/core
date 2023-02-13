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

#include <rtl/ustring.hxx>
#include <tools/ref.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <string>
#include <string_view>
#include <type_traits>
#include <libxml/xmlwriter.h>

namespace writerfilter
{

    class TagLogger
    {
    private:
        xmlTextWriterPtr m_pWriter;
        const char* m_pName;

    public:
        explicit TagLogger();
        ~TagLogger();

        static TagLogger& getInstance();

#ifdef DBG_UTIL
        void setFileName(const std::string & filename);
        void startDocument();
        void endDocument();

        void element(const std::string & name);
        void unoPropertySet(const css::uno::Reference<css::beans::XPropertySet>& rPropSet);
        void startElement(const std::string & name);
#endif
        void attribute(const std::string & name, const std::string & value);
#ifdef DBG_UTIL
        void attribute(const std::string & name, std::u16string_view value);
        void attribute(const std::string & name, sal_uInt32 value);
        template<typename T> std::enable_if_t<std::is_integral_v<T>, void>
        attribute(const std::string & name, T value)
        { return attribute(name, static_cast<sal_uInt32>(value)); }
        void attribute(const std::string & name, float value);
        void attribute(const std::string & name, const css::uno::Any& aAny);
        void chars(const std::string & chars);
        void chars(std::u16string_view chars);
        void endElement();
#endif
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

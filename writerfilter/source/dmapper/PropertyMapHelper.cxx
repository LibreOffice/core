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

#include <com/sun/star/text/TableColumnSeparator.hpp>
#include "TagLogger.hxx"
#include "PropertyMapHelper.hxx"

namespace writerfilter::dmapper
{
using namespace ::com::sun::star;

void lcl_DumpTableColumnSeparators(const uno::Any& rTableColumnSeparators)
{
#ifdef DBG_UTIL
    uno::Sequence<text::TableColumnSeparator> aSeq;
    rTableColumnSeparators >>= aSeq;

    TagLogger::getInstance().startElement("property.TableColumnSeparators");

    sal_uInt32 nLength = aSeq.getLength();
    for (sal_uInt32 n = 0; n < nLength; ++n)
    {
        TagLogger::getInstance().startElement("separator");

        TagLogger::getInstance().attribute("position", aSeq[n].Position);
        TagLogger::getInstance().attribute("visible", sal_uInt32(aSeq[n].IsVisible));

        TagLogger::getInstance().endElement();
    }

    TagLogger::getInstance().endElement();
#else
    (void)rTableColumnSeparators;
#endif // DBG_UTIL
}

#ifdef DBG_UTIL
void lcl_DumpPropertyValues(beans::PropertyValues const& rValues)
{
    TagLogger::getInstance().startElement("propertyValues");

    for (beans::PropertyValue const& propVal : rValues)
    {
        TagLogger::getInstance().startElement("propertyValue");

        TagLogger::getInstance().attribute("name", propVal.Name);

        try
        {
            sal_Int32 aInt = 0;
            propVal.Value >>= aInt;
            TagLogger::getInstance().attribute("value", aInt);
        }
        catch (...)
        {
        }

        if (propVal.Name == "TableColumnSeparators")
        {
            lcl_DumpTableColumnSeparators(propVal.Value);
        }

        TagLogger::getInstance().endElement();
    }
    TagLogger::getInstance().endElement();
}

void lcl_DumpPropertyValueSeq(css::uno::Sequence<css::beans::PropertyValues> const& rPropValSeq)
{
    TagLogger::getInstance().startElement("PropertyValueSeq");

    for (auto const& propVal : rPropValSeq)
    {
        lcl_DumpPropertyValues(propVal);
    }

    TagLogger::getInstance().endElement();
}
#endif // DBG_UTIL
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

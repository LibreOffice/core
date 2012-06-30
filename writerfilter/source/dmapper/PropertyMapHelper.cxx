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
#include <resourcemodel/TagLogger.hxx>
#include "PropertyMapHelper.hxx"

#if OSL_DEBUG_LEVEL > 1
namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

void lcl_DumpTableColumnSeparators(const TagLogger::Pointer_t pLogger, const uno::Any & rTableColumnSeparators)
{
    uno::Sequence<text::TableColumnSeparator> aSeq;
    rTableColumnSeparators >>= aSeq;

    pLogger->startElement("property.TableColumnSeparators");

    sal_uInt32 nLength = aSeq.getLength();
    for (sal_uInt32 n = 0; n < nLength; ++n)
    {
        pLogger->startElement("separator");

        pLogger->attribute("position", aSeq[n].Position);
        pLogger->attribute("visible", aSeq[n].IsVisible);

        pLogger->endElement();
    }

    pLogger->endElement();
}

void lcl_DumpPropertyValues(const TagLogger::Pointer_t pLogger, beans::PropertyValues & rValues)
{
    pLogger->startElement("propertyValues");

    beans::PropertyValue * pValues = rValues.getArray();

    for (sal_Int32 n = 0; n < rValues.getLength(); ++n)
    {
        pLogger->startElement("propertyValue");

        pLogger->attribute("name", pValues[n].Name);

        try
        {
            sal_Int32 aInt = 0;
            pValues[n].Value >>= aInt;
            pLogger->attribute("value", aInt);
        }
        catch (...)
        {
        }

        if ( pValues[n].Name == "TableColumnSeparators" )
        {
            lcl_DumpTableColumnSeparators(pLogger, pValues[n].Value);
        }

        pLogger->endElement();
    }
    pLogger->endElement();
}

void lcl_DumpPropertyValueSeq(const TagLogger::Pointer_t pLogger, PropertyValueSeq_t & rPropValSeq)
{
    pLogger->startElement("PropertyValueSeq");

    beans::PropertyValues * pValues = rPropValSeq.getArray();

    for (sal_Int32 n = 0; n < rPropValSeq.getLength(); ++n)
    {
        lcl_DumpPropertyValues(pLogger, pValues[n]);
    }

    pLogger->endElement();
}

void lcl_DumpPropertyValueSeqSeq(const TagLogger::Pointer_t pLogger, PropertyValueSeqSeq_t rPropValSeqSeq)
{
    pLogger->startElement("PropertyValueSeq");

    PropertyValueSeq_t * pValues = rPropValSeqSeq.getArray();

    for (sal_Int32 n = 0; n < rPropValSeqSeq.getLength(); ++n)
    {
        lcl_DumpPropertyValueSeq(pLogger, pValues[n]);
    }

    pLogger->endElement();
}

}
}
#endif // OSL_DEBUG_LEVEL > 1

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

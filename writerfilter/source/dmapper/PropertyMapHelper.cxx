/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PropertyMap.hxx,v $
 * $Revision: 1.18 $
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

        if (pValues[n].Name.equalsAscii("TableColumnSeparators"))
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
#endif // DEBUG

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

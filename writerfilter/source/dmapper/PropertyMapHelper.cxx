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

namespace writerfilter
{
namespace dmapper
{

using namespace ::com::sun::star;

XMLTag::Pointer_t lcl_TableColumnSeparatorsToTag(PropertyMap::iterator aIter)
{
    uno::Sequence<text::TableColumnSeparator> aSeq;
    aIter->second >>= aSeq;

    XMLTag::Pointer_t pResult(new XMLTag("property.TableColumnSeparators"));

    sal_uInt32 nLength = aSeq.getLength();
    for (sal_uInt32 n = 0; n < nLength; ++n)
    {
        XMLTag::Pointer_t pTagSeparator(new XMLTag("separator"));

        pTagSeparator->addAttr("position", aSeq[n].Position);
        pTagSeparator->addAttr("visible", aSeq[n].IsVisible);

        pResult->addTag(pTagSeparator);
    }

    return pResult;
}

}
}
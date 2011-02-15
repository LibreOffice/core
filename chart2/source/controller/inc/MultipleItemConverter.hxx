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
#ifndef CHART_ALLAXISITEMCONVERTER_HXX
#define CHART_ALLAXISITEMCONVERTER_HXX

#include <com/sun/star/frame/XModel.hpp>

#include "ItemConverter.hxx"

#include <vector>

namespace comphelper
{

/** Note: virtual const sal_uInt16 * GetWhichPairs() const; is still pure virtual
 */
class MultipleItemConverter : public ItemConverter
{
public:
    virtual ~MultipleItemConverter();

    virtual void FillItemSet( SfxItemSet & rOutItemSet ) const;
    virtual bool ApplyItemSet( const SfxItemSet & rItemSet );

    /// implemented empty (returns always false)
    virtual bool GetItemProperty( tWhichIdType nWhichId, tPropertyNameWithMemberId & rOutProperty ) const;

protected:
    MultipleItemConverter( SfxItemPool& rItemPool );

    ::std::vector< ItemConverter * >            m_aConverters;
};

} //  namespace comphelper

#endif

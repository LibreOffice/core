/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "WrappedSeriesAreaOrLineProperty.hxx"
#include "DataSeriesPointWrapper.hxx"
#include "macros.hxx"

using ::com::sun::star::uno::Any;
using ::rtl::OUString;


//.............................................................................
namespace chart
{
namespace wrapper
{

WrappedSeriesAreaOrLineProperty::WrappedSeriesAreaOrLineProperty(
      const rtl::OUString& rOuterName
    , const rtl::OUString& rInnerAreaTypeName
    , const rtl::OUString& rInnerLineTypeName
    , DataSeriesPointWrapper* pDataSeriesPointWrapper )
    : WrappedProperty( rOuterName, C2U( "" ) )
    , m_pDataSeriesPointWrapper( pDataSeriesPointWrapper )
    , m_aInnerAreaTypeName( rInnerAreaTypeName )
    , m_aInnerLineTypeName( rInnerLineTypeName )
{
}
WrappedSeriesAreaOrLineProperty::~WrappedSeriesAreaOrLineProperty()
{
}

//virtual
::rtl::OUString WrappedSeriesAreaOrLineProperty::getInnerName() const
{
    if( m_pDataSeriesPointWrapper && !m_pDataSeriesPointWrapper->isSupportingAreaProperties() )
        return m_aInnerLineTypeName;
    return m_aInnerAreaTypeName;
}

} //namespace wrapper
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_RESOURCEMODEL_UTIL_HXX
#define INCLUDED_RESOURCEMODEL_UTIL_HXX
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <WriterFilterDllApi.hxx>
#include <string>

namespace writerfilter
{
    using namespace ::std;
    using namespace ::com::sun::star;
    
    string WRITERFILTER_DLLPUBLIC xmlify(const string & str);
    
#ifdef DEBUG
    string WRITERFILTER_DLLPUBLIC propertysetToString
    (uno::Reference<beans::XPropertySet> const & rProps);
    
    string toString(uno::Reference< text::XTextRange > textRange);
    string toString(const string & rString);
#endif
}
#endif // INCLUDED_RESOURCEMODEL_UTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

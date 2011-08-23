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

#ifndef INCLUDED_THEMETABLE_HXX
#define INCLUDED_THEMETABLE_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <map>

namespace writerfilter {
namespace dmapper
{

struct ThemeTable_Impl;

class WRITERFILTER_DLLPRIVATE ThemeTable : public Properties, public Table
{
    ThemeTable_Impl *m_pImpl;

public:
    ThemeTable();
    virtual ~ThemeTable();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);
    
    const ::rtl::OUString getFontNameForTheme(const Id id) const;
};
typedef boost::shared_ptr< ThemeTable >          ThemeTablePtr;
}}

#endif //

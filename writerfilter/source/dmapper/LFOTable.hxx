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
#ifndef INCLUDED_LFOTABLE_HXX
#define INCLUDED_LFOTABLE_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>
//#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
//#include <PropertyMap.hxx>
//#endif
//#include <vector>

namespace writerfilter {
namespace dmapper
{

struct LFOTable_Impl;
class WRITERFILTER_DLLPRIVATE LFOTable : public Properties, public Table
{
    LFOTable_Impl   *m_pImpl;
public:
    LFOTable();
    virtual ~LFOTable();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

//    sal_uInt32          size();
    sal_Int32       GetListID(sal_uInt32 nLFO);
    //direct access in ooxml import
    void            AddListID( sal_Int32 nAbstractNumId );
};
typedef boost::shared_ptr< LFOTable >          LFOTablePtr;
}}

#endif //

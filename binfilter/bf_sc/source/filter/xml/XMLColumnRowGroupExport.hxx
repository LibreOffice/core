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

#ifndef _SC_XMLCOLUMNROWGROUPEXPORT_HXX
#define _SC_XMLCOLUMNROWGROUPEXPORT_HXX

#ifndef __SGI_STL_LIST
#include <list>
#endif
#include <rtl/ustring.hxx>

#include <sal/types.h>
namespace binfilter {

struct ScMyColumnRowGroup
{
    sal_Int32	nField;
    sal_Int16	nLevel;
    sal_Bool	bDisplay : 1;

    ScMyColumnRowGroup();
    sal_Bool operator< (const ScMyColumnRowGroup& rGroup) const;
};

typedef std::list <ScMyColumnRowGroup> ScMyColumnRowGroupVec;
typedef std::list <sal_Int32> ScMyFieldGroupVec;

class ScXMLExport;
class ScMyOpenCloseColumnRowGroup
{
    ScXMLExport&				rExport;
    const ::rtl::OUString			rName;
    ScMyColumnRowGroupVec		aTableStart;
    ScMyFieldGroupVec			aTableEnd;

    void OpenGroup(const ScMyColumnRowGroup& rGroup);
    void CloseGroup();
public:
    ScMyOpenCloseColumnRowGroup(ScXMLExport& rExport, sal_uInt32 nToken);
    ~ScMyOpenCloseColumnRowGroup();

    void NewTable();
    void AddGroup(const ScMyColumnRowGroup& aGroup, const sal_Int32 nEndField);
    sal_Bool IsGroupStart(const sal_Int32 nField);
    void OpenGroups(const sal_Int32 nField);
    sal_Bool IsGroupEnd(const sal_Int32 nField);
    void CloseGroups(const sal_Int32 nField);
    sal_Int32 GetLast();
    void Sort();
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

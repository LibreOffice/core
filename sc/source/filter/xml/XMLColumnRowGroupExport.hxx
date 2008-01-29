/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLColumnRowGroupExport.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:33:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SC_XMLCOLUMNROWGROUPEXPORT_HXX
#define _SC_XMLCOLUMNROWGROUPEXPORT_HXX

#include <list>
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

struct ScMyColumnRowGroup
{
    sal_Int32   nField;
    sal_Int16   nLevel;
    sal_Bool    bDisplay;

    ScMyColumnRowGroup();
    sal_Bool operator< (const ScMyColumnRowGroup& rGroup) const;
};

typedef std::list <ScMyColumnRowGroup> ScMyColumnRowGroupVec;
typedef std::list <sal_Int32> ScMyFieldGroupVec;

class ScXMLExport;
class ScMyOpenCloseColumnRowGroup
{
    ScXMLExport&                rExport;
    const rtl::OUString         rName;
    ScMyColumnRowGroupVec       aTableStart;
    ScMyFieldGroupVec           aTableEnd;

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

#endif


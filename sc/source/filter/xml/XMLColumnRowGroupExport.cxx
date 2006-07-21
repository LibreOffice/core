/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLColumnRowGroupExport.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:41:35 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#ifndef _SC_XMLCOLUMNROWGROUPEXPORT_HXX
#include "XMLColumnRowGroupExport.hxx"
#endif
#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif

#include <algorithm>

using namespace xmloff::token;

ScMyColumnRowGroup::ScMyColumnRowGroup()
{
}

sal_Bool ScMyColumnRowGroup::operator<(const ScMyColumnRowGroup& rGroup) const
{
    if (rGroup.nField > nField)
        return sal_True;
    else
        if (rGroup.nField == nField && rGroup.nLevel > nLevel)
            return sal_True;
        else
            return sal_False;
}

ScMyOpenCloseColumnRowGroup::ScMyOpenCloseColumnRowGroup(ScXMLExport& rTempExport, sal_uInt32 nToken)
    : rExport(rTempExport),
    aTableStart(),
    aTableEnd(),
    rName(rExport.GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TABLE, GetXMLToken(XMLTokenEnum(nToken))))
{
}

ScMyOpenCloseColumnRowGroup::~ScMyOpenCloseColumnRowGroup()
{
}

void ScMyOpenCloseColumnRowGroup::NewTable()
{
    aTableStart.clear();
    aTableEnd.clear();
}

void ScMyOpenCloseColumnRowGroup::AddGroup(const ScMyColumnRowGroup& aGroup, const sal_Int32 nEndField)
{
    aTableStart.push_back(aGroup);
    aTableEnd.push_back(nEndField);
}

sal_Bool ScMyOpenCloseColumnRowGroup::IsGroupStart(const sal_Int32 nField)
{
    sal_Bool bGroupStart(sal_False);
    if (!aTableStart.empty())
    {
        ScMyColumnRowGroupVec::iterator aItr(aTableStart.begin());
        sal_Int32 nItrField = aItr->nField;
        if ( nItrField < nField )
        {
            //  #103327# when used to find repeated rows at the beginning of a group,
            //  aTableStart may contain entries before nField. They must be skipped here
            //  (they will be used for OpenGroups later in the right order).

            ScMyColumnRowGroupVec::iterator aEnd(aTableStart.end());
            while ( aItr != aEnd && nItrField < nField )
            {
                ++aItr;
                if ( aItr != aEnd )
                    nItrField = aItr->nField;
            }
        }

        if (nItrField == nField)
            bGroupStart = sal_True;
    }
    return bGroupStart;
}

void ScMyOpenCloseColumnRowGroup::OpenGroup(const ScMyColumnRowGroup& rGroup)
{
    if (!rGroup.bDisplay)
        rExport.AddAttribute(XML_NAMESPACE_TABLE, XML_DISPLAY, XML_FALSE);
    rExport.StartElement( rName, sal_True);
}

void ScMyOpenCloseColumnRowGroup::OpenGroups(const sal_Int32 nField)
{
    ScMyColumnRowGroupVec::iterator aItr(aTableStart.begin());
    ScMyColumnRowGroupVec::iterator aEndItr(aTableStart.end());
    sal_Bool bReady(sal_False);
    while(!bReady && aItr != aEndItr)
    {
        if (aItr->nField == nField)
        {
            OpenGroup(*aItr);
            aItr = aTableStart.erase(aItr);
        }
        else
            bReady = sal_True;
    }
}

sal_Bool ScMyOpenCloseColumnRowGroup::IsGroupEnd(const sal_Int32 nField)
{
    sal_Bool bGroupEnd(sal_False);
    if (!aTableEnd.empty())
    {
        if (*(aTableEnd.begin()) == nField)
            bGroupEnd = sal_True;
    }
    return bGroupEnd;
}

void ScMyOpenCloseColumnRowGroup::CloseGroup()
{
    rExport.EndElement( rName, sal_True );
}

void ScMyOpenCloseColumnRowGroup::CloseGroups(const sal_Int32 nField)
{
    ScMyFieldGroupVec::iterator aItr(aTableEnd.begin());
    ScMyFieldGroupVec::iterator aEndItr(aTableEnd.end());
    sal_Bool bReady(sal_False);
    while(!bReady && aItr != aEndItr)
    {
        if (*aItr == nField)
        {
            CloseGroup();
            aItr = aTableEnd.erase(aItr);
        }
        else
            bReady = sal_True;
    }
}

sal_Int32 ScMyOpenCloseColumnRowGroup::GetLast()
{
    sal_Int32 maximum(-1);
    ScMyFieldGroupVec::iterator i(aTableEnd.begin());
    ScMyFieldGroupVec::iterator endi(aTableEnd.end());
    while (i != endi)
    {
        if (*i > maximum)
            maximum = *i;
        ++i;
    }
    return maximum;
}

void ScMyOpenCloseColumnRowGroup::Sort()
{
    aTableStart.sort();
    aTableEnd.sort();
}


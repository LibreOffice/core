/*************************************************************************
 *
 *  $RCSfile: PageMasterImportPropMapper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2000-10-23 15:30:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#ifndef _XMLOFF_PAGEMASTERIMPORTPROPMAPPER_HXX
#include "PageMasterImportPropMapper.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERPROPMAPPER_HXX
#include "PageMasterPropMapper.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include <xmloff/maptype.hxx>
#endif

using namespace ::com::sun::star;

PageMasterImportPropertyMapper::PageMasterImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper ) :
    SvXMLImportPropertyMapper( rMapper )
{
}

PageMasterImportPropertyMapper::~PageMasterImportPropertyMapper()
{
}

/*sal_Bool PageMasterImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

/*sal_Bool PageMasterImportPropertyMapper::handleNoItem(
        sal_Int32 nIndex,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

void PageMasterImportPropertyMapper::finished(::std::vector< XMLPropertyState >& rProperties ) const
{
    SvXMLImportPropertyMapper::finished(rProperties);
    XMLPropertyState* pAllPaddingProperty = NULL;
    XMLPropertyState* pAllBorderProperty = NULL;
    XMLPropertyState* pAllBorderWidthProperty = NULL;
    ::std::vector< XMLPropertyState >::iterator i = rProperties.begin();
    for (i; i != rProperties.end(); i++)
    {
        sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(i->mnIndex);
        switch (nContextID)
        {
            case CTF_PM_PADDINGALL :
            {
                pAllPaddingProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_PM_BORDERALL :
            {
                pAllBorderProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_PM_BORDERWIDTHALL :
            {
                pAllBorderWidthProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
        }
    }
    if (pAllPaddingProperty)
    {
        sal_Int32 nIndex = pAllPaddingProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllPaddingProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllBorderProperty)
    {
        sal_Int32 nIndex = pAllBorderProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllBorderProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllBorderWidthProperty)
    {
        sal_Int32 nIndex = pAllBorderWidthProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllBorderWidthProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
}

PageMasterHeaderImportPropertyMapper::PageMasterHeaderImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper ) :
    SvXMLImportPropertyMapper( rMapper )
{
}

PageMasterHeaderImportPropertyMapper::~PageMasterHeaderImportPropertyMapper()
{
}

/*sal_Bool PageMasterImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

/*sal_Bool PageMasterImportPropertyMapper::handleNoItem(
        sal_Int32 nIndex,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

void PageMasterHeaderImportPropertyMapper::finished(::std::vector< XMLPropertyState >& rProperties ) const
{
    SvXMLImportPropertyMapper::finished(rProperties);
    XMLPropertyState* pAllHeaderPaddingProperty = NULL;
    XMLPropertyState* pAllHeaderBorderProperty = NULL;
    XMLPropertyState* pAllHeaderBorderWidthProperty = NULL;
    ::std::vector< XMLPropertyState >::iterator i = rProperties.begin();
    for (i; i != rProperties.end(); i++)
    {
        sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(i->mnIndex);
        switch (nContextID)
        {
            case CTF_PM_HEADERPADDINGALL :
            {
                pAllHeaderPaddingProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_PM_HEADERBORDERALL :
            {
                pAllHeaderBorderProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_PM_HEADERBORDERWIDTHALL :
            {
                pAllHeaderBorderWidthProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
        }
    }
    if (pAllHeaderPaddingProperty)
    {
        sal_Int32 nIndex = pAllHeaderPaddingProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllHeaderPaddingProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllHeaderBorderProperty)
    {
        sal_Int32 nIndex = pAllHeaderBorderProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllHeaderBorderProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllHeaderBorderWidthProperty)
    {
        sal_Int32 nIndex = pAllHeaderBorderWidthProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllHeaderBorderWidthProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
}

PageMasterFooterImportPropertyMapper::PageMasterFooterImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper ) :
    SvXMLImportPropertyMapper( rMapper )
{
}

PageMasterFooterImportPropertyMapper::~PageMasterFooterImportPropertyMapper()
{
}

/*sal_Bool PageMasterImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

/*sal_Bool PageMasterImportPropertyMapper::handleNoItem(
        sal_Int32 nIndex,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

void PageMasterFooterImportPropertyMapper::finished(::std::vector< XMLPropertyState >& rProperties ) const
{
    SvXMLImportPropertyMapper::finished(rProperties);
    XMLPropertyState* pAllFooterPaddingProperty = NULL;
    XMLPropertyState* pAllFooterBorderProperty = NULL;
    XMLPropertyState* pAllFooterBorderWidthProperty = NULL;
    ::std::vector< XMLPropertyState >::iterator i = rProperties.begin();
    for (i; i != rProperties.end(); i++)
    {
        sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(i->mnIndex);
        switch (nContextID)
        {
            case CTF_PM_FOOTERPADDINGALL :
            {
                pAllFooterPaddingProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_PM_FOOTERBORDERALL :
            {
                pAllFooterBorderProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_PM_FOOTERBORDERWIDTHALL :
            {
                pAllFooterBorderWidthProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
        }
    }
    if (pAllFooterPaddingProperty)
    {
        sal_Int32 nIndex = pAllFooterPaddingProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllFooterPaddingProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllFooterBorderProperty)
    {
        sal_Int32 nIndex = pAllFooterBorderProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllFooterBorderProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllFooterBorderWidthProperty)
    {
        sal_Int32 nIndex = pAllFooterBorderWidthProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllFooterBorderWidthProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
}


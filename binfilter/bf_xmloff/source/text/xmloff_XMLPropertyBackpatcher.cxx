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


#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

#include "rtl/ustring.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLPROPERTYBACKPATCHER_HXX
#include "XMLPropertyBackpatcher.hxx"
#endif

#ifndef _XMLOFF_TEXTIMP_HXX_
#include "txtimp.hxx"	// XMLTextImportHelper partially implemented here
#endif
namespace binfilter {


using ::rtl::OUString;
using ::std::vector;
using ::std::map;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::XPropertySet;


template<class A>
XMLPropertyBackpatcher<A>::XMLPropertyBackpatcher(
    const ::rtl::OUString& sPropName) :
        sPropertyName(sPropName),
        bDefaultHandling(sal_False),
        bPreserveProperty(sal_False),
        sPreservePropertyName()
{
}

template<class A>
XMLPropertyBackpatcher<A>::~XMLPropertyBackpatcher()
{
    SetDefault();
}


template<class A>
void XMLPropertyBackpatcher<A>::ResolveId(
    const OUString& sName,
    A aValue)
{
    // insert ID into ID map
    aIDMap[sName] = aValue;
    
    // backpatch old references, if backpatch list exists
    if (aBackpatchListMap.count(sName))
    {
        // aah, we have a backpatch list!
        BackpatchListType* pList = 
            (BackpatchListType*)aBackpatchListMap[sName];

        // a) remove list from list map
        aBackpatchListMap.erase(sName);

        // b) for every item, set SequenceNumber
        //    (and preserve Property, if appropriate)
        Any aAny;
        aAny <<= aValue;
        if (bPreserveProperty)
        {
            // preserve version
            for(BackpatchListType::iterator aIter = pList->begin();
                aIter != pList->end();
                aIter++)
            {
                Reference<XPropertySet> xProp = (*aIter);
                Any aPres = xProp->getPropertyValue(sPreservePropertyName);
                xProp->setPropertyValue(sPropertyName, aAny);
                xProp->setPropertyValue(sPreservePropertyName, aPres);
            }
        }
        else
        {
            // without preserve
            for(BackpatchListType::iterator aIter = pList->begin();
                aIter != pList->end();
                aIter++)
            {
                (*aIter)->setPropertyValue(sPropertyName, aAny);
            }
        }

        // c) delete list
        delete pList;
    }
    // else: no backpatch list -> then we're finished
}

template<class A>
void XMLPropertyBackpatcher<A>::SetProperty(
    const Reference<XPropertySet> & xPropSet,
    const OUString& sName)
{
    Reference<XPropertySet> xNonConstPropSet(xPropSet);
    SetProperty(xNonConstPropSet, sName);
}

template<class A>
void XMLPropertyBackpatcher<A>::SetProperty(
    Reference<XPropertySet> & xPropSet,
    const OUString& sName)
{
    if (aIDMap.count(sName))
    {
        // we know this ID -> set property
        Any aAny;
        aAny <<= aIDMap[sName];
        xPropSet->setPropertyValue(sPropertyName, aAny);
    }
    else
    {
        // ID unknown -> into backpatch list for later fixup
        if (! aBackpatchListMap.count(sName))
        {
            // create backpatch list for this name
            BackpatchListType* pTmp = new BackpatchListType() ;
            aBackpatchListMap[sName] = (void*)pTmp;
        }

        // insert footnote
        ((BackpatchListType*)aBackpatchListMap[sName])->push_back(xPropSet);
    }
}

template<class A>
void XMLPropertyBackpatcher<A>::SetDefault()
{
    if (bDefaultHandling)
    {
        // not implemented yet
    }
}

// force instantiation of templates
template class XMLPropertyBackpatcher<sal_Int16>;
template class XMLPropertyBackpatcher<OUString>;



//
// XMLTextImportHelper
//
// Code from XMLTextImportHelper using the XMLPropertyBackpatcher is
// implemented here. The reason is that in the unxsols2 environment,
// all templates are instatiated as file local (switch
// -instances=static), and thus are not accessible from the outside.
//
// The previous solution was to force additional instantiation of
// XMLPropertyBackpatcher in txtimp.cxx. This solution combines all
// usage of the XMLPropertyBackpatcher in XMLPropertyBackpatcher.cxx
// instead.
//

XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetFootnoteBP()
{
    if (NULL == pFootnoteBackpatcher)
    {
        pFootnoteBackpatcher = 
            new XMLPropertyBackpatcher<sal_Int16>(sSequenceNumber);
    }
    return *pFootnoteBackpatcher;
}

XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetSequenceIdBP()
{
    if (NULL == pSequenceIdBackpatcher)
    {
        pSequenceIdBackpatcher = 
            new XMLPropertyBackpatcher<sal_Int16>(sSequenceNumber);
    }
    return *pSequenceIdBackpatcher;
}

XMLPropertyBackpatcher<OUString>& XMLTextImportHelper::GetSequenceNameBP()
{
    if (NULL == pSequenceNameBackpatcher)
    {
        pSequenceNameBackpatcher = 
            new XMLPropertyBackpatcher<OUString>(sSourceName);
    }
    return *pSequenceNameBackpatcher;
}

void XMLTextImportHelper::InsertFootnoteID(
    const OUString& sXMLId, 
    sal_Int16 nAPIId)
{
    GetFootnoteBP().ResolveId(sXMLId, nAPIId);
}

void XMLTextImportHelper::ProcessFootnoteReference(
    const OUString& sXMLId, 
    const Reference<XPropertySet> & xPropSet)
{
    GetFootnoteBP().SetProperty(xPropSet, sXMLId);
}

void XMLTextImportHelper::InsertSequenceID(
    const OUString& sXMLId, 
    const OUString& sName,
    sal_Int16 nAPIId)
{
    GetSequenceIdBP().ResolveId(sXMLId, nAPIId);
    GetSequenceNameBP().ResolveId(sXMLId, sName);
}

void XMLTextImportHelper::ProcessSequenceReference(
    const OUString& sXMLId, 
    const Reference<XPropertySet> & xPropSet)
{
    GetSequenceIdBP().SetProperty(xPropSet, sXMLId);
    GetSequenceNameBP().SetProperty(xPropSet, sXMLId);
}

void XMLTextImportHelper::_FinitBackpatcher()
{
    delete pFootnoteBackpatcher;
    delete pSequenceIdBackpatcher;
    delete pSequenceNameBackpatcher;
}
}//end of namespace binfilter

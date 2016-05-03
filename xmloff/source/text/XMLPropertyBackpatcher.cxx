/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.h>

#include <rtl/ustring.hxx>
#include "XMLPropertyBackpatcher.hxx"
#include <xmloff/txtimp.hxx>

using ::std::vector;
using ::std::map;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::beans::XPropertySet;


template<class A>
XMLPropertyBackpatcher<A>::XMLPropertyBackpatcher(
    const OUString& sPropName)
:   sPropertyName(sPropName)
,   bDefaultHandling(false)
,   bPreserveProperty(false)
,   sPreservePropertyName()
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
            static_cast<BackpatchListType*>(aBackpatchListMap[sName]);

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
                ++aIter)
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
                ++aIter)
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
        xPropSet->setPropertyValue(sPropertyName, css::uno::Any(aIDMap[sName]));
    }
    else
    {
        // ID unknown -> into backpatch list for later fixup
        if (! aBackpatchListMap.count(sName))
        {
            // create backpatch list for this name
            BackpatchListType* pTmp = new BackpatchListType() ;
            aBackpatchListMap[sName] = static_cast<void*>(pTmp);
        }

        // insert footnote
        static_cast<BackpatchListType*>(aBackpatchListMap[sName])->push_back(xPropSet);
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

struct XMLTextImportHelper::BackpatcherImpl
{
    /// backpatcher for references to footnotes and endnotes
    ::std::unique_ptr< XMLPropertyBackpatcher<sal_Int16> >
        m_pFootnoteBackpatcher;

    /// backpatchers for references to sequences
    ::std::unique_ptr< XMLPropertyBackpatcher<sal_Int16> >
        m_pSequenceIdBackpatcher;

    ::std::unique_ptr< XMLPropertyBackpatcher< OUString> >
        m_pSequenceNameBackpatcher;
};

std::shared_ptr<XMLTextImportHelper::BackpatcherImpl>
XMLTextImportHelper::MakeBackpatcherImpl()
{
    // n.b.: the shared_ptr stores the dtor!
    return std::shared_ptr<BackpatcherImpl>(new BackpatcherImpl);
}

static OUString GetSequenceNumber()
{
    return OUString("SequenceNumber");
}


// XMLTextImportHelper

// Code from XMLTextImportHelper using the XMLPropertyBackpatcher is
// implemented here. The reason is that in the unxsols2 environment,
// all templates are instantiated as file local (switch
// -instances=static), and thus are not accessible from the outside.

// The previous solution was to force additional instantiation of
// XMLPropertyBackpatcher in txtimp.cxx. This solution combines all
// usage of the XMLPropertyBackpatcher in XMLPropertyBackpatcher.cxx
// instead.


XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetFootnoteBP()
{
    if (!m_xBackpatcherImpl->m_pFootnoteBackpatcher.get())
    {
        m_xBackpatcherImpl->m_pFootnoteBackpatcher.reset(
            new XMLPropertyBackpatcher<sal_Int16>(GetSequenceNumber()));
    }
    return *m_xBackpatcherImpl->m_pFootnoteBackpatcher;
}

XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetSequenceIdBP()
{
    if (!m_xBackpatcherImpl->m_pSequenceIdBackpatcher.get())
    {
        m_xBackpatcherImpl->m_pSequenceIdBackpatcher.reset(
            new XMLPropertyBackpatcher<sal_Int16>(GetSequenceNumber()));
    }
    return *m_xBackpatcherImpl->m_pSequenceIdBackpatcher;
}

XMLPropertyBackpatcher<OUString>& XMLTextImportHelper::GetSequenceNameBP()
{
    static const char s_SourceName[] = "SourceName";
    if (!m_xBackpatcherImpl->m_pSequenceNameBackpatcher.get())
    {
        m_xBackpatcherImpl->m_pSequenceNameBackpatcher.reset(
            new XMLPropertyBackpatcher<OUString>(s_SourceName));
    }
    return *m_xBackpatcherImpl->m_pSequenceNameBackpatcher;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

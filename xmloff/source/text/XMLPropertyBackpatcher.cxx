/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.h>

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
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
,   bDefaultHandling(sal_False)
,   bPreserveProperty(sal_False)
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
    
    aIDMap[sName] = aValue;

    
    if (aBackpatchListMap.count(sName))
    {
        
        BackpatchListType* pList =
            (BackpatchListType*)aBackpatchListMap[sName];

        
        aBackpatchListMap.erase(sName);

        
        
        Any aAny;
        aAny <<= aValue;
        if (bPreserveProperty)
        {
            
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
            
            for(BackpatchListType::iterator aIter = pList->begin();
                aIter != pList->end();
                ++aIter)
            {
                (*aIter)->setPropertyValue(sPropertyName, aAny);
            }
        }

        
        delete pList;
    }
    
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
        
        Any aAny;
        aAny <<= aIDMap[sName];
        xPropSet->setPropertyValue(sPropertyName, aAny);
    }
    else
    {
        
        if (! aBackpatchListMap.count(sName))
        {
            
            BackpatchListType* pTmp = new BackpatchListType() ;
            aBackpatchListMap[sName] = (void*)pTmp;
        }

        
        ((BackpatchListType*)aBackpatchListMap[sName])->push_back(xPropSet);
    }
}

template<class A>
void XMLPropertyBackpatcher<A>::SetDefault()
{
    if (bDefaultHandling)
    {
        
    }
}


template class XMLPropertyBackpatcher<sal_Int16>;
template class XMLPropertyBackpatcher<OUString>;

struct XMLTextImportHelper::BackpatcherImpl
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    
    ::std::auto_ptr< XMLPropertyBackpatcher<sal_Int16> >
        m_pFootnoteBackpatcher;

    
    ::std::auto_ptr< XMLPropertyBackpatcher<sal_Int16> >
        m_pSequenceIdBackpatcher;

    ::std::auto_ptr< XMLPropertyBackpatcher< OUString> >
        m_pSequenceNameBackpatcher;
    SAL_WNODEPRECATED_DECLARATIONS_POP
};

::boost::shared_ptr<XMLTextImportHelper::BackpatcherImpl>
XMLTextImportHelper::MakeBackpatcherImpl()
{
    
    return ::boost::shared_ptr<BackpatcherImpl>(new BackpatcherImpl);
}

static OUString const& GetSequenceNumber()
{
    static OUString s_SequenceNumber("SequenceNumber");
    return s_SequenceNumber;
}

//

//




//




//

XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetFootnoteBP()
{
    if (!m_pBackpatcherImpl->m_pFootnoteBackpatcher.get())
    {
        m_pBackpatcherImpl->m_pFootnoteBackpatcher.reset(
            new XMLPropertyBackpatcher<sal_Int16>(GetSequenceNumber()));
    }
    return *m_pBackpatcherImpl->m_pFootnoteBackpatcher;
}

XMLPropertyBackpatcher<sal_Int16>& XMLTextImportHelper::GetSequenceIdBP()
{
    if (!m_pBackpatcherImpl->m_pSequenceIdBackpatcher.get())
    {
        m_pBackpatcherImpl->m_pSequenceIdBackpatcher.reset(
            new XMLPropertyBackpatcher<sal_Int16>(GetSequenceNumber()));
    }
    return *m_pBackpatcherImpl->m_pSequenceIdBackpatcher;
}

XMLPropertyBackpatcher<OUString>& XMLTextImportHelper::GetSequenceNameBP()
{
    static OUString s_SourceName("SourceName");
    if (!m_pBackpatcherImpl->m_pSequenceNameBackpatcher.get())
    {
        m_pBackpatcherImpl->m_pSequenceNameBackpatcher.reset(
            new XMLPropertyBackpatcher<OUString>(s_SourceName));
    }
    return *m_pBackpatcherImpl->m_pSequenceNameBackpatcher;
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

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


#include <xmloff/txtparae.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>

#include <vector>


#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include <com/sun/star/text/SectionFileLink.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltkmap.hxx>
#include "XMLTextNumRuleInfo.hxx"
#include "XMLSectionExport.hxx"
#include "XMLRedlineExport.hxx"
#include "MultiPropertySetHelper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::std;

using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::container::XIndexReplace;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::lang::XServiceInfo;

void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    const Reference<XTextContent> & rNextSectionContent,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    sal_Bool bAutoStyles)
{
    Reference<XTextSection> xNextSection;

    
    Reference<XPropertySet> xPropSet(rNextSectionContent, UNO_QUERY);
    if (xPropSet.is())
    {
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(sTextSection))
        {
            xPropSet->getPropertyValue(sTextSection) >>= xNextSection;
        }
        
    }

    exportListAndSectionChange(rPrevSection, xNextSection,
                               rPrevRule, rNextRule, bAutoStyles);
}

void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    MultiPropertySetHelper& rPropSetHelper,
    sal_Int16 nTextSectionId,
    const Reference<XTextContent> & rNextSectionContent,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    sal_Bool bAutoStyles)
{
    Reference<XTextSection> xNextSection;

    
    Reference<XPropertySet> xPropSet(rNextSectionContent, UNO_QUERY);
    if (xPropSet.is())
    {
        if( !rPropSetHelper.checkedProperties() )
            rPropSetHelper.hasProperties( xPropSet->getPropertySetInfo() );
        if( rPropSetHelper.hasProperty( nTextSectionId ))
        {
            xNextSection.set(rPropSetHelper.getValue( nTextSectionId , xPropSet,
                true ), uno::UNO_QUERY);
        }
        
    }

    exportListAndSectionChange(rPrevSection, xNextSection,
                               rPrevRule, rNextRule, bAutoStyles);
}

void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    const Reference<XTextSection> & rNextSection,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    sal_Bool bAutoStyles)
{
    
    if (rPrevSection != rNextSection)
    {
        

        
        XMLTextNumRuleInfo aEmptyNumRuleInfo;
        if ( !bAutoStyles )
            exportListChange(rPrevRule, aEmptyNumRuleInfo);

        
        
        vector< Reference<XTextSection> > aOldStack;
        Reference<XTextSection> aCurrent(rPrevSection);
        while(aCurrent.is())
        {
            
            
            if (pSectionExport->IsMuteSection(aCurrent))
                aOldStack.clear();

            aOldStack.push_back(aCurrent);
            aCurrent.set(aCurrent->getParentSection());
        }

        vector< Reference<XTextSection> > aNewStack;
        aCurrent.set(rNextSection);
        sal_Bool bMute = sal_False;
        while(aCurrent.is())
        {
            
            
            if (pSectionExport->IsMuteSection(aCurrent))
            {
                aNewStack.clear();
                bMute = sal_True;
            }

            aNewStack.push_back(aCurrent);
            aCurrent.set(aCurrent->getParentSection());
        }

        
        vector<Reference<XTextSection> > ::reverse_iterator aOld =
            aOldStack.rbegin();
        vector<Reference<XTextSection> > ::reverse_iterator aNew =
            aNewStack.rbegin();
        
        while ( (aOld != aOldStack.rend()) &&
                (aNew != aNewStack.rend()) &&
                (*aOld) == (*aNew) )
        {
            ++aOld;
            ++aNew;
        }

        
        
        if (aOld != aOldStack.rend())
        {
            vector<Reference<XTextSection> > ::iterator aOldForward(
                aOldStack.begin());
            while ((aOldForward != aOldStack.end()) &&
                   (*aOldForward != *aOld))
            {
                if ( !bAutoStyles && (NULL != pRedlineExport) )
                    pRedlineExport->ExportStartOrEndRedline(*aOldForward,
                                                                sal_False);
                pSectionExport->ExportSectionEnd(*aOldForward, bAutoStyles);
                ++aOldForward;
            }
            if (aOldForward != aOldStack.end())
            {
                if ( !bAutoStyles && (NULL != pRedlineExport) )
                    pRedlineExport->ExportStartOrEndRedline(*aOldForward,
                                                            sal_False);
                pSectionExport->ExportSectionEnd(*aOldForward, bAutoStyles);
            }
        }

        
        
        while (aNew != aNewStack.rend())
        {
            if ( !bAutoStyles && (NULL != pRedlineExport) )
                pRedlineExport->ExportStartOrEndRedline(*aNew, sal_True);
            pSectionExport->ExportSectionStart(*aNew, bAutoStyles);
            ++aNew;
        }

        
        if ( !bAutoStyles && !bMute )
            exportListChange(aEmptyNumRuleInfo, rNextRule);
    }
    else
    {
        
        if ( !bAutoStyles )
            exportListChange(rPrevRule, rNextRule);
    }

    
    rPrevSection.set(rNextSection);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

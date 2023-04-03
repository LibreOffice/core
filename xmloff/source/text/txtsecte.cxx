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


#include <xmloff/txtparae.hxx>

#include <vector>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextSection.hpp>
#include "XMLTextNumRuleInfo.hxx"
#include "XMLSectionExport.hxx"
#include "XMLRedlineExport.hxx"
#include <MultiPropertySetHelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

using ::com::sun::star::beans::XPropertySet;

void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    const Reference<XTextContent> & rNextSectionContent,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    bool bAutoStyles)
{
    Reference<XTextSection> xNextSection;

    // first: get current XTextSection
    Reference<XPropertySet> xPropSet(rNextSectionContent, UNO_QUERY);
    if (xPropSet.is())
    {
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(gsTextSection))
        {
            xPropSet->getPropertyValue(gsTextSection) >>= xNextSection;
        }
        // else: no current section
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
    bool bAutoStyles)
{
    Reference<XTextSection> xNextSection;

    // first: get current XTextSection
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
        // else: no current section
    }

    exportListAndSectionChange(rPrevSection, xNextSection,
                               rPrevRule, rNextRule, bAutoStyles);
}

void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    const Reference<XTextSection> & rNextSection,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    bool bAutoStyles)
{
    // old != new? -> maybe we have to start or end a new section
    if (rPrevSection != rNextSection)
    {
        // a new section started, or an old one gets closed!

        // close old list
        XMLTextNumRuleInfo aEmptyNumRuleInfo;
        if ( !bAutoStyles )
            exportListChange(rPrevRule, aEmptyNumRuleInfo);

        // Build stacks of old and new sections
        // Sections on top of mute sections should not be on the stack
        std::vector< Reference<XTextSection> > aOldStack;
        Reference<XTextSection> aCurrent(rPrevSection);
        while(aCurrent.is())
        {
            // if we have a mute section, ignore all its children
            // (all previous ones)
            if (m_pSectionExport->IsMuteSection(aCurrent))
                aOldStack.clear();

            aOldStack.push_back(aCurrent);
            aCurrent.set(aCurrent->getParentSection());
        }

        std::vector< Reference<XTextSection> > aNewStack;
        aCurrent.set(rNextSection);
        bool bMute = false;
        while(aCurrent.is())
        {
            // if we have a mute section, ignore all its children
            // (all previous ones)
            if (m_pSectionExport->IsMuteSection(aCurrent))
            {
                aNewStack.clear();
                bMute = true;
            }

            aNewStack.push_back(aCurrent);
            aCurrent.set(aCurrent->getParentSection());
        }

        // compare the two stacks
        std::vector<Reference<XTextSection> > ::reverse_iterator aOld =
            aOldStack.rbegin();
        std::vector<Reference<XTextSection> > ::reverse_iterator aNew =
            aNewStack.rbegin();
        // compare bottom sections and skip equal section
        while ( (aOld != aOldStack.rend()) &&
                (aNew != aNewStack.rend()) &&
                (*aOld) == (*aNew) )
        {
            ++aOld;
            ++aNew;
        }

        // close all elements of aOld ...
        // (order: newest to oldest)
        if (aOld != aOldStack.rend())
        {
            std::vector<Reference<XTextSection> > ::iterator aOldForward(
                aOldStack.begin());
            while ((aOldForward != aOldStack.end()) &&
                   (*aOldForward != *aOld))
            {
                if ( !bAutoStyles && (nullptr != m_pRedlineExport) )
                    m_pRedlineExport->ExportStartOrEndRedline(*aOldForward,
                                                                false);
                m_pSectionExport->ExportSectionEnd(*aOldForward, bAutoStyles);
                ++aOldForward;
            }
            if (aOldForward != aOldStack.end())
            {
                if ( !bAutoStyles && (nullptr != m_pRedlineExport) )
                    m_pRedlineExport->ExportStartOrEndRedline(*aOldForward,
                                                            false);
                m_pSectionExport->ExportSectionEnd(*aOldForward, bAutoStyles);
            }
        }

        // ...then open all of aNew
        // (order: oldest to newest)
        while (aNew != aNewStack.rend())
        {
            if ( !bAutoStyles && (nullptr != m_pRedlineExport) )
                m_pRedlineExport->ExportStartOrEndRedline(*aNew, true);
            m_pSectionExport->ExportSectionStart(*aNew, bAutoStyles);
            ++aNew;
        }

        // start new list
        if ( !bAutoStyles && !bMute )
            exportListChange(aEmptyNumRuleInfo, rNextRule);
    }
    else
    {
        // list change, if sections have not changed
        if ( !bAutoStyles )
            exportListChange(rPrevRule, rNextRule);
    }

    // save old section (old numRule gets saved in calling method)
    rPrevSection.set(rNextSection);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

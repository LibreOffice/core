/*************************************************************************
 *
 *  $RCSfile: txtsecte.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: dvo $ $Date: 2001-02-13 16:55:00 $
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

#ifndef _XMLOFF_TXTPARAE_HXX
#include "txtparae.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <vector>


#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#include "XMLTextNumRuleInfo.hxx"
#endif

#ifndef _XMLOFF_XMLSECTIONEXPORT_HXX_
#include "XMLSectionExport.hxx"
#endif

#ifndef _XMLOFF_XMLREDLINEEXPORT_HXX
#include "XMLRedlineExport.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::std;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::container::XIndexReplace;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::lang::XServiceInfo;

Reference<XText> lcl_findXText(const Reference<XTextSection>& rSect)
{
    Reference<XText> xText;

    Reference<XTextContent> xTextContent(rSect, UNO_QUERY);
    if (xTextContent.is())
    {
        xText = xTextContent->getAnchor()->getText();
    }

    return xText;
}

void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    const Reference<XTextContent> & rNextSectionContent,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    sal_Bool bAutoStyles)
{
    Reference<XTextSection> xNextSection;
    Reference<XDocumentIndex> xNextIndex;

    // first: get current XTextSection
    Reference<XPropertySet> xPropSet(rNextSectionContent, UNO_QUERY);
    if (xPropSet.is())
    {
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(sTextSection))
        {
            Any aAny = xPropSet->getPropertyValue(sTextSection);
            aAny >>= xNextSection;
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
    sal_Bool bAutoStyles)
{
    if ( bAutoStyles )
    {
        if ( rNextSection.is() )
        {
            pSectionExport->ExportSectionStart( rNextSection, bAutoStyles );
        }
    }
    else
    {
        // old != new? -> start/equal?
        if (rPrevSection != rNextSection)
        {
            // a new section started, or an old one gets closed!

            // close old list
            XMLTextNumRuleInfo aEmptyNumRule;
            exportListChange(rPrevRule, aEmptyNumRule);

            // Build stacks of old and new sections
            // Sections on top of mute sections should not be on the stack
            vector<Reference<XTextSection> > aOldStack;
            Reference<XTextSection> aCurrent = rPrevSection;
            while(aCurrent.is())
            {
                // if we have a mute section, ignore all its children
                // (all previous ones)
                if (pSectionExport->IsMuteSection(aCurrent))
                    aOldStack.clear();

                aOldStack.push_back(aCurrent);
                aCurrent = aCurrent->getParentSection();
            }

            vector<Reference<XTextSection> > aNewStack;
            aCurrent = rNextSection;
            while(aCurrent.is())
            {
                // if we have a mute section, ignore all its children
                // (all previous ones)
                if (pSectionExport->IsMuteSection(aCurrent))
                    aNewStack.clear();

                aNewStack.push_back(aCurrent);
                aCurrent = aCurrent->getParentSection();
            }

            // compare the two stacks
            vector<Reference<XTextSection> > ::reverse_iterator aOld =
                aOldStack.rbegin();
            vector<Reference<XTextSection> > ::reverse_iterator aNew =
                aNewStack.rbegin();
            // compare bottom sections and skip equal section
            while ( (aOld != aOldStack.rend()) &&
                    (aNew != aNewStack.rend()) &&
                    (*aOld) == (*aNew) )
            {
                aOld++;
                aNew++;
            }

            // close all elements of aOld ...
            // (order: newest to oldest)
            if (aOld != aOldStack.rend())
            {
                vector<Reference<XTextSection> > ::iterator aOldForward =
                    aOldStack.begin();
                while ((aOldForward != aOldStack.end()) &&
                       (*aOldForward != *aOld))
                {
                    if (NULL != pRedlineExport)
                        pRedlineExport->ExportStartOrEndRedline(*aOldForward,
                                                                sal_False);
                    pSectionExport->ExportSectionEnd(*aOldForward,
                                                     bAutoStyles);
                    aOldForward++;
                }
                if (aOldForward != aOldStack.end())
                {
                    if (NULL != pRedlineExport)
                        pRedlineExport->ExportStartOrEndRedline(*aOldForward,
                                                                sal_False);
                    pSectionExport->ExportSectionEnd(*aOldForward,
                                                     bAutoStyles);
                }
            }

            // ...then open all of aNew
            // (order: oldest to newest)
            while (aNew != aNewStack.rend())
            {
                if (NULL != pRedlineExport)
                    pRedlineExport->ExportStartOrEndRedline(*aNew, sal_True);
                pSectionExport->ExportSectionStart(*aNew, bAutoStyles);
                aNew++;
            }

            // start new list
            exportListChange(aEmptyNumRule, rNextRule);
        }
        else
        {
            // list change, if sections have not changed
            exportListChange(rPrevRule, rNextRule);
        }
    }

    // save old section (old numRule gets saved in calling method
    rPrevSection = rNextSection;
}


/*************************************************************************
 *
 *  $RCSfile: txtftne.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dvo $ $Date: 2000-10-25 08:49:58 $
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


/** @#file
 *
 * This file implements XMLTextParagraphExport methods to export
 * - footnotes
 * - endnotes
 * - footnote configuration elements
 * - endnote configuration elements
 */

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTE_HPP_
#include <com/sun/star/text/XFootnote.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XFOOTNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XENDNOTESSUPPLIER_HPP_
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_FOOTNOTENUMBERING_HPP_
#include <com/sun/star/text/FootnoteNumbering.hpp>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLNUMFE_HXX
#include "xmlnumfe.hxx"
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
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif
#ifndef _XMLOFF_XMLNUME_HXX
#include "xmlnume.hxx"
#endif
#ifndef _XMLOFF_TXTPARAE_HXX
#include "txtparae.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;


void XMLTextParagraphExport::exportTextFootnote(
    const Reference<XPropertySet> & rPropSet,
    const OUString& sText,
    sal_Bool bAutoStyles)
{
    // get footnote and associated text
    Any aAny;
    aAny = rPropSet->getPropertyValue(sFootnote);
    Reference<XFootnote> xFootnote;
    aAny >>= xFootnote;
    Reference<XText> xText(xFootnote, UNO_QUERY);

    // are we an endnote?
    Reference<XServiceInfo> xServiceInfo( xFootnote, UNO_QUERY );
    sal_Bool bIsEndnote = xServiceInfo->supportsService(sTextEndnoteService);

    if (bAutoStyles)
    {
        // handle formatting of citation mark
        Add( XML_STYLE_FAMILY_TEXT_TEXT, rPropSet );

        // handle formatting within footnote
        exportTextFootnoteHelper(xFootnote, xText, sText,
                                 bAutoStyles, bIsEndnote);
    }
    else
    {
        // create span (for citation mark) if necessary; footnote content
        // wil be handled via exportTextFootnoteHelper, exportText
        OUString sStyle = FindTextStyle( rPropSet );
        if( sStyle.getLength() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_TEXT, sXML_style_name,
                                      sStyle );
            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_TEXT,
                                      sXML_span, sal_False, sal_False );
            exportTextFootnoteHelper(xFootnote, xText, sText,
                                     bAutoStyles, bIsEndnote);
        }
        else
        {
            exportTextFootnoteHelper(xFootnote, xText, sText,
                                     bAutoStyles, bIsEndnote);
        }
    }
}


void XMLTextParagraphExport::exportTextFootnoteHelper(
    const Reference<XFootnote> & rFootnote,
    const Reference<XText> & rText,
    const OUString& sText,
    sal_Bool bAutoStyles,
    sal_Bool bIsEndnote)
{
    if (bAutoStyles)
    {
        exportText(rText, bAutoStyles);
    }
    else
    {
        // export reference Id (for reference fields)
        Reference<XPropertySet> xPropSet(rFootnote, UNO_QUERY);
        Any aAny = xPropSet->getPropertyValue(sReferenceId);
        sal_Int32 nNumber;
        aAny >>= nNumber;
        OUStringBuffer aBuf;
        aBuf.appendAscii("ftn");
        aBuf.append(nNumber);
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_id,
                                 aBuf.makeStringAndClear());

        SvXMLElementExport aNote(GetExport(), XML_NAMESPACE_TEXT,
                                 (bIsEndnote ? sXML_endnote : sXML_footnote),
                                 sal_False, sal_False);
        {
            // handle label vs. automatic numbering
            OUString sLabel = rFootnote->getLabel();
            if (sLabel.getLength()>0)
            {
                GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_label,
                                         sLabel);
            }
            // else: automatic numbering -> no attribute

            SvXMLElementExport aCite(GetExport(), XML_NAMESPACE_TEXT,
                                     (bIsEndnote ? sXML_endnote_citation :
                                                   sXML_footnote_citation),
                                     sal_False, sal_False);
            GetExport().GetDocHandler()->characters(sText);
        }

        {
            SvXMLElementExport aBody(GetExport(), XML_NAMESPACE_TEXT,
                                     (bIsEndnote ? sXML_endnote_body :
                                                   sXML_footnote_body),
                                     sal_False, sal_False);
            exportText(rText, bAutoStyles);
        }
    }
}


void XMLTextParagraphExport::exportTextFootnoteConfiguration()
{
    // footnote settings
    Reference<XFootnotesSupplier> aFootnotesSupplier(GetExport().GetModel(),
                                                     UNO_QUERY);
    Reference<XPropertySet> aFootnoteConfiguration(
        aFootnotesSupplier->getFootnoteSettings());
    exportTextFootnoteConfigurationHelper(aFootnoteConfiguration, sal_False);

    // endnote settings
    Reference<XEndnotesSupplier> aEndnotesSupplier(GetExport().GetModel(),
                                                   UNO_QUERY);
    Reference<XPropertySet> aEndnoteConfiguration(
        aEndnotesSupplier->getEndnoteSettings());
    exportTextFootnoteConfigurationHelper(aEndnoteConfiguration, sal_True);
}


void XMLTextParagraphExport::exportTextFootnoteConfigurationHelper(
    const Reference<XPropertySet> & rFootnoteConfig,
    sal_Bool bIsEndnote)
{
    Any aAny;
    OUString sTmp;

    // prefix
    aAny = rFootnoteConfig->getPropertyValue(sPrefix);
    aAny >>= sTmp;
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_num_prefix, sTmp);

    // suffix
    aAny = rFootnoteConfig->getPropertyValue(sSuffix);
    aAny >>= sTmp;
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_num_suffix, sTmp);

    // citation style ???
    aAny = rFootnoteConfig->getPropertyValue(sCharStyleName);
    aAny >>= sTmp;
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_citation_style_name,
                             sTmp);

    // default/paragraph style
    aAny = rFootnoteConfig->getPropertyValue(sParaStyleName);
    aAny >>= sTmp;
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_default_style_name,
                             sTmp);

    // page style
    aAny = rFootnoteConfig->getPropertyValue(sPageStyleName);
    aAny >>= sTmp;
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_master_page_name,
                             sTmp);

    // numbering style
    aAny = rFootnoteConfig->getPropertyValue(sNumberingType);
    sal_Int16 nNumbering;
    aAny >>= nNumbering;
    GetExport().AddAttributeASCII(XML_NAMESPACE_STYLE, sXML_num_format,
                                  SvxXMLNumRuleExport::GetNumFormatValue(
                                      nNumbering));
    const sal_Char* pLetterSync =
        SvxXMLNumRuleExport::GetNumLetterSync(nNumbering);
    if (NULL != pLetterSync)
    {
        GetExport().AddAttributeASCII(XML_NAMESPACE_STYLE,
                                      sXML_num_letter_sync, pLetterSync);
    }

    // StartAt / offset
    aAny = rFootnoteConfig->getPropertyValue(sStartAt);
    sal_Int16 nOffset;
    aAny >>= nOffset;
    OUStringBuffer sBuffer;
    SvXMLUnitConverter::convertNumber(sBuffer, (sal_Int32)nOffset);
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_offset,
                             sBuffer.makeStringAndClear());

    // some properties are for footnotes only
    if (!bIsEndnote)
    {
        // footnotes position
        aAny = rFootnoteConfig->getPropertyValue(
            sPositionEndOfDoc);
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                      sXML_footnotes_position,
                                      ( (*(sal_Bool *)aAny.getValue()) ?
                                        sXML_document : sXML_page ) );

        aAny = rFootnoteConfig->getPropertyValue(sFootnoteCounting);
        sal_Int16 nTmp;
        aAny >>= nTmp;
        sal_Char* pElement;
        switch (nTmp)
        {
            case FootnoteNumbering::PER_PAGE:
                pElement = sXML_page;
                break;
            case FootnoteNumbering::PER_CHAPTER:
                pElement = sXML_chapter;
                break;
            case FootnoteNumbering::PER_DOCUMENT:
            default:
                pElement = sXML_document;
                break;
        }
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                      sXML_start_numbering_at,
                                      pElement);
    }

    // element
    SvXMLElementExport aFootnoteConfigElement(GetExport(),
                                              XML_NAMESPACE_TEXT,
                                              ( bIsEndnote ?
                                                sXML_endnotes_configuration :
                                                sXML_footnotes_configuration ),
                                              sal_True, sal_True);

    // two element for footnote content
    if (!bIsEndnote)
    {
        {
            // end notice / quo vadis
            aAny = rFootnoteConfig->getPropertyValue(
                sEndNotice);
            aAny >>= sTmp;
            SvXMLElementExport aElem(GetExport(),
                                     XML_NAMESPACE_TEXT,
                                     sXML_footnote_continuation_notice_forward,
                                     sal_True, sal_False);
            GetExport().GetDocHandler()->characters(sTmp);
        }

        {
            // begin notice / ergo sum
            aAny = rFootnoteConfig->getPropertyValue(
                sBeginNotice);
            aAny >>= sTmp;
            SvXMLElementExport aElem(GetExport(),
                                     XML_NAMESPACE_TEXT,
                                    sXML_footnote_continuation_notice_backward,
                                     sal_True, sal_False);
            GetExport().GetDocHandler()->characters(sTmp);
        }
    }
}


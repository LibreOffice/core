/*************************************************************************
 *
 *  $RCSfile: XMLTextMarkImportContext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:06 $
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


#include "XMLTextMarkImportContext.hxx"


#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif


using namespace ::rtl;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLTextMarkImportContext, SvXMLImportContext);

XMLTextMarkImportContext::XMLTextMarkImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrefix,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rHelper(rHlp)
{
}

enum lcl_MarkType { TypeReference, TypeReferenceStart, TypeReferenceEnd,
                    TypeBookmark, TypeBookmarkStart, TypeBookmarkEnd };

static SvXMLEnumMapEntry __READONLY_DATA lcl_aMarkTypeMap[] =
{
    { sXML_reference,       TypeReference },
    { sXML_reference_start, TypeReferenceStart },
    { sXML_reference_end,   TypeReferenceEnd },
    { sXML_bookmark,        TypeBookmark },
    { sXML_bookmark_start,  TypeBookmarkStart },
    { sXML_bookmark_end,    TypeBookmarkEnd },
    { 0,                0 },
};

void XMLTextMarkImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    const OUString sAPI_reference_mark(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.ReferenceMark"));
    const OUString sAPI_bookmark(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Bookmark"));


    OUString sName;

    if (FindName(GetImport(), xAttrList, sName))
    {
        sal_uInt16 nTmp;
        if (SvXMLUnitConverter::convertEnum(nTmp, GetLocalName(),
                                            lcl_aMarkTypeMap))
        {
            switch ((lcl_MarkType)nTmp)
            {
                case TypeReference:
                    // export point reference mark
                    CreateAndInsertMark(GetImport(),
                                        sAPI_reference_mark,
                                        sName,
                                       rHelper.GetCursorAsRange()->getStart());
                    break;

                case TypeBookmark:
                    // export point bookmark
                    CreateAndInsertMark(GetImport(),
                                        sAPI_bookmark,
                                        sName,
                                       rHelper.GetCursorAsRange()->getStart());
                    break;

                case TypeBookmarkStart:
                    // save XTextRange for later construction of bookmark
                    rHelper.InsertBookmarkStartRange(
                        sName, rHelper.GetCursorAsRange()->getStart());
                    break;

                case TypeBookmarkEnd:
                {
                    // get old range, and construct
                    Reference<XTextRange> xStartRange;
                    if (rHelper.FindAndRemoveBookmarkStartRange(xStartRange,
                                                                sName))
                    {
                        Reference<XTextRange> xEndRange(
                            rHelper.GetCursorAsRange()->getEnd());

                        // check if beginning and end are in same XText
                        if (xStartRange->getText() == xEndRange->getText())
                        {
                            // create range for insertion
                            Reference<XTextCursor> xInsertionCursor =
                                rHelper.GetText()->createTextCursorByRange(
                                    xEndRange);
                            xInsertionCursor->gotoRange(xStartRange, sal_True);

                            //DBG_ASSERT(! xInsertionCursor->isCollapsed(),
                            //              "we want no point mark");
                            // can't assert, because someone could
                            // create a file with subsequence
                            // start/end elements

                            Reference<XTextRange> xInsertionRange(
                                xInsertionCursor, UNO_QUERY);

                            // insert reference
                            CreateAndInsertMark(GetImport(),
                                                sAPI_bookmark,
                                                sName,
                                                xInsertionRange);
                        }
                        // else: beginning/end in different XText -> ignore!
                    }
                    // else: no start found -> ignore!
                    break;
                }

                case TypeReferenceStart:
                case TypeReferenceEnd:
                    DBG_ERROR("reference start/end are handled in txtparai !");
                    break;

                default:
                    DBG_ERROR("unknown mark type");
                    break;
            }
        }
    }
}


void XMLTextMarkImportContext::CreateAndInsertMark(
    SvXMLImport& rImport,
    const OUString& sServiceName,
    const OUString& sMarkName,
    const Reference<XTextRange> & rRange)
{
    // create mark
    Reference<XMultiServiceFactory> xFactory(rImport.GetModel(),UNO_QUERY);
    if( xFactory.is() )
    {
        Reference<XInterface> xIfc = xFactory->createInstance(sServiceName);

        // set name
        Reference<XNamed> xNamed(xIfc, UNO_QUERY);
        if (xNamed.is())
        {
            xNamed->setName(sMarkName);

            // cast to XTextContent and attach to document
            Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
            if (xTextContent.is())
            {
                // if inserting marks, bAbsorb==sal_False will cause
                // collapsing of the given XTextRange.
                rImport.GetTextImport()->GetText()->insertTextContent(rRange,
                                                     xTextContent, sal_True);
            }
        }
    }
}

sal_Bool XMLTextMarkImportContext::FindName(
    SvXMLImport& rImport,
    const Reference<XAttributeList> & xAttrList,
    OUString& sName)
{
    sal_Bool bNameOK;

    // find name attribute first
    sal_Int32 nLength = xAttrList->getLength();
    for(sal_Int32 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             (0 == sLocalName.compareToAscii(sXML_name)) )
        {
            sName = xAttrList->getValueByIndex(nAttr);
            bNameOK = sal_True;
        }
    }

    return bNameOK;
}

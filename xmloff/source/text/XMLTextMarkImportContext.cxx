/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextMarkImportContext.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:03:56 $
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
#include "precompiled_xmloff.hxx"


#include "XMLTextMarkImportContext.hxx"


#ifndef _RTL_USTRING
#include <rtl/ustring.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include <xmloff/xmlimp.hxx>
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
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


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

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
    { XML_REFERENCE_MARK,           TypeReference },
    { XML_REFERENCE_MARK_START,     TypeReferenceStart },
    { XML_REFERENCE_MARK_END,       TypeReferenceEnd },
    { XML_BOOKMARK,                 TypeBookmark },
    { XML_BOOKMARK_START,           TypeBookmarkStart },
    { XML_BOOKMARK_END,             TypeBookmarkEnd },
    { XML_TOKEN_INVALID,            0 },
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
                            rHelper.GetCursorAsRange()->getStart());

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
                try
                {
                // if inserting marks, bAbsorb==sal_False will cause
                // collapsing of the given XTextRange.
                    rImport.GetTextImport()->GetText()->insertTextContent(rRange,
                        xTextContent, sal_True);
                }
                catch (com::sun::star::lang::IllegalArgumentException e)
                {
                    // ignore
                }
            }
        }
    }
}

sal_Bool XMLTextMarkImportContext::FindName(
    SvXMLImport& rImport,
    const Reference<XAttributeList> & xAttrList,
    OUString& sName)
{
    sal_Bool bNameOK = sal_False;

    // find name attribute first
    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken(sLocalName, XML_NAME)   )
        {
            sName = xAttrList->getValueByIndex(nAttr);
            bNameOK = sal_True;
        }
    }

    return bNameOK;
}

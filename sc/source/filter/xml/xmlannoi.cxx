/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlannoi.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:50:54 $
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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "xmlannoi.hxx"
#include "xmlimprt.hxx"
#include "xmlcelli.hxx"
#ifndef SC_XMLCONTI_HXX
#include "xmlconti.hxx"
#endif
#ifndef _SC_XMLTABLESHAPEIMPORTHELPER_HXX
#include "XMLTableShapeImportHelper.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLAnnotationContext::ScXMLAnnotationContext( ScXMLImport& rImport,
                                      USHORT nPrfx,
                                      const ::rtl::OUString& rLName,
                                      const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                      ScXMLTableRowCellContext* pTempCellContext) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    nParagraphCount(0),
    bDisplay(sal_False),
    bHasTextP(sal_False),
    bHasPos(sal_False),
    pShapeContext(NULL),
    pCellContext(pTempCellContext)
{
    uno::Reference<drawing::XShapes> xShapes (GetScImport().GetTables().GetCurrentXShapes());
    if (xShapes.is())
    {
        XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)GetScImport().GetShapeImport().get();
        pTableShapeImport->SetAnnotation(this);
        pShapeContext = GetScImport().GetShapeImport()->CreateGroupChildContext(
            GetScImport(), nPrfx, rLName, xAttrList, xShapes, sal_True);
    }

    pCellContext = pTempCellContext;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    const SvXMLTokenMap& rAttrTokenMap = GetScImport().GetTableAnnotationAttrTokenMap();
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_ANNOTATION_ATTR_AUTHOR:
            {
                sAuthorBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE:
            {
                sCreateDateBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_CREATE_DATE_STRING:
            {
                sCreateDateStringBuffer = sValue;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_DISPLAY:
            {
                bDisplay = IsXMLToken(sValue, XML_TRUE);
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_X:
            {
                bHasPos = sal_True;
            }
            break;
            case XML_TOK_TABLE_ANNOTATION_ATTR_Y:
            {
                bHasPos = sal_True;
            }
            break;
        }
    }
}

ScXMLAnnotationContext::~ScXMLAnnotationContext()
{
}

void ScXMLAnnotationContext::StartElement(const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList)
{
    if (pShapeContext)
        pShapeContext->StartElement(xAttrList);
}

SvXMLImportContext *ScXMLAnnotationContext::CreateChildContext( USHORT nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_DC == nPrefix )
    {
        if( IsXMLToken( rLName, XML_CREATOR ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, sAuthorBuffer);
        else if( IsXMLToken( rLName, XML_DATE ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, sCreateDateBuffer);
    }
    else if( XML_NAMESPACE_META == nPrefix )
    {
        if( IsXMLToken( rLName, XML_DATE_STRING ) )
            pContext = new ScXMLContentContext(GetScImport(), nPrefix,
                                            rLName, xAttrList, sCreateDateStringBuffer);
    }
/*  else if ((nPrefix == XML_NAMESPACE_TEXT) && IsXMLToken(rLName, XML_P) )
    {
        if (!bHasTextP)
        {
            bHasTextP = sal_True;
            sOUText.setLength(0);
        }
        if(nParagraphCount)
            sOUText.append(static_cast<sal_Unicode>('\n'));
        ++nParagraphCount;
        pContext = new ScXMLContentContext( GetScImport(), nPrefix, rLName, xAttrList, sOUText);
    }*/

    if( !pContext && pShapeContext )
        pContext = pShapeContext->CreateChildContext(nPrefix, rLName, xAttrList);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLAnnotationContext::Characters( const ::rtl::OUString& rChars )
{
    if (!bHasTextP)
        sOUText.append(rChars);
}

void ScXMLAnnotationContext::EndElement()
{
    if (pShapeContext)
    {
        pShapeContext->EndElement();
        delete pShapeContext;
    }

    ScMyImportAnnotation* pMyAnnotation = new ScMyImportAnnotation();
    pMyAnnotation->sAuthor = sAuthorBuffer.makeStringAndClear();
    pMyAnnotation->sCreateDate = sCreateDateBuffer.makeStringAndClear();
    if (!pMyAnnotation->sCreateDate.getLength())
        pMyAnnotation->sCreateDate = sCreateDateStringBuffer.makeStringAndClear();
    pMyAnnotation->sText = sOUText.makeStringAndClear();
    pMyAnnotation->bDisplay = bDisplay;

    if (xShape.is() && xShapes.is())
    {
        SvxShape* pShapeImp = SvxShape::getImplementation(xShape);
        if (pShapeImp)
        {
            SdrObject *pSdrObj = pShapeImp->GetSdrObject();
            if (pSdrObj)
            {
                if (bHasPos)
                {
                    pMyAnnotation->pItemSet = pSdrObj->GetMergedItemSet().Clone();
                    awt::Point aPos = xShape->getPosition();
                    awt::Size aSize = xShape->getSize();
                    Rectangle aRect(Point(aPos.X, aPos.Y), Size(aSize.Width, aSize.Height));
                    pMyAnnotation->pRect = new Rectangle(aRect);
                }

                if((pSdrObj->GetOutlinerParaObject()))
                    pMyAnnotation->pOPO = new OutlinerParaObject( *(pSdrObj->GetOutlinerParaObject()) );

                xShapes->remove(xShape);
            }
        }
    }

    XMLTableShapeImportHelper* pTableShapeImport = (XMLTableShapeImportHelper*)GetScImport().GetShapeImport().get();
    pTableShapeImport->SetAnnotation(NULL);

    pCellContext->AddAnnotation(pMyAnnotation);
}

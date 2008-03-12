/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLDetectiveContext.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:15:30 $
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



//___________________________________________________________________

#ifndef _SC_XMLDETECTIVECONTEXT_HXX
#include "XMLDetectiveContext.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif
#ifndef SC_RANGEUTL_HXX
#include "rangeutl.hxx"
#endif

#include <algorithm>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace xmloff::token;


//___________________________________________________________________

ScMyImpDetectiveObj::ScMyImpDetectiveObj() :
    aSourceRange(),
    eObjType( SC_DETOBJ_NONE ),
    bHasError( sal_False )
{
}

//___________________________________________________________________

sal_Bool ScMyImpDetectiveOp::operator<(const ScMyImpDetectiveOp& rDetOp) const
{
    return (nIndex < rDetOp.nIndex);
}

void ScMyImpDetectiveOpArray::Sort()
{
    aDetectiveOpList.sort();
}

sal_Bool ScMyImpDetectiveOpArray::GetFirstOp( ScMyImpDetectiveOp& rDetOp )
{
    if( aDetectiveOpList.empty() )
        return sal_False;
    ScMyImpDetectiveOpList::iterator aItr = aDetectiveOpList.begin();
    rDetOp = *aItr;
    aDetectiveOpList.erase( aItr );
    return sal_True;
}


//___________________________________________________________________

ScXMLDetectiveContext::ScXMLDetectiveContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        ScMyImpDetectiveObjVec* pNewDetectiveObjVec ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDetectiveObjVec( pNewDetectiveObjVec )
{
}

ScXMLDetectiveContext::~ScXMLDetectiveContext()
{
}

SvXMLImportContext *ScXMLDetectiveContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext*     pContext    = NULL;
    const SvXMLTokenMap&    rTokenMap   = GetScImport().GetDetectiveElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLName ) )
    {
        case XML_TOK_DETECTIVE_ELEM_HIGHLIGHTED:
            pContext = new ScXMLDetectiveHighlightedContext( GetScImport(), nPrefix, rLName, xAttrList, pDetectiveObjVec );
        break;
        case XML_TOK_DETECTIVE_ELEM_OPERATION:
            pContext = new ScXMLDetectiveOperationContext( GetScImport(), nPrefix, rLName, xAttrList );
        break;
    }
    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLName );

    return pContext;
}

void ScXMLDetectiveContext::EndElement()
{
}


//___________________________________________________________________

ScXMLDetectiveHighlightedContext::ScXMLDetectiveHighlightedContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        ScMyImpDetectiveObjVec* pNewDetectiveObjVec ):
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pDetectiveObjVec( pNewDetectiveObjVec ),
    aDetectiveObj(),
    bValid( sal_False )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetDetectiveHighlightedAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; ++nIndex )
    {
        const rtl::OUString& sAttrName  (xAttrList->getNameByIndex( nIndex ));
        const rtl::OUString& sValue     (xAttrList->getValueByIndex( nIndex ));
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CELL_RANGE:
            {
                sal_Int32 nOffset(0);
                GetScImport().LockSolarMutex();
                bValid = ScRangeStringConverter::GetRangeFromString( aDetectiveObj.aSourceRange, sValue, GetScImport().GetDocument(), nOffset );
                GetScImport().UnlockSolarMutex();
            }
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_DIRECTION:
                aDetectiveObj.eObjType = ScXMLConverter::GetDetObjTypeFromString( sValue );
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_CONTAINS_ERROR:
                aDetectiveObj.bHasError = IsXMLToken(sValue, XML_TRUE);
            break;
            case XML_TOK_DETECTIVE_HIGHLIGHTED_ATTR_MARKED_INVALID:
                {
                    if (IsXMLToken(sValue, XML_TRUE))
                        aDetectiveObj.eObjType = SC_DETOBJ_CIRCLE;
                }
            break;
        }
    }
}

ScXMLDetectiveHighlightedContext::~ScXMLDetectiveHighlightedContext()
{
}

SvXMLImportContext *ScXMLDetectiveHighlightedContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLDetectiveHighlightedContext::EndElement()
{
    switch( aDetectiveObj.eObjType )
    {
        case SC_DETOBJ_ARROW:
        case SC_DETOBJ_TOOTHERTAB:
        break;
        case SC_DETOBJ_FROMOTHERTAB:
        case SC_DETOBJ_CIRCLE:
            bValid = sal_True;
        break;
        default:
            bValid = sal_False;
    }
    if( bValid )
        pDetectiveObjVec->push_back( aDetectiveObj );
}


//___________________________________________________________________

ScXMLDetectiveOperationContext::ScXMLDetectiveOperationContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    aDetectiveOp(),
    bHasType( sal_False )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetDetectiveOperationAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; ++nIndex )
    {
        const rtl::OUString& sAttrName  (xAttrList->getNameByIndex( nIndex ));
        const rtl::OUString& sValue     (xAttrList->getValueByIndex( nIndex ));
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_DETECTIVE_OPERATION_ATTR_NAME:
                bHasType = ScXMLConverter::GetDetOpTypeFromString( aDetectiveOp.eOpType, sValue );
            break;
            case XML_TOK_DETECTIVE_OPERATION_ATTR_INDEX:
            {
                sal_Int32 nValue;
                if( SvXMLUnitConverter::convertNumber( nValue, sValue, 0 ) )
                    aDetectiveOp.nIndex = nValue;
            }
            break;
        }
    }
    ScUnoConversion::FillScAddress( aDetectiveOp.aPosition, rImport.GetTables().GetRealCellPos() );
}

ScXMLDetectiveOperationContext::~ScXMLDetectiveOperationContext()
{
}

SvXMLImportContext *ScXMLDetectiveOperationContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLDetectiveOperationContext::EndElement()
{
    if( bHasType && (aDetectiveOp.nIndex >= 0) )
        GetScImport().GetDetectiveOpArray()->AddDetectiveOp( aDetectiveOp );
}


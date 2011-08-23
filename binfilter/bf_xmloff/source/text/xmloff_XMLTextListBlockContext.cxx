/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX 
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif

#ifndef _XMLOFF_XMLIMP_HXX 
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLNUMI_HXX 
#include "xmlnumi.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX 
#include "nmspmap.hxx"
#endif

#ifndef _XMLTEXTLISTITEMCONTEXT_HXX
#include "XMLTextListItemContext.hxx"
#endif
#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextListBlockContext.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::binfilter::xmloff::token;

TYPEINIT1( XMLTextListBlockContext, SvXMLImportContext );

XMLTextListBlockContext::XMLTextListBlockContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rTxtImp,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Bool bOrd ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    rTxtImport( rTxtImp ),
    xParentListBlock( rTxtImp.GetListBlock() ),
    nLevel( 0 ),
    nLevels( 0 ),
    bOrdered( bOrd ),
    bRestartNumbering( sal_True ),
    bSetDefaults( sal_False ),
    sNumberingRules( RTL_CONSTASCII_USTRINGPARAM( "NumberingRules" ) )
{
    // Inherit style name from parent list, as well as the flags whether
    // numbering must be restarted and formats have to be created.
    OUString sParentStyleName;
    if( xParentListBlock.Is() )
    {
        XMLTextListBlockContext *pParent =
            (XMLTextListBlockContext *)&xParentListBlock;
        sStyleName = pParent->GetStyleName();
        xNumRules = pParent->GetNumRules();
        sParentStyleName = sStyleName;
        nLevels = pParent->nLevels;
        nLevel = pParent->GetLevel() + 1;
        bRestartNumbering = pParent->IsRestartNumbering();
        bSetDefaults = pParent->bSetDefaults;
    }

    const SvXMLTokenMap& rTokenMap =
        rTxtImport.GetTextListBlockAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_LIST_BLOCK_CONTINUE_NUMBERING:
            bRestartNumbering = !IsXMLToken(rValue, XML_TRUE);
            break;
        case XML_TOK_TEXT_LIST_BLOCK_STYLE_NAME:
            sStyleName = rValue;
            break;
        }
    }

    if( sStyleName.getLength() && sStyleName != sParentStyleName )
    {
        const Reference < XNameContainer >& rNumStyles =
            rTxtImport.GetNumberingStyles();
        if( rNumStyles.is() && rNumStyles->hasByName( sStyleName ) )
        {
            Reference < XStyle > xStyle;
            Any aAny = rNumStyles->getByName( sStyleName );
            aAny >>= xStyle;

            // If the style has not been used, the restart numbering has
            // to be set never.
            if( bRestartNumbering && !xStyle->isInUse() )
                bRestartNumbering = sal_False;

            Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
            aAny = xPropSet->getPropertyValue( sNumberingRules );
            aAny >>= xNumRules;
            nLevels = xNumRules->getCount();
        }
        else
        {
            const SvxXMLListStyleContext *pListStyle =
                rTxtImport.FindAutoListStyle( sStyleName );
            if( pListStyle )
            {
                xNumRules = pListStyle->GetNumRules();
                sal_Bool bUsed = xNumRules.is();
                if( !xNumRules.is() )
                {
                    pListStyle->CreateAndInsertAuto();
                    xNumRules = pListStyle->GetNumRules();
                }
                if( bRestartNumbering && !bUsed )
                    bRestartNumbering = sal_False;
                nLevels = pListStyle->GetLevels();
            }
        }
    }

    if( !xNumRules.is() )
    {
        // If no style name has been specified for this style and for any
        // parent or if no num rule this the specified name is existing,
        // create a new one.

        xNumRules =
            SvxXMLListStyleContext::CreateNumRule( GetImport().GetModel() );
        DBG_ASSERT( xNumRules.is(), "go no numbering rule" );
        if( !xNumRules.is() )
            return;

        nLevels = xNumRules->getCount();

        // Because its a new num rule, numbering mist be restarted never.
        bRestartNumbering = sal_False;
        bSetDefaults = sal_True;
    }

    if( nLevel >= nLevels )
        nLevel = nLevels-1;

    if( bSetDefaults )
    {
        // Because there is no list style sheet for this style, a default
        // format must be set for any level of this num rule.
        SvxXMLListStyleContext::SetDefaultStyle( xNumRules, nLevel,
                                                   bOrdered );
    }

    // Remember this list block.
    rTxtImport.SetListBlock( this );

    // There is no list item by now.
    rTxtImport.SetListItem( 0 );
}

XMLTextListBlockContext::~XMLTextListBlockContext()
{
}

void XMLTextListBlockContext::EndElement()
{
    // Numbering has not to be restarted if it has been restarted within
    // a child list.
    XMLTextListBlockContext *pParent =
        (XMLTextListBlockContext *)&xParentListBlock;
    if( pParent )
    {
        pParent->bRestartNumbering = bRestartNumbering;
    }

    // Restore current list block.
    rTxtImport.SetListBlock( pParent );

    // Any paragraph following the list within the same list item mist not
    // be numbered.
    rTxtImport.SetListItem( 0 );
}

SvXMLImportContext *XMLTextListBlockContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap =
                        rTxtImport.GetTextListBlockElemTokenMap();
    sal_Bool bHeader = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_LIST_HEADER:
        bHeader = sal_True;
    case XML_TOK_TEXT_LIST_ITEM:
        pContext = new XMLTextListItemContext( GetImport(), rTxtImport,
                                                nPrefix, rLocalName,
                                              xAttrList, bHeader );
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}



}//end of namespace binfilter

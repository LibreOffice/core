/*************************************************************************
 *
 *  $RCSfile: XMLTextListBlockContext.cxx,v $
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLE_HPP_
#include <com/sun/star/style/XStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
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
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLTEXTLISTITEMCONTEXT_HXX
#include "XMLTextListItemContext.hxx"
#endif
#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#include "XMLTextListBlockContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;

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
        sRealName = pParent->GetRealName();
        sParentStyleName = sStyleName;
        nLevels = pParent->nLevels;
        nLevel = pParent->GetLevel() + 1;
        bRestartNumbering = pParent->IsRestartNumbering();
        xGenNumRule = pParent->xGenNumRule;
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
            bRestartNumbering = rValue.compareToAscii( sXML_true ) != 0;
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

            Reference<XIndexReplace> xNumRule;
            Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
            aAny = xPropSet->getPropertyValue( sNumberingRules );
            aAny >>= xNumRule;
            nLevels = xNumRule->getCount();
            sRealName = sStyleName;
        }
        else
        {
            const SvxXMLListStyleContext *pListStyle =
                rTxtImport.FindAutoListStyle( sStyleName );
            if( pListStyle )
            {
                sal_Bool bUsed = pListStyle->IsUsed();
                if( !bUsed )
                    pListStyle->CreateAndInsertAuto();
                if( bRestartNumbering && !bUsed )
                    bRestartNumbering = sal_False;
                sRealName = pListStyle->GetRealName();
                nLevels = pListStyle->GetLevels();
            }
        }

    }

    if( !sRealName.getLength() )
    {
        // If no style name has been specified for this style and for any
        // parent or if no num rule this the specified name is existing,
        // create a new one.

        xGenNumRule =
            SvxXMLListStyleContext::CreateNumRule( sRealName,
                                                   GetImport().GetModel() );
        DBG_ASSERT( xGenNumRule.is(), "go no numbering rule" );
        DBG_ASSERT( sRealName.getLength(), "go no numbering rule name" );
        if( !xGenNumRule.is() && !sRealName.getLength() )
            return;

        nLevels = xGenNumRule->getCount();

        // Because its a new num rule, numbering mist be restarted never.
        bRestartNumbering = sal_False;
    }

    if( nLevel >= nLevels )
        nLevel = nLevels-1;

    if( xGenNumRule.is() )
    {
        // Because there is no list style sheet for this style, a default
        // format must be set for any level of this num rule.
        SvxXMLListStyleContext::SetDefaultStyle( xGenNumRule, nLevel,
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




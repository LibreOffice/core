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

#include <sal/config.h>

#include <o3tl/any.hxx>
#include <tools/debug.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlexppr.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/styleexp.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLEventExport.hxx>
#include <xmloff/maptype.hxx>
#include <memory>
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

using ::com::sun::star::document::XEventsSupplier;

XMLStyleExport::XMLStyleExport(
        SvXMLExport& rExp,
        const OUString& rPoolStyleName,
        SvXMLAutoStylePoolP *pAutoStyleP ) :
    rExport( rExp ),
    sIsPhysical( "IsPhysical" ),
    sIsAutoUpdate( "IsAutoUpdate" ),
    sFollowStyle( "FollowStyle" ),
    sNumberingStyleName( "NumberingStyleName" ),
    sOutlineLevel( "OutlineLevel" ),
    sPoolStyleName( rPoolStyleName ),
    pAutoStylePool( pAutoStyleP  )
{
}

XMLStyleExport::~XMLStyleExport()
{
}

void XMLStyleExport::exportStyleAttributes( const Reference< XStyle >& )
{
}

void XMLStyleExport::exportStyleContent( const Reference< XStyle >& )
{
}

bool XMLStyleExport::exportStyle(
        const Reference< XStyle >& rStyle,
        const OUString& rXMLFamily,
        const rtl::Reference < SvXMLExportPropertyMapper >& rPropMapper,
        const Reference< XNameAccess >& xStyles,
        const OUString* pPrefix )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
    Any aAny;

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        if( !*o3tl::doAccess<bool>(aAny) )
            return false;
    }

    // <style:style ...>
    GetExport().CheckAttrList();

    // style:name="..."
    OUString sName;

    if(pPrefix)
        sName = *pPrefix;
    sName += rStyle->getName();

    bool bEncoded = false;
    const OUString sEncodedStyleName(GetExport().EncodeStyleName( sName, &bEncoded ));
    GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, sEncodedStyleName );

    if( bEncoded )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
                                   sName);

    // style:family="..."
    if( !rXMLFamily.isEmpty() )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, rXMLFamily);

    if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
    {
        aAny = xPropSet->getPropertyValue( "Hidden" );
        bool bHidden = false;
        if ( ( aAny >>= bHidden ) && bHidden && GetExport( ).getDefaultVersion( ) == SvtSaveOptions::ODFVER_LATEST )
            GetExport( ).AddAttribute( XML_NAMESPACE_STYLE, XML_HIDDEN, "true" );
    }

    // style:parent-style-name="..."
    OUString sParentString(rStyle->getParentStyle());
    OUString sParent;

    if(!sParentString.isEmpty())
    {
        if(pPrefix)
            sParent = *pPrefix;
        sParent += sParentString;
    }
    else
        sParent = sPoolStyleName;

    if( !sParent.isEmpty() )
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_PARENT_STYLE_NAME,
                                    GetExport().EncodeStyleName( sParent ) );

    // style:next-style-name="..." (paragraph styles only)
    if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
    {
        aAny = xPropSet->getPropertyValue( sFollowStyle );
        OUString sNextName;
        aAny >>= sNextName;
        if( sName != sNextName )
        {
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NEXT_STYLE_NAME,
                          GetExport().EncodeStyleName( sNextName ) );
        }
    }

    // style:auto-update="..." (SW only)
    if( xPropSetInfo->hasPropertyByName( sIsAutoUpdate ) )
    {
        aAny = xPropSet->getPropertyValue( sIsAutoUpdate );
        if( *o3tl::doAccess<bool>(aAny) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_AUTO_UPDATE,
                                      XML_TRUE );
    }

    // style:default-outline-level"..."
    sal_Int32 nOutlineLevel = 0;
    if( xPropSetInfo->hasPropertyByName( sOutlineLevel ) )
    {
        Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
        if( PropertyState_DIRECT_VALUE == xPropState->getPropertyState( sOutlineLevel ) )
        {
            aAny = xPropSet->getPropertyValue( sOutlineLevel );
            aAny >>= nOutlineLevel;
            if( nOutlineLevel > 0 )
            {
                OUStringBuffer sTmp;
                sTmp.append( static_cast<sal_Int32>(nOutlineLevel));
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_DEFAULT_OUTLINE_LEVEL,
                                          sTmp.makeStringAndClear() );
            }
            else
            {
                /* Empty value for style:default-outline-level does exist
                   since ODF 1.2. Thus, suppress its export for former versions. (#i104889#)
                */
                if ( ( GetExport().getExportFlags() & SvXMLExportFlags::OASIS ) &&
                     GetExport().getDefaultVersion() >= SvtSaveOptions::ODFVER_012 )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              XML_DEFAULT_OUTLINE_LEVEL,
                                              OUString( "" ));
                }
            }
        }
    }

    // style:list-style-name="..." (SW paragarph styles only)
    if( xPropSetInfo->hasPropertyByName( sNumberingStyleName ) )
    {
        Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
        if( PropertyState_DIRECT_VALUE ==
                xPropState->getPropertyState( sNumberingStyleName  ) )
        {
            aAny = xPropSet->getPropertyValue( sNumberingStyleName );
            if( aAny.hasValue() )
            {
                OUString sListName;
                aAny >>= sListName;

                /* An direct set empty list style has to be written. Otherwise,
                   this information is lost and causes an error, if the parent
                   style has a list style set. (#i69523#)
                */
                if ( sListName.isEmpty() )
                {
                    GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                              XML_LIST_STYLE_NAME,
                                              sListName /* empty string */);
                }
                else
                {
                    // Written OpenDocument file format doesn't fit to the created text document (#i69627#)
                    bool bSuppressListStyle( false );
                    {
                        if ( !GetExport().writeOutlineStyleAsNormalListStyle() )
                        {
                            Reference< XChapterNumberingSupplier > xCNSupplier
                                (GetExport().GetModel(), UNO_QUERY);

                            OUString sOutlineName;
                            if (xCNSupplier.is())
                            {
                                Reference< XIndexReplace > xNumRule
                                    ( xCNSupplier->getChapterNumberingRules() );
                                assert(xNumRule.is());

                                Reference< XPropertySet > xNumRulePropSet
                                    (xNumRule, UNO_QUERY);
                                xNumRulePropSet->getPropertyValue("Name")
                                    >>= sOutlineName;
                                bSuppressListStyle = sListName == sOutlineName;
                            }
                        }
                    }

                    if ( !sListName.isEmpty() && !bSuppressListStyle )
                    {
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                                  XML_LIST_STYLE_NAME,
                                  GetExport().EncodeStyleName( sListName ) );
                    }
                }
            }
        }
        else if( nOutlineLevel > 0 )
        {

            bool bNoInheritedListStyle( true );

            Reference<XStyle> xStyle( xPropState, UNO_QUERY );
            while ( xStyle.is() )
            {
                OUString aParentStyle( xStyle->getParentStyle() );
                if ( aParentStyle.isEmpty() || !xStyles->hasByName( aParentStyle ) )
                {
                    break;
                }
                else
                {
                    xPropState.set( xStyles->getByName( aParentStyle ), UNO_QUERY );
                    if ( !xPropState.is() )
                    {
                        break;
                    }
                    if ( xPropState->getPropertyState( sNumberingStyleName ) == PropertyState_DIRECT_VALUE )
                    {
                        bNoInheritedListStyle = false;
                        break;
                    }
                    else
                    {
                        xStyle.set( xPropState, UNO_QUERY );
                    }
                }
            }
            if ( bNoInheritedListStyle )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_LIST_STYLE_NAME,
                                          OUString( "" ));
        }
    }

    // style:pool-id="..." is not required any longer since we use
    // english style names only
    exportStyleAttributes( rStyle );

    // TODO: style:help-file-name="..." and style:help-id="..." can neither
    // be modified by UI nor by API and that for, have not to be exported
    // currently.

    {
        // <style:style>
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE, XML_STYLE,
                                  true, true );

        rPropMapper->SetStyleName( sName );

        // <style:properties>
        ::std::vector< XMLPropertyState > aPropStates =
            rPropMapper->Filter( xPropSet, true );
        bool const bUseExtensionNamespaceForGraphicProperties(
                rXMLFamily != "drawing-page" &&
                rXMLFamily != "graphic" &&
                rXMLFamily != "presentation" &&
                rXMLFamily != "chart");
        rPropMapper->exportXML( GetExport(), aPropStates,
                                SvXmlExportFlags::IGN_WS,
                                bUseExtensionNamespaceForGraphicProperties );

        rPropMapper->SetStyleName( OUString() );

        exportStyleContent( rStyle );

        // <script:events>, if they are supported by this style
        Reference<XEventsSupplier> xEventsSupp(rStyle, UNO_QUERY);
        GetExport().GetEventExport().Export(xEventsSupp);
    }
    return true;
}

bool XMLStyleExport::exportDefaultStyle(
        const Reference< XPropertySet >& xPropSet,
          const OUString& rXMLFamily,
        const rtl::Reference < SvXMLExportPropertyMapper >& rPropMapper )
{
    Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();

    // <style:default-style ...>
    GetExport().CheckAttrList();

    {
        // style:family="..."
        if( !rXMLFamily.isEmpty() )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY,
                                      rXMLFamily );
        // <style:style>
        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                  XML_DEFAULT_STYLE,
                                  true, true );
        // <style:properties>
        ::std::vector< XMLPropertyState > aPropStates =
            rPropMapper->FilterDefaults( xPropSet );
        rPropMapper->exportXML( GetExport(), aPropStates,
                                     SvXmlExportFlags::IGN_WS );
    }
    return true;
}

void XMLStyleExport::exportStyleFamily(
    const sal_Char *pFamily,
    const OUString& rXMLFamily,
    const rtl::Reference < SvXMLExportPropertyMapper >& rPropMapper,
    bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    const OUString sFamily(OUString::createFromAscii(pFamily ));
    exportStyleFamily( sFamily, rXMLFamily, rPropMapper, bUsed, nFamily,
                       pPrefix);
}

void XMLStyleExport::exportStyleFamily(
    const OUString& rFamily, const OUString& rXMLFamily,
    const rtl::Reference < SvXMLExportPropertyMapper >& rPropMapper,
    bool bUsed, sal_uInt16 nFamily, const OUString* pPrefix)
{
    assert(GetExport().GetModel().is());
    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(), UNO_QUERY );
    if( !xFamiliesSupp.is() )
        return; // family not available in current model

    Reference< XNameAccess > xStyleCont;

    Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
    if( xFamilies->hasByName( rFamily ) )
        xFamilies->getByName( rFamily ) >>= xStyleCont;

    if( !xStyleCont.is() )
        return;

       // If next styles are supported and used styles should be exported only,
    // the next style may be unused but has to be exported, too. In this case
    // the names of all exported styles are remembered.
    std::unique_ptr<std::set<OUString> > pExportedStyles(nullptr);
    bool bFirstStyle = true;

    const uno::Sequence< OUString> aSeq = xStyleCont->getElementNames();
    const OUString* pIter = aSeq.getConstArray();
    const OUString* pEnd   = pIter + aSeq.getLength();
    for(;pIter != pEnd;++pIter)
    {
        Reference< XStyle > xStyle;
        try
        {
            xStyleCont->getByName( *pIter ) >>= xStyle;
        }
        catch(const lang::IndexOutOfBoundsException&)
        {
            // due to bugs in prior versions it is possible that
            // a binary file is missing some critical styles.
            // The only possible way to deal with this is to
            // not export them here and remain silent.
            continue;
        }
        catch(css::container::NoSuchElementException&)
        {
            continue;
        }

        assert(xStyle.is());
        if (!bUsed || xStyle->isInUse())
        {
            bool bExported = exportStyle( xStyle, rXMLFamily, rPropMapper,
                                          xStyleCont,pPrefix );
            if (bUsed && bFirstStyle && bExported)
            {
                // If this is the first style, find out whether next styles
                // are supported.
                Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
                Reference< XPropertySetInfo > xPropSetInfo =
                    xPropSet->getPropertySetInfo();

                if (xPropSetInfo->hasPropertyByName( sFollowStyle ))
                    pExportedStyles.reset(new std::set<OUString>());
                bFirstStyle = false;
            }

            if (pExportedStyles && bExported)
            {
                // If next styles are supported, remember this style's name.
                pExportedStyles->insert( xStyle->getName() );
            }
        }

        // if an auto style pool is given, remember this style's name as a
        // style name that must not be used by automatic styles.
        if (pAutoStylePool)
            pAutoStylePool->RegisterName( nFamily, xStyle->getName() );
    }

    if( pExportedStyles )
    {
        // if next styles are supported, export all next styles that are
        // unused and that for, haven't been exported in the first loop.
        pIter = aSeq.getConstArray();
        for(;pIter != pEnd;++pIter)
        {
            Reference< XStyle > xStyle;
            xStyleCont->getByName( *pIter ) >>= xStyle;

            assert(xStyle.is());

            Reference< XPropertySet > xPropSet( xStyle, UNO_QUERY );
            Reference< XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

            // styles that aren't existing really are ignored.
            if (xPropSetInfo->hasPropertyByName( sIsPhysical ))
            {
                Any aAny( xPropSet->getPropertyValue( sIsPhysical ) );
                if (!*o3tl::doAccess<bool>(aAny))
                    continue;
            }

            if (!xStyle->isInUse())
                continue;

            if (!xPropSetInfo->hasPropertyByName( sFollowStyle ))
            {
                continue;
            }

            OUString sNextName;
            xPropSet->getPropertyValue( sFollowStyle ) >>= sNextName;
            OUString sTmp( sNextName );
            // if the next style hasn't been exported by now, export it now
            // and remember its name.
            if (xStyle->getName() != sNextName &&
                0 == pExportedStyles->count( sTmp ))
            {
                xStyleCont->getByName( sNextName ) >>= xStyle;
                assert(xStyle.is());

                if (exportStyle(xStyle, rXMLFamily, rPropMapper, xStyleCont, pPrefix))
                    pExportedStyles->insert( sTmp );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

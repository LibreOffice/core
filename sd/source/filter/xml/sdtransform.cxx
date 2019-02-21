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

#include <svl/style.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>

#include <svx/svdoutl.hxx>
#include <editeng/xmlcnitm.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdogrp.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/outlobj.hxx>

#include <drawdoc.hxx>
#include "sdtransform.hxx"

using namespace ::com::sun::star::style;

class SdTransformOOo2xDocument
{
public:
    explicit SdTransformOOo2xDocument( SdDrawDocument& rDocument );

    void transform();

    void transformMasterPages();
    void transformDrawPages();

    void transformStyles();
    void transformStyles( SfxStyleFamily eFam );
    void transformStyle( SfxStyleSheetBase& rSheet );

    void transformShapes( SdrObjList const & rShapes );
    void transformShape( SdrObject& rObj );

    void transformTextShape( SdrTextObj& rTextShape );

    bool getBulletState( const SfxItemSet& rSet, SfxStyleSheetBase* pSheet, bool& rState );
    static bool getBulletState( const SfxItemSet& rSet, sal_uInt16 nWhich, bool& rState );

    static bool transformItemSet( SfxItemSet& rSet, bool bNumbering );

    static bool removeAlienAttributes( SfxItemSet& rSet );
    static bool removeAlienAttributes( SfxItemSet& rSet, sal_uInt16 nWhich );

    SdDrawDocument& mrDocument;
    SdrOutliner& mrOutliner;
};

/** transforms the given model from OOo 2.x to OOo 3.x. This maps
    the deprecated EE_PARA_BULLETSTATE and clears the EE_PARA_LRSPACE
    if used together with a EE_PARA_NUMBULLET */
void TransformOOo2xDocument( SdDrawDocument* pDocument )
{
    if( pDocument )
    {
        SdTransformOOo2xDocument aTransformer( *pDocument );
        aTransformer.transform();
    }
}

static const OUStringLiteral gsEnableNumbering( "enable-numbering" );
static const OUStringLiteral gsTextNamespace( "urn:oasis:names:tc:opendocument:xmlns:text:1.0" );
static const OUStringLiteral gsTrue( "true" );

SdTransformOOo2xDocument::SdTransformOOo2xDocument( SdDrawDocument& rDocument )
: mrDocument( rDocument )
, mrOutliner( rDocument.GetDrawOutliner() )
{
}

void SdTransformOOo2xDocument::transform()
{
    transformMasterPages();
    transformDrawPages();
    transformStyles();
}

void SdTransformOOo2xDocument::transformMasterPages()
{
    sal_uInt16 nMasterPageCount = mrDocument.GetMasterPageCount();
    for( sal_uInt16 nMasterPage = 0; nMasterPage < nMasterPageCount; nMasterPage++ )
    {
        SdrObjList* pPage = mrDocument.GetMasterPage( nMasterPage );
        if( pPage )
            transformShapes( *pPage );
    }
}

void SdTransformOOo2xDocument::transformDrawPages()
{
    sal_uInt16 nPageCount = mrDocument.GetPageCount();
    for( sal_uInt16 nPage = 0; nPage < nPageCount; nPage++ )
    {
        SdrObjList* pPage = mrDocument.GetPage( nPage );
        if( pPage )
            transformShapes( *pPage );
    }
}

void SdTransformOOo2xDocument::transformStyles()
{
    transformStyles( SfxStyleFamily::Para );
    transformStyles( SfxStyleFamily::Page );
}

void SdTransformOOo2xDocument::transformStyles( SfxStyleFamily eFam )
{

    rtl::Reference< SfxStyleSheetBasePool > xStyleSheetPool( mrDocument.GetStyleSheetPool() );

    SfxStyleSheetIterator aIter( xStyleSheetPool.get(), eFam );

    SfxStyleSheetBase* pSheet = aIter.First();
    while( pSheet )
    {
        transformStyle( *pSheet );
        pSheet = aIter.Next();
    }
}

void SdTransformOOo2xDocument::transformStyle( SfxStyleSheetBase& rSheet )
{
    SfxItemSet& rSet = rSheet.GetItemSet();

    bool bState = false;
    getBulletState( rSheet.GetItemSet(), rSheet.GetPool()->Find( rSheet.GetParent(), rSheet.GetFamily() ), bState );

    transformItemSet( rSet, bState );
    removeAlienAttributes( rSet );
}

void SdTransformOOo2xDocument::transformShapes( SdrObjList const & rShapes )
{
    const size_t nShapeCount = rShapes.GetObjCount();
    for( size_t nShape = 0; nShape < nShapeCount; ++nShape )
    {
        SdrObject* pObj = rShapes.GetObj( nShape );
        if( pObj )
            transformShape( *pObj );
    }
}

void SdTransformOOo2xDocument::transformShape( SdrObject& rObj )
{
    SdrTextObj* pTextShape = dynamic_cast< SdrTextObj* >( &rObj );
    if( pTextShape )
    {
        transformTextShape( *pTextShape );
        return;
    }

    SdrObjGroup* pGroupShape = dynamic_cast< SdrObjGroup* >( &rObj );
    if( pGroupShape )
    {
        SdrObjList* pObjList = pGroupShape->GetSubList();
        if( pObjList )
            transformShapes( *pObjList );
        return;
    }
}

void SdTransformOOo2xDocument::transformTextShape( SdrTextObj& rTextShape )
{

    if(rTextShape.IsEmptyPresObj())
        return;

    OutlinerParaObject* pOPO = rTextShape.GetOutlinerParaObject();
    if (!pOPO)
        return;

    mrOutliner.SetText( *pOPO );

    sal_Int32 nCount = mrOutliner.GetParagraphCount();

    bool bChange = false;

    for(sal_Int32 nPara = 0; nPara < nCount; nPara++)
    {
        SfxItemSet aParaSet( mrOutliner.GetParaAttribs( nPara ) );

        bool bItemChange = false;

        bool bState = false;
        const sal_Int16 nDepth = mrOutliner.GetDepth( nPara );
        if( (nDepth != -1) && (!getBulletState( aParaSet, mrOutliner.GetStyleSheet( nPara ), bState ) || !bState) )
        {
            // disable bullet if text::enable-bullet="false" is found
            if( (nDepth > 0 ) && (rTextShape.GetObjInventor()  == SdrInventor::Default) && (rTextShape.GetObjIdentifier() == OBJ_OUTLINETEXT) )
            {
                // for outline object and level > 0 burn in the style sheet because it will be changed to "outline 1"
                SfxStyleSheet* pStyleSheet = mrOutliner.GetStyleSheet( nPara );

                if( pStyleSheet )
                {
                    // optimize me: only put items hard into paragraph that are not equal to "outline 1" style!
                    SfxItemSet& rStyleSet = pStyleSheet->GetItemSet();

                    SfxWhichIter aIter(aParaSet);
                    sal_uInt16 nWhich(aIter.FirstWhich());

                    // now set all none hard attributes from the style
                    while(nWhich)
                    {
                        if(SfxItemState::SET != aParaSet.GetItemState(nWhich))
                        {
                            aParaSet.Put(rStyleSet.Get(nWhich));
                            bItemChange = true;
                        }

                        nWhich = aIter.NextWhich();
                    }
                }
            }

            mrOutliner.SetDepth( mrOutliner.GetParagraph( nPara ), -1 );

            bChange = true;
        }

        bItemChange |= transformItemSet( aParaSet, bState );

        bItemChange |= removeAlienAttributes( aParaSet );

        if( bItemChange )
        {
            mrOutliner.SetParaAttribs( nPara, aParaSet );
            bChange = true;
        }
    }

    if( bChange )
        rTextShape.SetOutlinerParaObject(mrOutliner.CreateParaObject());

    mrOutliner.Clear();
}

bool SdTransformOOo2xDocument::getBulletState( const SfxItemSet& rSet, SfxStyleSheetBase* pSheet, bool& rState )
{
    if( getBulletState( rSet, EE_PARA_XMLATTRIBS, rState ) )
        return true;

    if( getBulletState( rSet, SDRATTR_XMLATTRIBUTES, rState ) )
        return true;

    if( pSheet && getBulletState( pSheet->GetItemSet(), pSheet->GetPool()->Find( pSheet->GetParent(), pSheet->GetFamily() ), rState ) )
        return true;

    return false;
}

bool SdTransformOOo2xDocument::getBulletState( const SfxItemSet& rSet, sal_uInt16 nWhich, bool& rState )
{
    if( rSet.GetItemState( nWhich ) == SfxItemState::SET )
    {
        const SvXMLAttrContainerItem& rAttr = *rSet.GetItem<SvXMLAttrContainerItem>( nWhich );

        const sal_uInt16 nCount = rAttr.GetAttrCount();
        for( sal_uInt16 nItem = 0; nItem < nCount; nItem++ )
        {
            if( ( rAttr.GetAttrLName( nItem ) == gsEnableNumbering ) && ( rAttr.GetAttrNamespace( nItem ) == gsTextNamespace ) )
            {
                const OUString& sValue( rAttr.GetAttrValue( nItem ) );
                rState = sValue == gsTrue;
                return true;
            }
        }
    }

    return false;
}

bool SdTransformOOo2xDocument::transformItemSet( SfxItemSet& rSet, bool bNumbering )
{
    bool bRet = false;
    if( bNumbering )
    {
        SvxLRSpaceItem aItem( *rSet.GetItem<SvxLRSpaceItem>( EE_PARA_LRSPACE ) );
        if( (aItem.GetLeft() != 0) || (aItem.GetTextFirstLineOfst() != 0) )
        {
            aItem.SetLeftValue( 0 );
            aItem.SetTextFirstLineOfst( 0 );
            rSet.Put( aItem );
            bRet = true;
        }
    }

    return bRet;
}

bool SdTransformOOo2xDocument::removeAlienAttributes( SfxItemSet& rSet )
{
    bool b = removeAlienAttributes( rSet, EE_PARA_XMLATTRIBS );
    b |= removeAlienAttributes( rSet, SDRATTR_XMLATTRIBUTES );
    return b;
}

bool SdTransformOOo2xDocument::removeAlienAttributes( SfxItemSet& rSet, sal_uInt16 nWhich )
{
    if( rSet.GetItemState( nWhich ) == SfxItemState::SET )
    {
        const SvXMLAttrContainerItem& rAttr = *rSet.GetItem<SvXMLAttrContainerItem>( nWhich );

        const sal_uInt16 nCount = rAttr.GetAttrCount();
        for( sal_uInt16 nItem = 0; nItem < nCount; nItem++ )
        {
            if( ( rAttr.GetAttrLName( nItem ) == gsEnableNumbering ) && ( rAttr.GetAttrNamespace( nItem ) == gsTextNamespace ) )
            {
                if( nCount == 1 )
                {
                    rSet.ClearItem( nWhich );
                }
                else
                {
                    SvXMLAttrContainerItem aNewItem( nWhich );

                    const sal_uInt16 nFound = nItem;
                    for( nItem = 0; nItem < nCount; nItem++ )
                    {
                        if( nItem != nFound )
                        {
                            OUString const& rNamespace(rAttr.GetAttrNamespace(nItem));
                            OUString const& rPrefix(rAttr.GetAttrPrefix(nItem));
                            if (rPrefix.isEmpty())
                            {
                                aNewItem.AddAttr(rAttr.GetAttrLName(nItem), rAttr.GetAttrValue(nItem));
                            }
                            else
                            {
                                aNewItem.AddAttr(rPrefix, rNamespace, rAttr.GetAttrLName(nItem), rAttr.GetAttrValue(nItem));
                            }
                        }
                    }

                    rSet.Put( aNewItem );
                }
                return true;
            }
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

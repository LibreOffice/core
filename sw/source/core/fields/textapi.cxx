/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <textapi.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>

#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

using namespace com::sun::star;

static const SvxItemPropertySet* ImplGetSvxTextPortionPropertySet()
{
    static const SfxItemPropertyMapEntry aSvxTextPortionPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
        SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {MAP_CHAR_LEN("TextField"),                     EE_FEATURE_FIELD,   &::getCppuType((const uno::Reference< text::XTextField >*)0),   beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextPortionType"),               WID_PORTIONTYPE,    &::getCppuType((const ::rtl::OUString*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN("TextUserDefinedAttributes"),     EE_CHAR_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {MAP_CHAR_LEN("ParaUserDefinedAttributes"),     EE_PARA_XMLATTRIBS,     &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >*)0)  ,        0,     0},
        {0,0,0,0,0,0}
    };
    static SvxItemPropertySet aSvxTextPortionPropertySet( aSvxTextPortionPropertyMap, EditEngine::GetGlobalItemPool() );
    return &aSvxTextPortionPropertySet;
}

SwTextAPIObject::SwTextAPIObject( SwTextAPIEditSource* p )
: SvxUnoText( p, ImplGetSvxTextPortionPropertySet(), uno::Reference < text::XText >() )
, pSource(p)
{
}

SwTextAPIObject::~SwTextAPIObject() throw()
{
    pSource->Dispose();
    delete pSource;
}

struct SwTextAPIEditSource_Impl
{
    // needed for "internal" refcounting
    SfxItemPool*                    mpPool;
    SwDoc*                          mpDoc;
    Outliner*                       mpOutliner;
    SvxOutlinerForwarder*           mpTextForwarder;
    sal_Int32                       mnRef;
};

SwTextAPIEditSource::SwTextAPIEditSource( const SwTextAPIEditSource& rSource )
: SvxEditSource( *this )
{
    // shallow copy; uses internal refcounting
    pImpl = rSource.pImpl;
    pImpl->mnRef++;
}

SvxEditSource* SwTextAPIEditSource::Clone() const
{
    return new SwTextAPIEditSource( *this );
}

void SwTextAPIEditSource::UpdateData()
{
    // data is kept in outliner all the time
}

SwTextAPIEditSource::SwTextAPIEditSource(SwDoc* pDoc)
: pImpl(new SwTextAPIEditSource_Impl)
{
    pImpl->mpPool = &pDoc->GetDocShell()->GetPool();
    pImpl->mpDoc = pDoc;
    pImpl->mpOutliner = 0;
    pImpl->mpTextForwarder = 0;
    pImpl->mnRef = 1;
}

SwTextAPIEditSource::~SwTextAPIEditSource()
{
    if (!--pImpl->mnRef)
        delete pImpl;
}

void SwTextAPIEditSource::Dispose()
{
    pImpl->mpPool=0;
    pImpl->mpDoc=0;
    DELETEZ(pImpl->mpTextForwarder);
    DELETEZ(pImpl->mpOutliner);
}

SvxTextForwarder* SwTextAPIEditSource::GetTextForwarder()
{
    if( !pImpl->mpPool )
        return 0; // mpPool == 0 can be used to flag this as disposed

    if( !pImpl->mpOutliner )
    {
        //init draw model first
        pImpl->mpDoc->GetOrCreateDrawModel();
        pImpl->mpOutliner = new Outliner( pImpl->mpPool, OUTLINERMODE_TEXTOBJECT );
        pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
    }

    if( !pImpl->mpTextForwarder )
        pImpl->mpTextForwarder = new SvxOutlinerForwarder( *pImpl->mpOutliner, 0 );

    return pImpl->mpTextForwarder;
}

void SwTextAPIEditSource::SetText( OutlinerParaObject& rText )
{
    if ( pImpl->mpPool )
    {
        if( !pImpl->mpOutliner )
        {
            //init draw model first
            pImpl->mpDoc->GetOrCreateDrawModel();
            pImpl->mpOutliner = new Outliner( pImpl->mpPool, OUTLINERMODE_TEXTOBJECT );
            pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
        }

        pImpl->mpOutliner->SetText( rText );
    }
}

void SwTextAPIEditSource::SetString( const String& rText )
{
    if ( pImpl->mpPool )
    {
        if( !pImpl->mpOutliner )
        {
            //init draw model first
            pImpl->mpDoc->GetOrCreateDrawModel();
            pImpl->mpOutliner = new Outliner( pImpl->mpPool, OUTLINERMODE_TEXTOBJECT );
            pImpl->mpDoc->SetCalcFieldValueHdl( pImpl->mpOutliner );
        }
        else
            pImpl->mpOutliner->Clear();
        pImpl->mpOutliner->Insert( rText );
    }
}

OutlinerParaObject* SwTextAPIEditSource::CreateText()
{
    if ( pImpl->mpPool && pImpl->mpOutliner )
        return pImpl->mpOutliner->CreateParaObject();
    else
        return 0;
}

String SwTextAPIEditSource::GetText()
{
    if ( pImpl->mpPool && pImpl->mpOutliner )
        return pImpl->mpOutliner->GetEditEngine().GetText();
    else
        return String();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

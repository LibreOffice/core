/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVGREADER_HXX
#define INCLUDED_SVGREADER_HXX

#include <filter/dllapi.h>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>

namespace svgi
{

class SVGReader
{
    const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >        m_xContext;
    const ::com::sun::star::uno::Reference< com::sun::star::io::XInputStream >              m_xInputStream;
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >  m_xDocumentHandler;

public:
    FILTER_DLLPUBLIC SVGReader( const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>&  xContext,
               const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >&               xInputStream,
               const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler );

    FILTER_DLLPUBLIC sal_Bool parseAndConvert();
};

} // namespace svgi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

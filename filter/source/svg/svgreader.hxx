/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *    OpenOffice.org - a multi-platform office productivity suite
 *
 *    Author:
 *      Fridrich Strba  <fridrich.strba@bluewin.ch>
 *      Thorsten Behrens <tbehrens@novell.com>
 *
 *      Copyright (C) 2008, Novell Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

#ifndef INCLUDED_SVGREADER_HXX
#define INCLUDED_SVGREADER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XInputStream.hpp>

namespace svgi
{

class SVGReader
{
    const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >      m_xServiceFactory;
    const ::com::sun::star::uno::Reference< com::sun::star::io::XInputStream >              m_xInputStream;
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >  m_xDocumentHandler;

public:
    SVGReader( const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>&           xServiceFactory,
               const com::sun::star::uno::Reference< com::sun::star::io::XInputStream >&               xInputStream,
               const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& xDocumentHandler );

    sal_Bool parseAndConvert();
};

} // namespace svgi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Fridrich Strba  <fridrich.strba@bluewin.ch>
 *       Thorsten Behrens <tbehrens@novell.com>
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
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

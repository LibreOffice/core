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
 *      Parts copyright 2005 by Sun Microsystems, Inc.
 *
 *   The Contents of this file are made available subject to
 *   the terms of GNU Lesser General Public License Version 3.
 *
 ************************************************************************/

#pragma once
#if 1

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

namespace svgi
{
    /// Creates a XDocumentHandler that serializes directly to an XOutputStream
    ::com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler>
        createSerializer(const ::com::sun::star::uno::Reference<com::sun::star::io::XOutputStream>& );
}

#endif // _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HDL_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

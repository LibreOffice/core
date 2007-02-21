/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLStreamImpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2007-02-21 12:30:10 $
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
#ifndef INCLUDED_OOXML_STREAM_IMPL_HXX
#define INCLUDED_OOXML_STREAM_IMPL_HXX

#ifndef INCLUDED_OOXML_DOCUMENT_HXX
#include <ooxml/OOXMLDocument.hxx>
#endif

#ifndef _COMPHELPER_STORAGEHELPER_HXX
#include <comphelper/storagehelper.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_XRELATIONSHIPACCESS_HPP_
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif

namespace ooxml
{

using namespace com::sun::star;

class OOXMLStreamImpl : public OOXMLStream
{
    uno::Reference<embed::XStorage> mxStorage;
    uno::Reference<embed::XRelationshipAccess> mxRelationshipAccess;
    uno::Reference<uno::XComponentContext> mxContext;
    uno::Reference<io::XInputStream> mxInputStream;
    uno::Reference<io::XStream> mxDocumentStream;
public:
    typedef boost::shared_ptr<OOXMLStreamImpl> Pointer_t;

    OOXMLStreamImpl
    (uno::Reference<uno::XComponentContext> xContext,
     uno::Reference<io::XInputStream> xStream);
    virtual ~OOXMLStreamImpl();

    void init();

    uno::Reference<xml::sax::XParser> getParser();
    uno::Reference<io::XInputStream> getInputStream();
};
}
#endif // INCLUDED_OOXML_STREAM_IMPL_HXX

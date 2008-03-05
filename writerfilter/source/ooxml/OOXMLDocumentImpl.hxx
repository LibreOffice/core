/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OOXMLDocumentImpl.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:03:51 $
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
#ifndef INCLUDED_OOXML_DOCUMENT_IMPL_HXX
#define INCLUDED_OOXML_DOCUMENT_IMPL_HXX

#ifndef INCLUDED_OOXML_DOCUMENT_HXX
#include <ooxml/OOXMLDocument.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XFAST_TOKEN_HANDLER_HPP_
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#endif

#include "OOXMLPropertySet.hxx"

namespace writerfilter {
namespace ooxml
{

using namespace ::com::sun::star;

class OOXMLDocumentImpl : public OOXMLDocument
{
    OOXMLStream::Pointer_t mpStream;
    rtl::OUString msXNoteId;

    uno::Reference<frame::XModel> mxModel;
    uno::Reference<drawing::XShapes> mxShapes;

protected:
    virtual void resolveFastSubStream(Stream & rStream,
                                      OOXMLStream::StreamType_t nType);

    virtual void resolveFastSubStreamWithId(Stream & rStream,
                                      writerfilter::Reference<Stream>::Pointer_t pStream,
                      sal_uInt32 nId);

    writerfilter::Reference<Stream>::Pointer_t
    getSubStream(const rtl::OUString & rId);

    writerfilter::Reference<Stream>::Pointer_t
    getXNoteStream(OOXMLStream::StreamType_t nType,
                   const rtl::OUString & rNoteId);

public:
    OOXMLDocumentImpl(OOXMLStream::Pointer_t pStream);
    virtual ~OOXMLDocumentImpl();

    virtual void resolve(Stream & rStream);

    virtual string getType() const;

    virtual void resolveFootnote(Stream & rStream,
                                 const rtl::OUString & rNoteId);
    virtual void resolveEndnote(Stream & rStream,
                                const rtl::OUString & rNoteId);
    virtual void resolveHeader(Stream & rStream,
                               const sal_Int32 type,
                               const rtl::OUString & rId);
    virtual void resolveFooter(Stream & rStream,
                               const sal_Int32 type,
                               const rtl::OUString & rId);

    virtual void resolveComment(Stream & rStream, const rtl::OUString & rId);

    virtual OOXMLPropertySet * getPicturePropSet
    (const ::rtl::OUString & rId);
    virtual void resolvePicture(Stream & rStream, const rtl::OUString & rId);

    virtual ::rtl::OUString getTargetForId(const ::rtl::OUString & rId);

    virtual void setModel(uno::Reference<frame::XModel> xModel);
    virtual uno::Reference<frame::XModel> getModel();
    virtual void setShapes(uno::Reference<drawing::XShapes> xShapes);
    virtual uno::Reference<drawing::XShapes> getShapes();
    virtual uno::Reference<io::XInputStream> getInputStream();
    virtual uno::Reference<io::XInputStream> getInputStreamForId(const rtl::OUString & rId);
    virtual void setXNoteId(const rtl::OUString & rId);
    virtual const ::rtl::OUString & getXNoteId() const;
};
}}
#endif // OOXML_DOCUMENT_IMPL_HXX

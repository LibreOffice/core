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
#ifndef INCLUDED_OOXML_DOCUMENT_IMPL_HXX
#define INCLUDED_OOXML_DOCUMENT_IMPL_HXX

#include <ooxml/OOXMLDocument.hxx>

#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>

#include "OOXMLPropertySet.hxx"

namespace writerfilter {
namespace ooxml
{

using namespace ::com::sun::star;

class OOXMLDocumentImpl : public OOXMLDocument
{
    OOXMLStream::Pointer_t mpStream;
    rtl::OUString msXNoteId;
    Id mXNoteType;

    uno::Reference<frame::XModel> mxModel;
    uno::Reference<drawing::XDrawPage> mxDrawPage;
    uno::Reference<xml::sax::XFastShapeContextHandler> mxShapeContext;

    bool mbIsSubstream;

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
                   const Id & rType,
                   const rtl::OUString & rNoteId);

    void setIsSubstream( bool bSubstream ) { mbIsSubstream = bSubstream; };

public:
    OOXMLDocumentImpl(OOXMLStream::Pointer_t pStream);
    virtual ~OOXMLDocumentImpl();

    virtual void resolve(Stream & rStream);

    virtual string getType() const;

    virtual void resolveFootnote(Stream & rStream,
                                 const Id & rType,
                                 const rtl::OUString & rNoteId);
    virtual void resolveEndnote(Stream & rStream,
                                const Id & rType,
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
    virtual void setDrawPage(uno::Reference<drawing::XDrawPage> xDrawPage);
    virtual uno::Reference<drawing::XDrawPage> getDrawPage();
    virtual uno::Reference<io::XInputStream> getInputStream();
    virtual uno::Reference<io::XInputStream> getStorageStream();
    virtual uno::Reference<io::XInputStream> getInputStreamForId(const rtl::OUString & rId);
    virtual void setXNoteId(const rtl::OUString & rId);
    virtual const ::rtl::OUString & getXNoteId() const;
    virtual void setXNoteType(const Id & rId);
    virtual const Id & getXNoteType() const;
    virtual const ::rtl::OUString & getTarget() const;
    virtual uno::Reference<xml::sax::XFastShapeContextHandler> getShapeContext( );
    virtual void setShapeContext( uno::Reference<xml::sax::XFastShapeContextHandler> xContext );
};
}}
#endif // OOXML_DOCUMENT_IMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

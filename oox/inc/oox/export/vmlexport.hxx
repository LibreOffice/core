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

#ifndef _OOX_EXPORT_VMLEXPORT_HXX_
#define _OOX_EXPORT_VMLEXPORT_HXX_

#include <oox/dllapi.h>
#include <oox/export/drawingml.hxx>
#include <sax/fshelper.hxx>
#include <filter/msfilter/escherex.hxx>
#include <editeng/outlobj.hxx>

namespace rtl {
    class OString;
    class OStringBuffer;
}

namespace oox {

namespace vml {

/// Interface to be implemented by the parent exporter that knows how to handle shape text.
class OOX_DLLPUBLIC VMLTextExport
{
public:
    virtual void WriteOutliner(const OutlinerParaObject& rParaObj) = 0;
    virtual oox::drawingml::DrawingML& GetDrawingML() = 0;
protected:
    VMLTextExport() {}
    virtual ~VMLTextExport() {}
};

class OOX_DLLPUBLIC VMLExport : public EscherEx
{
    /// Fast serializer to output the data
    ::sax_fastparser::FSHelperPtr m_pSerializer;

    /// Parent exporter, used for text callback.
    VMLTextExport* m_pTextExport;

    /// The object we're exporting.
    const SdrObject* m_pSdrObject;

    /// Fill the shape attributes as they come.
    ::sax_fastparser::FastAttributeList *m_pShapeAttrList;

    /// Remember the shape type.
    sal_uInt32 m_nShapeType;

    /// Remember the shape flags.
    sal_uInt32 m_nShapeFlags;

    /// Remember style, the most important shape attribute ;-)
    rtl::OStringBuffer *m_pShapeStyle;

    /// Remember which shape types we had already written.
    bool *m_pShapeTypeWritten;

public:
                        VMLExport( ::sax_fastparser::FSHelperPtr pSerializer, VMLTextExport* pTextExport = 0 );
    virtual             ~VMLExport();

    ::sax_fastparser::FSHelperPtr
                        GetFS() { return m_pSerializer; }

    /// Export the sdr object as VML.
    ///
    /// Call this when you need to export the object as VML.
    sal_uInt32 AddSdrObject( const SdrObject& rObj );

protected:
    /// Add an attribute to the generated <v:shape/> element.
    ///
    /// This should be called from within StartShape() to ensure that the
    /// added attribute is preserved.
    void                AddShapeAttribute( sal_Int32 nAttribute, const rtl::OString& sValue );

    using EscherEx::StartShape;
    using EscherEx::EndShape;

    /// Start the shape for which we just collected the information.
    ///
    /// Returns the element's tag number, -1 means we wrote nothing.
    virtual sal_Int32   StartShape();

    /// End the shape.
    ///
    /// The parameter is just what we got from StartShape().
    virtual void        EndShape( sal_Int32 nShapeElement );

    virtual void        Commit( EscherPropertyContainer& rProps, const Rectangle& rRect );

private:

    virtual void OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance = 0 );
    virtual void CloseContainer();

    virtual sal_uInt32 EnterGroup( const String& rShapeName, const Rectangle* pBoundRect = 0 );
    virtual void LeaveGroup();

    virtual void AddShape( sal_uInt32 nShapeType, sal_uInt32 nShapeFlags, sal_uInt32 nShapeId = 0 );

private:
    /// Create an OString representing the id from a numerical id.
    static rtl::OString ShapeIdString( sal_uInt32 nId );

    /// Add starting and ending point of a line to the m_pShapeAttrList.
    void AddLineDimensions( const Rectangle& rRectangle );

    /// Add position and size to the OStringBuffer.
    void AddRectangleDimensions( rtl::OStringBuffer& rBuffer, const Rectangle& rRectangle );
};

} // namespace vml

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

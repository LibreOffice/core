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

#ifndef _OOX_EXPORT_VMLEXPORT_HXX_
#define _OOX_EXPORT_VMLEXPORT_HXX_

#include <oox/dllapi.h>
#include <oox/export/drawingml.hxx>
#include <sax/fshelper.hxx>
#include <filter/msfilter/escherex.hxx>
#include <editeng/outlobj.hxx>


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

    /// Anchoring.
    sal_Int16 m_eHOri, m_eVOri, m_eHRel, m_eVRel;

    /// Parent position.
    const Point* m_pNdTopLeft;

    /// The object we're exporting.
    const SdrObject* m_pSdrObject;

    /// Fill the shape attributes as they come.
    ::sax_fastparser::FastAttributeList *m_pShapeAttrList;

    /// Remember the shape type.
    sal_uInt32 m_nShapeType;

    /// Remember the shape flags.
    sal_uInt32 m_nShapeFlags;

    /// Remember style, the most important shape attribute ;-)
    OStringBuffer *m_pShapeStyle;

    /// Remember which shape types we had already written.
    bool *m_pShapeTypeWritten;

public:
                        VMLExport( ::sax_fastparser::FSHelperPtr pSerializer, VMLTextExport* pTextExport = 0 );
    virtual             ~VMLExport();

    ::sax_fastparser::FSHelperPtr
                        GetFS() { return m_pSerializer; }

    void SetFS(::sax_fastparser::FSHelperPtr pSerializer);

    /// Export the sdr object as VML.
    ///
    /// Call this when you need to export the object as VML.
    sal_uInt32 AddSdrObject( const SdrObject& rObj, sal_Int16 eHOri = -1,
            sal_Int16 eVOri = -1, sal_Int16 eHRel = -1,
            sal_Int16 eVRel = -1, const Point* pNdTopLeft = 0 );

protected:
    /// Add an attribute to the generated <v:shape/> element.
    ///
    /// This should be called from within StartShape() to ensure that the
    /// added attribute is preserved.
    void                AddShapeAttribute( sal_Int32 nAttribute, const OString& sValue );

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

    virtual sal_uInt32 EnterGroup( const OUString& rShapeName, const Rectangle* pBoundRect = 0 );
    virtual void LeaveGroup();

    virtual void AddShape( sal_uInt32 nShapeType, sal_uInt32 nShapeFlags, sal_uInt32 nShapeId = 0 );

private:
    /// Create an OString representing the id from a numerical id.
    static OString ShapeIdString( sal_uInt32 nId );

    /// Add flip X and\or flip Y
    void AddFlipXY( );

    /// Add starting and ending point of a line to the m_pShapeAttrList.
    void AddLineDimensions( const Rectangle& rRectangle );

    /// Add position and size to the OStringBuffer.
    void AddRectangleDimensions( OStringBuffer& rBuffer, const Rectangle& rRectangle );
};

} // namespace vml

} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

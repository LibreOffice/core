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

#include <oox/dllapi.h>
#include <sax/fshelper.hxx>
#include <svx/escherex.hxx>

namespace rtl {
    class OString;
    class OStringBuffer;
}

namespace oox {

namespace vml {

class OOX_DLLPUBLIC VMLExport : public EscherEx
{
    /// Fast serializer to output the data
    ::sax_fastparser::FSHelperPtr m_pSerializer;

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
                        VMLExport( ::sax_fastparser::FSHelperPtr pSerializer );
    virtual             ~VMLExport();

    ::sax_fastparser::FSHelperPtr
                        GetFS() { return m_pSerializer; }

    /// Export the sdr object as VML.
    ///
    /// Call this when you need to export the object as VML.
    using EscherEx::AddSdrObject;

protected:
    /// Add an attribute to the generated <v:shape/> element.
    ///
    /// This should be called from within StartShape() to ensure that the
    /// added attribute is preserved.
    void                AddShapeAttribute( sal_Int32 nAttribute, const rtl::OString& sValue );

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

    virtual void OpenContainer( UINT16 nEscherContainer, int nRecInstance = 0 );
    virtual void CloseContainer();

    virtual UINT32 EnterGroup( const String& rShapeName, const Rectangle* pBoundRect = 0 );
    virtual void LeaveGroup();

    virtual void AddShape( UINT32 nShapeType, UINT32 nShapeFlags, UINT32 nShapeId = 0 );

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

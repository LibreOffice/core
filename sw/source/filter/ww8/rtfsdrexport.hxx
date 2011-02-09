/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Miklos Vajna.
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

#ifndef _RTFSdrEXPORT_HXX_
#define _RTFSdrEXPORT_HXX_

#include <filter/msfilter/escherex.hxx>
#include <editeng/outlobj.hxx>
#include <rtl/strbuf.hxx>

#include <map>

class RtfExport;
class RtfAttributeOutput;

class RtfSdrExport : public EscherEx
{
    RtfExport &m_rExport;

    RtfAttributeOutput &m_rAttrOutput;

    const SdrObject* m_pSdrObject;

    /// Remember the shape type.
    sal_uInt32 m_nShapeType;

    /// Remember the shape flags.
    sal_uInt32 m_nShapeFlags;

    /// Remember style, the most important shape attribute ;-)
    rtl::OStringBuffer *m_pShapeStyle;

    std::map<rtl::OString,rtl::OString> m_aShapeProps;

    /// Remember which shape types we had already written.
    bool *m_pShapeTypeWritten;

public:
                        RtfSdrExport( RtfExport &rExport );
    virtual             ~RtfSdrExport();

    /// Export the sdr object as Sdr.
    ///
    /// Call this when you need to export the object as Sdr in RTF.
    sal_uInt32 AddSdrObject( const SdrObject& rObj );

protected:
    /// Add an attribute to the generated shape element.
    ///
    /// This should be called from within StartShape() to ensure that the
    /// added attribute is preserved.
    void                AddShapeAttribute( sal_Int32 nAttribute, const rtl::OString& sValue );

    /// Start the shape for which we just collected the information.
    ///
    /// Returns the element's tag number, -1 means we wrote nothing.
    using EscherEx::StartShape;
    virtual sal_Int32   StartShape();

    /// End the shape.
    ///
    /// The parameter is just what we got from StartShape().
    using EscherEx::EndShape;
    virtual void        EndShape( sal_Int32 nShapeElement );

    virtual void        Commit( EscherPropertyContainer& rProps, const Rectangle& rRect );

private:

    virtual void OpenContainer( sal_uInt16 nEscherContainer, int nRecInstance = 0 );
    virtual void CloseContainer();

    using EscherEx::EnterGroup;
    virtual sal_uInt32 EnterGroup( const String& rShapeName, const Rectangle* pBoundRect = 0 );
    virtual void LeaveGroup();

    virtual void AddShape( sal_uInt32 nShapeType, sal_uInt32 nShapeFlags, sal_uInt32 nShapeId = 0 );

private:
    /// Add starting and ending point of a line to the m_pShapeAttrList.
    void AddLineDimensions( const Rectangle& rRectangle );

    /// Add position and size to the OStringBuffer.
    void AddRectangleDimensions( rtl::OStringBuffer& rBuffer, const Rectangle& rRectangle );

    void WriteOutliner(const OutlinerParaObject& rParaObj);
};

#endif // _RTFSdrEXPORT_HXX_
/* vi:set shiftwidth=4 expandtab: */

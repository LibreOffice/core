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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_RTFSDREXPORT_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_RTFSDREXPORT_HXX

#include <filter/msfilter/escherex.hxx>
#include <editeng/outlobj.hxx>
#include <rtl/strbuf.hxx>

#include <map>
#include <set>

class RtfExport;
class RtfAttributeOutput;
class SwFrameFormat;

/// Handles export of drawings using RTF markup
class RtfSdrExport : public EscherEx
{
    RtfExport& m_rExport;

    RtfAttributeOutput& m_rAttrOutput;

    const SdrObject* m_pSdrObject;

    /// Remember the shape type.
    sal_uInt32 m_nShapeType;

    /// Remember the shape flags.
    sal_uInt32 m_nShapeFlags;

    /// Remember style, the most important shape attribute ;-)
    OStringBuffer m_aShapeStyle;

    std::map<OString,OString> m_aShapeProps;

    /// Remember which shape types we had already written.
    bool* m_pShapeTypeWritten;

    /// List of TextBoxes in this document: they are exported as part of their shape, never alone.
    std::set<const SwFrameFormat*> m_aTextBoxes;

public:
    explicit RtfSdrExport(RtfExport& rExport);
    virtual             ~RtfSdrExport();

    /// Export the sdr object as Sdr.
    ///
    /// Call this when you need to export the object as Sdr in RTF.
    sal_uInt32 AddSdrObject(const SdrObject& rObj);

    /// Is this a standalone TextFrame, or used as a TextBox of a shape?
    bool isTextBox(const SwFrameFormat& rFrameFormat);

protected:
    /// Start the shape for which we just collected the information.
    ///
    /// Returns the element's tag number, -1 means we wrote nothing.
    using EscherEx::StartShape;
    sal_Int32   StartShape();

    /// End the shape.
    ///
    /// The parameter is just what we got from StartShape().
    using EscherEx::EndShape;
    void        EndShape(sal_Int32 nShapeElement);

    virtual void        Commit(EscherPropertyContainer& rProps, const Rectangle& rRect) override;

private:

    virtual void OpenContainer(sal_uInt16 nEscherContainer, int nRecInstance = 0) override;
    virtual void CloseContainer() override;

    virtual sal_uInt32 EnterGroup(const OUString& rShapeName, const Rectangle* pBoundRect = 0) override;
    virtual void LeaveGroup() override;

    virtual void AddShape(sal_uInt32 nShapeType, sal_uInt32 nShapeFlags, sal_uInt32 nShapeId = 0) override;

private:
    /// Add starting and ending point of a line to the m_pShapeAttrList.
    void AddLineDimensions(const Rectangle& rRectangle);

    /// Add position and size to the OStringBuffer.
    void AddRectangleDimensions(OStringBuffer& rBuffer, const Rectangle& rRectangle);

    void WriteOutliner(const OutlinerParaObject& rParaObj);

    /// Exports the pib property of the shape
    void impl_writeGraphic();
};

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_RTFSDREXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

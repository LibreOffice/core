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

#ifndef INCLUDED_VCL_VECTORGRAPHICDATA_HXX
#define INCLUDED_VCL_VECTORGRAPHICDATA_HXX

#include <basegfx/range/b2drange.hxx>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <vcl/bitmapex.hxx>
#include <vcl/wmfexternal.hxx>
#include <rtl/ustring.hxx>
#include <deque>
#include <memory>
#include <algorithm>


typedef css::uno::Sequence<sal_Int8> VectorGraphicDataArray;


// helper to convert any Primitive2DSequence to a good quality BitmapEx,
// using default parameters and graphic::XPrimitive2DRenderer

BitmapEx VCL_DLLPUBLIC convertPrimitive2DSequenceToBitmapEx(
    const std::deque< css::uno::Reference< css::graphic::XPrimitive2D > >& rSequence,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels = 500000);


enum class VectorGraphicDataType
{
    Svg = 0,
    Emf = 1,
    Wmf = 2,
    Pdf = 3
};

class VCL_DLLPUBLIC VectorGraphicData
{
private:
    // the file and length
    VectorGraphicDataArray      maVectorGraphicDataArray;

    // The absolute Path if available
    OUString const              maPath;

    // on demand created content
    bool                        mbSequenceCreated;
    basegfx::B2DRange           maRange;
    std::deque< css::uno::Reference< css::graphic::XPrimitive2D > > maSequence;
    BitmapEx                    maReplacement;
    size_t                      mNestedBitmapSize;
    VectorGraphicDataType const meVectorGraphicDataType;

    // extra:
    std::unique_ptr<WmfExternal> mpExternalHeader;

    // If the vector format has more pages this denotes which page to render
    sal_Int32 mnPageIndex;

    // on demand creators
    void ensurePdfReplacement();
    void ensureReplacement();
    void ensureSequenceAndRange();

    VectorGraphicData(const VectorGraphicData&) = delete;
    VectorGraphicData& operator=(const VectorGraphicData&) = delete;

public:
    VectorGraphicData(
        const VectorGraphicDataArray& rVectorGraphicDataArray,
        const OUString& rPath,
        VectorGraphicDataType eVectorDataType,
        sal_Int32 nPageIndex = -1);
    VectorGraphicData(const OUString& rPath, VectorGraphicDataType eVectorDataType);
    ~VectorGraphicData();

    /// compare op
    bool operator==(const VectorGraphicData& rCandidate) const;

    /// special: needed for emf/wmf, maybe replaced by scaling the result later (?)
    void setWmfExternalHeader(const WmfExternal& aExtHeader);

    /// data read
    const VectorGraphicDataArray& getVectorGraphicDataArray() const { return maVectorGraphicDataArray; }
    sal_uInt32 getVectorGraphicDataArrayLength() const { return maVectorGraphicDataArray.getLength(); }
    enum class State { UNPARSED, PARSED };
    std::pair<State, size_t> getSizeBytes();
    const OUString& getPath() const { return maPath; }
    const VectorGraphicDataType& getVectorGraphicDataType() const { return meVectorGraphicDataType; }

    /// data read and evtl. on demand creation
    const basegfx::B2DRange& getRange() const;
    const std::deque< css::uno::Reference< css::graphic::XPrimitive2D > >& getPrimitive2DSequence() const;
    const BitmapEx& getReplacement() const;
    BitmapChecksum GetChecksum() const;

    sal_Int32 getPageIndex() const { return std::max(sal_Int32(0), mnPageIndex); }

    void setPageIndex(sal_Int32 nPageIndex)
    {
        mnPageIndex = nPageIndex;
    }

    bool isPrimitiveSequenceCreated() const { return mbSequenceCreated; }
};

typedef std::shared_ptr< VectorGraphicData > VectorGraphicDataPtr;

#endif // INCLUDED_VCL_VECTORGRAPHICDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

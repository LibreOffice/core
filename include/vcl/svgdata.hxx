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

#ifndef INCLUDED_VCL_SVGDATA_HXX
#define INCLUDED_VCL_SVGDATA_HXX

#include <basegfx/range/b2drange.hxx>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <com/sun/star/graphic/XPrimitive2D.hpp>
#include <vcl/bitmapex.hxx>
#include <rtl/ustring.hxx>



typedef boost::shared_array< sal_uInt8 > SvgDataArray;
typedef ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XPrimitive2D > Primitive2DReference;
typedef ::com::sun::star::uno::Sequence< Primitive2DReference > Primitive2DSequence;


// helper to convert any Primitive2DSequence to a good quality BitmapEx,
// using default parameters and graphic::XPrimitive2DRenderer

BitmapEx VCL_DLLPUBLIC convertPrimitive2DSequenceToBitmapEx(
    const Primitive2DSequence& rSequence,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels = 500000);



class VCL_DLLPUBLIC SvgData : private boost::noncopyable
{
private:
    // the file and length
    SvgDataArray            maSvgDataArray;
    sal_uInt32              mnSvgDataArrayLength;

    // The absolute Path if available
    OUString           maPath;

    // on demand created content
    basegfx::B2DRange       maRange;
    Primitive2DSequence     maSequence;
    BitmapEx                maReplacement;

    // on demand creators
    void ensureReplacement();
    void ensureSequenceAndRange();

public:
    SvgData(const SvgDataArray& rSvgDataArray, sal_uInt32 nSvgDataArrayLength, const OUString& rPath);
    SvgData(const OUString& rPath);

    /// data read
    const SvgDataArray& getSvgDataArray() const { return maSvgDataArray; }
    sal_uInt32 getSvgDataArrayLength() const { return mnSvgDataArrayLength; }
    const OUString& getPath() const { return maPath; }

    /// data read and evtl. on demand creation
    const basegfx::B2DRange& getRange() const;
    const Primitive2DSequence& getPrimitive2DSequence() const;
    const BitmapEx& getReplacement() const;
};

typedef boost::shared_ptr< SvgData > SvgDataPtr;

#endif // INCLUDED_VCL_SVGDATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_EMFIO_INC_EMFREADER_HXX
#define INCLUDED_EMFIO_INC_EMFREADER_HXX

#include "mtftools.hxx"

namespace emfio
{
    class EmfReader : public MtfTools
    {
    private:
        sal_Int32   mnRecordCount;

        bool        mbRecordPath : 1;
        bool        mbEMFPlus : 1;
        bool        mbEMFPlusDualMode : 1;
        /// Another format is read already, can ignore actual EMF data.
        bool mbReadOtherGraphicFormat = false;
        basegfx::B2DTuple maSizeHint;

        bool        ReadHeader();
        // reads and converts the rectangle
        static tools::Rectangle ReadRectangle(sal_Int32, sal_Int32, sal_Int32, sal_Int32);

    public:
        EmfReader(SvStream& rStreamWMF, GDIMetaFile& rGDIMetaFile);
        ~EmfReader();

        bool ReadEnhWMF();
        void ReadGDIComment(sal_uInt32 nCommentId);
        /// Parses EMR_COMMENT_MULTIFORMATS.
        void ReadMultiformatsComment();
        void SetSizeHint(const basegfx::B2DTuple& rSizeHint) { maSizeHint = rSizeHint; }

    private:
        template <class T> void ReadAndDrawPolyPolygon(sal_uInt32 nNextPos);
        template <class T> void ReadAndDrawPolyLine(sal_uInt32 nNextPos);
        template <class T> tools::Polygon ReadPolygon(sal_uInt32 nStartIndex, sal_uInt32 nPoints, sal_uInt32 nNextPos);
        template <class T> tools::Polygon ReadPolygonWithSkip(const bool skipFirst, sal_uInt32 nNextPos);

        tools::Rectangle ReadRectangle();
        void ReadEMFPlusComment(sal_uInt32 length, bool& bHaveDC);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

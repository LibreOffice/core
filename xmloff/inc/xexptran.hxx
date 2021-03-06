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

#pragma once

#include <rtl/ustring.hxx>
#include <com/sun/star/drawing/HomogenMatrix.hpp>

#include <memory>
#include <vector>

struct ImpSdXMLExpTransObj2DBase;
struct ImpSdXMLExpTransObj3DBase;
class SvXMLUnitConverter;
class SvXMLImport;
class SvXMLExport;

namespace basegfx
{
    class B2DTuple;
    class B2DHomMatrix;
    class B3DHomMatrix;
}

class SdXMLImExTransform2D
{
    // NOTE: This uses shared_ptr, because with unique_ptr the code
    // fails to compile because of incomplete type.
    std::vector< std::shared_ptr< ImpSdXMLExpTransObj2DBase > > maList;
    OUString                                  msString;

public:
    SdXMLImExTransform2D() {}

    void AddRotate(double fNew);
    void AddTranslate(const ::basegfx::B2DTuple& rNew);
    void AddSkewX(double fNew);

    bool NeedsAction() const { return !maList.empty(); }
    void GetFullTransform(::basegfx::B2DHomMatrix& rFullTrans);
    const OUString& GetExportString(const SvXMLUnitConverter& rConv);
    void SetString(const OUString& rNew, const SvXMLUnitConverter& rConv);
};

class SdXMLImExTransform3D
{
    // NOTE: This uses shared_ptr, because with unique_ptr the code
    // fails to compile because of incomplete type.
    std::vector< std::shared_ptr< ImpSdXMLExpTransObj3DBase > > maList;
    OUString                                  msString;

public:
    SdXMLImExTransform3D() {}
    SdXMLImExTransform3D(const OUString& rNew, const SvXMLUnitConverter& rConv);

    void AddMatrix(const ::basegfx::B3DHomMatrix& rNew);

    void AddHomogenMatrix(const css::drawing::HomogenMatrix& xHomMat);
    bool NeedsAction() const { return !maList.empty(); }
    void GetFullTransform(::basegfx::B3DHomMatrix& rFullTrans);
    bool GetFullHomogenTransform(css::drawing::HomogenMatrix& xHomMat);
    const OUString& GetExportString(const SvXMLUnitConverter& rConv);
    void SetString(const OUString& rNew, const SvXMLUnitConverter& rConv);
};

class SdXMLImExViewBox
{
    OUString msString;
    double   mfX;
    double   mfY;
    double   mfW;
    double   mfH;

public:
    SdXMLImExViewBox(double fX, double fY, double fW, double fH);
    SdXMLImExViewBox(const OUString& rNew, const SvXMLUnitConverter& rConv);

    double GetX() const { return mfX; }
    double GetY() const { return mfY; }
    double GetWidth() const { return mfW; }
    double GetHeight() const { return mfH; }
    const OUString& GetExportString();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

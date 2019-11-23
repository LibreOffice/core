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

#ifndef INCLUDED_VCL_METAMASKSCALEACTION_HXX
#define INCLUDED_VCL_METAMASKSCALEACTION_HXX

#include <tools/color.hxx>
#include <tools/gen.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/MetaAction.hxx>

class OutputDevice;
class SvStream;

class VCL_DLLPUBLIC MetaMaskScaleAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Color const         maColor;
    Point               maPt;
    Size                maSz;

public:
                        MetaMaskScaleAction();
    MetaMaskScaleAction(MetaMaskScaleAction const &) = default;
    MetaMaskScaleAction(MetaMaskScaleAction &&) = default;
    MetaMaskScaleAction & operator =(MetaMaskScaleAction const &) = delete; // due to MetaAction
    MetaMaskScaleAction & operator =(MetaMaskScaleAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaMaskScaleAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                             const Bitmap& rBmp,
                                             const Color& rColor );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Color&        GetColor() const { return maColor; }
    const Point&        GetPoint() const { return maPt; }
    const Size&         GetSize() const { return maSz; }
};

#endif // INCLUDED_VCL_METAMASKSCALEACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_VCL_METABMPSCALEPARTACTION_HXX
#define INCLUDED_VCL_METABMPSCALEPARTACTION_HXX

#include <tools/gen.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/MetaAction.hxx>

class OutputDevice;
class SvStream;

class VCL_DLLPUBLIC MetaBmpScalePartAction final : public MetaAction
{
private:

    Bitmap              maBmp;
    Point               maDstPt;
    Size                maDstSz;
    Point               maSrcPt;
    Size                maSrcSz;

public:
                        MetaBmpScalePartAction();
    MetaBmpScalePartAction(MetaBmpScalePartAction const &) = default;
    MetaBmpScalePartAction(MetaBmpScalePartAction &&) = default;
    MetaBmpScalePartAction & operator =(MetaBmpScalePartAction const &) = delete; // due to MetaAction
    MetaBmpScalePartAction & operator =(MetaBmpScalePartAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaBmpScalePartAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

                        MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const Bitmap&       GetBitmap() const { return maBmp; }
    const Point&        GetDestPoint() const { return maDstPt; }
    const Size&         GetDestSize() const { return maDstSz; }
    const Point&        GetSrcPoint() const { return maSrcPt; }
    const Size&         GetSrcSize() const { return maSrcSz; }
};

#endif // INCLUDED_VCL_METABMPSCALEPARTACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

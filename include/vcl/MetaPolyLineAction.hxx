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

#ifndef INCLUDED_VCL_METAPOLYLINEACTION_HXX
#define INCLUDED_VCL_METAPOLYLINEACTION_HXX

#include <tools/poly.hxx>

#include <vcl/lineinfo.hxx>
#include <vcl/MetaAction.hxx>

class OutputDevice;
class SvStream;

class VCL_DLLPUBLIC MetaPolyLineAction final : public MetaAction
{
private:

    LineInfo            maLineInfo;
    tools::Polygon      maPoly;

public:
                        MetaPolyLineAction();
    MetaPolyLineAction(MetaPolyLineAction const &) = default;
    MetaPolyLineAction(MetaPolyLineAction &&) = default;
    MetaPolyLineAction & operator =(MetaPolyLineAction const &) = delete; // due to MetaAction
    MetaPolyLineAction & operator =(MetaPolyLineAction &&) = delete; // due to MetaAction
private:
    virtual             ~MetaPolyLineAction() override;
public:
    virtual void        Execute( OutputDevice* pOut ) override;
    virtual rtl::Reference<MetaAction> Clone() override;
    virtual void        Write( SvStream& rOStm, ImplMetaWriteData* pData ) override;
    virtual void        Read( SvStream& rIStm, ImplMetaReadData* pData ) override;

    explicit            MetaPolyLineAction( const tools::Polygon& );
    explicit            MetaPolyLineAction( const tools::Polygon&, const LineInfo& );

    virtual void        Move( long nHorzMove, long nVertMove ) override;
    virtual void        Scale( double fScaleX, double fScaleY ) override;

    const tools::Polygon& GetPolygon() const { return maPoly; }
    const LineInfo&     GetLineInfo() const { return maLineInfo; }
};

#endif // INCLUDED_VCL_METAPOLYLINEACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

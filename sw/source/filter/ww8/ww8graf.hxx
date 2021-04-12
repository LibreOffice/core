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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8GRAF_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8GRAF_HXX

#include <vector>
#include <stack>

#include <filter/msfilter/msdffimp.hxx>
#include <svx/svdpage.hxx>

#include "writerhelper.hxx"
#include "ww8struc.hxx"

struct EscherShape
{
    sal_uLong mnEscherShapeOrder;
    sal_uLong mnNoInlines;
    bool mbInHellLayer;
    bool mbInHeaderFooter;
    EscherShape( sal_uLong nEscherShapeOrder,
                 bool bInHellLayer,
                 bool _bInHeaderFooter )
        : mnEscherShapeOrder(nEscherShapeOrder),
          mnNoInlines(0),
          mbInHellLayer(bInHellLayer),
          mbInHeaderFooter( _bInHeaderFooter )
    {}
};

class wwZOrderer
{
private:
    // consider that objects in page header/footer
    // are always behind objects in page body. Thus, assure, that in vector
    // <maEscherLayer> objects in page header|footer are inserted before
    // objects in page body - see method <GetEscherObjectPos(..)>.
    //No of objects in doc before starting (always 0 unless using file->insert
    //and probably 0 then as well
    std::vector<EscherShape> maEscherLayer;
    typedef std::vector<EscherShape>::iterator myeiter;

    std::vector<short> maDrawHeight;

    std::stack<sal_uInt16> maIndexes;

    sw::util::SetLayer maSetLayer;

    sal_uLong mnNoInitialObjects;
    sal_uLong mnInlines;
    SdrPage* mpDrawPg;
    const SvxMSDffShapeOrders *mpShapeOrders;

    sal_uInt16 GetEscherObjectIdx(sal_uLong nSpId);
    myeiter MapEscherIdxToIter(sal_uLong nIdx);
    // new parameter <_bInHeaderFooter>, indicating
    // that object is in header or footer
    sal_uLong GetEscherObjectPos( sal_uLong nSpId,
                                  const bool bInHellLayer,
                              const bool _bInHeaderFooter );
    sal_uLong GetDrawingObjectPos(short nWwHeight);
    void InsertObject(SdrObject *pObject, sal_uLong nPos);
public:
    wwZOrderer(const sw::util::SetLayer &rSetLayer, SdrPage* pDrawPg,
        const SvxMSDffShapeOrders *pShapeOrders);
    void InsertTextLayerObject(SdrObject* pObject);
    /*
     We should have separate ZOrder classes for 95- and 97+ and
     instantiate the appropriate one at run time.
     */
    void InsertDrawingObject(SdrObject* pObj, short nWwHeight);
    // new parameter <_bInHeaderFooter>, indicating that object is in header or footer
    void InsertEscherObject( SdrObject* pObject,
                             sal_uLong nSpId,
                             const bool bInHellLayer,
                             const bool _bInHeaderFooter );
    void InsideEscher(sal_uLong nIndex);
    void OutsideEscher();
};

void WW8FSPAShadowToReal(const WW8_FSPA_SHADOW& rFSPAS, WW8_FSPA& rPic);
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

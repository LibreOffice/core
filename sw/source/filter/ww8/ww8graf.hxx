/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef _WW8GRAF_HXX
#define _WW8GRAF_HXX

#include <vector>
#include <stack>
#include "writerhelper.hxx"

struct EscherShape
{
    sal_uLong mnEscherShapeOrder;
    sal_uLong mnNoInlines;
    // --> OD 2004-12-13 #117915# - new member <mbInHeaderFooter>
    bool mbInHeaderFooter;
    EscherShape( sal_uLong nEscherShapeOrder,
                 bool _bInHeaderFooter )
        : mnEscherShapeOrder(nEscherShapeOrder),
          mnNoInlines(0),
          mbInHeaderFooter( _bInHeaderFooter )
    {}
    // <--
};

class wwZOrderer
{
private:
    // --> OD 2004-12-13 #117915# - consider that objects in page header/footer
    // are always behind objects in page body. Thus, assure, that in vector
    // <maEscherLayer> objects in page header|footer are inserted before
    // objects in page body - see method <GetEscherObjectPos(..)>.
    //No of objects in doc before starting (always 0 unless using file->insert
    //and probably 0 then as well
    std::vector<EscherShape> maEscherLayer;
    // <--
    typedef std::vector<EscherShape>::iterator myeiter;

    std::vector<short> maDrawHeight;
    typedef std::vector<short>::iterator myditer;

    std::stack<sal_uInt16> maIndexes;

    sw::util::SetLayer maSetLayer;

    sal_uLong mnNoInitialObjects;
    sal_uLong mnInlines;
    SdrPage* mpDrawPg;
    const SvxMSDffShapeOrders *mpShapeOrders;

    sal_uInt16 GetEscherObjectIdx(sal_uLong nSpId);
    myeiter MapEscherIdxToIter(sal_uLong nIdx);
    // --> OD 2004-12-13 #117915# - new parameter <_bInHeaderFooter>, indicating
    // that object is in header or footer
    sal_uLong GetEscherObjectPos( sal_uLong nSpId,
                              const bool _bInHeaderFooter );
    // <--
    sal_uLong GetDrawingObjectPos(short nWwHeight);
    bool InsertObject(SdrObject *pObject, sal_uLong nPos);
public:
    wwZOrderer(const sw::util::SetLayer &rSetLayer, SdrPage* pDrawPg,
        const SvxMSDffShapeOrders *pShapeOrders);
    void InsertTextLayerObject(SdrObject* pObject);
    /*
     cmc: We should have have separate ZOrder classes for 95- and 97+ and
     instantiate the appropriate one at run time.
     */
    void InsertDrawingObject(SdrObject* pObj, short nWwHeight);
    // --> OD 2004-12-13 #117915# - new parameter <_bInHeaderFooter>, indicating
    // that object is in header or footer
    void InsertEscherObject( SdrObject* pObject,
                             sal_uLong nSpId,
                             const bool _bInHeaderFooter );
    // <--
    void InsideEscher(sal_uLong nIndex);
    void OutsideEscher();
};

void WW8FSPAShadowToReal( WW8_FSPA_SHADOW* pFSPAS, WW8_FSPA* pPic );
#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */

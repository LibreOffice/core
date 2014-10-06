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


#ifndef _SWTBLFMT_HXX
#define _SWTBLFMT_HXX

#include <frmfmt.hxx>

class SwDoc;

class SW_DLLPUBLIC SwTableFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    //UUUU
    SwTableFmt(SwAttrPool& rPool,const sal_Char* pFmtNm, SwFrmFmt *pDrvdFrm);
    SwTableFmt(SwAttrPool& rPool,const String &rFmtNm, SwFrmFmt *pDrvdFrm);

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.
    //UUUU
    virtual ~SwTableFmt();

    //UUUU
    virtual bool supportsFullDrawingLayerFillAttributeSet() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableFmt)
};

class SwTableLineFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    //UUUU
    SwTableLineFmt(SwAttrPool& rPool,const sal_Char* pFmtNm, SwFrmFmt *pDrvdFrm);
    SwTableLineFmt(SwAttrPool& rPool,const String &rFmtNm, SwFrmFmt *pDrvdFrm);

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.
    //UUUU
    virtual ~SwTableLineFmt();

    //UUUU
    virtual bool supportsFullDrawingLayerFillAttributeSet() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableLineFmt)
};

class SW_DLLPUBLIC SwTableBoxFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    //UUUU
    SwTableBoxFmt(SwAttrPool& rPool,const sal_Char* pFmtNm, SwFrmFmt *pDrvdFrm);
    SwTableBoxFmt(SwAttrPool& rPool,const String &rFmtNm, SwFrmFmt *pDrvdFrm);

    // zum Erkennen von Veraenderungen (haupts. TableBoxAttribute)
    virtual void Modify(const SfxPoolItem* pOld,const SfxPoolItem* pNewValue);

public:
    TYPEINFO();     //Bereits in Basisklasse Client drin.
    //UUUU
    virtual ~SwTableBoxFmt();

    //UUUU
    virtual bool supportsFullDrawingLayerFillAttributeSet() const;

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBoxFmt)
};


#endif

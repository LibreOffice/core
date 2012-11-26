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
    SwTableFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {}
    SwTableFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableSetRange )
    {}


public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwTableFmt)
};

class SwTableLineFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    SwTableLineFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {}
    SwTableLineFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableLineSetRange )
    {}

public:
    DECL_FIXEDMEMPOOL_NEWDEL(SwTableLineFmt)
};

class SW_DLLPUBLIC SwTableBoxFmt : public SwFrmFmt
{
    friend class SwDoc;

protected:
    SwTableBoxFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, pFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}
    SwTableBoxFmt( SwAttrPool& rPool, const String &rFmtNm,
                    SwFrmFmt *pDrvdFrm )
        : SwFrmFmt( rPool, rFmtNm, pDrvdFrm, RES_FRMFMT, aTableBoxSetRange )
    {}

    // zum Erkennen von Veraenderungen (haupts. TableBoxAttribute)
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNewValue );

public:

    DECL_FIXEDMEMPOOL_NEWDEL(SwTableBoxFmt)
};


#endif

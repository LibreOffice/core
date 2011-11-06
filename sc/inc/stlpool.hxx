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



#ifndef SC_STLPOOL_HXX
#define SC_STLPOOL_HXX

#include <svl/style.hxx>


class ScStyleSheet;
class ScDocument;

class ScStyleSheetPool : public SfxStyleSheetPool
{
public:
                        ScStyleSheetPool( SfxItemPool&  rPool,
                                          ScDocument*   pDocument );

    void                SetDocument( ScDocument* pDocument );
    ScDocument*         GetDocument() const { return pDoc; }

    virtual void        Remove( SfxStyleSheetBase* pStyle );

    void                SetActualStyleSheet ( SfxStyleSheetBase* pActStyleSheet )
                                { pActualStyleSheet = pActStyleSheet; }

    SfxStyleSheetBase*  GetActualStyleSheet ()
                                { return pActualStyleSheet; }

    void                CreateStandardStyles();
    void                CopyStdStylesFrom( ScStyleSheetPool* pSrcPool );
//UNUSED2008-05  void               UpdateStdNames();

    void                CopyStyleFrom( ScStyleSheetPool* pSrcPool,
                                        const String& rName, SfxStyleFamily eFamily );

    ScStyleSheet*       FindCaseIns( const String& rName, SfxStyleFamily eFam );

//UNUSED2009-05 void                SetForceStdName( const String* pSet );
    const String*       GetForceStdName() const { return pForceStdName; }

    virtual SfxStyleSheetBase& Make( const String&, SfxStyleFamily eFam,
                                     sal_uInt16 nMask = 0xffff, sal_uInt16 nPos = 0xffff );

protected:
    virtual             ~ScStyleSheetPool();

    using SfxStyleSheetPool::Create;    // calcwarnings: Create(const SfxStyleSheet&) - ever used?

    virtual SfxStyleSheetBase* Create( const String&    rName,
                                       SfxStyleFamily   eFamily,
                                       sal_uInt16           nMask);
    virtual SfxStyleSheetBase* Create( const SfxStyleSheetBase& rStyle );

private:
    SfxStyleSheetBase*  pActualStyleSheet;
    ScDocument*         pDoc;
    const String*       pForceStdName;
};

#endif     // SC_STLPOOL_HXX


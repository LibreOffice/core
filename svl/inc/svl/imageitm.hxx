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



#ifndef _SVT_IMAGEITM_HXX
#define _SVT_IMAGEITM_HXX

#include "svl/svldllapi.h"
#include <svl/intitem.hxx>

class String;

struct SfxImageItem_Impl;
class SVL_DLLPUBLIC SfxImageItem : public SfxInt16Item
{
    SfxImageItem_Impl*      pImp;
public:
                            SfxImageItem( sal_uInt16 nWhich = 0, sal_uInt16 nImage = 0 );
                            SfxImageItem( sal_uInt16 nWhich, const String& rURL );
                            SfxImageItem( const SfxImageItem& );
    virtual                 ~SfxImageItem();

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    void                    SetRotation( long nValue );
    long                    GetRotation() const;
    void                    SetMirrored( sal_Bool bSet );
    sal_Bool                    IsMirrored() const;
    String                  GetURL() const;
};

#endif // _SFX_IMAGEITM_HXX

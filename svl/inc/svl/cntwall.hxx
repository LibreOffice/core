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


#ifndef _CNTWALL_HXX
#define _CNTWALL_HXX

#include "svl/svldllapi.h"

#ifndef SHL_HXX
#include <tools/shl.hxx>
#endif
#include <tools/color.hxx>
#include <svl/poolitem.hxx>

class SvStream;

class SVL_DLLPUBLIC CntWallpaperItem : public SfxPoolItem
{
private:
    UniString               _aURL;
    Color                   _nColor;
    sal_uInt16                  _nStyle;

public:
                            CntWallpaperItem( sal_uInt16 nWhich );
                            CntWallpaperItem( sal_uInt16 nWhich, SvStream& rStream, sal_uInt16 nVersion );
                            CntWallpaperItem( const CntWallpaperItem& rCpy );
                            ~CntWallpaperItem();

    virtual sal_uInt16 GetVersion(sal_uInt16) const;

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Create( SvStream&, sal_uInt16 nItemVersion ) const;
    virtual SvStream&       Store( SvStream&, sal_uInt16 nItemVersion ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue  ( const com::sun::star::uno::Any& rVal,
                                         sal_uInt8 nMemberId = 0 );

    void                    SetBitmapURL( const UniString& rURL ) { _aURL = rURL; }
    void                    SetColor( Color nColor ) { _nColor = nColor; }
    void                    SetStyle( sal_uInt16 nStyle ) { _nStyle = nStyle; }

    const UniString&        GetBitmapURL() const { return _aURL; }
    Color                   GetColor() const { return _nColor; }
    sal_uInt16                  GetStyle() const { return _nStyle; }
};

////////////////////////////////////////////////////////////////////////////////

#endif // _CNTWALL_HXX


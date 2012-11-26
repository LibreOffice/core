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


#ifndef _SVX_CHARROTATEITEM_HXX
#define _SVX_CHARROTATEITEM_HXX

// include ---------------------------------------------------------------

#include <svl/intitem.hxx>
#include <editeng/editengdllapi.h>

// class SvxCharRotateItem ----------------------------------------------

/* [Description]

    This item defines a character rotation value (0,1 degree). Currently
    character can only be rotated 90,0 and 270,0 degrees.
    The flag FitToLine defines only a UI-Information -
    if true it must also create a SvxCharScaleItem.

*/

class EDITENG_DLLPUBLIC SvxCharRotateItem : public SfxUInt16Item
{
    sal_Bool bFitToLine;
public:
    POOLITEM_FACTORY()
    SvxCharRotateItem( sal_uInt16 nValue = 0,
                       sal_Bool bFitIntoLine = sal_False,
                       const sal_uInt16 nId = 0);

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16) const;
    virtual SvStream&       Store(SvStream & rStrm, sal_uInt16 nIVer) const;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper * = 0 ) const;

    virtual sal_Bool PutValue( const com::sun::star::uno::Any& rVal,
                                    sal_uInt8 nMemberId );
    virtual sal_Bool QueryValue( com::sun::star::uno::Any& rVal,
                                sal_uInt8 nMemberId ) const;

    inline SvxCharRotateItem& operator=( const SvxCharRotateItem& rItem )
    {
        SetValue( rItem.GetValue() );
        SetFitToLine( rItem.IsFitToLine() );
        return *this;
    }

    virtual int              operator==( const SfxPoolItem& ) const;

    // our currently only degree values
    void SetTopToBotton()                   { SetValue( 2700 ); }
    void SetBottomToTop()                   { SetValue(  900 ); }
    sal_Bool IsTopToBotton() const          { return 2700 == GetValue(); }
    sal_Bool IsBottomToTop() const          { return  900 == GetValue(); }

    sal_Bool IsFitToLine() const            { return bFitToLine; }
    void SetFitToLine( sal_Bool b )         { bFitToLine = b; }
};

#endif


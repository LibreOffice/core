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


#ifndef _SVX_ALGITEM_HXX
#define _SVX_ALGITEM_HXX

// include ---------------------------------------------------------------

#include <svx/svxids.hrc>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <editeng/svxenum.hxx>
#include "svx/svxdllapi.h"

class SvStream;

//------------------------------------------------------------------------



class SVX_DLLPUBLIC SvxHorJustifyItem: public SfxEnumItem
{
public:
    POOLITEM_FACTORY()
    SvxHorJustifyItem( const sal_uInt16 nId = 0);

    SvxHorJustifyItem(
        const SvxCellHorJustify eJustify /*= SVX_HOR_JUSTIFY_STANDARD*/,
        const sal_uInt16 nId );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual sal_uInt16          GetValueCount() const;
    virtual String          GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

    inline  SvxHorJustifyItem& operator=(const SvxHorJustifyItem& rHorJustify)
            {
                SetValue( rHorJustify.GetValue() );
                return *this;
            }
};


//------------------------------------------------------------------------


class SVX_DLLPUBLIC SvxVerJustifyItem: public SfxEnumItem
{
public:
    POOLITEM_FACTORY()
    SvxVerJustifyItem( const sal_uInt16 nId = 0 );

    SvxVerJustifyItem(
        const SvxCellVerJustify eJustify /*= SVX_VER_JUSTIFY_STANDARD*/,
        const sal_uInt16 nId  );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual sal_uInt16          GetValueCount() const;
    virtual String          GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

    inline  SvxVerJustifyItem& operator=(const SvxVerJustifyItem& rVerJustify)
            {
                SetValue( rVerJustify.GetValue() );
                return *this;
            }
};


//------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxOrientationItem: public SfxEnumItem
{
public:
    SvxOrientationItem(
        const SvxCellOrientation eOrientation /*= SVX_ORIENTATION_STANDARD*/,
        const sal_uInt16 nId );

    SvxOrientationItem(
        sal_Int32 nRotation, sal_Bool bStacked,
        const sal_uInt16 nId );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual sal_uInt16          GetValueCount() const;
    virtual String          GetValueText( sal_uInt16 nVal ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*    Create( SvStream& rStream, sal_uInt16 nVer ) const;

    inline  SvxOrientationItem& operator=(const SvxOrientationItem& rOrientation)
            {
                SetValue( rOrientation.GetValue() );
                return *this;
            }

    /** Returns sal_True, if the item represents STACKED state. */
    sal_Bool                    IsStacked() const;
    /** Returns the rotation this item represents (returns nStdAngle for STANDARD and STACKED state). */
    sal_Int32                   GetRotation( sal_Int32 nStdAngle = 0 ) const;
    /** Fills this item according to passed item values. */
    void                    SetFromRotation( sal_Int32 nRotation, sal_Bool bStacked );
};

//------------------------------------------------------------------------

class SVX_DLLPUBLIC SvxMarginItem: public SfxPoolItem
{
    sal_Int16       nLeftMargin;
    sal_Int16       nTopMargin;
    sal_Int16       nRightMargin;
    sal_Int16       nBottomMargin;
public:
    POOLITEM_FACTORY()
    SvxMarginItem( const sal_uInt16 nId = 0 );
    SvxMarginItem( sal_Int16 nLeft, sal_Int16 nTop /*= 0*/,
                   sal_Int16 nRight /*= 0*/, sal_Int16 nBottom /*= 0*/,
                   const sal_uInt16 nId  );
    SvxMarginItem( const SvxMarginItem& );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const;
    virtual SvStream&        Store( SvStream&, sal_uInt16 nItemVersion ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

            sal_Int16           GetLeftMargin() const {return nLeftMargin; }
            sal_Bool            SetLeftMargin(sal_Int16 nLeft);
            sal_Int16           GetTopMargin() const {return nTopMargin; }
            sal_Bool            SetTopMargin(sal_Int16 nTop);
            sal_Int16           GetRightMargin() const {return nRightMargin; }
            sal_Bool            SetRightMargin(sal_Int16 nRight);
            sal_Int16           GetBottomMargin() const {return nBottomMargin; }
            sal_Bool            SetBottomMargin(sal_Int16 nBottom);

    inline  SvxMarginItem& operator=(const SvxMarginItem& rMargin)
            {
                nLeftMargin = rMargin.nLeftMargin;
                nTopMargin = rMargin.nTopMargin;
                nRightMargin = rMargin.nRightMargin;
                nBottomMargin = rMargin.nBottomMargin;
                return *this;
            }
};

#endif


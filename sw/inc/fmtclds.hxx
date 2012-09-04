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
#ifndef _FMTCLDS_HXX
#define _FMTCLDS_HXX

#include <editeng/borderline.hxx>
#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include <hintids.hxx>
#include <format.hxx>
#include <boost/ptr_container/ptr_vector.hpp>

/// ColumnDescriptor
class SwColumn
{
    sal_uInt16 nWish;   /**< Desired width, borders included.
                         It is inversely proportional to the ratio of
                         desired width environment / current width column. */
    sal_uInt16 nUpper;  ///< Top border.
    sal_uInt16 nLower;  ///< Bottom border.
    sal_uInt16 nLeft;   ///< Left border.
    sal_uInt16 nRight;  ///< Right border.

public:
    SwColumn();

    sal_Bool operator==( const SwColumn & ) const;


    void SetWishWidth( sal_uInt16 nNew ) { nWish  = nNew; }
    void SetUpper( sal_uInt16  nNew ) { nUpper = nNew; }
    void SetLower( sal_uInt16  nNew ) { nLower = nNew; }
    void SetLeft ( sal_uInt16  nNew ) { nLeft  = nNew; }
    void SetRight( sal_uInt16  nNew ) { nRight = nNew; }

    sal_uInt16 GetWishWidth() const { return nWish;  }
    sal_uInt16 GetUpper() const { return nUpper; }
    sal_uInt16 GetLower() const { return nLower; }
    sal_uInt16 GetLeft () const { return nLeft; }
    sal_uInt16 GetRight() const { return nRight; }
};

typedef boost::ptr_vector<SwColumn> SwColumns;

enum SwColLineAdj
{
    COLADJ_NONE,
    COLADJ_TOP,
    COLADJ_CENTER,
    COLADJ_BOTTOM
};

class SW_DLLPUBLIC SwFmtCol : public SfxPoolItem
{
    editeng::SvxBorderStyle eLineStyle;     ///< style of the separator line
    sal_uLong   nLineWidth;                 ///< Width of the separator line.
    Color   aLineColor;                     ///< Color of the separator line.

    sal_uInt16   nLineHeight;               /**< Percentile height of lines.
                                          (Based on height of columns including UL). */

    SwColLineAdj eAdj;                      ///< Line will be adjusted top, centered or bottom.

    SwColumns   aColumns;                   ///< Information concerning the columns.
    sal_uInt16  nWidth;                     ///< Total desired width of all columns.
    sal_Int16   aWidthAdjustValue;

    sal_Bool bOrtho;            /**< Only if this flag is set, the setting of GutterWidth will
                             be accompanied by a "visual rearrangement".
                             The flag must be reset if widths of columns or borders are changed.
                             When it is set (again) the visual arrangement is recalculated.
                             The flag is initially set. */

    SW_DLLPRIVATE void Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct );

public:
    SwFmtCol();
    SwFmtCol( const SwFmtCol& );
    ~SwFmtCol();
    //#i120133#
    sal_Int16 GetAdjustValue() const { return aWidthAdjustValue; }
    void SetAdjustValue( const sal_Int16& n ) { aWidthAdjustValue = n; }

    SwFmtCol& operator=( const SwFmtCol& );

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const SwColumns &GetColumns() const { return aColumns; }
          SwColumns &GetColumns()       { return aColumns; }
    sal_uInt16           GetNumCols() const { return aColumns.size(); }

    editeng::SvxBorderStyle     GetLineStyle() const  { return eLineStyle;}
    sal_uLong           GetLineWidth() const  { return nLineWidth;}
    const Color&    GetLineColor() const { return aLineColor;}


    SwColLineAdj     GetLineAdj() const { return eAdj; }
    sal_Bool             IsOrtho()    const { return bOrtho; }
    sal_uInt16           GetWishWidth() const { return nWidth; }
    sal_uInt8            GetLineHeight()const { return nLineHeight; }

    /** @return USHRT_MAX if ambiguous.
     @return smallest width if bMin is true. */
    sal_uInt16 GetGutterWidth( sal_Bool bMin = sal_False ) const;

    void SetLineStyle(editeng::SvxBorderStyle eStyle)        { eLineStyle = eStyle;}
    void SetLineWidth(sal_uLong nLWidth)        { nLineWidth = nLWidth;}
    void SetLineColor(const Color& rCol )   { aLineColor = rCol;}
    void SetLineHeight( sal_uInt8 nNew )     { nLineHeight = nNew; }
    void SetLineAdj( SwColLineAdj eNew ){ eAdj = eNew; }
    void SetWishWidth( sal_uInt16 nNew )    { nWidth = nNew; }

    /** This function allows to (repeatedly) initialize the columns.
     The Ortho flag is set automatically. */
    void Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct );

    /** Adjusts borders for columns in aColumns.
     If flag bOrtho is set, columns are visually re-arranged.
     If the flag is not set, columns widths are not changed and
     borders are adjusted. */
    void SetGutterWidth( sal_uInt16 nNew, sal_uInt16 nAct );

    /** This too re-arranges columns automatically if flag is set.
     Only in this case the second parameter is needed and evaluated. */
    void SetOrtho( sal_Bool bNew, sal_uInt16 nGutterWidth, sal_uInt16 nAct );

    /// For the reader
    void _SetOrtho( sal_Bool bNew ) { bOrtho = bNew; }

    /** Calculates current width of column nCol.
     The ratio of desired width of this column to return value is
     proportional to ratio of total desired value to nAct. */
    sal_uInt16 CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const;

    /** As above except that it @return the width of PrtArea -
     that corresponds to what constitutes the column for the user. */
    sal_uInt16 CalcPrtColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const;
};

inline const SwFmtCol &SwAttrSet::GetCol(sal_Bool bInP) const
    { return (const SwFmtCol&)Get( RES_COL,bInP); }

inline const SwFmtCol &SwFmt::GetCol(sal_Bool bInP) const
    { return aSet.GetCol(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

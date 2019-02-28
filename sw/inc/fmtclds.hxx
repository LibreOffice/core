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
#ifndef INCLUDED_SW_INC_FMTCLDS_HXX
#define INCLUDED_SW_INC_FMTCLDS_HXX

#include <editeng/borderline.hxx>
#include <tools/color.hxx>
#include <tools/solar.h>
#include <svl/poolitem.hxx>
#include "swdllapi.h"
#include "hintids.hxx"
#include "format.hxx"

#include <vector>

/// ColumnDescriptor
class SwColumn
{
    sal_uInt16 m_nWish;   /**< Desired width, borders included.
                         It is inversely proportional to the ratio of
                         desired width environment / current width column. */
    sal_uInt16 m_nLeft;   ///< Left border.
    sal_uInt16 m_nRight;  ///< Right border.

public:
    SwColumn();

    bool operator==( const SwColumn & ) const;

    void SetWishWidth( sal_uInt16 nNew ) { m_nWish  = nNew; }
    void SetLeft ( sal_uInt16  nNew ) { m_nLeft  = nNew; }
    void SetRight( sal_uInt16  nNew ) { m_nRight = nNew; }

    sal_uInt16 GetWishWidth() const { return m_nWish;  }
    sal_uInt16 GetLeft () const { return m_nLeft; }
    sal_uInt16 GetRight() const { return m_nRight; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

typedef std::vector<SwColumn> SwColumns;

enum SwColLineAdj
{
    COLADJ_NONE,
    COLADJ_TOP,
    COLADJ_CENTER,
    COLADJ_BOTTOM
};

class SW_DLLPUBLIC SwFormatCol : public SfxPoolItem
{
    SvxBorderLineStyle m_eLineStyle;     ///< style of the separator line
    sal_uLong   m_nLineWidth;                 ///< Width of the separator line.
    Color   m_aLineColor;                     ///< Color of the separator line.

    sal_uInt16   m_nLineHeight;               /**< Percentile height of lines.
                                          (Based on height of columns including UL). */

    SwColLineAdj m_eAdj;                      ///< Line will be adjusted top, centered or bottom.

    SwColumns   m_aColumns;                   ///< Information concerning the columns.
    sal_uInt16  m_nWidth;                     ///< Total desired width of all columns.
    sal_Int16   m_aWidthAdjustValue;

    bool m_bOrtho;            /**< Only if this flag is set, the setting of GutterWidth will
                             be accompanied by a "visual rearrangement".
                             The flag must be reset if widths of columns or borders are changed.
                             When it is set (again) the visual arrangement is recalculated.
                             The flag is initially set. */

    SAL_DLLPRIVATE void Calc( sal_uInt16 nGutterWidth, sal_uInt16 nAct );

public:
    SwFormatCol();
    SwFormatCol( const SwFormatCol& );
    virtual ~SwFormatCol() override;
    //#i120133#
    sal_Int16 GetAdjustValue() const { return m_aWidthAdjustValue; }
    void SetAdjustValue( sal_Int16 n ) { m_aWidthAdjustValue = n; }

    SwFormatCol& operator=( const SwFormatCol& );

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText,
                                  const IntlWrapper& rIntl ) const override;

    virtual bool QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const SwColumns &GetColumns() const { return m_aColumns; }
          SwColumns &GetColumns()       { return m_aColumns; }
    sal_uInt16           GetNumCols() const { return m_aColumns.size(); }

    SvxBorderLineStyle     GetLineStyle() const  { return m_eLineStyle;}
    sal_uLong           GetLineWidth() const  { return m_nLineWidth;}
    const Color&    GetLineColor() const { return m_aLineColor;}

    SwColLineAdj     GetLineAdj() const { return m_eAdj; }
    bool             IsOrtho()    const { return m_bOrtho; }
    sal_uInt16           GetWishWidth() const { return m_nWidth; }
    sal_uInt8            GetLineHeight()const { return m_nLineHeight; }

    /** @return USHRT_MAX if ambiguous.
     @return smallest width if bMin is true. */
    sal_uInt16 GetGutterWidth( bool bMin = false ) const;

    void SetLineStyle(SvxBorderLineStyle eStyle)        { m_eLineStyle = eStyle;}
    void SetLineWidth(sal_uLong nLWidth)        { m_nLineWidth = nLWidth;}
    void SetLineColor(const Color& rCol )   { m_aLineColor = rCol;}
    void SetLineHeight( sal_uInt8 nNew )     { m_nLineHeight = nNew; }
    void SetLineAdj( SwColLineAdj eNew ){ m_eAdj = eNew; }
    void SetWishWidth( sal_uInt16 nNew )    { m_nWidth = nNew; }

    /** This function allows to (repeatedly) initialize the columns.
     The Ortho flag is set automatically. */
    void Init( sal_uInt16 nNumCols, sal_uInt16 nGutterWidth, sal_uInt16 nAct );

    /** Adjusts borders for columns in aColumns.
     If flag m_bOrtho is set, columns are visually re-arranged.
     If the flag is not set, columns widths are not changed and
     borders are adjusted. */
    void SetGutterWidth( sal_uInt16 nNew, sal_uInt16 nAct );

    /** This too re-arranges columns automatically if flag is set.
     Only in this case the second parameter is needed and evaluated. */
    void SetOrtho( bool bNew, sal_uInt16 nGutterWidth, sal_uInt16 nAct );

    /// For the reader
    void SetOrtho_( bool bNew ) { m_bOrtho = bNew; }

    /** Calculates current width of column nCol.
     The ratio of desired width of this column to return value is
     proportional to ratio of total desired value to nAct. */
    sal_uInt16 CalcColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const;

    /** As above except that it @return the width of PrtArea -
     that corresponds to what constitutes the column for the user. */
    sal_uInt16 CalcPrtColWidth( sal_uInt16 nCol, sal_uInt16 nAct ) const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline const SwFormatCol &SwAttrSet::GetCol(bool bInP) const
    { return Get( RES_COL,bInP); }

inline const SwFormatCol &SwFormat::GetCol(bool bInP) const
    { return m_aSet.GetCol(bInP); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

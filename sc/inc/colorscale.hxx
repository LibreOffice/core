/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#pragma once
#if 1

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <formula/grammar.hxx>
#include <tools/color.hxx>
#include <rangelst.hxx>
#include "conditio.hxx"

#include <vector>

//TODO: merge this with conditio.hxx

class ScDocument;
class ScFormulaCell;
class ScTokenArray;
struct ScDataBarInfo;
class BitmapEx;

// don't change the order
// they are also used in the dialog to determine the position
// in the list box
enum ScColorScaleEntryType
{
    COLORSCALE_AUTO,
    COLORSCALE_MIN,
    COLORSCALE_MAX,
    COLORSCALE_PERCENTILE,
    COLORSCALE_VALUE,
    COLORSCALE_PERCENT,
    COLORSCALE_FORMULA,
};

class SC_DLLPUBLIC ScColorScaleEntry
{
private:
    double mnVal;
    Color maColor;
    boost::scoped_ptr<ScFormulaCell> mpCell;
    ScColorScaleEntryType meType;

public:
    ScColorScaleEntry(double nVal, const Color& rCol);
    ScColorScaleEntry();
    ScColorScaleEntry(const ScColorScaleEntry& rEntry);
    ScColorScaleEntry(ScDocument* pDoc, const ScColorScaleEntry& rEntry);
    ~ScColorScaleEntry();

    const Color& GetColor() const;
    void SetColor(const Color&);
    double GetValue() const;
    void SetValue(double nValue);
    void SetFormula(const rtl::OUString& rFormula, ScDocument* pDoc, const ScAddress& rAddr, formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT);
    void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab, SCTAB nTabNo);
    void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    const ScTokenArray* GetFormula() const;
    rtl::OUString GetFormula( formula::FormulaGrammar::Grammar eGrammar ) const;

    ScColorScaleEntryType GetType() const;
    void SetType( ScColorScaleEntryType eType );

#if DUMP_FORMAT_INFO
    void dumpInfo(rtl::OUStringBuffer& rBuf) const;
#endif
};

namespace databar
{

enum ScAxisPostion
{
    NONE,
    AUTOMATIC,
    MIDDLE
};

}

struct SC_DLLPUBLIC ScDataBarFormatData
{
    ScDataBarFormatData():
        maAxisColor(COL_BLACK),
        mbGradient(true),
        mbNeg(true),
        meAxisPosition(databar::AUTOMATIC),
        mbOnlyBar(false){}

    ScDataBarFormatData(const ScDataBarFormatData& r):
        maPositiveColor(r.maPositiveColor),
        maAxisColor(r.maAxisColor),
        mbGradient(r.mbGradient),
        mbNeg(r.mbNeg),
        meAxisPosition(r.meAxisPosition),
        mbOnlyBar(r.mbOnlyBar)
    {
        if(r.mpNegativeColor)
            mpNegativeColor.reset(new Color(*r.mpNegativeColor));

        if(r.mpLowerLimit)
            mpLowerLimit.reset( new ScColorScaleEntry(*r.mpLowerLimit));
        if(r.mpUpperLimit)
            mpUpperLimit.reset( new ScColorScaleEntry(*r.mpUpperLimit));
    }

    /**
     * Color for all Positive Values and if mbNeg == false also for negative ones
     */
    Color maPositiveColor;
    /**
     * Specifies the color for negative values. This is optional and depends on mbNeg.
     *
     * Default color is 0xFF0000, this value is not set
     */
    boost::scoped_ptr<Color> mpNegativeColor;
    /**
     * Color of the axis if used
     * Default color is black
     */
    Color maAxisColor;
    /**
     * Paint the bars with gradient. If this is used the default is to draw with
     * borders.
     *
     * Default is true
     */
    bool mbGradient;
    /**
     * Use different color for negative values. Color is specified in
     * mpNegativeColor and defaults to 0xFF0000
     *
     * Default is true
     */
    bool mbNeg; //differentiate between negative values
    /**
     * Paint negative values into the same direction as positive values
     * If false we will set the mid point according to the upper and lower limit and negative
     * values are painted to the left and positive to the right
     *
     * Default is false
     */
    databar::ScAxisPostion meAxisPosition;

    /**
     * If TRUE we only show the bar and not the value
     */
    bool mbOnlyBar;

    boost::scoped_ptr<ScColorScaleEntry> mpUpperLimit;
    boost::scoped_ptr<ScColorScaleEntry> mpLowerLimit;
};

enum ScIconSetType
{
    IconSet_3Arrows,
    IconSet_3ArrowsGray,
    IconSet_3Flags,
    IconSet_3TrafficLights1,
    IconSet_3TrafficLights2,
    IconSet_3Signs,
    IconSet_3Symbols,
    IconSet_3Symbols2,
    IconSet_4Arrows,
    IconSet_4ArrowsGray,
    IconSet_4RedToBlack,
    IconSet_4Rating,
    IconSet_4TrafficLights,
    IconSet_5Arrows,
    IconSet_5ArrowsGray,
    IconSet_5Ratings,
    IconSet_5Quarters
};

struct ScIconSetMap {
    const char* pName;
    ScIconSetType eType;
    sal_Int32 nElements;
};

class SC_DLLPUBLIC ScColorFormat : public ScFormatEntry
{
public:
    ScColorFormat(ScDocument* pDoc);
    virtual ~ScColorFormat();

    void SetRange(const ScRangeList& rList);
    const ScRangeList& GetRange() const;

    virtual void DataChanged(const ScRange& rRange) = 0;
    virtual void SetParent(ScConditionalFormat* pParent);

    virtual void startRendering();
    virtual void endRendering();

protected:
    std::vector<double>& getValues() const;

    double getMinValue() const;
    double getMaxValue() const;

    ScConditionalFormat* mpParent;

private:

    struct ScColorFormatCache
    {
        std::vector<double> maValues;
    };
    mutable boost::scoped_ptr<ScColorFormatCache> mpCache;
};

class SC_DLLPUBLIC ScColorScaleFormat : public ScColorFormat
{
private:
    typedef boost::ptr_vector<ScColorScaleEntry> ColorScaleEntries;
    ColorScaleEntries maColorScales;

    double GetMinValue() const;
    double GetMaxValue() const;

    void calcMinMax(double& nMin, double& nMax) const;
    bool CheckEntriesForRel(const ScRange& rRange) const;
    double CalcValue(double nMin, double nMax, ColorScaleEntries::const_iterator& rItr) const;
public:
    ScColorScaleFormat(ScDocument* pDoc);
    ScColorScaleFormat(ScDocument* pDoc, const ScColorScaleFormat& rFormat);
    virtual ~ScColorScaleFormat();
    virtual ScColorFormat* Clone(ScDocument* pDoc = NULL) const;

    Color* GetColor(const ScAddress& rAddr) const;
    void AddEntry(ScColorScaleEntry* pEntry);

    virtual void DataChanged(const ScRange& rRange);
    virtual void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab);
    virtual void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    virtual condformat::ScFormatEntryType GetType() const;
    typedef ColorScaleEntries::iterator iterator;
    typedef ColorScaleEntries::const_iterator const_iterator;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    size_t size() const;

#if DUMP_FORMAT_INFO
    virtual void dumpInfo(rtl::OUStringBuffer& rBuf) const;
#endif
};

class SC_DLLPUBLIC ScDataBarFormat : public ScColorFormat
{
public:
    ScDataBarFormat(ScDocument* pDoc);
    ScDataBarFormat(ScDocument* pDoc, const ScDataBarFormat& rFormat);
    virtual ScColorFormat* Clone(ScDocument* pDoc = NULL) const;

    ScDataBarInfo* GetDataBarInfo(const ScAddress& rAddr) const;

    void SetDataBarData( ScDataBarFormatData* pData );
    const ScDataBarFormatData* GetDataBarData() const;

    virtual void DataChanged(const ScRange& rRange);
    virtual void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab);
    virtual void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    virtual condformat::ScFormatEntryType GetType() const;

#if DUMP_FORMAT_INFO
    virtual void dumpInfo(rtl::OUStringBuffer& rBuf) const;
#endif
private:
    double getMin(double nMin, double nMax) const;
    double getMax(double nMin, double nMax) const;

    boost::scoped_ptr<ScDataBarFormatData> mpFormatData;
};

struct ScIconSetFormatData
{
    ScIconSetType eIconSetType;
    bool mbShowValue;
    /**
     * Specifies whether the icons should be shown in reverse order
     */
    bool mbReverse;
    boost::ptr_vector<ScColorScaleEntry> maEntries;

    ScIconSetFormatData():
        mbShowValue(true),
        mbReverse(false) {}
};

class SC_DLLPUBLIC ScIconSetFormat : public ScColorFormat
{
public:
    ScIconSetFormat(ScDocument* pDoc);
    ScIconSetFormat(ScDocument* pDoc, const ScIconSetFormat& rFormat);

    virtual ScColorFormat* Clone(ScDocument* pDoc = NULL) const;

    ScIconSetInfo* GetIconSetInfo(const ScAddress& rAddr) const;

    void SetIconSetData( ScIconSetFormatData* pData );
    const ScIconSetFormatData* GetIconSetData() const;

    virtual void DataChanged(const ScRange& rRange);
    virtual void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab);
    virtual void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    virtual condformat::ScFormatEntryType GetType() const;

    static ScIconSetMap* getIconSetMap();
    static BitmapEx& getBitmap( ScIconSetType eType, sal_Int32 nIndex );

    typedef boost::ptr_vector<ScColorScaleEntry>::iterator iterator;
    typedef boost::ptr_vector<ScColorScaleEntry>::const_iterator const_iterator;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

#if DUMP_FORMAT_INFO
    virtual void dumpInfo(rtl::OUStringBuffer& rBuf) const;
#endif
private:

    double GetMinValue() const;
    double GetMaxValue() const;
    double CalcValue(double nMin, double nMax, ScIconSetFormat::const_iterator& itr) const;

    boost::scoped_ptr<ScIconSetFormatData> mpFormatData;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

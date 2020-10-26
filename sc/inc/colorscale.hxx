/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_COLORSCALE_HXX
#define INCLUDED_SC_INC_COLORSCALE_HXX

#include <formula/grammar.hxx>
#include <tools/color.hxx>
#include "conditio.hxx"
#include "document.hxx"

#include <memory>
#include <vector>
#include <optional>

//TODO: merge this with conditio.hxx

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
    std::unique_ptr<ScFormulaCell> mpCell;
    std::unique_ptr<ScFormulaListener> mpListener;
    ScConditionalFormat* mpFormat;
    Color maColor;
    ScColorScaleEntryType meType;

    void setListener();

public:
    ScColorScaleEntry(double nVal, const Color& rCol, ScColorScaleEntryType eType = COLORSCALE_VALUE);
    ScColorScaleEntry();
    ScColorScaleEntry(const ScColorScaleEntry& rEntry);
    ScColorScaleEntry(ScDocument* pDoc, const ScColorScaleEntry& rEntry);
    ~ScColorScaleEntry() COVERITY_NOEXCEPT_FALSE;

    const Color& GetColor() const { return maColor;}
    void SetColor(const Color&);
    double GetValue() const;
    void SetValue(double nValue);
    void SetFormula(const OUString& rFormula, ScDocument& rDoc, const ScAddress& rAddr,
            formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT);

    void UpdateReference( const sc::RefUpdateContext& rCxt );
    void UpdateInsertTab( const sc::RefUpdateInsertTabContext& rCxt );
    void UpdateDeleteTab( const sc::RefUpdateDeleteTabContext& rCxt );
    void UpdateMoveTab( const sc::RefUpdateMoveTabContext& rCxt );

    const ScTokenArray* GetFormula() const;
    OUString GetFormula( formula::FormulaGrammar::Grammar eGrammar ) const;

    ScColorScaleEntryType GetType() const { return meType;}
    void SetType( ScColorScaleEntryType eType );

    void SetRepaintCallback(ScConditionalFormat* pParent);
    void SetRepaintCallback(const std::function<void()>& func);
};

namespace databar
{

enum ScAxisPosition
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
        mnMinLength(0),
        mnMaxLength(100),
        mbOnlyBar(false),
        mpUpperLimit(new ScColorScaleEntry()),
        mpLowerLimit(new ScColorScaleEntry())
    {}

    ScDataBarFormatData(const ScDataBarFormatData& r):
        maPositiveColor(r.maPositiveColor),
        maAxisColor(r.maAxisColor),
        mbGradient(r.mbGradient),
        mbNeg(r.mbNeg),
        meAxisPosition(r.meAxisPosition),
        mnMinLength(r.mnMinLength),
        mnMaxLength(r.mnMaxLength),
        mbOnlyBar(r.mbOnlyBar)
    {
        mxNegativeColor = r.mxNegativeColor;

        if(r.mpLowerLimit)
            mpLowerLimit.reset( new ScColorScaleEntry(*r.mpLowerLimit));
        else
            mpLowerLimit.reset(new ScColorScaleEntry());
        if(r.mpUpperLimit)
            mpUpperLimit.reset( new ScColorScaleEntry(*r.mpUpperLimit));
        else
            mpUpperLimit.reset(new ScColorScaleEntry());
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
    std::optional<Color> mxNegativeColor;
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
    databar::ScAxisPosition meAxisPosition;
    /**
     * Minimal length of a databar in percent of cell length
     * Value has to be in the range [0, 100)
     */
    double mnMinLength;
    /**
     * Maximal length of a databar in percent of cell length
     * Value has to be in the range (0, 100]
     */
    double mnMaxLength;

    /**
     * If TRUE we only show the bar and not the value
     */
    bool mbOnlyBar;

    std::unique_ptr<ScColorScaleEntry, o3tl::default_delete<ScColorScaleEntry>> mpUpperLimit;
    std::unique_ptr<ScColorScaleEntry, o3tl::default_delete<ScColorScaleEntry>> mpLowerLimit;
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
    IconSet_3Smilies,
    IconSet_3Stars,
    IconSet_3Triangles,
    IconSet_3ColorSmilies,
    IconSet_4Arrows,
    IconSet_4ArrowsGray,
    IconSet_4RedToBlack,
    IconSet_4Rating,
    IconSet_4TrafficLights,
    IconSet_5Arrows,
    IconSet_5ArrowsGray,
    IconSet_5Ratings,
    IconSet_5Quarters,
    IconSet_5Boxes
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
    virtual ~ScColorFormat() override;

    const ScRangeList& GetRange() const;

    virtual void SetParent(ScConditionalFormat* pParent) override;

    virtual void startRendering() override;
    virtual void endRendering() override;

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
    mutable std::unique_ptr<ScColorFormatCache> mpCache;
};

typedef std::vector<std::unique_ptr<ScColorScaleEntry, o3tl::default_delete<ScColorScaleEntry>>> ScColorScaleEntries;

class SC_DLLPUBLIC ScColorScaleFormat final : public ScColorFormat
{
private:
    ScColorScaleEntries maColorScales;

    double GetMinValue() const;
    double GetMaxValue() const;

    void calcMinMax(double& nMin, double& nMax) const;
    double CalcValue(double nMin, double nMax, const ScColorScaleEntries::const_iterator& rItr) const;
public:
    ScColorScaleFormat(ScDocument* pDoc);
    ScColorScaleFormat(ScDocument* pDoc, const ScColorScaleFormat& rFormat);
    virtual ~ScColorScaleFormat() override;
    virtual ScColorFormat* Clone(ScDocument* pDoc) const override;

    virtual void SetParent(ScConditionalFormat* pParent) override;

    std::optional<Color> GetColor(const ScAddress& rAddr) const;
    void AddEntry(ScColorScaleEntry* pEntry);

    virtual void UpdateReference( sc::RefUpdateContext& rCxt ) override;
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt ) override;
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt ) override;
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt ) override;

    virtual Type GetType() const override;
    ScColorScaleEntries::iterator begin();
    ScColorScaleEntries::const_iterator begin() const;
    ScColorScaleEntries::iterator end();
    ScColorScaleEntries::const_iterator end() const;

    ScColorScaleEntry* GetEntry(size_t nPos);
    const ScColorScaleEntry* GetEntry(size_t nPos) const;

    size_t size() const;

    /**
     * Makes sure that the maColorScales contain at least 2 entries.
     * Should be called when it is not sure that the format is valid.
     */
    void EnsureSize();
};

class SC_DLLPUBLIC ScDataBarFormat final : public ScColorFormat
{
public:
    ScDataBarFormat(ScDocument* pDoc);
    ScDataBarFormat(ScDocument* pDoc, const ScDataBarFormat& rFormat);
    virtual ScColorFormat* Clone(ScDocument* pDoc) const override;

    virtual void SetParent(ScConditionalFormat* pParent) override;

    std::unique_ptr<ScDataBarInfo> GetDataBarInfo(const ScAddress& rAddr) const;

    void SetDataBarData( ScDataBarFormatData* pData );
    const ScDataBarFormatData* GetDataBarData() const;
    ScDataBarFormatData* GetDataBarData();

    virtual void UpdateReference( sc::RefUpdateContext& rCxt ) override;
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt ) override;
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt ) override;
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt ) override;

    virtual Type GetType() const override;

    /**
     * Makes sure that the mpFormatData does not contain valid entries.
     * Should be called when it is not sure that the format is valid.
     */
    void EnsureSize();

private:
    double getMin(double nMin, double nMax) const;
    double getMax(double nMin, double nMax) const;

    std::unique_ptr<ScDataBarFormatData> mpFormatData;
};

struct ScIconSetFormatData
{
    ScIconSetType eIconSetType;
    bool mbShowValue;
    /**
     * Specifies whether the icons should be shown in reverse order
     */
    bool mbReverse;
    typedef std::vector<std::unique_ptr<ScColorScaleEntry, o3tl::default_delete<ScColorScaleEntry>>> Entries_t;
    Entries_t m_Entries;
    bool mbCustom;
    // the std::pair points to exactly one image
    // std..pair::second == -1 means no image
    std::vector<std::pair<ScIconSetType, sal_Int32> > maCustomVector;

    ScIconSetFormatData(ScIconSetType eType = IconSet_3Arrows):
        eIconSetType(eType),
        mbShowValue(true),
        mbReverse(false),
        mbCustom(false)
        {}

    ScIconSetFormatData(ScIconSetFormatData const&);
    ScIconSetFormatData& operator=(ScIconSetFormatData const&) = delete; //TODO?
};

class SC_DLLPUBLIC ScIconSetFormat final : public ScColorFormat
{
public:
    ScIconSetFormat(ScDocument* pDoc);
    ScIconSetFormat(ScDocument* pDoc, const ScIconSetFormat& rFormat);

    virtual ScColorFormat* Clone(ScDocument* pDoc) const override;

    virtual void SetParent(ScConditionalFormat* pParent) override;

    std::unique_ptr<ScIconSetInfo> GetIconSetInfo(const ScAddress& rAddr) const;

    void SetIconSetData( ScIconSetFormatData* pData );
    const ScIconSetFormatData* GetIconSetData() const;
    ScIconSetFormatData* GetIconSetData();

    virtual void UpdateReference( sc::RefUpdateContext& rCxt ) override;
    virtual void UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt ) override;
    virtual void UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt ) override;
    virtual void UpdateMoveTab( sc::RefUpdateMoveTabContext& rCxt ) override;

    virtual Type GetType() const override;

    static const ScIconSetMap g_IconSetMap[];
    static const char* getIconSetName( ScIconSetType eType );
    static sal_Int32 getIconSetElements( ScIconSetType eType );
    static OUString getIconName(ScIconSetType eType, sal_Int32 nIndex);
    static BitmapEx& getBitmap(sc::IconSetBitmapMap& rBitmapMap, ScIconSetType eType, sal_Int32 nIndex);

    typedef ScIconSetFormatData::Entries_t::iterator iterator;
    typedef ScIconSetFormatData::Entries_t::const_iterator const_iterator;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    size_t size() const;

    /**
     * Makes sure that the mpFormatData does not contain valid entries.
     * Should be called when it is not sure that the format is valid.
     */
    void EnsureSize();

private:

    double GetMinValue() const;
    double GetMaxValue() const;
    double CalcValue(double nMin, double nMax, const ScIconSetFormat::const_iterator& itr) const;

    std::unique_ptr<ScIconSetFormatData> mpFormatData;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

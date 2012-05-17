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

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <formula/grammar.hxx>
#include <tools/color.hxx>
#include <rangelst.hxx>

//TODO: merge this with conditio.hxx

class ScDocument;
class ScFormulaCell;
class ScTokenArray;
class ScDataBarInfo;

struct ScDataBarFormatData
{
    Color maPositiveColor;
    bool mbGradient;
};

class SC_DLLPUBLIC ScColorScaleEntry
{
private:
    double mnVal;
    Color maColor;
    boost::scoped_ptr<ScFormulaCell> mpCell;

    bool mbMin;
    bool mbMax;
    bool mbPercent;
public:
    ScColorScaleEntry(double nVal, const Color& rCol);
    ScColorScaleEntry(const ScColorScaleEntry& rEntry);
    ScColorScaleEntry(ScDocument* pDoc, const ScColorScaleEntry& rEntry);
    ~ScColorScaleEntry();

    const Color& GetColor() const;
    double GetValue() const;
    void SetFormula(const rtl::OUString& rFormula, ScDocument* pDoc, const ScAddress& rAddr, formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_DEFAULT);
    void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab, SCTAB nTabNo);
    void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    bool GetMin() const;
    bool GetMax() const;
    bool GetPercent() const;
    bool HasFormula() const;
    const ScTokenArray* GetFormula() const;
    void SetMin(bool bMin);
    void SetMax(bool bMax);
    void SetPercent(bool bPercent);
};

enum ScColorFormatType
{
    COLORSCALE,
    DATABAR
};

class SC_DLLPUBLIC ScColorFormat
{
public:
    ScColorFormat(ScDocument* pDoc);
    ScColorFormat(ScDocument* pDoc, const ScColorFormat& rFormat);
    virtual ~ScColorFormat();

    void SetRange(const ScRangeList& rList);
    const ScRangeList& GetRange() const;

    virtual void DataChanged(const ScRange& rRange) = 0;
    virtual void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab) = 0;
    virtual void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz ) = 0;

    virtual ScColorFormat* Clone(ScDocument* pDoc = NULL) const = 0;

    virtual ScColorFormatType GetType() const = 0;

protected:
    ScRangeList maRanges;
    ScDocument* mpDoc;
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

    virtual ScColorFormatType GetType() const;
    typedef ColorScaleEntries::iterator iterator;
    typedef ColorScaleEntries::const_iterator const_iterator;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
};

class SC_DLLPUBLIC ScDataBarFormat : public ScColorFormat
{
public:
    ScDataBarFormat(ScDocument* pDoc);
    ScDataBarFormat(ScDocument* pDoc, const ScDataBarFormat& rFormat);

    ScDataBarInfo* GetDataBarInfo(const ScAddress& rAddr) const;

    virtual void DataChanged(const ScRange& rRange);
    virtual void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab);
    virtual void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    virtual ScColorFormatType GetType() const;
private:
    ScDataBarFormatData maFormatData;
};

class SC_DLLPUBLIC ScColorFormatList
{
private:
    typedef boost::ptr_vector<ScColorFormat> ColorFormatContainer;
    ColorFormatContainer maColorScaleFormats;
public:
    ScColorFormatList() {};
    ScColorFormatList(ScDocument* pDoc, const ScColorFormatList& rList);

    typedef ColorFormatContainer::iterator iterator;
    typedef ColorFormatContainer::const_iterator const_iterator;

    ScColorFormat* GetFormat(sal_uInt32 nFormat);
    void AddFormat( ScColorFormat* pFormat );

    void DataChanged(const ScRange& rRange);
    void UpdateMoveTab(SCTAB nOldTab, SCTAB nNewTab);
    void UpdateReference( UpdateRefMode eUpdateRefMode,
            const ScRange& rRange, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    size_t size() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

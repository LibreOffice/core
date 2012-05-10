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
#include <tools/color.hxx>
#include <rangelst.hxx>

//TODO: merge this with conditio.hxx

class ScDocument;

class SC_DLLPUBLIC ScColorScaleEntry
{
private:
    double mnVal;
    Color maColor;

    bool mbMin;
    bool mbMax;
    bool mbPercent;
public:
    ScColorScaleEntry(double nVal, const Color& rCol);
    ScColorScaleEntry(const ScColorScaleEntry& rEntry);

    const Color& GetColor() const;
    double GetValue() const;

    bool GetMin() const;
    bool GetMax() const;
    bool GetPercent() const;
    void SetMin(bool bMin);
    void SetMax(bool bMax);
    void SetPercent(bool bPercent);
};

class SC_DLLPUBLIC ScColorScaleFormat
{
private:
    ScRangeList maRanges;
    ScDocument* mpDoc;
    typedef boost::ptr_vector<ScColorScaleEntry> ColorScaleEntries;
    ColorScaleEntries maColorScales;

    double GetMinValue() const;
    double GetMaxValue() const;

    void calcMinMax(double& nMin, double& nMax) const;
public:
    ScColorScaleFormat(ScDocument* pDoc);

    Color* GetColor(const ScAddress& rAddr) const;
    void AddEntry(ScColorScaleEntry* pEntry);
    void SetRange(const ScRangeList& rList);

    typedef ColorScaleEntries::iterator iterator;
    typedef ColorScaleEntries::const_iterator const_iterator;
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
};

class ScColorScaleFormatList
{
private:
    typedef boost::ptr_vector<ScColorScaleFormat> ColorScaleFormatContainer;
    boost::ptr_vector<ScColorScaleFormat> maColorScaleFormats;
public:
    ScColorScaleFormatList() {};

    typedef ColorScaleFormatContainer::iterator iterator;
    typedef ColorScaleFormatContainer::const_iterator const_iterator;

    ScColorScaleFormat* GetFormat(sal_uInt32 nFormat);
    void AddFormat( ScColorScaleFormat* pFormat );

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    size_t size() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

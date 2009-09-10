/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interpre.hxx,v $
 * $Revision: 1.35.44.2 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_DOUBLEREF_HXX
#define SC_DOUBLEREF_HXX

#include "address.hxx"

class ScDocument;
class ScBaseCell;
class ScDBQueryParamBase;
class ScQueryParamBase;

// ============================================================================

class ScDBRangeBase
{
public:
    enum RefType { INTERNAL, EXTERNAL };

    virtual ~ScDBRangeBase() = 0;

    RefType getType() const;

    virtual SCCOL getFirstFieldColumn() const = 0;
    virtual SCCOL findFieldColumn(SCCOL nColIndex) const = 0;
    virtual SCCOL findFieldColumn(const ::rtl::OUString& rStr, sal_uInt16* pErr = NULL) const = 0;
    virtual ScDBQueryParamBase* createQueryParam(const ScDBRangeBase* pQueryRef) const = 0;
    virtual bool isRangeEqual(const ScRange& rRange) const = 0;
    virtual bool fillQueryEntries(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef) const= 0;

protected:
    ScDBRangeBase(ScDocument* pDoc, RefType eType);
    ScDocument* getDoc() const;

private:
    ScDBRangeBase(); // disabled

    ScDocument* mpDoc;
    RefType meType;
};

// ============================================================================

class ScDBInternalRange : public ScDBRangeBase
{
public:
    explicit ScDBInternalRange(ScDocument* pDoc, const ScRange& rRange);
    virtual ~ScDBInternalRange();

    const ScRange& getRange() const;

    virtual SCCOL getFirstFieldColumn() const;
    virtual SCCOL findFieldColumn(SCCOL nColIndex) const;
    virtual SCCOL findFieldColumn(const ::rtl::OUString& rStr, sal_uInt16* pErr = NULL) const;
    virtual ScDBQueryParamBase* createQueryParam(const ScDBRangeBase* pQueryRef) const;
    virtual bool isRangeEqual(const ScRange& rRange) const;
    virtual bool fillQueryEntries(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef) const;

private:
    sal_uInt16 getCellString(String& rStr, ScBaseCell* pCell) const;

private:
    ScRange maRange;
};

// ============================================================================

class ScDBExternalRange : public ScDBRangeBase
{
public:
    explicit ScDBExternalRange(ScDocument* pDoc);
    virtual ~ScDBExternalRange();

    virtual SCCOL getFirstFieldColumn() const;
    virtual SCCOL findFieldColumn(SCCOL nColIndex) const;
    virtual SCCOL findFieldColumn(const ::rtl::OUString& rStr, sal_uInt16* pErr = NULL) const;
    virtual ScDBQueryParamBase* createQueryParam(const ScDBRangeBase* pQueryRef) const;
    virtual bool isRangeEqual(const ScRange& rRange) const;
    virtual bool fillQueryEntries(ScQueryParamBase* pParam, const ScDBRangeBase* pDBRef) const;
};

#endif

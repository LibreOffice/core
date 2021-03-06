/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xerecord.hxx"
#include "xeroot.hxx"

#include <colorscale.hxx>

#include <memory>

enum XclExpExtType
{
    XclExpExtDataBarType,
    XclExpExtDataFooType
};

struct XclExpExtCondFormatData
{
    // -1 means don't write priority
    sal_Int32 nPriority;
    OString aGUID;
    const ScFormatEntry* pEntry;
};

/**
 * Base class for ext entries. Extend this class to provide the needed functionality
 */
class XclExpExt : public XclExpRecordBase, public XclExpRoot
{
public:
    explicit XclExpExt( const XclExpRoot& rRoot );
    virtual XclExpExtType GetType() = 0;

protected:
    OString maURI;
};

class XclExpExtCfvo : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpExtCfvo( const XclExpRoot& rRoot, const ScColorScaleEntry& rEntry, const ScAddress& rPos, bool bFirst );
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    ScColorScaleEntryType meType;
    OString maValue;
    bool mbFirst;
};

class XclExpExtNegativeColor
{
public:
    XclExpExtNegativeColor( const Color& rColor );
    void SaveXml( XclExpXmlStream& rStrm);

private:
    Color maColor;
};

class XclExpExtAxisColor
{
public:
    XclExpExtAxisColor( const Color& maColor );
    void SaveXml( XclExpXmlStream& rStrm );

private:
    Color maAxisColor;
};

class XclExpExtIcon : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtIcon( const XclExpRoot& rRoot, const std::pair<ScIconSetType, sal_Int32>& rCustomEntry);
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    const char* pIconSetName;
    sal_Int32 nIndex;
};

class XclExpExtCF : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtCF( const XclExpRoot& rRoot, const ScCondFormatEntry& rFormat );
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    OUString aFormula;
    ScCondFormatEntry mrFormat;
};

class XclExpExtDataBar : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtDataBar( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos );
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    databar::ScAxisPosition meAxisPosition;
    bool mbGradient;
    double mnMinLength;
    double mnMaxLength;

    std::unique_ptr<XclExpExtCfvo> mpLowerLimit;
    std::unique_ptr<XclExpExtCfvo> mpUpperLimit;
    std::unique_ptr<XclExpExtNegativeColor> mpNegativeColor;
    std::unique_ptr<XclExpExtAxisColor> mpAxisColor;

};

class XclExpExtIconSet : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtIconSet(const XclExpRoot& rRoot, const ScIconSetFormat& rFormat, const ScAddress& rPos);
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    XclExpRecordList<XclExpExtCfvo> maCfvos;
    XclExpRecordList<XclExpExtIcon> maCustom;
    bool mbCustom;
    bool mbReverse;
    bool mbShowValue;
    const char* mpIconSetName;
};


class XclExpExtCfRule : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpExtCfRule( const XclExpRoot& rRoot, const ScFormatEntry& rFormat, const ScAddress& rPos, const OString& rId, sal_Int32 nPriority );
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    XclExpRecordRef mxEntry;
    OString     maId;
    const char* pType;
    sal_Int32   mnPriority;
    const char* mOperator;
};

typedef rtl::Reference<XclExpExt> XclExpExtRef;

class XclExpExtConditionalFormatting : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtConditionalFormatting( const XclExpRoot& rRoot, std::vector<XclExpExtCondFormatData>& rData, const ScRangeList& rRange);
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

private:
    XclExpRecordList<XclExpExtCfRule> maCfRules;
    ScRangeList maRange;
};

typedef rtl::Reference<XclExpExtConditionalFormatting> XclExpExtConditionalFormattingRef;

class XclExpExtCondFormat : public XclExpExt
{
public:
    XclExpExtCondFormat( const XclExpRoot& rRoot );
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

    virtual XclExpExtType GetType() override { return XclExpExtDataBarType; }

    void AddRecord( XclExpExtConditionalFormatting* pFormat );

private:
    XclExpRecordList< XclExpExtConditionalFormatting > maCF;
};

class XclExpExtCalcPr : public XclExpExt
{
public:
    XclExpExtCalcPr( const XclExpRoot& rRoot, formula::FormulaGrammar::AddressConvention eConv );
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

    virtual XclExpExtType GetType() override { return XclExpExtDataFooType; }

private:
    OString maSyntax;
};

class XclExtLst : public XclExpRecordBase, public XclExpRoot
{
public:
    explicit XclExtLst( const XclExpRoot& rRoot);
    virtual void SaveXml( XclExpXmlStream& rStrm ) override;

    void AddRecord( XclExpExt* pEntry );
    void AddRecord( const XclExpExtRef& aEntry ) { AddRecord(aEntry.get()); }

    XclExpExt* GetItem( XclExpExtType eType );

private:
    XclExpRecordList< XclExpExt > maExtEntries;
};

typedef rtl::Reference< XclExtLst > XclExtLstRef;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

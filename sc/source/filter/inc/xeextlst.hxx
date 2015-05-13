/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XEEXTLST_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XEEXTLST_HXX

#include "xerecord.hxx"
#include "xeroot.hxx"

#include "colorscale.hxx"

#include <memory>

enum XclExpExtType
{
    XclExpExtDataBarType
};

/**
 * Base class for ext entries. Extend this class to provide the needed functionality
 *
 * Right now the only supported subclass is XclExpExtCondFormat
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
    virtual void SaveXml( XclExpXmlStream& rStrm ) SAL_OVERRIDE;

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

class XclExpExtDataBar : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtDataBar( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos );
    virtual void SaveXml( XclExpXmlStream& rStrm ) SAL_OVERRIDE;

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

typedef std::shared_ptr<XclExpExtDataBar> XclExpExtDataBarRef;

class XclExpExtCfRule : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpExtCfRule( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos, const OString& rId );
    virtual void SaveXml( XclExpXmlStream& rStrm ) SAL_OVERRIDE;

private:
    XclExpExtDataBarRef maDataBar;
    OString maId;
};

typedef std::shared_ptr<XclExpExt> XclExpExtRef;
typedef std::shared_ptr<XclExpExtCfRule> XclExpExtCfRuleRef;

class XclExpExtConditionalFormatting : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtConditionalFormatting( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos, const OString& rId );
    virtual void SaveXml( XclExpXmlStream& rStrm ) SAL_OVERRIDE;

private:
    XclExpExtCfRuleRef maCfRule;
    ScRangeList maRange;
};

typedef std::shared_ptr<XclExpExtConditionalFormatting> XclExpExtConditionalFormattingRef;

class XclExpExtCondFormat : public XclExpExt
{
public:
    XclExpExtCondFormat( const XclExpRoot& rRoot );
    virtual void SaveXml( XclExpXmlStream& rStrm ) SAL_OVERRIDE;

    virtual XclExpExtType GetType() SAL_OVERRIDE { return XclExpExtDataBarType; }

    void AddRecord( XclExpExtConditionalFormattingRef aFormat );

private:
    XclExpRecordList< XclExpExtConditionalFormatting > maCF;
};

class XclExtLst : public XclExpRecordBase, public XclExpRoot
{
public:
    explicit XclExtLst( const XclExpRoot& rRoot);
    virtual void SaveXml( XclExpXmlStream& rStrm ) SAL_OVERRIDE;

    void AddRecord( XclExpExtRef aEntry );

    XclExpExtRef GetItem( XclExpExtType eType );

private:
    XclExpRecordList< XclExpExt > maExtEntries;
};

typedef std::shared_ptr< XclExtLst > XclExtLstRef;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

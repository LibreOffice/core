/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_EXCEL_EXTLST_HXX
#define SC_EXCEL_EXTLST_HXX

#include "xerecord.hxx"
#include "xeroot.hxx"

#include "colorscale.hxx"

#include <boost/scoped_ptr.hpp>

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
    rtl::OString maURI;
};

class XclExpExtCfvo : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpExtCfvo( const XclExpRoot& rRoot, const ScColorScaleEntry& rEntry, const ScAddress& rPos );
    virtual void SaveXml( XclExpXmlStream& rStrm );

private:
    ScColorScaleEntryType meType;
    rtl::OString maValue;
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
    virtual void SaveXml( XclExpXmlStream& rStrm );

private:
    databar::ScAxisPostion meAxisPosition;

    boost::scoped_ptr<XclExpExtCfvo> mpLowerLimit;
    boost::scoped_ptr<XclExpExtCfvo> mpUpperLimit;
    boost::scoped_ptr<XclExpExtNegativeColor> mpNegativeColor;
    boost::scoped_ptr<XclExpExtAxisColor> mpAxisColor;

};

typedef boost::shared_ptr<XclExpExtDataBar> XclExpExtDataBarRef;

class XclExpExtCfRule : public XclExpRecordBase, protected XclExpRoot
{
public:
    XclExpExtCfRule( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos, const rtl::OString& rId );
    virtual void SaveXml( XclExpXmlStream& rStrm );

private:
    XclExpExtDataBarRef maDataBar;
    rtl::OString maId;
};

typedef boost::shared_ptr<XclExpExt> XclExpExtRef;
typedef boost::shared_ptr<XclExpExtCfRule> XclExpExtCfRuleRef;

class XclExpExtConditionalFormatting : public XclExpRecordBase, protected XclExpRoot
{
public:
    explicit XclExpExtConditionalFormatting( const XclExpRoot& rRoot, const ScDataBarFormat& rFormat, const ScAddress& rPos, const rtl::OString& rId );
    virtual void SaveXml( XclExpXmlStream& rStrm );

private:
    XclExpExtCfRuleRef maCfRule;
    ScRangeList maRange;
};

typedef boost::shared_ptr<XclExpExtConditionalFormatting> XclExpExtConditionalFormattingRef;

class XclExpExtCondFormat : public XclExpExt
{
public:
    XclExpExtCondFormat( const XclExpRoot& rRoot );
    virtual void SaveXml( XclExpXmlStream& rStrm );

    virtual XclExpExtType GetType() { return XclExpExtDataBarType; }

    void AddRecord( XclExpExtConditionalFormattingRef aFormat );

private:
    XclExpRecordList< XclExpExtConditionalFormatting > maCF;
};

class XclExtLst : public XclExpRecordBase, public XclExpRoot
{
public:
    explicit XclExtLst( const XclExpRoot& rRoot);
    virtual void SaveXml( XclExpXmlStream& rStrm );

    void AddRecord( XclExpExtRef aEntry );

    XclExpExtRef GetItem( XclExpExtType eType );

private:
    XclExpRecordList< XclExpExt > maExtEntries;
};

typedef boost::shared_ptr< XclExtLst > XclExtLstRef;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

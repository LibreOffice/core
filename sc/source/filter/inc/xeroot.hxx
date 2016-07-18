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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XEROOT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XEROOT_HXX

#include <com/sun/star/beans/NamedValue.hpp>

#include "xlroot.hxx"
#include "compiler.hxx"
#include <memory>

// Forward declarations of objects in public use ==============================

class XclExpRecordBase;
class XclExpString;

typedef std::shared_ptr< XclExpRecordBase >  XclExpRecordRef;
typedef std::shared_ptr< XclExpString >      XclExpStringRef;

// Global data ================================================================

class XclExpTabInfo;
class XclExpAddressConverter;
class XclExpFormulaCompiler;
class XclExpProgressBar;
class XclExpSst;
class XclExpPalette;
class XclExpFontBuffer;
class XclExpNumFmtBuffer;
class XclExpXFBuffer;
class XclExpLinkManager;
class XclExpNameManager;
class XclExpObjectManager;
class XclExpFilterManager;
class XclExpPivotTableManager;
class XclExpDxfs;
class XclExpXmlPivotTableManager;
class XclExpTablesManager;
namespace sc { class CompileFormulaContext; }

/** Stores global buffers and data needed for Excel export filter. */
struct XclExpRootData : public XclRootData
{
    typedef std::shared_ptr< XclExpTabInfo >             XclExpTabInfoRef;
    typedef std::shared_ptr< XclExpAddressConverter >    XclExpAddrConvRef;
    typedef std::shared_ptr< XclExpFormulaCompiler >     XclExpFmlaCompRef;
    typedef std::shared_ptr< XclExpProgressBar >         XclExpProgressRef;

    typedef std::shared_ptr< XclExpSst >                 XclExpSstRef;
    typedef std::shared_ptr< XclExpPalette >             XclExpPaletteRef;
    typedef std::shared_ptr< XclExpFontBuffer >          XclExpFontBfrRef;
    typedef std::shared_ptr< XclExpNumFmtBuffer >        XclExpNumFmtBfrRef;
    typedef std::shared_ptr< XclExpXFBuffer >            XclExpXFBfrRef;
    typedef std::shared_ptr< XclExpNameManager >         XclExpNameMgrRef;
    typedef std::shared_ptr< XclExpLinkManager >         XclExpLinkMgrRef;
    typedef std::shared_ptr< XclExpObjectManager >       XclExpObjectMgrRef;
    typedef std::shared_ptr< XclExpFilterManager >       XclExpFilterMgrRef;
    typedef std::shared_ptr< XclExpPivotTableManager >   XclExpPTableMgrRef;
    typedef std::shared_ptr< XclExpDxfs >                XclExpDxfsRef;

    XclExpTabInfoRef    mxTabInfo;          /// Calc->Excel sheet index conversion.
    XclExpAddrConvRef   mxAddrConv;         /// The address converter.
    XclExpFmlaCompRef   mxFmlaComp;         /// The formula compiler.
    XclExpProgressRef   mxProgress;         /// The export progress bar.

    XclExpSstRef        mxSst;              /// The shared string table.
    XclExpPaletteRef    mxPalette;          /// The color buffer.
    XclExpFontBfrRef    mxFontBfr;          /// All fonts in the file.
    XclExpNumFmtBfrRef  mxNumFmtBfr;        /// All number formats in the file.
    XclExpXFBfrRef      mxXFBfr;            /// All XF records in the file.
    XclExpNameMgrRef    mxNameMgr;          /// Internal defined names.
    XclExpLinkMgrRef    mxGlobLinkMgr;      /// Global link manager for defined names.
    XclExpLinkMgrRef    mxLocLinkMgr;       /// Local link manager for a sheet.
    XclExpObjectMgrRef  mxObjMgr;           /// All drawing objects.
    XclExpFilterMgrRef  mxFilterMgr;        /// Manager for filtered areas in all sheets.
    XclExpPTableMgrRef  mxPTableMgr;        /// All pivot tables and pivot caches.
    XclExpDxfsRef       mxDxfs;             /// All delta formatting entries

    std::shared_ptr<XclExpXmlPivotTableManager> mxXmlPTableMgr;
    std::shared_ptr<XclExpTablesManager> mxTablesMgr;
    std::shared_ptr<sc::CompileFormulaContext> mpCompileFormulaCxt;

    ScCompiler::OpCodeMapPtr  mxOpCodeMap;  /// mapping between op-codes and names

    bool                mbRelUrl;           /// true = Store URLs relative.

    OStringBuffer       maStringBuf;        /// buffer to avoid massive OUString allocations

    explicit            XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
                            tools::SvRef<SotStorage> xRootStrg, ScDocument& rDoc, rtl_TextEncoding eTextEnc );
    virtual             ~XclExpRootData();
};

/** Access to global data from other classes. */
class XclExpRoot : public XclRoot
{
public:
    explicit            XclExpRoot( XclExpRootData& rExpRootData );

    /** Returns this root instance - for code readability in derived classes. */
    inline const XclExpRoot& GetRoot() const { return *this; }
    /** Returns true, if URLs should be stored relative to the document location. */
    inline bool         IsRelUrl() const { return mrExpData.mbRelUrl; }
    sc::CompileFormulaContext& GetCompileFormulaContext() const { return *mrExpData.mpCompileFormulaCxt; }

    /** Returns the buffer for Calc->Excel sheet index conversion. */
    XclExpTabInfo&      GetTabInfo() const;
    /** Returns the address converter. */
    XclExpAddressConverter& GetAddressConverter() const;
    /** Returns the formula compiler to produce formula token arrays. */
    XclExpFormulaCompiler& GetFormulaCompiler() const;
    /** Returns the export progress bar. */
    XclExpProgressBar&  GetProgressBar() const;

    /** Returns the shared string table. */
    XclExpSst&          GetSst() const;
    /** Returns the color buffer. */
    XclExpPalette&      GetPalette() const;
    /** Returns the font buffer. */
    XclExpFontBuffer&   GetFontBuffer() const;
    /** Returns the number format buffer. */
    XclExpNumFmtBuffer& GetNumFmtBuffer() const;
    /** Returns the cell formatting attributes buffer. */
    XclExpXFBuffer&     GetXFBuffer() const;
    /** Returns the global link manager for defined names. */
    XclExpLinkManager&  GetGlobalLinkManager() const;
    /** Returns the local link manager for the current sheet. */
    XclExpLinkManager&  GetLocalLinkManager() const;
    /** Returns the buffer that contains internal defined names. */
    XclExpNameManager&  GetNameManager() const;
    /** Returns the drawing object manager. */
    XclExpObjectManager& GetObjectManager() const;
    /** Returns the filter manager. */
    XclExpFilterManager& GetFilterManager() const;
    /** Returns the pivot table manager. */
    XclExpPivotTableManager& GetPivotTableManager() const;
    /** Returns the differential formatting list */
    XclExpDxfs&          GetDxfs() const;

    /** Clean and return the OStringBuffer */
    inline OStringBuffer&   GetStringBuf() const { mrExpData.maStringBuf.setLength(0); return mrExpData.maStringBuf; }

    XclExpXmlPivotTableManager& GetXmlPivotTableManager();

    XclExpTablesManager& GetTablesManager();

    /** Is called when export filter starts to create the Excel document (all BIFF versions). */
    void                InitializeConvert();
    /** Is called when export filter starts to create the workbook global data (>=BIFF5). */
    void                InitializeGlobals();
    /** Is called when export filter starts to create data for a single sheet (all BIFF versions). */
    void                InitializeTable( SCTAB nScTab );
    /** Is called before export filter starts to write the records to the stream. */
    void                InitializeSave();
    /** Returns the reference to a record (or record list) representing a root object.
        @param nRecId  Identifier that specifies which record is returned. */
    XclExpRecordRef     CreateRecord( sal_uInt16 nRecId ) const;

    bool                IsDocumentEncrypted() const;

    static css::uno::Sequence< css::beans::NamedValue > GenerateEncryptionData( const OUString& aPass );
    css::uno::Sequence< css::beans::NamedValue > GetEncryptionData() const;
    css::uno::Sequence< css::beans::NamedValue > GenerateDefaultEncryptionData() const;

private:

    /** Returns the local or global link manager, depending on current context. */
    XclExpRootData::XclExpLinkMgrRef const & GetLocalLinkMgrRef() const;

private:
    XclExpRootData& mrExpData;      /// Reference to the global export data struct.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

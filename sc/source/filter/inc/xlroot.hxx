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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_XLROOT_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_XLROOT_HXX

#include <com/sun/star/beans/NamedValue.hpp>

#include <i18nlangtag/lang.h>
#include <sot/storage.hxx>
#include "xlconst.hxx"
#include "xltools.hxx"
#include <documentimport.hxx>
#include <memory>

namespace comphelper { class IDocPasswordVerifier; }

// Forward declarations of objects in public use ==============================

class DateTime;

struct XclAddress;
struct XclRange;

// Global data ================================================================

#ifdef DBG_UTIL
/** Counts the number of created root objects. */
struct XclDebugObjCounter
{
    sal_Int32           mnObjCnt;
    inline explicit     XclDebugObjCounter() : mnObjCnt( 0 ) {}
                        ~XclDebugObjCounter();
};
#endif

class SfxMedium;
class ScEditEngineDefaulter;
class ScHeaderEditEngine;
class EditEngine;
class ScExtDocOptions;
class XclFontPropSetHelper;
class XclChPropSetHelper;
class XclTracer;

struct RootData;//!

/** Stores global buffers and data needed elsewhere in the Excel filters. */
struct XclRootData
#ifdef DBG_UTIL
    : public XclDebugObjCounter
#endif
{
    typedef std::shared_ptr< ScEditEngineDefaulter > ScEEDefaulterRef;
    typedef std::shared_ptr< ScHeaderEditEngine >    ScHeaderEERef;
    typedef std::shared_ptr< EditEngine >            EditEngineRef;
    typedef std::shared_ptr< XclFontPropSetHelper >  XclFontPropSetHlpRef;
    typedef std::shared_ptr< XclChPropSetHelper >    XclChPropSetHlpRef;
    typedef std::shared_ptr< ScExtDocOptions >       ScExtDocOptRef;
    typedef std::shared_ptr< XclTracer >             XclTracerRef;
    typedef std::shared_ptr< RootData >              RootDataRef;

    XclBiff             meBiff;             /// Current BIFF version.
    XclOutput           meOutput;           /// Current Output format.
    SfxMedium&          mrMedium;           /// The medium to import from.
    tools::SvRef<SotStorage>       mxRootStrg;         /// The root OLE storage of imported/exported file.
    ScDocument&         mrDoc;              /// The source or destination document.
    ScDocumentImport    maDocImport;
    OUString            maDocUrl;           /// Document URL of imported/exported file.
    OUString            maBasePath;         /// Base path of imported/exported file (path of maDocUrl).
    OUString            maUserName;         /// Current user name.
    const OUString      maDefPassword;      /// The default password used for stream encryption.
    rtl_TextEncoding    meTextEnc;          /// Text encoding to import/export byte strings.
    LanguageType        meSysLang;          /// System language.
    LanguageType        meDocLang;          /// Document language (import: from file, export: from system).
    LanguageType        meUILang;           /// UI language (import: from file, export: from system).
    sal_Int16           mnDefApiScript;     /// Default script type for blank cells (API constant).
    ScAddress           maScMaxPos;         /// Highest Calc cell position.
    ScAddress           maXclMaxPos;        /// Highest Excel cell position.
    ScAddress           maMaxPos;           /// Highest position valid in Calc and Excel.

    ScEEDefaulterRef    mxEditEngine;       /// Edit engine for rich strings etc.
    ScHeaderEERef       mxHFEditEngine;     /// Edit engine for header/footer.
    EditEngineRef       mxDrawEditEng;      /// Edit engine for text boxes.

    XclFontPropSetHlpRef mxFontPropSetHlp;  /// Property set helper for fonts.
    XclChPropSetHlpRef  mxChPropSetHlp;     /// Property set helper for chart filter.

    ScExtDocOptRef      mxExtDocOpt;        /// Extended document options.
    XclTracerRef        mxTracer;           /// Filter tracer.
    RootDataRef         mxRD;               /// Old RootData struct. Will be removed.

    double              mfScreenPixelX;     /// Width of a screen pixel (1/100 mm).
    double              mfScreenPixelY;     /// Height of a screen pixel (1/100 mm).
    long                mnCharWidth;        /// Width of '0' in default font (twips).
    SCTAB               mnScTab;            /// Current Calc sheet index.
    const bool          mbExport;           /// false = Import, true = Export.

    explicit            XclRootData( XclBiff eBiff, SfxMedium& rMedium,
                            tools::SvRef<SotStorage> xRootStrg, ScDocument& rDoc,
                            rtl_TextEncoding eTextEnc, bool bExport );
    virtual             ~XclRootData();
};

class SfxObjectShell;
class ScModelObj;
class OutputDevice;
class SvNumberFormatter;
class SdrPage;
class ScStyleSheetPool;
class ScRangeName;
struct XclFontData;

/** Access to global data for a filter object (imported or exported document) from other classes. */
class XclRoot
{
public:
    explicit            XclRoot( XclRootData& rRootData );
                        XclRoot( const XclRoot& rRoot );

    virtual             ~XclRoot();

    XclRoot&            operator=( const XclRoot& rRoot );

    /** Returns old RootData struct. Deprecated. */
    inline RootData&    GetOldRoot() const { return *mrData.mxRD; }

    /** Returns the current BIFF version of the importer/exporter. */
    inline XclBiff      GetBiff() const { return mrData.meBiff; }
    /** Returns the current output format of the importer/exporter. */
    inline XclOutput    GetOutput() const { return mrData.meOutput; }
    /** Returns true, if currently a document is imported. */
    inline bool         IsImport() const { return !mrData.mbExport; }
    /** Returns the text encoding to import/export byte strings. */
    inline rtl_TextEncoding GetTextEncoding() const { return mrData.meTextEnc; }
    /** Returns the system language, i.e. for number formats. */
    inline LanguageType GetSysLanguage() const { return mrData.meSysLang; }
    /** Returns the document language. */
    inline LanguageType GetDocLanguage() const { return mrData.meDocLang; }
    /** Returns the UI language. */
    inline LanguageType GetUILanguage() const { return mrData.meUILang; }
    /** Returns the default script type, e.g. for blank cells. */
    inline sal_Int16    GetDefApiScript() const { return mrData.mnDefApiScript; }
    /** Returns the width of the '0' character (default font) for the current printer (twips). */
    inline long         GetCharWidth() const { return mrData.mnCharWidth; }
    /** Returns the current Calc sheet index. */
    inline bool         IsInGlobals() const { return mrData.mnScTab == SCTAB_GLOBAL; }
    /** Returns the current Calc sheet index. */
    inline SCTAB        GetCurrScTab() const { return mrData.mnScTab; }

    /** Calculates the width of the passed number of pixels in 1/100 mm. */
    sal_Int32           GetHmmFromPixelX( double fPixelX ) const;
    /** Calculates the height of the passed number of pixels in 1/100 mm. */
    sal_Int32           GetHmmFromPixelY( double fPixelY ) const;

    /** Returns the medium to import from. */
    inline SfxMedium&   GetMedium() const { return mrData.mrMedium; }
    /** Returns the document URL of the imported/exported file. */
    inline const OUString& GetDocUrl() const { return mrData.maDocUrl; }
    /** Returns the base path of the imported/exported file. */
    inline const OUString& GetBasePath() const { return mrData.maBasePath; }
    /** Returns the current user name. */
    inline const OUString& GetUserName() const { return mrData.maUserName; }

    /** Returns the default password used for stream encryption. */
    inline const OUString& GetDefaultPassword() const { return mrData.maDefPassword; }
    /** Requests and verifies a password from the medium or the user. */
    css::uno::Sequence< css::beans::NamedValue >
        RequestEncryptionData( ::comphelper::IDocPasswordVerifier& rVerifier ) const;

    /** Returns the OLE2 root storage of the imported/exported file.
        @return  Pointer to root storage or 0, if the file is a simple stream. */
    inline tools::SvRef<SotStorage> GetRootStorage() const { return mrData.mxRootStrg; }
    /** Returns true, if the document contains a VBA storage. */
    bool                HasVbaStorage() const;

    /** Tries to open a storage as child of the specified storage for reading or writing. */
    tools::SvRef<SotStorage>       OpenStorage( tools::SvRef<SotStorage> xStrg, const OUString& rStrgName ) const;
    /** Tries to open a storage as child of the root storage for reading or writing. */
    tools::SvRef<SotStorage>       OpenStorage( const OUString& rStrgName ) const;
    /** Tries to open a new stream in the specified storage for reading or writing. */
    tools::SvRef<SotStorageStream> OpenStream( tools::SvRef<SotStorage> xStrg, const OUString& rStrmName ) const;
    /** Tries to open a new stream in the root storage for reading or writing. */
    tools::SvRef<SotStorageStream> OpenStream( const OUString& rStrmName ) const;

    /** Returns the destination document (import) or source document (export). */
    ScDocument& GetDoc() const;
    /** Returns pointer to the destination document (import) or source document (export). */
    ScDocument& GetDocRef() const;

    /** Returns the object shell of the Calc document. May be 0 (i.e. import from clipboard). */
    SfxObjectShell*     GetDocShell() const;
    /** Returns the object model of the Calc document. */
    ScModelObj*         GetDocModelObj() const;
    /** Returns pointer to the printer of the Calc document. */
    OutputDevice*       GetPrinter() const;
    /** Returns the style sheet pool of the Calc document. */
    ScStyleSheetPool&   GetStyleSheetPool() const;
    /** Returns the defined names container of the Calc document. */
    ScRangeName&        GetNamedRanges() const;
    /** Returns the drawing layer page of the passed sheet, if present. */
    SdrPage*            GetSdrPage( SCTAB nScTab ) const;

    /** Returns the number formatter of the Calc document. */
    SvNumberFormatter&  GetFormatter() const;
    /** Returns the null date of the current number formatter. */
    DateTime            GetNullDate() const;
    /** Returns the base year depending on the current null date (1900 or 1904). */
    sal_uInt16          GetBaseYear() const;
    /** Converts a date/time value to a floating-point value. */
    double              GetDoubleFromDateTime( const DateTime& rDateTime ) const;
    /** Converts a floating-point value to a date/time value. */
    DateTime            GetDateTimeFromDouble( double fValue ) const;

    /** Returns the edit engine for import/export of rich strings etc. */
    ScEditEngineDefaulter& GetEditEngine() const;
    /** Returns the edit engine for import/export of headers/footers. */
    ScHeaderEditEngine& GetHFEditEngine() const;
    /** Returns the edit engine for import/export of drawing text boxes. */
    EditEngine&         GetDrawEditEngine() const;

    /** Returns the property set helper for fonts. */
    XclFontPropSetHelper& GetFontPropSetHelper() const;
    /** Returns the property set helper for the chart filters. */
    XclChPropSetHelper& GetChartPropSetHelper() const;

    /** Returns the extended document options. */
    ScExtDocOptions&    GetExtDocOptions() const;
    /** Returns the filter tracer. */
    XclTracer&          GetTracer() const;

    /** Returns the highest possible cell address in a Calc document. */
    inline const ScAddress& GetScMaxPos() const { return mrData.maScMaxPos; }
    /** Returns the highest possible cell address in an Excel document (using current BIFF version). */
    inline const ScAddress& GetXclMaxPos() const { return mrData.maXclMaxPos; }
    /** Returns the highest possible cell address valid in Calc and Excel (using current BIFF version). */
    inline const ScAddress& GetMaxPos() const { return mrData.maMaxPos; }

    /** Sets the document language. */
    inline void         SetDocLanguage( LanguageType eLang ) { mrData.meDocLang = eLang; }
    /** Sets the UI language, i.e. if it has been read from a file. */
    inline void         SetUILanguage( LanguageType eLang ) { mrData.meUILang = eLang; }
    /** Sets the text encoding to import/export byte strings. */
    void                SetTextEncoding( rtl_TextEncoding eTextEnc );
    /** Sets the width of the '0' character (default font) for the current printer (twips).
        @param rFontData  The font used for the '0' character. */
    void                SetCharWidth( const XclFontData& rFontData );
    /** Sets the current Calc sheet index. */
    inline void         SetCurrScTab( SCTAB nScTab ) { mrData.mnScTab = nScTab; }
    /** Increases the current Calc sheet index by 1. */
    inline void         IncCurrScTab() { ++mrData.mnScTab; }

private:
    XclRootData& mrData;        /// Reference to the global data struct.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

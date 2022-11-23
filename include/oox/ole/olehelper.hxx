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

#ifndef INCLUDED_OOX_OLE_OLEHELPER_HXX
#define INCLUDED_OOX_OLE_OLEHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <filter/msfilter/msocximex.hxx>
#include <oox/dllapi.h>
#include <oox/helper/binarystreambase.hxx>
#include <oox/helper/graphichelper.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <tools/ref.hxx>
#include <memory>

namespace com::sun::star {
    namespace awt { class XControlModel; }
    namespace awt { struct Size; }
    namespace form { class XFormComponent; }
    namespace frame { class XModel; }
    namespace io { class XOutputStream; }
    namespace uno { class XComponentContext; }
}

class SotStorage;
class SotStorageStream;
class SvGlobalName;

namespace oox {
    class BinaryInputStream;
    class BinaryOutputStream;
}

namespace oox::ole {
    class ControlModelBase;
    class EmbeddedControl;


#define OLE_GUID_STDFONT "{0BE35203-8F91-11CE-9DE3-00AA004BB851}"
#define OLE_GUID_STDPIC  "{0BE35204-8F91-11CE-9DE3-00AA004BB851}"


const sal_uInt16 OLE_STDFONT_NORMAL     = 400;
const sal_uInt16 OLE_STDFONT_BOLD       = 700;

const sal_uInt8 OLE_STDFONT_ITALIC      = 0x02;
const sal_uInt8 OLE_STDFONT_UNDERLINE   = 0x04;
const sal_uInt8 OLE_STDFONT_STRIKE      = 0x08;

/** Stores data about a StdFont font structure. */
struct StdFontInfo
{
    OUString     maName;         ///< Font name.
    sal_uInt32          mnHeight;       ///< Font height (1/10,000 points).
    sal_uInt16          mnWeight;       ///< Font weight (normal/bold).
    sal_uInt16          mnCharSet;      ///< Font charset.
    sal_uInt8           mnFlags;        ///< Font flags.

    explicit            StdFontInfo();
    explicit            StdFontInfo(
                            OUString aName,
                            sal_uInt32 nHeight );
};


/** Stores data about a StdHlink hyperlink. */
struct StdHlinkInfo
{
    OUString     maTarget;
    OUString     maLocation;
    OUString     maDisplay;
};


/** Static helper functions for OLE import/export. */
namespace OleHelper
{
    /** Returns the UNO RGB color from the passed encoded OLE color.

        @param bDefaultColorBgr
            True = OLE default color type is treated as BGR color.
            False = OLE default color type is treated as palette color.
     */
    OOX_DLLPUBLIC ::Color decodeOleColor(
                            const GraphicHelper& rGraphicHelper,
                            sal_uInt32 nOleColor,
                            bool bDefaultColorBgr );

    /** Returns the OLE color from the passed UNO RGB color.
     */
    OOX_DLLPUBLIC sal_uInt32   encodeOleColor( sal_Int32 nRgbColor );
    inline sal_uInt32          encodeOleColor( Color nRgbColor ) { return encodeOleColor(sal_Int32(nRgbColor)); }

    /** Imports a GUID from the passed binary stream and returns its string
        representation (in uppercase characters).
     */
    OOX_DLLPUBLIC OUString importGuid( BinaryInputStream& rInStrm );
    OOX_DLLPUBLIC void exportGuid( BinaryOutputStream& rOutStrm, const SvGlobalName& rId );

    /** Imports an OLE StdFont font structure from the current position of the
        passed binary stream.
     */
    OOX_DLLPUBLIC bool         importStdFont(
                            StdFontInfo& orFontInfo,
                            BinaryInputStream& rInStrm,
                            bool bWithGuid );

    /** Imports an OLE StdPic picture from the current position of the passed
        binary stream.
     */
    OOX_DLLPUBLIC bool         importStdPic(
                            StreamDataSequence& orGraphicData,
                            BinaryInputStream& rInStrm );
}

class OOX_DLLPUBLIC OleFormCtrlExportHelper final
{
    std::unique_ptr<::oox::ole::EmbeddedControl> mpControl;
    ::oox::ole::ControlModelBase* mpModel;
    ::oox::GraphicHelper maGrfHelper;
    css::uno::Reference< css::frame::XModel > mxDocModel;
    css::uno::Reference< css::awt::XControlModel > mxControlModel;

    OUString maName;
    OUString maTypeName;
    OUString maFullName;
    OUString maGUID;
public:
    OleFormCtrlExportHelper( const css::uno::Reference< css::uno::XComponentContext >& rxCtx, const css::uno::Reference< css::frame::XModel >& xDocModel, const css::uno::Reference< css::awt::XControlModel >& xModel );
    ~OleFormCtrlExportHelper();

    std::u16string_view getGUID() const
    {
        std::u16string_view sResult;
        if ( maGUID.getLength() > 2 )
            sResult = maGUID.subView(1, maGUID.getLength() - 2 );
        return sResult;
    }
    const OUString& getFullName() const { return maFullName; }
    const OUString& getTypeName() const { return maTypeName; }
    const OUString& getName() const { return maName; }
    bool isValid() const { return mpModel != nullptr; }
    void exportName( const css::uno::Reference< css::io::XOutputStream >& rxOut );
    void exportCompObj( const css::uno::Reference< css::io::XOutputStream >& rxOut );
    void exportControl( const css::uno::Reference< css::io::XOutputStream >& rxOut, const css::awt::Size& rSize, bool bAutoClose = false );
};

// ideally it would be great to get rid of SvxMSConvertOCXControls
// however msfilter/source/msfilter/svdfppt.cxx still uses
// SvxMSConvertOCXControls as a base class, unfortunately oox depends on
// msfilter. Probably the solution would be to move the svdfppt.cxx
// implementation into the sd module itself.
class OOX_DLLPUBLIC MSConvertOCXControls : public SvxMSConvertOCXControls
{
    css::uno::Reference< css::uno::XComponentContext > mxCtx;
    ::oox::GraphicHelper maGrfHelper;

protected:
    bool importControlFromStream( ::oox::BinaryInputStream& rInStrm,
                                  css::uno::Reference< css::form::XFormComponent > & rxFormComp,
                                  std::u16string_view rGuidString );
    bool importControlFromStream( ::oox::BinaryInputStream& rInStrm,
                                  css::uno::Reference< css::form::XFormComponent > & rxFormComp,
                                  const OUString& rGuidString,
                                  sal_Int32 nSize );
public:
    MSConvertOCXControls( const css::uno::Reference< css::frame::XModel >& rxModel );
    virtual ~MSConvertOCXControls() override;
    bool ReadOCXStorage( tools::SvRef<SotStorage> const & rSrc1, css::uno::Reference< css::form::XFormComponent > & rxFormComp );
    bool ReadOCXCtlsStream(tools::SvRef<SotStorageStream> const & rSrc1, css::uno::Reference< css::form::XFormComponent > & rxFormComp,
                                   sal_Int32 nPos, sal_Int32 nSize );
    static bool WriteOCXStream( const css::uno::Reference< css::frame::XModel >& rxModel, tools::SvRef<SotStorage> const &rSrc1, const css::uno::Reference< css::awt::XControlModel > &rControlModel, const css::awt::Size& rSize,OUString &rName);
    static bool WriteOCXExcelKludgeStream( const css::uno::Reference< css::frame::XModel >& rxModel, const css::uno::Reference< css::io::XOutputStream >& xOutStrm, const css::uno::Reference< css::awt::XControlModel > &rControlModel, const css::awt::Size& rSize,OUString &rName);
};


} // namespace oox::ole

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

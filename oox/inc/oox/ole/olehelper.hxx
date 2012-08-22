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

#ifndef OOX_OLE_OLEHELPER_HXX
#define OOX_OLE_OLEHELPER_HXX

#include <rtl/ustring.hxx>
#include "oox/helper/binarystreambase.hxx"
#include "oox/helper/storagebase.hxx"
#include "oox/helper/graphichelper.hxx"
#include "com/sun/star/form/XFormComponent.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/frame/XModel.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/drawing/XShapes.hpp"
#include "com/sun/star/awt/XControlModel.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/io/XOutputStream.hpp"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include "com/sun/star/container/XIndexContainer.hpp"
#include <filter/msfilter/msocximex.hxx>
#include "oox/dllapi.h"
#include "sot/storage.hxx"

namespace oox {
    class BinaryInputStream;
    class BinaryOutputStream;
    class BinaryXInputStream;
    class GraphicHelper;
}

namespace oox {

typedef ::boost::shared_ptr< oox::BinaryXInputStream > BinaryXInputStreamRef;

namespace ole {


// ============================================================================

#define OLE_GUID_STDFONT "{0BE35203-8F91-11CE-9DE3-00AA004BB851}"
#define OLE_GUID_STDPIC  "{0BE35204-8F91-11CE-9DE3-00AA004BB851}"
#define OLE_GUID_STDHLINK "{79EAC9D0-BAF9-11CE-8C82-00AA004BA90B}"

// ============================================================================

const sal_uInt16 OLE_STDFONT_NORMAL     = 400;
const sal_uInt16 OLE_STDFONT_BOLD       = 700;

const sal_uInt8 OLE_STDFONT_ITALIC      = 0x02;
const sal_uInt8 OLE_STDFONT_UNDERLINE   = 0x04;
const sal_uInt8 OLE_STDFONT_STRIKE      = 0x08;

/** Stores data about a StdFont font structure. */
struct StdFontInfo
{
    ::rtl::OUString     maName;         ///< Font name.
    sal_uInt32          mnHeight;       ///< Font height (1/10,000 points).
    sal_uInt16          mnWeight;       ///< Font weight (normal/bold).
    sal_uInt16          mnCharSet;      ///< Font charset.
    sal_uInt8           mnFlags;        ///< Font flags.

    explicit            StdFontInfo();
    explicit            StdFontInfo(
                            const ::rtl::OUString& rName,
                            sal_uInt32 nHeight,
                            sal_uInt16 nWeight = OLE_STDFONT_NORMAL,
                            sal_uInt16 nCharSet = WINDOWS_CHARSET_ANSI,
                            sal_uInt8 nFlags = 0 );
};

// ============================================================================

/** Stores data about a StdHlink hyperlink. */
struct StdHlinkInfo
{
    ::rtl::OUString     maTarget;
    ::rtl::OUString     maLocation;
    ::rtl::OUString     maDisplay;
    ::rtl::OUString     maFrame;
};

// ============================================================================

/** Static helper functions for OLE import/export. */
class OOX_DLLPUBLIC OleHelper
{
public:
    /** Returns the UNO RGB color from the passed encoded OLE color.

        @param bDefaultColorBgr
            True = OLE default color type is treated as BGR color.
            False = OLE default color type is treated as palette color.
     */
    static sal_Int32    decodeOleColor(
                            const GraphicHelper& rGraphicHelper,
                            sal_uInt32 nOleColor,
                            bool bDefaultColorBgr = true );

    /** Returns the OLE color from the passed UNO RGB color.
     */
    static sal_uInt32   encodeOleColor( sal_Int32 nRgbColor );

    /** Imports a GUID from the passed binary stream and returns its string
        representation (in uppercase characters).
     */
    static ::rtl::OUString importGuid( BinaryInputStream& rInStrm );

    /** Imports an OLE StdFont font structure from the current position of the
        passed binary stream.
     */
    static bool         importStdFont(
                            StdFontInfo& orFontInfo,
                            BinaryInputStream& rInStrm,
                            bool bWithGuid );

    /** Imports an OLE StdPic picture from the current position of the passed
        binary stream.
     */
    static bool         importStdPic(
                            StreamDataSequence& orGraphicData,
                            BinaryInputStream& rInStrm,
                            bool bWithGuid );

private:
                        OleHelper();        // not implemented
                        ~OleHelper();       // not implemented
};

// ideally it would be great to get rid of SvxMSConvertOCXControls
// however msfilter/source/msfilter/svdfppt.cxx still uses
// SvxMSConvertOCXControls as a base class, unfortunately oox depends on
// msfilter. Probably the solution would be to move the svdfppt.cxx
// implementation into the sd module itself.
class OOX_DLLPUBLIC MSConvertOCXControls : public SvxMSConvertOCXControls
{
#if SvxMSConvertOCXControlsRemoved
    com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > mxShapes;
    com::sun::star::uno::Reference< com::sun::star::drawing::XDrawPage > mxDrawPage;
    com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >  mxFormComps;
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > mxServiceFactory;
#endif
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > mxCtx;
    ::oox::GraphicHelper maGrfHelper;

    bool importControlFromStream( ::oox::BinaryInputStream& rInStrm,
                                  ::com::sun::star::uno::Reference< com::sun::star::form::XFormComponent > & rxFormComp,
                                  const ::rtl::OUString& rGuidString );
    bool importControlFromStream( ::oox::BinaryInputStream& rInStrm,
                                  ::com::sun::star::uno::Reference< com::sun::star::form::XFormComponent > & rxFormComp,
                                  const ::rtl::OUString& rGuidString,
                                  sal_Int32 nSize );
public:
    MSConvertOCXControls( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxModel );
    ~MSConvertOCXControls();
    sal_Bool ReadOCXStorage( SotStorageRef& rSrc1, ::com::sun::star::uno::Reference< com::sun::star::form::XFormComponent > & rxFormComp );
    sal_Bool ReadOCXCtlsStream(SotStorageStreamRef& rSrc1, ::com::sun::star::uno::Reference< com::sun::star::form::XFormComponent > & rxFormComp,
                                   sal_Int32 nPos, sal_Int32 nSize );
    static sal_Bool WriteOCXStream( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxModel, SotStorageRef &rSrc1, const com::sun::star::uno::Reference< com::sun::star::awt::XControlModel > &rControlModel, const com::sun::star::awt::Size& rSize,rtl::OUString &rName);

#if SvxMSConvertOCXControlsRemoved
    const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > & GetShapes();
    const com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer > &  GetFormComps();
    virtual const com::sun::star::uno::Reference<
        com::sun::star::drawing::XDrawPage > & GetDrawPage();
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > & GetServiceFactory();
    virtual sal_Bool InsertControl(
        const com::sun::star::uno::Reference<
        com::sun::star::form::XFormComponent >& /*rFComp*/,
        const com::sun::star::awt::Size& /*rSize*/,
        com::sun::star::uno::Reference<
        com::sun::star::drawing::XShape >* /*pShape*/,
        sal_Bool /*bFloatingCtrl*/ ) {return sal_False;}
#endif
};


// ============================================================================

} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

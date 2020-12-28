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

#ifndef INCLUDED_SVX_XMLGRHLP_HXX
#define INCLUDED_SVX_XMLGRHLP_HXX

#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>
#include <vcl/graph.hxx>
#include <rtl/ref.hxx>

#include <string_view>
#include <vector>
#include <unordered_map>
#include <utility>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <svx/svxdllapi.h>

enum class SvXMLGraphicHelperMode
{
    Read, Write
};

struct SvxGraphicHelperStream_Impl
{
    css::uno::Reference < css::embed::XStorage > xStorage;
    css::uno::Reference < css::io::XStream > xStream;
};

class SVXCORE_DLLPUBLIC SvXMLGraphicHelper final : public cppu::WeakComponentImplHelper<css::document::XGraphicObjectResolver,
                                                                                    css::document::XGraphicStorageHandler,
                                                                                    css::document::XBinaryStreamResolver>
{
private:
    ::osl::Mutex                maMutex;
    css::uno::Reference < css::embed::XStorage > mxRootStorage;
    OUString             maCurStorageName;
    std::vector< css::uno::Reference< css::io::XOutputStream > >
                                maGrfStms;

    std::unordered_map<OUString, css::uno::Reference<css::graphic::XGraphic>> maGraphicObjects;
    std::unordered_map<Graphic, std::pair<OUString, OUString>> maExportGraphics;

    SvXMLGraphicHelperMode      meCreateMode;
    OUString                    maOutputMimeType;

    SVX_DLLPRIVATE static bool          ImplGetStreamNames( const OUString& rURLStr,
                                                    OUString& rPictureStorageName,
                                                    OUString& rPictureStreamName );
    SVX_DLLPRIVATE css::uno::Reference < css::embed::XStorage >
                                            ImplGetGraphicStorage( const OUString& rPictureStorageName );
    SVX_DLLPRIVATE SvxGraphicHelperStream_Impl
                                            ImplGetGraphicStream( const OUString& rPictureStorageName,
                                                      const OUString& rPictureStreamName );
    SVX_DLLPRIVATE static OUString      ImplGetGraphicMimeType( const OUString& rFileName );
    SVX_DLLPRIVATE Graphic                  ImplReadGraphic( const OUString& rPictureStorageName,
                                                 const OUString& rPictureStreamName );

                                SvXMLGraphicHelper();
                                virtual ~SvXMLGraphicHelper() override;
    void                        Init( const css::uno::Reference < css::embed::XStorage >& xXMLStorage,
                                      SvXMLGraphicHelperMode eCreateMode,
                                      const OUString& rGraphicMimeType = OUString() );

    virtual void SAL_CALL       disposing() override;

    SVX_DLLPRIVATE OUString implSaveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                                            OUString & rOutMimeType,
                                            std::u16string_view rRequestName);

public:
                                SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode );

    static rtl::Reference<SvXMLGraphicHelper> Create( const css::uno::Reference < css::embed::XStorage >& rXMLStorage,
                                        SvXMLGraphicHelperMode eCreateMode );
    static rtl::Reference<SvXMLGraphicHelper>  Create( SvXMLGraphicHelperMode eCreateMode,
                                        const OUString& rMimeType = OUString() );

public:

    // XGraphicObjectResolver
    virtual OUString SAL_CALL resolveGraphicObjectURL( const OUString& aURL ) override;

    // XGraphicStorageHandler
    virtual css::uno::Reference<css::graphic::XGraphic> SAL_CALL
        loadGraphic(OUString const & aURL) override;

    virtual css::uno::Reference<css::graphic::XGraphic> SAL_CALL
        loadGraphicFromOutputStream(css::uno::Reference<css::io::XOutputStream> const & rxOutputStream) override;

    virtual OUString SAL_CALL
        saveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic) override;

    virtual OUString SAL_CALL
        saveGraphicByName(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString & rOutSavedMimeType, OUString const & rRequestName) override;

    virtual css::uno::Reference<css::io::XInputStream> SAL_CALL
        createInputStream(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic) override;

    // XBinaryStreamResolver
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream( const OUString& rURL ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL createOutputStream(  ) override;
    virtual OUString SAL_CALL resolveOutputStream( const css::uno::Reference< css::io::XOutputStream >& rxBinaryStream ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

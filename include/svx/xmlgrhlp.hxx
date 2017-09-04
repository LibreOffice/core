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
#include <svtools/grfmgr.hxx>
#include <vector>
#include <set>
#include <utility>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
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

class SVX_DLLPUBLIC SvXMLGraphicHelper : public cppu::WeakComponentImplHelper< css::document::XGraphicObjectResolver,
                                                                    css::document::XBinaryStreamResolver >
{
private:

    typedef ::std::pair< OUString, OUString >                                             URLPair;
    typedef ::std::vector< URLPair >                                                                    URLPairVector;
    typedef ::std::vector< GraphicObject >                                                              GraphicObjectVector;
    typedef ::std::set< OUString >                                                               URLSet;
    typedef ::std::vector< css::uno::Reference< css::io::XOutputStream > >    GraphicOutputStreamVector;

    ::osl::Mutex                maMutex;
    css::uno::Reference < css::embed::XStorage > mxRootStorage;
    OUString             maCurStorageName;
    URLPairVector               maGrfURLs;
    GraphicObjectVector         maGrfObjs;
    GraphicOutputStreamVector   maGrfStms;
    URLSet                      maURLSet;
    SvXMLGraphicHelperMode      meCreateMode;
    OUString                    maOutputMimeType;
    bool                        mbDirect;

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
    SVX_DLLPRIVATE bool                 ImplWriteGraphic( const OUString& rPictureStorageName,
                                                  const OUString& rPictureStreamName,
                                                  const OUString& rGraphicId,
                                                  bool bUseGfxLink );
    SVX_DLLPRIVATE void                     ImplInsertGraphicURL( const OUString& rURLStr, sal_uInt32 nInsertPos, OUString const & rRequestedFileName );

protected:
                                SvXMLGraphicHelper();
                                virtual ~SvXMLGraphicHelper() override;
    void                        Init( const css::uno::Reference < css::embed::XStorage >& xXMLStorage,
                                      SvXMLGraphicHelperMode eCreateMode,
                                      const OUString& rGraphicMimeType,
                                      bool bDirect );

    virtual void SAL_CALL       disposing() override;

public:
                                SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode );

    static SvXMLGraphicHelper*  Create( const css::uno::Reference < css::embed::XStorage >& rXMLStorage,
                                        SvXMLGraphicHelperMode eCreateMode,
                                        const OUString& rGraphicMimeType,
                                        bool bDirect = true );
    static SvXMLGraphicHelper*  Create( SvXMLGraphicHelperMode eCreateMode );

    static void                 Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper );

public:

    // XGraphicObjectResolver
    virtual OUString SAL_CALL resolveGraphicObjectURL( const OUString& aURL ) override;

    // XBinaryStreamResolver
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream( const OUString& rURL ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL createOutputStream(  ) override;
    virtual OUString SAL_CALL resolveOutputStream( const css::uno::Reference< css::io::XOutputStream >& rxBinaryStream ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

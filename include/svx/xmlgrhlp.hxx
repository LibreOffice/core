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

#include <cppuhelper/compbase2.hxx>
#include <osl/mutex.hxx>
#include <svtools/grfmgr.hxx>
#include <vector>
#include <set>
#include <utility>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <svx/svxdllapi.h>

enum SvXMLGraphicHelperMode
{
    GRAPHICHELPER_MODE_READ = 0,
    GRAPHICHELPER_MODE_WRITE = 1
};

struct SvxGraphicHelperStream_Impl
{
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage > xStorage;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XStream > xStream;
};

class SVX_DLLPUBLIC SvXMLGraphicHelper : public ::cppu::WeakComponentImplHelper2<   ::com::sun::star::document::XGraphicObjectResolver,
                                                                    ::com::sun::star::document::XBinaryStreamResolver >
{
private:

    typedef ::std::pair< OUString, OUString >                                             URLPair;
    typedef ::std::vector< URLPair >                                                                    URLPairVector;
    typedef ::std::vector< GraphicObject >                                                              GraphicObjectVector;
    typedef ::std::set< OUString >                                                               URLSet;
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > >    GraphicOutputStreamVector;

    ::osl::Mutex                maMutex;
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage > mxRootStorage;
    OUString             maCurStorageName;
    URLPairVector               maGrfURLs;
    GraphicObjectVector         maGrfObjs;
    GraphicOutputStreamVector   maGrfStms;
    URLSet                      maURLSet;
    SvXMLGraphicHelperMode      meCreateMode;
    bool                        mbDirect;

    SVX_DLLPRIVATE bool                 ImplGetStreamNames( const OUString& rURLStr,
                                                    OUString& rPictureStorageName,
                                                    OUString& rPictureStreamName );
    SVX_DLLPRIVATE css::uno::Reference < css::embed::XStorage >
                                            ImplGetGraphicStorage( const OUString& rPictureStorageName );
    SVX_DLLPRIVATE SvxGraphicHelperStream_Impl
                                            ImplGetGraphicStream( const OUString& rPictureStorageName,
                                                      const OUString& rPictureStreamName,
                                                      bool bTruncate );
    SVX_DLLPRIVATE OUString                 ImplGetGraphicMimeType( const OUString& rFileName ) const;
    SVX_DLLPRIVATE Graphic                  ImplReadGraphic( const OUString& rPictureStorageName,
                                                 const OUString& rPictureStreamName );
    SVX_DLLPRIVATE bool                 ImplWriteGraphic( const OUString& rPictureStorageName,
                                                  const OUString& rPictureStreamName,
                                                  const OUString& rGraphicId,
                                                  bool bUseGfxLink );
    SVX_DLLPRIVATE void                     ImplInsertGraphicURL( const OUString& rURLStr, sal_uInt32 nInsertPos, OUString& rRequestedFileName );

protected:
                                SvXMLGraphicHelper();
                                virtual ~SvXMLGraphicHelper();
    void                        Init( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage >& xXMLStorage,
                                      SvXMLGraphicHelperMode eCreateMode,
                                      bool bDirect );

    virtual void SAL_CALL       disposing() override;

public:
                                SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode );

    static SvXMLGraphicHelper*  Create( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage >& rXMLStorage,
                                        SvXMLGraphicHelperMode eCreateMode,
                                        bool bDirect = true );
    static SvXMLGraphicHelper*  Create( SvXMLGraphicHelperMode eCreateMode );

    static void                 Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper );

public:

    // XGraphicObjectResolver
    virtual OUString SAL_CALL resolveGraphicObjectURL( const OUString& aURL ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XBinaryStreamResolver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream( const OUString& rURL ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL createOutputStream(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL resolveOutputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxBinaryStream ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

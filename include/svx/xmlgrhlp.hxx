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

#ifndef _XMLGRHLP_HXX
#define _XMLGRHLP_HXX

#include <cppuhelper/compbase2.hxx>
#include <osl/mutex.hxx>
#include <svtools/grfmgr.hxx>
#include <vector>
#include <set>
#include <utility>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include "svx/svxdllapi.h"

// ----------------------
// - SvXMLGraphicHelper -
// ----------------------

enum SvXMLGraphicHelperMode
{
    GRAPHICHELPER_MODE_READ = 0,
    GRAPHICHELPER_MODE_WRITE = 1
};

// ----------------------
// - SvXMLGraphicHelper -
// ----------------------
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
    sal_Bool                    mbDirect;

    SVX_DLLPRIVATE sal_Bool                 ImplGetStreamNames( const OUString& rURLStr,
                                                    OUString& rPictureStorageName,
                                                    OUString& rPictureStreamName );
    SVX_DLLPRIVATE css::uno::Reference < css::embed::XStorage >
                                            ImplGetGraphicStorage( const OUString& rPictureStorageName );
    SVX_DLLPRIVATE SvxGraphicHelperStream_Impl
                                            ImplGetGraphicStream( const OUString& rPictureStorageName,
                                                      const OUString& rPictureStreamName,
                                                      sal_Bool bTruncate );
    SVX_DLLPRIVATE OUString                 ImplGetGraphicMimeType( const OUString& rFileName ) const;
    SVX_DLLPRIVATE Graphic                  ImplReadGraphic( const OUString& rPictureStorageName,
                                                 const OUString& rPictureStreamName );
    SVX_DLLPRIVATE sal_Bool                 ImplWriteGraphic( const OUString& rPictureStorageName,
                                                  const OUString& rPictureStreamName,
                                                  const OUString& rGraphicId,
                                                  bool bUseGfxLink );
    SVX_DLLPRIVATE void                     ImplInsertGraphicURL( const OUString& rURLStr, sal_uInt32 nInsertPos, OUString& rRequestedFileName );

protected:
                                SvXMLGraphicHelper();
                                ~SvXMLGraphicHelper();
    void                        Init( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage >& xXMLStorage,
                                      SvXMLGraphicHelperMode eCreateMode,
                                      sal_Bool bDirect );

    virtual void SAL_CALL       disposing();

public:
                                SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode );

    static SvXMLGraphicHelper*  Create( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage >& rXMLStorage,
                                        SvXMLGraphicHelperMode eCreateMode,
                                        sal_Bool bDirect = sal_True );
    static SvXMLGraphicHelper*  Create( SvXMLGraphicHelperMode eCreateMode );

    static void                 Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper );

public:

    // XGraphicObjectResolver
    virtual OUString SAL_CALL resolveGraphicObjectURL( const OUString& aURL ) throw(::com::sun::star::uno::RuntimeException);

    // XBinaryStreamResolver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream( const OUString& rURL ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL createOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL resolveOutputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxBinaryStream ) throw (::com::sun::star::uno::RuntimeException);
};


// for instantiation via service manager
namespace svx
{
/** Create this with createInstanceWithArguments. service name
    "com.sun.star.comp.Svx.GraphicImportHelper", one argument which is the
    XStorage.  Without arguments no helper class is created.  With an empty
    argument the helper class is created and initialized like in the CTOR to
    SvXMLGraphicHelper that only gets the create mode.

    You should call dispose after you no longer need this component.

    uses eCreateMode == GRAPHICHELPER_MODE_READ, bDirect == sal_True in
    SvXMLGraphicHelper
 */
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvXMLGraphicImportHelper_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw( ::com::sun::star::uno::Exception );
SVX_DLLPUBLIC ::com::sun::star::uno::Sequence< OUString > SAL_CALL SvXMLGraphicImportHelper_getSupportedServiceNames() throw();
SVX_DLLPUBLIC OUString SAL_CALL SvXMLGraphicImportHelper_getImplementationName() throw();

/** Create this with createInstanceWithArguments. service name
    "com.sun.star.comp.Svx.GraphicExportHelper", one argument which is the
    XStorage.  Without arguments no helper class is created.  With an empty
    argument the helper class is created and initialized like in the CTOR to
    SvXMLGraphicHelper that only gets the create mode

    To write the Pictures stream, you have to call dispose at this component.
    Make sure you call dipose before you commit the parent storage.

    uses eCreateMode == GRAPHICHELPER_MODE_WRITE, bDirect == sal_True in
    SvXMLGraphicHelper
 */
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SvXMLGraphicExportHelper_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw( ::com::sun::star::uno::Exception );
SVX_DLLPUBLIC ::com::sun::star::uno::Sequence< OUString > SAL_CALL SvXMLGraphicExportHelper_getSupportedServiceNames() throw();
SVX_DLLPUBLIC OUString SAL_CALL SvXMLGraphicExportHelper_getImplementationName() throw();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

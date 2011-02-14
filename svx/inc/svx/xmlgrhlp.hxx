/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

    typedef ::std::pair< ::rtl::OUString, ::rtl::OUString >                                             URLPair;
    typedef ::std::vector< URLPair >                                                                    URLPairVector;
    typedef ::std::vector< GraphicObject >                                                              GraphicObjectVector;
    typedef ::std::set< ::rtl::OUString >                                                               URLSet;
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > >    GraphicOutputStreamVector;

    ::osl::Mutex                maMutex;
    ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage > mxRootStorage;
    ::rtl::OUString             maCurStorageName;
    URLPairVector               maGrfURLs;
    GraphicObjectVector         maGrfObjs;
    GraphicOutputStreamVector   maGrfStms;
    URLSet                      maURLSet;
    SvXMLGraphicHelperMode      meCreateMode;
    sal_Bool                    mbDirect;

    SVX_DLLPRIVATE sal_Bool                 ImplGetStreamNames( const ::rtl::OUString& rURLStr,
                                                    ::rtl::OUString& rPictureStorageName,
                                                    ::rtl::OUString& rPictureStreamName );
    SVX_DLLPRIVATE ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage > ImplGetGraphicStorage( const ::rtl::OUString& rPictureStorageName );
    SVX_DLLPRIVATE SvxGraphicHelperStream_Impl ImplGetGraphicStream( const ::rtl::OUString& rPictureStorageName,
                                                      const ::rtl::OUString& rPictureStreamName,
                                                      sal_Bool bTruncate );
    SVX_DLLPRIVATE String                      ImplGetGraphicMimeType( const String& rFileName ) const;
    SVX_DLLPRIVATE Graphic                      ImplReadGraphic( const ::rtl::OUString& rPictureStorageName,
                                                 const ::rtl::OUString& rPictureStreamName );
    SVX_DLLPRIVATE sal_Bool                 ImplWriteGraphic( const ::rtl::OUString& rPictureStorageName,
                                                  const ::rtl::OUString& rPictureStreamName,
                                                  const ::rtl::OUString& rGraphicId );
    SVX_DLLPRIVATE void                     ImplInsertGraphicURL( const ::rtl::OUString& rURLStr, sal_uInt32 nInsertPos, rtl::OUString& rRequestedFileName );

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
    virtual ::rtl::OUString SAL_CALL resolveGraphicObjectURL( const ::rtl::OUString& aURL ) throw(::com::sun::star::uno::RuntimeException);

    // XBinaryStreamResolver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream( const ::rtl::OUString& rURL ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL createOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL resolveOutputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxBinaryStream ) throw (::com::sun::star::uno::RuntimeException);
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
SVX_DLLPUBLIC ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SvXMLGraphicImportHelper_getSupportedServiceNames() throw();
SVX_DLLPUBLIC ::rtl::OUString SAL_CALL SvXMLGraphicImportHelper_getImplementationName() throw();

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
SVX_DLLPUBLIC ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SvXMLGraphicExportHelper_getSupportedServiceNames() throw();
SVX_DLLPUBLIC ::rtl::OUString SAL_CALL SvXMLGraphicExportHelper_getImplementationName() throw();
}

#endif

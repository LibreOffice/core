/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlgrhlp.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-10-18 13:53:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLGRHLP_HXX
#define _XMLGRHLP_HXX

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif
#include <vector>
#include <set>
#include <utility>

#ifndef _COM_SUN_STAR_DOCUMENT_XGRAPHICOBJECTRESOLVER_HPP_
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XBINARYSTREAMRESOLVER_HPP_
#include <com/sun/star/document/XBinaryStreamResolver.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
                                                      BOOL bTruncate );
    SVX_DLLPRIVATE String                      ImplGetGraphicMimeType( const String& rFileName ) const;
    SVX_DLLPRIVATE Graphic                      ImplReadGraphic( const ::rtl::OUString& rPictureStorageName,
                                                 const ::rtl::OUString& rPictureStreamName );
    SVX_DLLPRIVATE sal_Bool                 ImplWriteGraphic( const ::rtl::OUString& rPictureStorageName,
                                                  const ::rtl::OUString& rPictureStreamName,
                                                  const ::rtl::OUString& rGraphicId );
    SVX_DLLPRIVATE void                     ImplInsertGraphicURL( const ::rtl::OUString& rURLStr, sal_uInt32 nInsertPos );

protected:
                                SvXMLGraphicHelper();
                                ~SvXMLGraphicHelper();
    void                        Init( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage >& xXMLStorage,
                                      SvXMLGraphicHelperMode eCreateMode,
                                      BOOL bDirect );

    virtual void SAL_CALL       disposing();

public:
                                SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode );

    static SvXMLGraphicHelper*  Create( const ::com::sun::star::uno::Reference < ::com::sun::star::embed::XStorage >& rXMLStorage,
                                        SvXMLGraphicHelperMode eCreateMode,
                                        BOOL bDirect = TRUE );
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

#endif

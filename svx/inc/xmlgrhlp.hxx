/*************************************************************************
 *
 *  $RCSfile: xmlgrhlp.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
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

class SotStorage;

class SvXMLGraphicHelper : public ::cppu::WeakComponentImplHelper2< ::com::sun::star::document::XGraphicObjectResolver,
                                                                    ::com::sun::star::document::XBinaryStreamResolver >
{
private:

    typedef ::std::pair< ::rtl::OUString, ::rtl::OUString >                                             URLPair;
    typedef ::std::vector< URLPair >                                                                    URLPairVector;
    typedef ::std::vector< GraphicObject >                                                              GraphicObjectVector;
    typedef ::std::set< ::rtl::OUString >                                                               URLSet;
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > >    GraphicOutputStreamVector;

    ::osl::Mutex                maMutex;
    SotStorage*                 mpRootStorage;
    SotStorageRef               mxGraphicStorage;
    ::rtl::OUString             maCurStorageName;
    URLPairVector               maGrfURLs;
    GraphicObjectVector         maGrfObjs;
    GraphicOutputStreamVector   maGrfStms;
    URLSet                      maURLSet;
    SvXMLGraphicHelperMode      meCreateMode;
    sal_Bool                    mbDirect;

    sal_Bool                    ImplGetStreamNames( const ::rtl::OUString& rURLStr,
                                                    ::rtl::OUString& rPictureStorageName,
                                                    ::rtl::OUString& rPictureStreamName );
    SotStorageRef               ImplGetGraphicStorage( const ::rtl::OUString& rPictureStorageName );
    SotStorageStreamRef         ImplGetGraphicStream( const ::rtl::OUString& rPictureStorageName,
                                                      const ::rtl::OUString& rPictureStreamName,
                                                      BOOL bTruncate );
    String                      ImplGetGraphicMimeType( const String& rFileName ) const;
    Graphic                     ImplReadGraphic( const ::rtl::OUString& rPictureStorageName,
                                                 const ::rtl::OUString& rPictureStreamName );
    sal_Bool                    ImplWriteGraphic( const ::rtl::OUString& rPictureStorageName,
                                                  const ::rtl::OUString& rPictureStreamName,
                                                  const ::rtl::OUString& rGraphicId );
    void                        ImplInsertGraphicURL( const ::rtl::OUString& rURLStr, sal_uInt32 nInsertPos );

protected:
                                SvXMLGraphicHelper();
                                ~SvXMLGraphicHelper();
    void                        Init( SotStorage* pXMLStorage,
                                      SvXMLGraphicHelperMode eCreateMode,
                                      BOOL bDirect );

    virtual void SAL_CALL       disposing();

public:
                                SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode );

    static SvXMLGraphicHelper*  Create( SotStorage& rXMLStorage,
                                        SvXMLGraphicHelperMode eCreateMode,
                                        BOOL bDirect = TRUE );
    static SvXMLGraphicHelper*  Create( SvXMLGraphicHelperMode eCreateMode );

    static void                 Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper );

    void                        Flush();

public:

    // XGraphicObjectResolver
    virtual ::rtl::OUString SAL_CALL resolveGraphicObjectURL( const ::rtl::OUString& aURL ) throw(::com::sun::star::uno::RuntimeException);

    // XBinaryStreamResolver
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getInputStream( const ::rtl::OUString& rURL ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > SAL_CALL createOutputStream(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL resolveOutputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rxBinaryStream ) throw (::com::sun::star::uno::RuntimeException);
};

#endif

/*************************************************************************
 *
 *  $RCSfile: localfilelayer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cyrillem $ $Date: 2002-06-07 16:47:07 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_
#define CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XUPDATABLELAYER_HPP_
#include <drafts/com/sun/star/configuration/backend/XUpdatableLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XUPDATABLELAYER_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XTIMESTAMPED_HPP_
#include <drafts/com/sun/star/configuration/backend/XTimeStamped.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XTIMESTAMPED_HPP_

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif // _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif // _CPPUHELPER_IMPLBASE2_HXX_

namespace configmgr {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
//namespace backend = css::configuration::backend ;
namespace backend = drafts::com::sun::star::configuration::backend ;

/**
  Implementation of the XUpdatableLayer interface for a local
  file access.
  The read data is accessible through a canned implementation of
  an XML parser, and the write data is defined through a canned
  implementation of an XML writer.
  The layer is defined by the URL of the file containing its
  contents, and that file will be either read or updated by
  the access to the handlers.
  The timestamp is refreshed on each read operation only.
  */
class LocalFileLayer : public cppu::WeakImplHelper2<backend::XUpdatableLayer,
                                                    backend::XTimeStamped> {
    public :
        /**
          Constructor using the file URL to define the source
          of the layer contents.

          @param xContext   component context, used to access canned services
          @param aFileUrl   file URL
          */
        LocalFileLayer(const uno::Reference<uno::XComponentContext>& xContext,
                       const rtl::OUString& aFileUrl) ;
        /** Destructor */
        ~LocalFileLayer(void) ;
        // XUpdatableLayer
        virtual void SAL_CALL readData(
                const uno::Reference<backend::XLayerHandler>& xHandler)
            throw (uno::RuntimeException) ;
        virtual void SAL_CALL replaceWith(
                const uno::Reference<backend::XLayer>& aNewLayer)
            throw (uno::RuntimeException) ;
        // XTimeStamped
        virtual rtl::OUString SAL_CALL getTimestamp(void)
            throw (uno::RuntimeException) { return mTimestamp ; }

        /**
          Returns the reference to the layer writer.

          @return   layer writer reference
          */
        const uno::Reference<backend::XLayerHandler>&
            getLayerWriter(void) const ;
        /**
          Returns a timestamp in the official backend format
          YYYYMMDDhhmmssZ associated to a file defined by its URL.

          @param aFileUrl   URL of the file
          @return   timestamp
          */
        static rtl::OUString getTimestamp(const rtl::OUString& aFileUrl) ;

    protected :
    private :
        /** Component context for service access */
        const uno::Reference<uno::XComponentContext>& mContext ;
        /** URL of the file being accessed */
        rtl::OUString mFileUrl ;
        /** Timestamp of the data at the last read operation */
        rtl::OUString mTimestamp ;
        /** XLayer implementation used for readData */
        uno::Reference<backend::XLayer> mLayerReader ;
        /** XLayerHandler implementation for getWriteHandler */
        uno::Reference<backend::XLayerHandler> mLayerWriter ;
} ;

} // configmgr

#endif // CONFIGMGR_LOCALBE_LOCALFILELAYER_HXX_

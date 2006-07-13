/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kdepathslayer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 13:21:28 $
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

#ifndef KDEPATHSLAYER_HXX_
#define KDEPATHSLAYER_HXX_

#ifndef KDEBACKEND_HXX_
#include "kdebackend.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

/**
  Implementation of the XLayer interface for the KDE values mapped into
  the org.openoffice.Office.Paths configuration component.
  */
class KDEPathsLayer : public cppu::WeakImplHelper2<backend::XLayer, util::XTimeStamped>
{
public :
    /**
      Constructor given the component context

      @param xContext       The component context
    */

    KDEPathsLayer(const uno::Reference<uno::XComponentContext>& xContext);

    // XLayer
    virtual void SAL_CALL readData(
        const uno::Reference<backend::XLayerHandler>& xHandler)
        throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException) ;

    // XTimeStamped
    virtual rtl::OUString SAL_CALL getTimestamp(void)
        throw (uno::RuntimeException);

    protected:

    /** Destructor */
    ~KDEPathsLayer(void) {}

    private :
        uno::Reference<backend::XLayerContentDescriber> m_xLayerContentDescriber ;
  } ;

#endif // KDEPATHSLAYER

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: kdepathslayer.hxx,v $
 * $Revision: 1.3 $
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

#ifndef KDEPATHSLAYER_HXX_
#define KDEPATHSLAYER_HXX_

#include "kdebackend.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif
#include <com/sun/star/util/XTimeStamped.hpp>
#include <cppuhelper/implbase2.hxx>

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

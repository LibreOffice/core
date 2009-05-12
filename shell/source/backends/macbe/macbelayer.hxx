/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macbelayer.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _MACBELAYER_HXX_
#define _MACBELAYER_HXX_

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/PropertyInfo.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif
#include <com/sun/star/util/XTimeStamped.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase2.hxx>

namespace css = com::sun::star;
namespace uno = css::uno;
namespace lang = css::lang;
namespace backend = css::configuration::backend;
namespace util = css::util;

/**
  Implementation of the XLayer interfaces for fixed values
 */

class MacOSXLayer : public cppu::WeakImplHelper2<backend::XLayer, util::XTimeStamped>
{
public:

    /**
       Constructor given the requested component name

       @param aTimestamp     timestamp indicating last modifictaion
    */
    MacOSXLayer(const uno::Reference<uno::XComponentContext>& xContext);

    /** Destructor */
    virtual ~MacOSXLayer(void) {}

    // XLayer
    virtual void SAL_CALL readData(const uno::Reference<backend::XLayerHandler>& xHandler)
        throw ( backend::MalformedDataException,
                lang::NullPointerException,
                lang::WrappedTargetException,
                uno::RuntimeException);

    // XTimeStamped
    virtual rtl::OUString SAL_CALL getTimestamp(void)
        throw (uno::RuntimeException);

protected:

    rtl::OUString m_aComponent;

    uno::Reference<backend::XLayerContentDescriber> m_xLayerContentDescriber;

};

class MacOSXPathLayer : public MacOSXLayer
{
    public:
    MacOSXPathLayer(const uno::Reference<uno::XComponentContext>& i_xContext) :
        MacOSXLayer( i_xContext ) {}
    virtual ~MacOSXPathLayer() {}

    // XLayer
    virtual void SAL_CALL readData(const uno::Reference<backend::XLayerHandler>& i_xHandler)
        throw ( backend::MalformedDataException,
                lang::NullPointerException,
                lang::WrappedTargetException,
                uno::RuntimeException);
};

#endif // _MACBELAYER_HXX_

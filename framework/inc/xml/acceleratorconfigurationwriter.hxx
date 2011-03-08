/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_

//_______________________________________________
// own includes

#include <accelerators/acceleratorcache.hxx>
#include <accelerators/keymapping.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>

//_______________________________________________
// interface includes

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

//_______________________________________________
// other includes
#include <salhelper/singletonref.hxx>
#include <rtl/ustring.hxx>

namespace framework{

class AcceleratorConfigurationWriter : private ThreadHelpBase
{
    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short  needed to write the xml configuration. */
        css::uno::Reference< css::xml::sax::XDocumentHandler > m_xConfig;

        //---------------------------------------
        /** @short  reference to the outside container, where this
                    writer must work on. */
        const AcceleratorCache& m_rContainer;

        //---------------------------------------
        /** @short  is used to map key codes to its
                    string representation.

            @descr  To perform this operatio is
                    created only one times and holded
                    alive forever ...*/
        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        /** @short  connect this new writer instance
                    to an outside container, which should be
                    flushed to the underlying XML configuration.

            @param  rContainer
                    a reference to the outside container.

            @param  xConfig
                    used to write the configuration there.
          */
        AcceleratorConfigurationWriter(const AcceleratorCache&                                       rContainer,
                                       const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig   );

        //---------------------------------------
        /** @short  does nothing real ... */
        virtual ~AcceleratorConfigurationWriter();

        //---------------------------------------
        /** @short  TODO */
        virtual void flush();

    //-------------------------------------------
    // helper

    private:

        //---------------------------------------
        /** @short  TODO */
        void impl_ts_writeKeyCommandPair(const css::awt::KeyEvent&                                     aKey    ,
                                         const ::rtl::OUString&                                        sCommand,
                                         const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig );
};

} // namespace framework

#endif // _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

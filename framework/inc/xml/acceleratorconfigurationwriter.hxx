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

#ifndef _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_

#include <accelerators/acceleratorcache.hxx>
#include <accelerators/keymapping.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

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

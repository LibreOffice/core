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

#ifndef _UCBHELPER_ACTIVEDATASTREAMER_HXX
#define _UCBHELPER_ACTIVEDATASTREAMER_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>

namespace ucbhelper
{

//=========================================================================

/**
  * This class implements the interface com::sun::star::io::XActiveDataStreamer.
  * Instances of this class can be passed with the parameters of an
  * "open" command.
  */

class ActiveDataStreamer : public cppu::OWeakObject,
                           public com::sun::star::lang::XTypeProvider,
                           public com::sun::star::io::XActiveDataStreamer
{
    com::sun::star::uno::Reference<
        com::sun::star::io::XStream > m_xStream;

public:
     // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XActiveDataStreamer methods.
    virtual void SAL_CALL setStream( const com::sun::star::uno::Reference< com::sun::star::io::XStream >& xStream )
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference< com::sun::star::io::XStream > SAL_CALL getStream()
        throw( com::sun::star::uno::RuntimeException );
};

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_ACTIVEDATASTREAMER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

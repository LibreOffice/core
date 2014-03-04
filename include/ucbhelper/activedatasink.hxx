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

#ifndef INCLUDED_UCBHELPER_ACTIVEDATASINK_HXX
#define INCLUDED_UCBHELPER_ACTIVEDATASINK_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace ucbhelper
{



/**
  * This class implements the interface com::sun::star::io::XActiveDataSink.
  * Instances of this class can be passed with the parameters of an
  * "open" command.
  */

class UCBHELPER_DLLPUBLIC ActiveDataSink : public cppu::OWeakObject,
                       public com::sun::star::lang::XTypeProvider,
                       public com::sun::star::io::XActiveDataSink
{
    com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > m_xStream;

public:
    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XActiveDataSink methods.
    virtual void SAL_CALL
    setInputStream( const com::sun::star::uno::Reference<
                            com::sun::star::io::XInputStream >& aStream )
        throw( com::sun::star::uno::RuntimeException, std::exception );
    virtual com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > SAL_CALL
    getInputStream()
        throw( com::sun::star::uno::RuntimeException, std::exception );
};

} /* namespace ucbhelper */

#endif /* ! INCLUDED_UCBHELPER_ACTIVEDATASINK_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

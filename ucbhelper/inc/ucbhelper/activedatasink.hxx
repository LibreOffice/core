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

#ifndef _UCBHELPER_ACTIVEDATASINK_HXX
#define _UCBHELPER_ACTIVEDATASINK_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper
{

//=========================================================================

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
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XActiveDataSink methods.
    virtual void SAL_CALL
    setInputStream( const com::sun::star::uno::Reference<
                            com::sun::star::io::XInputStream >& aStream )
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Reference<
        com::sun::star::io::XInputStream > SAL_CALL
    getInputStream()
        throw( com::sun::star::uno::RuntimeException );
};

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_ACTIVEDATASINK_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

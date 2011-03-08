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

#ifndef _UCBHELPER_COMMANDENVIRONMENT_HXX
#define _UCBHELPER_COMMANDENVIRONMENT_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace ucbhelper
{

struct CommandEnvironment_Impl;

//=========================================================================

/**
  * This class implements the interface
  * com::sun::star::ucb::XCommandEnvironement. Instances of this class can
  * be used to supply environments to commands executed by UCB contents.
  */
class UCBHELPER_DLLPUBLIC CommandEnvironment : public cppu::OWeakObject,
                           public com::sun::star::lang::XTypeProvider,
                           public com::sun::star::ucb::XCommandEnvironment
{
    CommandEnvironment_Impl* m_pImpl;

private:
    UCBHELPER_DLLPRIVATE CommandEnvironment( const CommandEnvironment& );               // n.i.
    UCBHELPER_DLLPRIVATE CommandEnvironment& operator=( const CommandEnvironment& );    // n.i.

public:
    /**
      * Constructor.
      *
      * @param rxInteractionHandler is the implementation of an Interaction
      *        Handler or an empty reference.
      * @param rxProgressHandler is the implementation of a Progress
      *        Handler or an empty reference.
      */
    CommandEnvironment(
            const com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionHandler >&
                    rxInteractionHandler,
            const com::sun::star::uno::Reference<
                com::sun::star::ucb::XProgressHandler >&
                    rxProgressHandler );
    /**
      * Destructor.
      */
    virtual ~CommandEnvironment();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

     // XCommandEnvironemnt
    virtual com::sun::star::uno::Reference<
                com::sun::star::task::XInteractionHandler > SAL_CALL
    getInteractionHandler()
        throw ( com::sun::star::uno::RuntimeException );

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XProgressHandler > SAL_CALL
    getProgressHandler()
        throw ( com::sun::star::uno::RuntimeException );
};

} /* namespace ucbhelper */

#endif /* !_UCBHELPER_COMMANDENVIRONMENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

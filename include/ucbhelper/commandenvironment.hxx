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

#ifndef INCLUDED_UCBHELPER_COMMANDENVIRONMENT_HXX
#define INCLUDED_UCBHELPER_COMMANDENVIRONMENT_HXX

#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace ucbhelper
{

struct CommandEnvironment_Impl;



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
        throw ( com::sun::star::uno::RuntimeException, std::exception );

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XProgressHandler > SAL_CALL
    getProgressHandler()
        throw ( com::sun::star::uno::RuntimeException, std::exception );
};

} /* namespace ucbhelper */

#endif /* ! INCLUDED_UCBHELPER_COMMANDENVIRONMENT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_UNOTOOLS_CONFIGVALUECONTAINER_HXX
#define INCLUDED_UNOTOOLS_CONFIGVALUECONTAINER_HXX

#include <config_options.h>
#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/uno/Type.hxx>
#include <memory>
#include <mutex>


namespace com::sun::star::uno { template <typename > class Reference; }
namespace com::sun::star::uno { class XComponentContext; }
namespace osl { class Mutex; }

namespace utl
{

    struct OConfigurationValueContainerImpl;
    struct NodeValueAccessor;

    //= OConfigurationValueContainer

    /** allows simple access to static configuration structures.

        <p>The basic idea of this class is that it's clients (usually derived classes) simply register an
        address in memory and a node path, and upon explicit request, the configuration value and the memory
        are synchronized.<br/>
        This means that when calling <method>read</method>, the current configuration values are copied into
        the memory registered for them, and upon calling <method>write</method> the current values in memory
        are set in the configuration nodes.</p>

        <p>This way, the usage of this class is pretty straight forward: derive your own class, spend some members
        to it, and bind these members to configuration node (usually done in the ctor of the derived class).<br/>
        In the dtor, simply call <method>write</method> and <method>commit</method>.</p>

        <p>There is no auto-commit mechanism in the dtor: In the usual scenario, when you derive from this class
        and bind some members of your derived class to config nodes, this means that your members will be destroyed
        before your base class' dtor is called, so accessing the memory during such a theoretical auto-commit would
        yield undefined behaviour.</p>
    */
    class UNLESS_MERGELIBS(UNOTOOLS_DLLPUBLIC) OConfigurationValueContainer
    {
    private:
        std::unique_ptr<OConfigurationValueContainerImpl> m_pImpl;

    protected:

        // construction/destruction

        /** constructs the object

            @param _rxORB
                specifies the service factory which should be used to access the configuration
            @param _rAccessSafety
                As this class is intended to manipulate objects it does not hold itself (see the various
                registerXXX methods), it needs to guard these access for multi threading safety.<br/>
                The mutex given here is locked whenever such an access occurs.
            @param _rConfigLocation
                is an ASCII string describing the configurations node path
            @param _nAccessFlags
                specifies different aspects of the configuration aspect to be created, e.g. it's update mode etc.<br/>
                See the CVC_xxx constants for what you can use here.
            @param _nLevels
                specifies the number of levels to access under the node given by <arg>_pConfigLocation</arg>
        */
        OConfigurationValueContainer(
            const css::uno::Reference< css::uno::XComponentContext >& _rxORB,
            std::mutex& _rAccessSafety,
            const OUString& _rConfigLocation,
            const sal_Int32 _nLevels
        );

        /// dtor
        ~OConfigurationValueContainer();

        // registering data containers

        /** registers a data accessor of an arbitrary type.

            <p>Usually, in your derived class you simply add a member of the correct type of the configuration
            value, and then call this method with the address of this member.</p>

            @param _rRelativePathAscii
                is a relative (ASCII) path of the node which should be "mirrored" into the accessor.
            @param _pContainer
                points to the accessors location in memory. Usually, this is simply an address of your derived class
            @param _rValueType
                is the type of your accessor. This type must be supported by the configuration.
        */
        void    registerExchangeLocation(
            const OUString& _rRelativePathAscii,
            void* _pContainer,
            const css::uno::Type& _rValueType
        );

    public:
        /** reads the configuration data

            <p>The current values of the nodes bound (using the registerXXX methods) is copied into their
            respective exchange locations.</p>

            <p>Please note that any changes done to your exchange locations are overridden with the current config
            values.</p>

            @see write
        */
        void    read( );

        /** commits any changes done

            <p>Note that calling <method>write</method>(<sal_True/) is the same as calling <method>commit</method>(<TRUE/>).</p>

            The current values in the exchange locations are written to the configuration nodes
            before the changes are committed.<br/>

            @precond
                The access must have been created for update access
        */
        void    commit();

    private:
        /// implements the ctors
        void implConstruct(
            const OUString& _rConfigLocation,
            const sal_Int32 _nLevels
        );

        /// registers a value container
        void    implRegisterExchangeLocation( const NodeValueAccessor& _rAccessor );
    };

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_CONFIGVALUECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

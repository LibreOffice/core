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
#include "unotools/unotoolsdllapi.h"

#ifndef UNOTOOLS_CONFIGVALUECONTAINER_HXX
#define UNOTOOLS_CONFIGVALUECONTAINER_HXX
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/mutex.hxx>

//.........................................................................
namespace utl
{
//.........................................................................

#define CVC_READONLY_ACCESS     0x0000
#define CVC_UPDATE_ACCESS       0x0001

#define CVC_LAZY_UPDATE         0x0000
#define CVC_IMMEDIATE_UPDATE    0x0002

    struct OConfigurationValueContainerImpl;
    struct NodeValueAccessor;
    //=====================================================================
    //= OConfigurationValueContainer
    //=====================================================================
    /** allows simple access to static configuration structures.

        <p>The basic idea of this class is that it's clients (usually derived classes) simply register an
        address in memory and a node path, and upon explicit request, the configuration value and the memory
        are syncronized.<br/>
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
    class UNOTOOLS_DLLPUBLIC OConfigurationValueContainer
    {
    private:
        OConfigurationValueContainerImpl*
                    m_pImpl;

    protected:
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&
                    getServiceFactory( ) const;

    protected:
        //-----------------------------------------------------------------
        // construction/destruction

        /** constructs the object

            @param _rxORB
                specifies the service factory which should be used to access the configuration
            @param _rAccessSafety
                As this class is intented to manipulate objects it does not hold itself (see the various
                registerXXX methods), it needs to guard these access for muti threading safety.<br/>
                The mutex given here is locked whenever such an access occurs.
            @param _pConfigLocation
                is an ASCII string describing the configurations node path
            @param _nAccessFlags
                specifies different aspects of the configuration aspect to be created, e.g. it's update mode etc.<br/>
                See the CVC_xxx constants for what you can use here.
            @param _nLevels
                specifies the number of levels to access under the node given by <arg>_pConfigLocation</arg>
        */
        OConfigurationValueContainer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            ::osl::Mutex& _rAccessSafety,
            const sal_Char* _pConfigLocation,
            const sal_uInt16 _nAccessFlags = CVC_UPDATE_ACCESS | CVC_LAZY_UPDATE,
            const sal_Int32 _nLevels = -1
        );

        /** constructs the object

            @param _rxORB
                specifies the service factory which should be used to access the configuration
            @param _rAccessSafety
                As this class is intented to manipulate objects it does not hold itself (see the various
                registerXXX methods), it needs to guard these access for muti threading safety.<br/>
                The mutex given here is locked whenever such an access occurs.
            @param _rConfigLocation
                describes the configurations node path
            @param _nAccessFlags
                specifies different aspects of the configuration aspect to be created, e.g. it's update mode etc.<br/>
                See the CVC_xxx constants for what you can use here.
            @param _nLevels
                specifies the number of levels to access under the node given by <arg>_pConfigLocation</arg>
        */
        OConfigurationValueContainer(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
            ::osl::Mutex& _rAccessSafety,
            const ::rtl::OUString& _rConfigLocation,
            const sal_uInt16 _nAccessFlags = CVC_UPDATE_ACCESS | CVC_LAZY_UPDATE,
            const sal_Int32 _nLevels = -1
        );

        /// dtor
        ~OConfigurationValueContainer();

        //-----------------------------------------------------------------
        // registering data containers

        /** registers a data accessor of an arbitrary type.

            <p>Usually, in your derived class you simply add a member of the correct type of the configuration
            value, and then call this method with the address of this member.</p>

            <p>If the value you want to access may be <NULL/> at runtime, and if you want to recognize such
            <NULL/> values, you may consider using <method>registerNullValueExchangeLocation</method>.</p>

            @param _pRelativePathAscii
                is a relative (ASCII) path of the node which should be "mirrored" into the accessor.
            @param _pContainer
                points to the accessors location in memory. Usually, this is simply an address of your derived class
            @param _rValueType
                is the type of your accessort. This type must be supported by the configuration.
        */
        void    registerExchangeLocation(
            const sal_Char* _pRelativePathAscii,
            void* _pContainer,
            const ::com::sun::star::uno::Type& _rValueType
        );

        /** registers a data accessor of an arbitrary type.

            <p>Usually, in your derived class you simply add a member of type <type scope="com.sun.star.uno">Any</type>,
            and then call this method with the address of this member.</p>

            @param _pRelativePathAscii
                is a relative (ASCII) path of the node which should be "mirrored" into the accessor.
            @param _pContainer
                points to the Any you want to hold the value
        */
        void    registerNullValueExchangeLocation(
            const sal_Char* _pRelativePathAscii,
            ::com::sun::star::uno::Any* _pContainer
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

        /** updates the configuration data

            <p>The current values in memory (your exchange locations registered using the registerXXX methods) is
            forwarded to their respective configuration nodes.</p>

            <p>Note that calling <method>write</method>(<sal_True/) is the same as calling <method>commit</method>(<TRUE/>).</p>

            @precond
                The access must have been created for update access

            @param _bCommit
                If set to <TRUE/>, an automatic commit is done after the values have been synchronized.<br/>
                If set to <FALSE/>, you must explicitly call <method>commit</method> to make your changes persistent.

            @see read
            @see commit
        */
        void    write( sal_Bool _bCommit = sal_True );

        /** commits any changes done

            <p>Note that calling <method>write</method>(<sal_True/) is the same as calling <method>commit</method>(<TRUE/>).</p>

            @precond
                The access must have been created for update access

            @param _bWrite
                If <TRUE/>, the current values in the exchange locations are written to the configuration nodes
                before the changes are committed.<br/>
                If <FALSE/>, only the current values in the config nodes (as present since the last call to
                <method>write</method>) are committed.
        */
        void    commit( sal_Bool _bWrite = sal_True );

    private:
        /// implements the ctors
        void implConstruct(
            const ::rtl::OUString& _rConfigLocation,
            const sal_uInt16 _nAccessFlags,
            const sal_Int32 _nLevels
        );

        /// registers a value container
        void    implRegisterExchangeLocation( const NodeValueAccessor& _rAccessor );
    };

//.........................................................................
}   // namespace utl
//.........................................................................

#endif // UNOTOOLS_CONFIGVALUECONTAINER_HXX


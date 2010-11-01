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

#ifndef SVTOOLS_SOURCE_FILEPICKER_FPSMARTCONTENT_HXX
#define SVTOOLS_SOURCE_FILEPICKER_FPSMARTCONTENT_HXX

#include "fpinteraction.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
/** === end UNO includes === **/
#include <ucbhelper/content.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= SmartContent
    //====================================================================
    /** a "smart content" which basically wraps an UCB content, but caches some informations
        so that repeatedly recreating it may be faster
    */
    class SmartContent
    {
    public:
        enum State
        {
            NOT_BOUND,  // never bound
            UNKNOWN,    // bound, but validity is unknown
            VALID,      // bound to an URL, and valid
            INVALID     // bound to an URL, and invalid
        };

    private:
        ::rtl::OUString                                                                  m_sURL;
        ::ucbhelper::Content*                                                                  m_pContent;
        State                                                                            m_eState;
        ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XCommandEnvironment >  m_xCmdEnv;
        ::com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionHandler > m_xOwnInteraction;
        ::svt::OFilePickerInteractionHandler*                                            m_pOwnInteraction;

    private:
        enum Type { Folder, Document };
        /// checks if the currently bound content is a folder or document
        sal_Bool implIs( const ::rtl::OUString& _rURL, Type _eType );

        SmartContent( const SmartContent& _rSource );               // never implemented
        SmartContent& operator=( const SmartContent& _rSource );    // never implemented

    public:
        SmartContent();
        SmartContent( const ::rtl::OUString& _rInitialURL );
        ~SmartContent();

    public:

        /** create and set a specialized interaction handler at the internal used command environment.

            @param eInterceptions
                    will be directly forwarded to OFilePickerInteractionHandler::enableInterceptions()
        */
        void enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::EInterceptedInteractions eInterceptions);

        /** disable the specialized interaction handler and use the global UI interaction handler only.
        */
        void enableDefaultInteractionHandler();

        /** return the internal used interaction handler object ...
            Because this pointer will be valid only, if the uno object is hold
            alive by it's uno reference (and this reference is set on the
            command environment) we must return NULL, in case this environment does
            not exist!
        */
        ::svt::OFilePickerInteractionHandler* getOwnInteractionHandler() const;

        /** describes different types of interaction handlers
         */
        enum InteractionHandlerType
        {
            IHT_NONE,
            IHT_OWN,
            IHT_DEFAULT
        };

        /** return the type of the internal used interaction handler object ...

            @seealso InteractionHandlerType
        */
        InteractionHandlerType queryCurrentInteractionHandler() const;

        /** disable internal used interaction handler object ...
         */
        void disableInteractionHandler();

        /** returns the current state of the content

            @seealso State
        */
        inline  State       getState( ) const { return m_eState; }

        /** checks if the content is valid
            <p>Note that "not (is valid)" is not the same as "is invalid"</p>
        */
        inline  sal_Bool    isValid( ) const { return VALID == getState(); }

        /** checks if the content is valid
            <p>Note that "not (is invalid)" is not the same as "is valid"</p>
        */
        inline  sal_Bool    isInvalid( ) const { return INVALID == getState(); }

        /** checks if the content is bound
        */
        inline  sal_Bool    isBound( ) const { return NOT_BOUND != getState(); }

        /** returns the URL of the content
        */
        inline ::rtl::OUString  getURL() const { return m_pContent ? m_pContent->getURL() : m_sURL; }

        /** (re)creates the content for the given URL

            <p>Note that getState will return either UNKNOWN or INVALID after the call returns,
            but never VALID. The reason is that there are content providers which allow to construct
            content objects, even if the respective contents are not accessible. They tell about this
            only upon working with the content object (e.g. when asking for the IsFolder).</p>

            @postcond
                <member>getState</member> does not return NOT_BOUND after the call returns
        */
        void    bindTo( const ::rtl::OUString& _rURL );

        /** retrieves the title of the content
            @precond
                the content is bound and not invalid
        */
        void    getTitle( ::rtl::OUString& /* [out] */ _rTitle );

        /** checks if the content has a parent folder
            @precond
                the content is bound and not invalid
        */
        sal_Bool    hasParentFolder( );

        /** checks if sub folders below the content can be created
            @precond
                the content is bound and not invalid
        */
        sal_Bool    canCreateFolder( );

        /** binds to the given URL, checks whether or not it refers to a folder

            @postcond
                the content is not in the state UNKNOWN
        */
        inline  sal_Bool    isFolder( const ::rtl::OUString& _rURL )
        {
            return implIs( _rURL, Folder );
        }

        /** binds to the given URL, checks whether or not it refers to a document

            @postcond
                the content is not in the state UNKNOWN
        */
        inline  sal_Bool    isDocument(  const ::rtl::OUString& _rURL )
        {
            return implIs( _rURL, Document );
        }

        /** checks if the content is existent (it is if and only if it is a document or a folder)
        */
        inline  sal_Bool    is( const ::rtl::OUString& _rURL )
        {
            return  implIs( _rURL, Folder ) || implIs( _rURL, Document );
        }

        inline  sal_Bool    isFolder( )     { return isFolder( getURL() ); }
        inline  sal_Bool    isDocument( )   { return isDocument( getURL() ); }
        inline  sal_Bool    is( )           { return is( getURL() ); }
    };

//........................................................................
} // namespace svt
//........................................................................

#endif // SVTOOLS_SOURCE_FILEPICKER_FPSMARTCONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

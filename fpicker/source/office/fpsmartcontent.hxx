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

#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_FPSMARTCONTENT_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_FPSMARTCONTENT_HXX

#include "fpinteraction.hxx"

#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <ucbhelper/content.hxx>


namespace svt
{



    //= SmartContent

    /** a "smart content" which basically wraps an UCB content, but caches some information
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
        OUString                                                                  m_sURL;
        ::ucbhelper::Content*                                                            m_pContent;
        State                                                                            m_eState;
        ::com::sun::star::uno::Reference < ::com::sun::star::ucb::XCommandEnvironment >  m_xCmdEnv;
        ::com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionHandler > m_xOwnInteraction;
        ::svt::OFilePickerInteractionHandler*                                            m_pOwnInteraction;

    private:
        enum Type { Folder, Document };
        /// checks if the currently bound content is a folder or document
        bool implIs( const OUString& _rURL, Type _eType );

        SmartContent( const SmartContent& _rSource ) = delete;
        SmartContent& operator=( const SmartContent& _rSource ) = delete;

    public:
        SmartContent();
        SmartContent( const OUString& _rInitialURL );
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
            alive by its uno reference (and this reference is set on the
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
        inline  bool    isValid( ) const { return VALID == getState(); }

        /** checks if the content is valid
            <p>Note that "not (is invalid)" is not the same as "is valid"</p>
        */
        inline  bool    isInvalid( ) const { return INVALID == getState(); }

        /** checks if the content is bound
        */
        inline  bool    isBound( ) const { return NOT_BOUND != getState(); }

        /** returns the URL of the content
        */
        inline OUString  getURL() const { return m_pContent ? m_pContent->getURL() : m_sURL; }

        /** (re)creates the content for the given URL

            <p>Note that getState will return either UNKNOWN or INVALID after the call returns,
            but never VALID. The reason is that there are content providers which allow to construct
            content objects, even if the respective contents are not accessible. They tell about this
            only upon working with the content object (e.g. when asking for the IsFolder).</p>

            @postcond
                <member>getState</member> does not return NOT_BOUND after the call returns
        */
        void    bindTo( const OUString& _rURL );

        /** retrieves the title of the content
            @precond
                the content is bound and not invalid
        */
        void    getTitle( OUString& /* [out] */ _rTitle );

        /** checks if the content has a parent folder
            @precond
                the content is bound and not invalid
        */
        bool    hasParentFolder( );

        /** checks if sub folders below the content can be created
            @precond
                the content is bound and not invalid
        */
        bool    canCreateFolder( );

        /** creates a new folder with the given title and return the corresponding URL.

            @return
                the URL of the created folder or an empty string
          */
        OUString    createFolder( const OUString& _rTitle );

        /** binds to the given URL, checks whether or not it refers to a folder

            @postcond
                the content is not in the state UNKNOWN
        */
        inline  bool    isFolder( const OUString& _rURL )
        {
            return implIs( _rURL, Folder );
        }

        /** checks if the content is existent (it is if and only if it is a document or a folder)
        */
        inline  bool    is( const OUString& _rURL )
        {
            return  implIs( _rURL, Folder ) || implIs( _rURL, Document );
        }

        inline  bool    isFolder( )     { return isFolder( getURL() ); }
    };


} // namespace svt


#endif // INCLUDED_FPICKER_SOURCE_OFFICE_FPSMARTCONTENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

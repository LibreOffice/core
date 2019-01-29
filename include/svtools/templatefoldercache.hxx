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

#ifndef INCLUDED_SVTOOLS_TEMPLATEFOLDERCACHE_HXX
#define INCLUDED_SVTOOLS_TEMPLATEFOLDERCACHE_HXX

#include <svtools/svtdllapi.h>
#include <o3tl/deleter.hxx>
#include <memory>

namespace svt
{


    //= TemplateFolderCache

    class TemplateFolderCacheImpl;
    /** allows to cache the state of the template directories of OOo
        <p>Usually, this class is used together with an instance of a the
        <service scope="com.sun.star.frame">DocumentTemplates</service> service. It allows to scan the template folders
        of the Office, and updates the configuration so that it reflects the most recent state of the folders.<br/>
        As this is an expensive, the TemplateFolderCache has been introduced. It caches the state of the template
        folders, and allows to determine if the DocumentTemplates service needs to be invoked to do the (much more expensive)
        update.</p>
    @example C++
    <listing>
        TemplateFolderCache aTemplateFolders;
        if ( aTemplateFolders.needsUpdate() )
        {
            // store the current state
            aCache.storeState();

            // create the DocumentTemplates instance
            Reference< XDocumentTemplates > xTemplates = ...;

            // update the templates configuration
            xTemplates->update();
        }

        // do anything which relies on a up-to-date template configuration
    </listing>
    */
    class SVT_DLLPUBLIC TemplateFolderCache
    {
    private:
        std::unique_ptr<TemplateFolderCacheImpl, o3tl::default_delete<TemplateFolderCacheImpl>> m_pImpl;

    public:
        /** ctor.
        @param _bAutoStoreState
            Set this to <TRUE/> if you want the instance to automatically store the state of the template folders upon
            destruction.<br/>
            If set to <FALSE/>, you would explicitly need to call <method>storeState</method> to do this.<br/>
            If the current state is not known (e.g. because you did not call needsUpdate, which retrieves it),
            it is not retrieved in the dtor, regardless of the <arg>_bAutoStoreState</arg> flag.
        */
        TemplateFolderCache( bool _bAutoStoreState = false );
        ~TemplateFolderCache( );

        /** determines whether or not the template configuration needs to be updated
        @return
            <TRUE/> if the template configuration needs to be updated
        */
        bool    needsUpdate();

        /** stores the current state of the template folders in the cache  */
        void        storeState();
    };


}   // namespace svt


#endif // INCLUDED_SVTOOLS_TEMPLATEFOLDERCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

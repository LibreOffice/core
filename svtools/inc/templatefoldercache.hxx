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

#ifndef SFX2_TEMPLATEFOLDERCACHE_HXX
#define SFX2_TEMPLATEFOLDERCACHE_HXX

#include "svtools/svtdllapi.h"
#include <sal/types.h>

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= TemplateFolderCache
    //=====================================================================
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
        TemplateFolderCacheImpl*        m_pImpl;

    public:
        /** ctor.
        @param _bAutoStoreState
            Set this to <TRUE/> if you want the instance to automatically store the state of the template folders upon
            destruction.<br/>
            If set to <FALSE/>, you would epplicitly need to call <method>storeState</method> to do this.<br/>
            If the current state is not known (e.g. because you did not call needsUpdate, which retrieves it),
            it is not retrieved in the dtor, regardless of the <arg>_bAutoStoreState</arg> flag.
        */
        TemplateFolderCache( sal_Bool _bAutoStoreState = sal_False );
        ~TemplateFolderCache( );

        /** determines whether or not the template configuration needs to be updated
        @param _bForceCheck
            set this to <TRUE/> if you want the object to rescan the template folders in every case. The default (<FALSE/>)
            means that once the information has been retrieved in a first call, every second call returns the same result
            as the first one, even if in the meantime the template folders changed.
        @return
            <TRUE/> if the template configuration needs to be updated
        */
        sal_Bool    needsUpdate( sal_Bool _bForceCheck = sal_False );

        /** stores the current state of the template folders in the cache
        @param _bForceRetrieval
            if set to <TRUE/>, the current state of the template folders is retrieved again, even if it is already known.
            Usually, you set this to <FALSE/>: After calling <method>needsUpdate</method>, the state is know and does not
            need to be read again.
        */
        void        storeState( sal_Bool _bForceRetrieval = sal_False );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SFX2_TEMPLATEFOLDERCACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

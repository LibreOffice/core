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

#ifndef INCLUDED_BASIC_BASICMANAGERREPOSITORY_HXX
#define INCLUDED_BASIC_BASICMANAGERREPOSITORY_HXX

#include <basic/basicdllapi.h>

namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::uno { template <typename > class Reference; }

class BasicManager;


namespace basic
{


    //= BasicManagerRepository

    /** specifies a callback for instances which are interested in BasicManagers
        created by the BasicManagerRepository.
    */
    class BASIC_DLLPUBLIC SAL_NO_VTABLE BasicManagerCreationListener
    {
    public:
        /** is called when a BasicManager has been created

            @param  _rxForDocument
                denotes the document for which the BasicManager has been created. If this is <NULL/>,
                then the BasicManager is the application-wide BasicManager.

            @param  _pBasicManager
                denotes the BasicManager which has been created. The listener might for instance
                decide to add global variables to it, or otherwise initialize it.
        */
        virtual void onBasicManagerCreated(
            const css::uno::Reference< css::frame::XModel >& _rxForDocument,
            BasicManager& _rBasicManager
        ) = 0;

    protected:
        ~BasicManagerCreationListener() {}
    };


    //= BasicManagerRepository

    class BASIC_DLLPUBLIC BasicManagerRepository
    {
    public:
        /** returns the BasicManager belonging to the given document

            If the BasicManager does not yet exist, it is created. In this case, if the application's
            BasicManager does not yet exist, it is also created. This is necessary since
            the application's BasicManager acts as parent for all document's BasicManagers.

            If you're interested in this case - the implicit creation of the application's BasicManager -,
            then you need to register as BasicManagerCreationListener.

            @param _rxDocumentModel
                denotes the document model whose BasicManager is to be retrieved. Must not be <NULL/>.
                The document should support the XDocumentPropertiesSupplier
                interface, for retrieving
                its title, which is needed in some error conditions.
                Also it <em>must</em> support the XStorageBasedDocument interface, since we
                must be able to retrieve the document's storage. If this interface is <em>not</em>
                supported, creating a new BasicManager will certainly fail.

            @return
                the BasicManager for this model.

            @attention
                The returned BasicManager instances is owned by the repository. In particular,
                you are not allowed to delete it. Instead, the given model is observed: As soon
                as it's closed, the associated BasicManager is deleted.
        */
        static BasicManager* getDocumentBasicManager(
            const css::uno::Reference< css::frame::XModel >& _rxDocumentModel
        );

        /** returns the application-wide BasicManager

        @attention
            If the BasicManager is newly created, then it is still owned by the repository.
            In particular, you are not allowed to delete it. Instead, call resetApplicationBasicManager
            to release the BasicManager.
        */
        static BasicManager* getApplicationBasicManager();

        /** resets the application-wide BasicManager to <NULL/>
        */
        static void resetApplicationBasicManager();

        /** registers a BasicManagerCreationListener instance which is notified whenever
            the repository creates a BasicManager instance.

            Note that this listener is <em>not</em> called when somebody else
            creates BasicManager instances.

            If the same listener is registered multiple times, it is also notified
            multiple times, and needs to be revoked once for each registration.
        */
        static  void    registerCreationListener(
                BasicManagerCreationListener& _rListener
            );

        /** revokes a BasicManagerCreationListener instance which has previously
            been registered to be notified about created BasicManager instances.
        */
        static  void    revokeCreationListener(
                BasicManagerCreationListener& _rListener
            );
    };


} // namespace basic


#endif // INCLUDED_BASIC_BASICMANAGERREPOSITORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

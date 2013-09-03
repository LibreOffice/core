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

#ifndef SVX_DATABASELOCATIONINPUT_HXX
#define SVX_DATABASELOCATIONINPUT_HXX

#include "svx/svxdllapi.h"
#include "com/sun/star/uno/XComponentContext.hpp"

class PushButton;
namespace svt { class OFileURLControl; }

#include <memory>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= DatabaseLocationInputController
    //====================================================================
    class DatabaseLocationInputController_Impl;
    /** helper class to control controls needed to input a database location

        If you allow, in your dialog, to save a database document, then you usually
        have a OFileURLControl for inputting the actual location, and a push button
        to browse for a location.

        This helper class controls such two UI elements.
    */
    class SVX_DLLPUBLIC DatabaseLocationInputController
    {
    public:
        DatabaseLocationInputController(
            const css::uno::Reference<css::uno::XComponentContext>& _rContext,
            ::svt::OFileURLControl&                 _rLocationInput,
            PushButton&                             _rBrowseButton
        );
        ~DatabaseLocationInputController();

        /** sets the given URL at the input control, after translating it into a system path
        */
        void    setURL( const OUString& _rURL );

        /** returns the current database location, in form of an URL (not a system path)
        */
        OUString  getURL() const;

        /** prepares committing the database location entered in the input field

            Effectively, this method checks whether the file in the location already
            exists, and if so, it asks the user whether to overwrite it.

            If the method is called multiple times, this check only happens when the location
            changed since the last call.
        */
        bool    prepareCommit();

    private:
        ::std::auto_ptr< DatabaseLocationInputController_Impl >
                m_pImpl;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_DATABASELOCATIONINPUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

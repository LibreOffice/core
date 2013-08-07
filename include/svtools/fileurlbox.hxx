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

#ifndef SVTOOLS_FILEURLBOX_HXX
#define SVTOOLS_FILEURLBOX_HXX

#include "svtools/svtdllapi.h"
#include <svtools/inettbc.hxx>

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= FileURLBox
    //=====================================================================
    class SVT_DLLPUBLIC FileURLBox : public SvtURLBox
    {
    private:
        String      m_sPreservedText;

    public:
        FileURLBox( Window* _pParent, WinBits _nStyle );

    protected:
        virtual long        PreNotify( NotifyEvent& rNEvt );
        virtual long        Notify( NotifyEvent& rNEvt );

    public:
        /** transforms the given URL content into a system-dependent notation, if possible, and
            sets it as current display text

            <p>If the user enters an URL such as "file:///c:/some%20directory", then this will be converted
            to "c:\some directory" for better readability.</p>

            @param _rURL
                denotes the URL to set. Note that no check is made whether it is a valid URL - this
                is the responsibility of the caller.

            @see SvtURLBox::GetURL
        */
        void    DisplayURL( const String& _rURL );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_FILEURLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

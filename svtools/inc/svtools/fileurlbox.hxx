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
    protected:
        String      m_sPreservedText;

    public:
        FileURLBox( Window* _pParent );
        FileURLBox( Window* _pParent, WinBits _nStyle );
        FileURLBox( Window* _pParent, const ResId& _rId );

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

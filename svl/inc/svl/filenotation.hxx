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

#ifndef SVTOOLS_FILENOTATION_HXX
#define SVTOOLS_FILENOTATION_HXX

#include "svl/svldllapi.h"
#include <rtl/ustring.hxx>

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= OFileNotation
    //=====================================================================
    class SVL_DLLPUBLIC OFileNotation
    {
    protected:
        OUString     m_sSystem;
        OUString     m_sFileURL;

    public:
        enum NOTATION
        {
            N_SYSTEM,
            N_URL
        };

        OFileNotation( const OUString& _rUrlOrPath );
        OFileNotation( const OUString& _rUrlOrPath, NOTATION _eInputNotation );

        OUString get(NOTATION _eOutputNotation);

    private:
        SVL_DLLPRIVATE void construct( const OUString& _rUrlOrPath );
        SVL_DLLPRIVATE bool    implInitWithSystemNotation( const OUString& _rSystemPath );
        SVL_DLLPRIVATE bool    implInitWithURLNotation( const OUString& _rURL );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_FILENOTATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

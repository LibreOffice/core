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

#ifndef XMLOFF_FORMS_LOGGING_HXX
#define XMLOFF_FORMS_LOGGING_HXX

#include <sal/types.h>
#include <stack>

namespace rtl { class Logfile; }

namespace xmloff
{

#ifdef TIMELOG

    //= OStackedLogging
    class OStackedLogging
    {
    private:
        ::std::stack< ::rtl::Logfile* >     m_aLogger;

    protected:
        OStackedLogging() { }

    protected:
        void    enterContext( const sal_Char* _pContextName );
        void    leaveTopContext( );
    };

#define ENTER_LOG_CONTEXT( name ) enterContext( name )
#define LEAVE_LOG_CONTEXT( ) leaveTopContext( )

#else
    struct OStackedLogging { };

#define ENTER_LOG_CONTEXT( name )
#define LEAVE_LOG_CONTEXT( )

#endif

}   // namespace xmloff

#endif // XMLOFF_FORMS_LOGGING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

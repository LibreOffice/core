/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef XMLOFF_FORMS_LOGGING_HXX
#define XMLOFF_FORMS_LOGGING_HXX

#include <sal/types.h>
#include <stack>

namespace rtl { class Logfile; }

//.........................................................................
namespace xmloff
{
//.........................................................................

#ifdef TIMELOG

    //=====================================================================
    //= OStackedLogging
    //=====================================================================
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

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // XMLOFF_FORMS_LOGGING_HXX


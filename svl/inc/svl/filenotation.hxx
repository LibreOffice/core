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
        ::rtl::OUString     m_sSystem;
        ::rtl::OUString     m_sFileURL;

    public:
        enum NOTATION
        {
            N_SYSTEM,
            N_URL
        };

        OFileNotation( const ::rtl::OUString& _rUrlOrPath );
        OFileNotation( const ::rtl::OUString& _rUrlOrPath, NOTATION _eInputNotation );

        ::rtl::OUString get(NOTATION _eOutputNotation);

    private:
        SVL_DLLPRIVATE void construct( const ::rtl::OUString& _rUrlOrPath );
        SVL_DLLPRIVATE bool    implInitWithSystemNotation( const ::rtl::OUString& _rSystemPath );
        SVL_DLLPRIVATE bool    implInitWithURLNotation( const ::rtl::OUString& _rURL );
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_FILENOTATION_HXX


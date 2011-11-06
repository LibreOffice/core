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



#ifndef SC_SOLVERUTIL_HXX
#define SC_SOLVERUTIL_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
    namespace sheet { class XSolver; }
} } }


class ScSolverUtil
{
public:
    static void GetImplementations( com::sun::star::uno::Sequence<rtl::OUString>& rImplNames,
                                    com::sun::star::uno::Sequence<rtl::OUString>& rDescriptions );
    static com::sun::star::uno::Reference<com::sun::star::sheet::XSolver> GetSolver( const rtl::OUString& rImplName );
    static com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> GetDefaults( const rtl::OUString& rImplName );
};

#endif


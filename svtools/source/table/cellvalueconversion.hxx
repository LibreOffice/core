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



#ifndef SVTOOLS_CELLVALUECONVERSION_HXX
#define SVTOOLS_CELLVALUECONVERSION_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/Any.hxx>
/** === end UNO includes === **/

#include <boost/scoped_ptr.hpp>

namespace comphelper
{
    class ComponentContext;
}

//......................................................................................................................
namespace svt
{
//......................................................................................................................

    //==================================================================================================================
    //= CellValueConversion
    //==================================================================================================================
    struct CellValueConversion_Data;
    class CellValueConversion
    {
    public:
        CellValueConversion( ::comphelper::ComponentContext const & i_context );
        ~CellValueConversion();

        ::rtl::OUString convertToString( const ::com::sun::star::uno::Any& i_cellValue );

    private:
        ::boost::scoped_ptr< CellValueConversion_Data > m_pData;
    };

//......................................................................................................................
} // namespace svt
//......................................................................................................................

#endif // SVTOOLS_CELLVALUECONVERSION_HXX

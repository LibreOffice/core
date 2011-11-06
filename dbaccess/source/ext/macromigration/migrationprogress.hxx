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



#ifndef DBACCESS_MIGRATIONPROGRESS_HXX
#define DBACCESS_MIGRATIONPROGRESS_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace dbmm
{
//........................................................................

    //====================================================================
    //= IMigrationProgress
    //====================================================================
    class SAL_NO_VTABLE IMigrationProgress
    {
    public:
        virtual void    startObject( const ::rtl::OUString& _rObjectName, const ::rtl::OUString& _rCurrentAction, const sal_uInt32 _nRange ) = 0;
        virtual void    setObjectProgressText( const ::rtl::OUString& _rText ) = 0;
        virtual void    setObjectProgressValue( const sal_uInt32 _nValue ) = 0;
        virtual void    endObject() = 0;
        virtual void    start( const sal_uInt32 _nOverallRange ) = 0;
        virtual void    setOverallProgressText( const ::rtl::OUString& _rText ) = 0;
        virtual void    setOverallProgressValue( const sal_uInt32 _nValue ) = 0;
    };

//........................................................................
} // namespace dbmm
//........................................................................

#endif // DBACCESS_MIGRATIONPROGRESS_HXX

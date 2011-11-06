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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include "hsqldb/HTools.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace connectivity { namespace hsqldb
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    //====================================================================
    //= HTools
    //====================================================================
    //--------------------------------------------------------------------
    void HTools::appendTableFilterCrit( ::rtl::OUStringBuffer& _inout_rBuffer, const ::rtl::OUString& _rCatalog,
        const ::rtl::OUString _rSchema, const ::rtl::OUString _rName, bool _bShortForm )
    {
        _inout_rBuffer.appendAscii( " WHERE " );
        if ( _rCatalog.getLength() )
        {
            _inout_rBuffer.appendAscii( _bShortForm ? "TABLE_CAT" : "TABLE_CATALOG" );
            _inout_rBuffer.appendAscii( " = '" );
            _inout_rBuffer.append     ( _rCatalog );
            _inout_rBuffer.appendAscii( "' AND " );
        }
        if ( _rSchema.getLength() )
        {
            _inout_rBuffer.appendAscii( _bShortForm ? "TABLE_SCHEM" : "TABLE_SCHEMA" );
            _inout_rBuffer.appendAscii( " = '" );
            _inout_rBuffer.append     ( _rSchema );
            _inout_rBuffer.appendAscii( "' AND " );
        }
        _inout_rBuffer.appendAscii( "TABLE_NAME = '" );
        _inout_rBuffer.append     ( _rName );
        _inout_rBuffer.appendAscii( "'" );
    }

//........................................................................
} } // namespace connectivity::hsqldb
//........................................................................

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


#include <vbahelper/vbadialogbase.hxx>
#include <vbahelper/vbahelper.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

// fails silently
void
VbaDialogBase::Show() throw(uno::RuntimeException)
{
    rtl::OUString aURL;
    if ( m_xModel.is() )
    {
        aURL = mapIndexToName( mnIndex );
        if( aURL.getLength() == 0  )
            throw uno::RuntimeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " Unable to open the specified dialog " ) ),
                uno::Reference< XInterface > () );
        dispatchRequests( m_xModel, aURL );
    }
}


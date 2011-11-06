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


#ifndef PROPCONTROLOBSERVER_HXX
#define PROPCONTROLOBSERVER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/inspection/XPropertyControl.hpp>
/** === end UNO includes === **/

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= IPropertyControlObserver
    //====================================================================
    /** non-UNO version of the XPropertyControlObserver
    */
    class IPropertyControlObserver
    {
    public:
        virtual void    focusGained( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _Control ) = 0;
        virtual void    valueChanged( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _Control ) = 0;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // PROPCONTROLOBSERVER_HXX


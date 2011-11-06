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



#ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_
#define __FRAMEWORK_CLASSES_TASKCREATOR_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          a helper to create new tasks or sub frames for "_blank" or FrameSearchFlag::CREATE
    @descr          There are different places to create new (task)frames. Its not easy to service this code!
                    Thats the reason for this helper. He capsulate asynchronous/synchronous creation by calling
                    a simple interface.

    @devstatus      ready to use
    @threadsafe     yes
    @modified       20.08.2003 08:37, as96863
*//*-*************************************************************************************************************/
class TaskCreator : private ThreadHelpBase
{
    //_______________________
    // member
    private:

        css::uno::Reference< css::lang::XMultiServiceFactory >  m_xSMGR;

    //_______________________
    // interface
    public:

                 TaskCreator( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );
        virtual ~TaskCreator(                                                                     );

        css::uno::Reference< css::frame::XFrame > createTask( const ::rtl::OUString& sName    ,
                                                                    sal_Bool         bVisible );

    //_______________________
    // helper
    private:

}; // class TaskCreator

} // namespace framework

#endif // #ifndef __FRAMEWORK_CLASSES_TASKCREATOR_HXX_

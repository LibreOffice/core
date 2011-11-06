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



#ifndef COMMANDIMAGEPROVIDER_HXX
#define COMMANDIMAGEPROVIDER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <vcl/image.hxx>

#include <boost/shared_ptr.hpp>

//........................................................................
namespace frm
{
//........................................................................

    //=====================================================================
    //= ICommandImageProvider
    //=====================================================================
    typedef ::rtl::OUString                                 CommandURL;
    typedef ::com::sun::star::uno::Sequence< CommandURL >   CommandURLs;
    typedef ::std::vector< Image >                          CommandImages;

    class SAL_NO_VTABLE ICommandImageProvider
    {
    public:
        virtual CommandImages   getCommandImages(
                                    const CommandURLs& _rCommandURLs,
                                    const bool _bLarge,
                                    const bool _bHiContrast
                                ) const = 0;

        virtual ~ICommandImageProvider() { }
    };

    typedef ::boost::shared_ptr< const ICommandImageProvider >  PCommandImageProvider;

    //=====================================================================
    //= factory
    //=====================================================================
    PCommandImageProvider
        createDocumentCommandImageProvider(
            const ::comphelper::ComponentContext& _rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& _rxDocument
        );

//........................................................................
} // namespace frm
//........................................................................

#endif // COMMANDIMAGEPROVIDER_HXX

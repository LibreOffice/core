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



#ifndef _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_

//_______________________________________________
// own includes

#include <accelerators/acceleratorcache.hxx>
#include <accelerators/keymapping.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>

//_______________________________________________
// interface includes

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

//_______________________________________________
// other includes
#include <salhelper/singletonref.hxx>
#include <rtl/ustring.hxx>

namespace framework{

class AcceleratorConfigurationWriter : private ThreadHelpBase
{
    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short  needed to write the xml configuration. */
        css::uno::Reference< css::xml::sax::XDocumentHandler > m_xConfig;

        //---------------------------------------
        /** @short  reference to the outside container, where this
                    writer must work on. */
        const AcceleratorCache& m_rContainer;

        //---------------------------------------
        /** @short  is used to map key codes to its
                    string representation.

            @descr  To perform this operatio is
                    created only one times and holded
                    alive forever ...*/
        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        /** @short  connect this new writer instance
                    to an outside container, which should be
                    flushed to the underlying XML configuration.

            @param  rContainer
                    a reference to the outside container.

            @param  xConfig
                    used to write the configuration there.
          */
        AcceleratorConfigurationWriter(const AcceleratorCache&                                       rContainer,
                                       const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig   );

        //---------------------------------------
        /** @short  does nothing real ... */
        virtual ~AcceleratorConfigurationWriter();

        //---------------------------------------
        /** @short  TODO */
        virtual void flush();

    //-------------------------------------------
    // helper

    private:

        //---------------------------------------
        /** @short  TODO */
        void impl_ts_writeKeyCommandPair(const css::awt::KeyEvent&                                     aKey    ,
                                         const ::rtl::OUString&                                        sCommand,
                                         const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig );
};

} // namespace framework

#endif // _FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_

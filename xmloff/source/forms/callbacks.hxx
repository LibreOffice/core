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



#ifndef _XMLOFF_FORMS_CALLBACKS_HXX_
#define _XMLOFF_FORMS_CALLBACKS_HXX_

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <vos/ref.hxx>

class SvXMLImport;
class SvXMLExport;
class SvXMLExportPropertyMapper;
class SvXMLStyleContext;

//.........................................................................
namespace xmloff
{
//.........................................................................

    class OAttribute2Property;

    //=====================================================================
    //= IFormsExportContext
    //=====================================================================
    class IFormsExportContext
    {
    public:
        virtual SvXMLExport&                                getGlobalContext() = 0;
        virtual ::vos::ORef< SvXMLExportPropertyMapper >    getStylePropertyMapper() = 0;

        /** steps through a collection and exports all children of this collection
        */
        virtual void    exportCollectionElements(
            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxCollection) = 0;

        virtual ::rtl::OUString                             getObjectStyleName(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject ) = 0;
    };

    //=====================================================================
    //= IEventAttacherManager
    //=====================================================================
    class IEventAttacherManager
    {
    public:
        virtual void registerEvents(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxElement,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            ) = 0;
    };

    //=====================================================================
    //= IEventAttacher
    //=====================================================================
    class IEventAttacher
    {
    public:
        virtual void registerEvents(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::script::ScriptEventDescriptor >& _rEvents
            ) = 0;
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_CALLBACKS_HXX_


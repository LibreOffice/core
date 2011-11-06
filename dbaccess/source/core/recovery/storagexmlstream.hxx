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



#ifndef STORAGEXMLSTREAM_HXX
#define STORAGEXMLSTREAM_HXX

#include "storagestream.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
/** === end UNO includes === **/

#include <memory>

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= StorageXMLOutputStream
    //====================================================================
    struct StorageXMLOutputStream_Data;
    class DBACCESS_DLLPRIVATE StorageXMLOutputStream : public StorageOutputStream
    {
    public:
        StorageXMLOutputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        ~StorageXMLOutputStream();

        // StorageOutputStream overridables
        virtual void close();

        void    addAttribute( const ::rtl::OUString& i_rName, const ::rtl::OUString& i_rValue ) const;

        void    startElement( const ::rtl::OUString& i_rElementName ) const;
        void    endElement() const;

        void    ignorableWhitespace( const ::rtl::OUString& i_rWhitespace ) const;
        void    characters( const ::rtl::OUString& i_rCharacters ) const;

    private:
        StorageXMLOutputStream();                                           // never implemented
        StorageXMLOutputStream( const StorageXMLOutputStream& );            // never implemented
        StorageXMLOutputStream& operator=( const StorageXMLOutputStream& ); // never implemented

    private:
        ::std::auto_ptr< StorageXMLOutputStream_Data >   m_pData;
    };

    //====================================================================
    //= StorageXMLInputStream
    //====================================================================
    struct StorageXMLInputStream_Data;
    class DBACCESS_DLLPRIVATE StorageXMLInputStream : public StorageInputStream
    {
    public:
        StorageXMLInputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        ~StorageXMLInputStream();

        void    import(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >& i_rHandler
                );

    private:
        StorageXMLInputStream();                                            // never implemented
        StorageXMLInputStream( const StorageXMLInputStream& );              // never implemented
        StorageXMLInputStream& operator=( const StorageXMLInputStream& );   // never implemented

    private:
        ::std::auto_ptr< StorageXMLInputStream_Data >   m_pData;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // STORAGEXMLSTREAM_HXX

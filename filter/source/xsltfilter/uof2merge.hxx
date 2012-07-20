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
 **************************************************************/

 // MARKER(update_precomp.py): autogen include statement, do not remove
//This file is about the conversion of the UOF v2.0 and ODF document format from CS2C 20120610.
#ifndef FILTER_SOURCE_XSLTFILTER_UOF2MERGE_HXX
#define FILTER_SOURCE_XSLTFILTER_UOF2MERGE_HXX

#include "uof2storage.hxx"

namespace com { namespace sun { namespace star {
    namespace xml {
        namespace sax {
            class XParser;
            class XExtendedDocumentHandler;
        }
    }
}}}

namespace rtl{
    class OUStringBuffer;
}

namespace XSLT{

class UOF2Merge
{
public:
    UOF2Merge(UOF2Storage& rStorage, const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory);
    ~UOF2Merge();

    bool merge();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > getMergedInStream() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > getSaxWriter();

    void addNamespace( const ::rtl::OUString& rName, const ::rtl::OUString& rURL );
    bool isInsertedNamespace( const ::rtl::OUString& rName ) const;
    bool getBase64Codec( ::rtl::OUStringBuffer& rBuffer, const ::rtl::OUString& rObjPath);
private:
    UOF2Storage& m_rUOF2Storage;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > m_xPipeInStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream > m_xPipeOutStream;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XParser > m_xSaxParser;
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > m_xExtDocHdl;
    ::std::map< ::rtl::OUString, ::rtl::OUString > m_aNamespaceMap;

    void init();

    void startUOFRootXML( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& xUOFXMLInStream );
    void endUOFRootXML();
};

}

#endif

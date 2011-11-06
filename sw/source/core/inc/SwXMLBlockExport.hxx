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


#ifndef _SW_XMLBLOCKEXPORT_HXX
#define _SW_XMLBLOCKEXPORT_HXX

#include <xmloff/xmlexp.hxx>

class String;
class SwXMLTextBlocks;

class SwXMLBlockListExport : public SvXMLExport
{
private:
    SwXMLTextBlocks &rBlockList;
public:
    // #110680#
    SwXMLBlockListExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SwXMLTextBlocks & rBlocks,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SwXMLBlockListExport ( void ) {}
    sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass);
    void _ExportAutoStyles() {}
    void _ExportMasterStyles () {}
    void _ExportContent() {}
};

class SwXMLTextBlockExport : public SvXMLExport
{
private:
    SwXMLTextBlocks &rBlockList;
public:
    // #110680#
    SwXMLTextBlockExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SwXMLTextBlocks & rBlocks,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler);

    virtual ~SwXMLTextBlockExport ( void ) {}
    sal_uInt32 exportDoc(enum ::xmloff::token::XMLTokenEnum /*eClass*/) {return 0;}
    sal_uInt32 exportDoc(const String & rText);
    void _ExportAutoStyles() {}
    void _ExportMasterStyles () {}
    void _ExportContent() {}
};
#endif

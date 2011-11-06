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


#ifndef _SW_XMLBLOCKIMPORT_HXX
#define _SW_XMLBLOCKIMPORT_HXX

#include <xmloff/xmlimp.hxx>

class SwXMLTextBlocks;
class SwXMLBlockListImport : public SvXMLImport
{
private:
    SwXMLTextBlocks &rBlockList;
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    // #110680#
    SwXMLBlockListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SwXMLTextBlocks &rBlocks );

    SwXMLTextBlocks& getBlockList ( void )
    {
        return rBlockList;
    }
    virtual ~SwXMLBlockListImport ( void )
        throw();
};

class SwXMLTextBlockImport : public SvXMLImport
{
private:
    SwXMLTextBlocks &rBlockList;
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    sal_Bool bTextOnly;
    String &m_rText;

    // #110680#
    SwXMLTextBlockImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SwXMLTextBlocks &rBlocks, String &rNewText, sal_Bool bNewTextOnly );

    SwXMLTextBlocks& getBlockList ( void )
    {
        return rBlockList;
    }
    virtual ~SwXMLTextBlockImport ( void )
        throw();
    virtual void SAL_CALL endDocument(void)
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
};
#endif

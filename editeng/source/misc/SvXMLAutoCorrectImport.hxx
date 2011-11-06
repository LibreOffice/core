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


#ifndef _SV_XMLAUTOCORRECTIMPORT_HXX
#define _SV_XMLAUTOCORRECTIMPORT_HXX

#ifndef _SVSTOR_HXX
#include <sot/storage.hxx>
#endif
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <editeng/svxacorr.hxx>

class SvXMLAutoCorrectImport : public SvXMLImport
{
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SvxAutocorrWordList     *pAutocorr_List;
    SvxAutoCorrect          &rAutoCorrect;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStorage;
    //SvStorageRef            &rStorage;

    // #110680#
    SvXMLAutoCorrectImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SvxAutocorrWordList *pNewAutocorr_List,
        SvxAutoCorrect &rNewAutoCorrect,
        const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rNewStorage);

    ~SvXMLAutoCorrectImport ( void ) throw ();
};

class SvXMLWordListContext : public SvXMLImportContext
{
private:
    SvXMLAutoCorrectImport & rLocalRef;
public:
    SvXMLWordListContext ( SvXMLAutoCorrectImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLWordListContext ( void );
};

class SvXMLWordContext : public SvXMLImportContext
{
private:
    SvXMLAutoCorrectImport & rLocalRef;
public:
    SvXMLWordContext ( SvXMLAutoCorrectImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLWordContext ( void );
};


class SvXMLExceptionListImport : public SvXMLImport
{
protected:

    // This method is called after the namespace map has been updated, but
    // before a context for the current element has been pushed.
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                  const ::rtl::OUString& rLocalName,
                  const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
public:
    SvStringsISortDtor  &rList;

    // #110680#
    SvXMLExceptionListImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        SvStringsISortDtor & rNewList );

    ~SvXMLExceptionListImport ( void ) throw ();
};

class SvXMLExceptionListContext : public SvXMLImportContext
{
private:
    SvXMLExceptionListImport & rLocalRef;
public:
    SvXMLExceptionListContext ( SvXMLExceptionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLExceptionListContext ( void );
};

class SvXMLExceptionContext : public SvXMLImportContext
{
private:
    SvXMLExceptionListImport & rLocalRef;
public:
    SvXMLExceptionContext ( SvXMLExceptionListImport& rImport,
                           sal_uInt16 nPrefix,
                           const rtl::OUString& rLocalName,
                           const ::com::sun::star::uno::Reference<
                           ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    ~SvXMLExceptionContext ( void );
};


#endif

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



#ifndef _LINGUISTIC_CONVDICXML_HXX_
#define _LINGUISTIC_CONVDICXML_HXX_

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include "linguistic/misc.hxx"
#include "defs.hxx"


class ConvDic;

///////////////////////////////////////////////////////////////////////////

class ConvDicXMLExport : public SvXMLExport
{
    ConvDic     &rDic;
    sal_Bool    bSuccess;

protected:
    //void ExportNodes(const SmNode *pIn, int nLevel);

public:
    ConvDicXMLExport( ConvDic &rConvDic,
        const rtl::OUString &rFileName,
        com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler > &rHandler) :
        SvXMLExport ( utl::getProcessServiceFactory(), rFileName, rHandler ),
        rDic        ( rConvDic ),
        bSuccess    ( sal_False )
    {
    }
    virtual ~ConvDicXMLExport()
    {
    }

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    // SvXMLExport
    void _ExportAutoStyles()    {}
    void _ExportMasterStyles()  {}
    void _ExportContent();
    sal_uInt32 exportDoc( enum ::xmloff::token::XMLTokenEnum eClass );

    sal_Bool    Export();
};


class ConvDicXMLImport : public SvXMLImport
{
    ConvDic        *pDic;       // conversion dictionary to be used
                                // if != NULL: whole file will be read and
                                //   all entries will be added to the dictionary
                                // if == NULL: no entries will be added
                                //   but the language and conversion type will
                                //   still be determined!

    sal_Int16           nLanguage;          // language of the dictionary
    sal_Int16       nConversionType;    // conversion type the dictionary is used for
    sal_Bool        bSuccess;

public:

    //!!  see comment for pDic member
    ConvDicXMLImport( ConvDic *pConvDic, const rtl::OUString /*&rFileName*/ ) :
        SvXMLImport ( utl::getProcessServiceFactory(), IMPORT_ALL ),
        pDic        ( pConvDic )
    {
        nLanguage       = LANGUAGE_NONE;
        nConversionType = -1;
        bSuccess        = sal_False;
    }

    virtual ~ConvDicXMLImport() throw ()
    {
    }

    // XServiceInfo (override parent method)
    ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL startDocument(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL endDocument(void) throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException );

    virtual SvXMLImportContext * CreateContext(
        sal_uInt16 nPrefix, const rtl::OUString &rLocalName,
        const com::sun::star::uno::Reference < com::sun::star::xml::sax::XAttributeList > &rxAttrList );

    ConvDic *   GetDic()                    { return pDic; }
    sal_Int16       GetLanguage() const         { return nLanguage; }
    sal_Int16   GetConversionType() const   { return nConversionType; }
    sal_Bool    GetSuccess() const          { return bSuccess; }

    void        SetLanguage( sal_Int16 nLang )              { nLanguage = nLang; }
    void        SetConversionType( sal_Int16 nType )    { nConversionType = nType; }
};

///////////////////////////////////////////////////////////////////////////

#endif


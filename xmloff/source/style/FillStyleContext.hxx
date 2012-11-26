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



#ifndef _XMLOFF_FILLSTYLECONTEXTS_HXX_
#define _XMLOFF_FILLSTYLECONTEXTS_HXX_

#include <com/sun/star/io/XOutputStream.hpp>
#include <xmloff/xmlstyle.hxx>
#include<rtl/ustring.hxx>

//////////////////////////////////////////////////////////////////////////////
// draw:gardient context

class XMLGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    XMLGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLGradientStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:hatch context

class XMLHatchStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    XMLHatchStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLHatchStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:fill-image context

class XMLBitmapStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;
    ::com::sun::star::uno::Reference < ::com::sun::star::io::XOutputStream > mxBase64Stream;

public:
    XMLBitmapStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLBitmapStyleContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:transparency context

class XMLTransGradientStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    XMLTransGradientStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLTransGradientStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLMarkerStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    XMLMarkerStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLMarkerStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

//////////////////////////////////////////////////////////////////////////////
// draw:marker context

class XMLDashStyleContext: public SvXMLStyleContext
{
private:
    ::com::sun::star::uno::Any  maAny;
    rtl::OUString               maStrName;

public:
    XMLDashStyleContext( SvXMLImport& rImport,  sal_uInt16 nPrfx,  const rtl::OUString& rLName,
                           const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual ~XMLDashStyleContext();

    virtual void EndElement();

    virtual sal_Bool IsTransient() const;
};

#endif  // _XMLOFF_FILLSTYLECONTEXTS_HXX_

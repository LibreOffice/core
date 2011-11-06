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




#ifndef OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER
#define OOX_DRAWINGML_DIAGRAMFRAGMENTHANDLER

#include "oox/core/fragmenthandler.hxx"
#include "oox/drawingml/diagram/diagram.hxx"

namespace oox { namespace drawingml {


class DiagramDataFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramDataFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramDataPtr pDataPtr ) throw();
    virtual ~DiagramDataFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramDataPtr  mpDataPtr;
};



class DiagramLayoutFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramLayoutFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramLayoutPtr pDataPtr ) throw();
    virtual ~DiagramLayoutFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramLayoutPtr    mpDataPtr;
};

class DiagramQStylesFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramQStylesFragmentHandler( oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramQStylesPtr pDataPtr ) throw();
    virtual ~DiagramQStylesFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramQStylesPtr   mpDataPtr;
};


class DiagramColorsFragmentHandler : public ::oox::core::FragmentHandler
{
public:
    DiagramColorsFragmentHandler( ::oox::core::XmlFilterBase& rFilter, const ::rtl::OUString& rFragmentPath, const DiagramColorsPtr pDataPtr ) throw();
    virtual ~DiagramColorsFragmentHandler() throw();

    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:

    DiagramColorsPtr    mpDataPtr;
};

} }


#endif

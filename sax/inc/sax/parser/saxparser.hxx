/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SAX_SAXPARSER_HXX_
#define _SAX_SAXPARSER_HXX_

#include "sax/dllapi.h"
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <rtl/ref.hxx>

#include <boost/shared_ptr.hpp>

#include <map>
#include <memory>
#include "sax/tools/saxobject.hxx"
#include "sax/tools/attributemap.hxx"

#include <boost/scoped_ptr.hpp>

namespace sax
{

// --------------------------------------------------------------------

class SaxParser;
class SaxContext;

typedef rtl::Reference< SaxParser > SaxParserRef;
typedef rtl::Reference< SaxContext > SaxContextRef;

/** base class for each implementation that handles all sax calls for an element */
class SAX_DLLPUBLIC SaxContext : public SaxObject
{
public:
    SaxContext( const SaxParserRef& xParser );
    virtual ~SaxContext();

    /** receives notification of the beginning of an element .
     */
    virtual void StartElement( sal_uInt32 aElementToken, const AttributeMap& rAttributes );

    /** receives notification of character data.
     */
    virtual void Characters( const sal_Char *pCharacters, sal_uInt32 nLength );

    /** receives notification of the end of an element.
     */
    virtual void EndElement( sal_uInt32 aElementToken );

    /** is called by the SaxParser for each child element inside this instances element */
    virtual SaxContextRef CreateContext( sal_uInt32 aElementToken, const AttributeMap& rAttributes );

    const SaxParserRef& getParser() const { return mxParser; }
private:
    SaxParserRef    mxParser;
};

// --------------------------------------------------------------------

class SaxParserImpl;

/** base class for loading a single xml stream. Derived classes must call
    parseStream to start parsing and are notified through virtual methods
    for sax events. */
class SAX_DLLPUBLIC SaxParser : public SaxObject
{
public:
    SaxParser();
    virtual ~SaxParser();

    /** returns the unicode representation of a token from the xml stream
        that was unknown to the SaxTokenMap from the derived class. */
    rtl::OUString GetCustomToken( sal_uInt32 nToken );

    /** returns the unicode representation of a namespace from the xml stream
        that was unknown to the SaxTokenMap from the derived class. */
    rtl::OUString GetCustomNamespace( sal_uInt32 nToken );

    /** returns the system id of the currently parsed stream */
    const rtl::OUString& GetSystemId() const;

    /** starts parsing of the source xml stream provided in the given sax InputSource */
    virtual void parseStream( const ::com::sun::star::xml::sax::InputSource& rInputSource ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    /** is called once when parsing of the xml stream starts */
    virtual void StartDocument();

    /** is called once for each element in the xml stream.
        Default implementation calls StartElement() on the topmost contex. */
    virtual void StartElement( sal_uInt32 aElementToken, const AttributeMap& rAttributes );

    /** is called for characters betwen elements in the xml stream.
        Default implementation calls Characters() on the topmost contex.
        @param pCharacters The characters in utf-8 encoding
        @param nLength the size in bytes of the utf-8 string
    */
    virtual void Characters( const sal_Char *pCharacters, sal_uInt32 nLength );

    /** is called once at the end of each element in the xml stream.
        Default implementation calls EndElement() on the topmost contex. */
    virtual void EndElement( sal_uInt32 aElementToken );

    /** is called once after parsing the xml stream is finished */
    virtual void EndDocument();

    /** is called once for each element in the xml stream and before StartElement() is called.
        Default implementation calls CreateContext at the topmost context.
        Returned contexts are pushed on a stack and removed after the corresponding EndElement call. */
    virtual SaxContextRef CreateContext( sal_uInt32 aElementToken, const AttributeMap& rAttributes );

    /** must be implemented from derived classes. The returned SaxTokenMap is used to convert
        element names and attribute names and values to tokens. */
    virtual const SaxTokenMapRef& getTokenMap() = 0;

private:
    void AddNamespace( sal_uInt32 nNamespaceId, sal_uInt32 nNamespaceURIToken );

    boost::scoped_ptr< SaxParserImpl > mpImpl;
};

}

#endif // _SAX_SAXPARSER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

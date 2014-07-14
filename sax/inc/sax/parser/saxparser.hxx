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



#ifndef _SAX_SAXPARSER_HXX_
#define _SAX_SAXPARSER_HXX_

#include "sax/dllapi.h"
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <rtl/ref.hxx>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

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

    /** is called for characters between elements in the xml stream.
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

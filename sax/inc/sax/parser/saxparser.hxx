/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saxparser.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:50:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SAX_SAXPARSER_HXX_
#define _SAX_SAXPARSER_HXX_

#ifndef INCLUDED_SAX_DLLAPI_H
#include "sax/dllapi.h"
#endif

#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include <map>
#include <memory>

#ifndef _SAX_OBJECT_HXX_
#include "sax/tools/saxobject.hxx"
#endif

#ifndef _SAX_ATTRIBUTEMAP_HXX_
#include "sax/tools/attributemap.hxx"
#endif

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

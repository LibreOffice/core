/*************************************************************************
 *
 *  $RCSfile: xml_helper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-28 10:50:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _XMLSCRIPT_XML_HELPER_HXX_
#define _XMLSCRIPT_XML_HELPER_HXX_

#include <vector>

#ifndef _RTL_BYTESEQ_HXX_
#include <rtl/byteseq.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_XIMPORTER_HXX_
#include <com/sun/star/xml/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XEXTENDEDDOCUMENTHANDLER_HDL_
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HXX_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HXX_
#include <com/sun/star/io/XOutputStream.hpp>
#endif


namespace xmlscript
{

/*##################################################################################################

    IMPORTING

##################################################################################################*/

//==================================================================================================
struct NameSpaceUid
{
    /** URI defining XML namespace
    */
    ::rtl::OUString     sURI;
    /** Identifier given for URI (given back in createRootContext(), createChildContext() callbacks
    */
    sal_Int32           nUid;

    inline NameSpaceUid( ::rtl::OUString const & sURI_, sal_Int32 nUid_ ) SAL_THROW( () )
        : sURI( sURI_ )
        , nUid( nUid_ )
        {}
};

/** Creates a document handler to be used for SAX1 parser that can handle namespaces.
    Give a list of NameSpaceUid structs defining namespace mappings to integers (performance).
    Implementing the XImporter interface, you will get a createRootContext() for the root
    element of your XML document and subsequent createChildContext() callbacks for each
    sub element.
    Namespaces of tags are identified by their integer value.

    @param pNamespaceUids
           array of namespace mappings
    @param nNameSpaceUids
           number of element in namespace mappings array
    @param nUnknownNamespaceUid
           namespace id given for unrecognized namespace prefix
           (one that is not given via pNamespaceUids)
    @param xImporter
           initial import object being called for root context
    @param bSingleThreadedUse
           flag whether context management is synchronized.
    @return
            document handler for parser
*/
::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >
SAL_CALL createDocumentHandler(
    NameSpaceUid const * pNamespaceUids, sal_Int32 nNameSpaceUids,
    sal_Int32 nUnknownNamespaceUid,
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::XImporter > const & xImporter,
    bool bSingleThreadedUse = true )
    SAL_THROW( () );


/*##################################################################################################

    EXPORTING

##################################################################################################*/

//==================================================================================================
class XMLElement
    : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XAttributeList >
{
public:
    inline XMLElement( ::rtl::OUString const & name )
        SAL_THROW( () )
        : _name( name )
        {}

    /** Adds a sub element of element.

        @param xElem
               element reference
    */
    void SAL_CALL addSubElement( ::com::sun::star::uno::Reference<
                                 ::com::sun::star::xml::sax::XAttributeList > const & xElem )
        SAL_THROW( () );

    /** Gets sub element of given index.  The index follows order in which sub elements
        were added.

        @param nIndex
               index of sub element
    */
    ::com::sun::star::uno::Reference<
    ::com::sun::star::xml::sax::XAttributeList > SAL_CALL getSubElement( sal_Int32 nIndex )
        SAL_THROW( () );

    /** Adds an attribute to elements

        @param rAttrName qname of attribute
        @param rValue value string of element
    */
    void SAL_CALL addAttribute(
        ::rtl::OUString const & rAttrName, ::rtl::OUString const & rValue )
        SAL_THROW( () );

    /** Gets the tag name (qname) of element

        @return
                qname of element
    */
    inline ::rtl::OUString SAL_CALL getName() SAL_THROW( () )
        { return _name; }

    /** Dumps out element (and all sub element).

        @param xOut
               document handler to be written to
    */
    void SAL_CALL dump( ::com::sun::star::uno::Reference<
                        ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut );

    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNameByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTypeByName( ::rtl::OUString const & rName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByIndex( sal_Int16 nPos )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getValueByName( ::rtl::OUString const & rName )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    ::rtl::OUString _name;

    ::std::vector< ::rtl::OUString > _attrNames;
    ::std::vector< ::rtl::OUString > _attrValues;

    ::std::vector< ::com::sun::star::uno::Reference<
                   ::com::sun::star::xml::sax::XAttributeList > > _subElems;
};


/*##################################################################################################

    STREAMING

##################################################################################################*/

//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
SAL_CALL createInputStream(
    ::rtl::ByteSequence const & rInData )
    SAL_THROW( () );

//==================================================================================================
::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
SAL_CALL createOutputStream(
    ::rtl::ByteSequence * pOutData )
    SAL_THROW( () );

};

#endif

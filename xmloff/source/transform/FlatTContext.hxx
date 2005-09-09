/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FlatTContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:44:17 $
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

#ifndef _XMLOFF_FLATTCONTEXT_HXX
#define _XMLOFF_FLATTCONTEXT_HXX

#ifndef _XMLOFF_PERSATTRLISTTCONTEXT_HXX
#include "PersAttrListTContext.hxx"
#endif


class XMLPersTextContentTContext : public XMLPersAttrListTContext
{
    ::rtl::OUString m_aCharacters;

public:
    TYPEINFO();

    // A contexts constructor does anything that is required if an element
    // starts. Namespace processing has been done already.
    // Note that virtual methods cannot be used inside constructors. Use
    // StartElement instead if this is required.
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // attr list/text content persistence + attribute processing
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nActionMap );

    // attr list/text content persistence + renaming
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // attr list/text content persistence + renaming + attribute processing
    XMLPersTextContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    // A contexts destructor does anything that is required if an element
    // ends. By default, nothing is done.
    // Note that virtual methods cannot be used inside destructors. Use
    // EndElement instead if this is required.
    virtual ~XMLPersTextContentTContext();

    // This method is called for all characters that are contained in the
    // current element.
    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void ExportContent();

    const ::rtl::OUString& GetTextContent() const { return m_aCharacters; }
};

#endif  //  _XMLOFF_FLATTCONTEXT_HXX


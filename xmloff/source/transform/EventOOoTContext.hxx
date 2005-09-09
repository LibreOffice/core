/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EventOOoTContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:43:15 $
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

#ifndef _XMLOFF_EVENTOOOTCONTEXT_HXX
#define _XMLOFF_EVENTOOOTCONTEXT_HXX

#ifndef _XMLOFF_PERSATTRLISTTCONTEXT_HXX
#include "PersAttrListTContext.hxx"
#endif

class XMLTransformerOOoEventMap_Impl;

class XMLEventOOoTransformerContext : public XMLPersAttrListTContext
{
    sal_Bool m_bPersistent;

public:
    TYPEINFO();

    XMLEventOOoTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_Bool bPersistent = sal_False  );
    virtual ~XMLEventOOoTransformerContext();

    static XMLTransformerOOoEventMap_Impl *CreateEventMap();
    static void FlushEventMap( XMLTransformerOOoEventMap_Impl *p );
    static sal_uInt16 GetEventName( const ::rtl::OUString& rName,
                                    ::rtl::OUString& rNewName,
                                       XMLTransformerOOoEventMap_Impl& rMap );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    virtual sal_Bool IsPersistent() const;
};

#endif  //  _XMLOFF_EVENTOOOTCONTEXT_HXX


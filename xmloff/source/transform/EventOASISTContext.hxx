/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EventOASISTContext.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:42:40 $
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

#ifndef _XMLOFF_EVENTOASISTCONTEXT_HXX
#define _XMLOFF_EVENTOASISTCONTEXT_HXX

#ifndef _XMLOFF_RENAMEELEMTCONTEXT_HXX
#include "RenameElemTContext.hxx"
#endif

class XMLTransformerOASISEventMap_Impl;

class XMLEventOASISTransformerContext : public XMLRenameElemTransformerContext
{
public:
    TYPEINFO();

    XMLEventOASISTransformerContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );
    virtual ~XMLEventOASISTransformerContext();

    static XMLTransformerOASISEventMap_Impl *CreateFormEventMap();
    static XMLTransformerOASISEventMap_Impl *CreateEventMap();
    static void FlushEventMap( XMLTransformerOASISEventMap_Impl *p );
    static ::rtl::OUString GetEventName( sal_uInt16 nPrefix,
                             const ::rtl::OUString& rName,
                             XMLTransformerOASISEventMap_Impl& rMap,
                             XMLTransformerOASISEventMap_Impl* pMap2    );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

#endif  //  _XMLOFF_EVENTOASISTCONTEXT_HXX


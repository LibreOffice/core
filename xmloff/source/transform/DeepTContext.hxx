/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DeepTContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:40:12 $
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

#ifndef _XMLOFF_DEEPTCONTEXT_HXX
#define _XMLOFF_DEEPTCONTEXT_HXX

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#include <vector>

#ifndef _XMLOFF_PERSATTRLISTTCONTEXT_HXX
#include "PersAttrListTContext.hxx"
#endif
#ifndef _XMLOFF_TCONTEXTVECTOR_HXX
#include "TContextVector.hxx"
#endif

class XMLPersElemContentTContext : public XMLPersAttrListTContext
{
    XMLTransformerContextVector m_aChildContexts;

public:
    TYPEINFO();

    // element content persistence only
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName );

    // element content persistence + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nActionMap );

    // element content persistence + renaming
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken );

    // element content persistence + renaming + attribute processing
    XMLPersElemContentTContext( XMLTransformerBase& rTransformer,
                           const ::rtl::OUString& rQName,
                              sal_uInt16 nPrefix,
                              ::xmloff::token::XMLTokenEnum eToken,
                              sal_uInt16 nActionMap );

    virtual ~XMLPersElemContentTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    void AddContent( XMLTransformerContext *pContent );

    sal_Bool HasElementContent() const
    {
        return static_cast<sal_Bool>( !m_aChildContexts.empty() );
    }

    virtual void ExportContent();
};

#endif  //  _XMLOFF_DEEPTCONTEXT_HXX


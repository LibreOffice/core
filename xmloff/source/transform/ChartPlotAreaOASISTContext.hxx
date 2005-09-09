/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartPlotAreaOASISTContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:37:07 $
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
#ifndef XMLOFF_XMLCHARTPLOTAREAOASISTCONTEXT_HXX
#define XMLOFF_XMLCHARTPLOTAREAOASISTCONTEXT_HXX

#ifndef _XMLOFF_PROCATTRTCONTEXT_HXX
#include "ProcAttrTContext.hxx"
#endif
#ifndef _XMLOFF_PERSATTRLISTTCONTEXT_HXX
#include "PersAttrListTContext.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

class XMLChartPlotAreaOASISTContext  : public XMLProcAttrTransformerContext
{
public:
    TYPEINFO();

    explicit XMLChartPlotAreaOASISTContext(
        XMLTransformerBase & rTransformer, const ::rtl::OUString & rQName );
    virtual ~XMLChartPlotAreaOASISTContext();

    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const ::rtl::OUString& rLocalName,
                                   const ::rtl::OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    void ExportCategories();

private:
    ::rtl::Reference< XMLPersAttrListTContext > m_rCategoriesContext;
};

// XMLOFF_XMLCHARTPLOTAREAOASISTCONTEXT_HXX
#endif

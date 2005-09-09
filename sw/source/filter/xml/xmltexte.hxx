/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmltexte.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:27:36 $
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

#ifndef _XMLTEXTE_HXX
#define _XMLTEXTE_HXX

#ifndef _XMLOFF_TEXTPARAE_HXX_
#include <xmloff/txtparae.hxx>
#endif
#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif

class SwXMLExport;
class SvXMLAutoStylePoolP;
class SwNoTxtNode;

namespace com { namespace sun { namespace star { namespace style {
                class XStyle; } } } }

class SwXMLTextParagraphExport : public XMLTextParagraphExport
{
    const ::rtl::OUString sTextTable;
    const ::rtl::OUString sEmbeddedObjectProtocol;
    const ::rtl::OUString sGraphicObjectProtocol;

    const SvGlobalName aAppletClassId;
    const SvGlobalName aPluginClassId;
    const SvGlobalName aIFrameClassId;
    const SvGlobalName aOutplaceClassId;

    SwNoTxtNode *GetNoTxtNode(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet ) const;

protected:
    virtual void exportStyleContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );

    virtual void _collectTextEmbeddedAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet );
    virtual void _exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );

    virtual void exportTable(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress );

public:
    SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& rAutoStylePool );
    ~SwXMLTextParagraphExport();

    virtual void setTextEmbeddedGraphicURL(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet,
        ::rtl::OUString& rStreamName ) const;
};


#endif  //  _XMLTEXTE_HXX

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextListBlockContext.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 14:54:08 $
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

#ifndef _XMLTEXTLISTBLOCKCONTEXT_HXX
#define _XMLTEXTLISTBLOCKCONTEXT_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

class XMLTextImportHelper;

class XMLTextListBlockContext : public SvXMLImportContext
{
    XMLTextImportHelper&    rTxtImport;

    ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > xNumRules;

    const ::rtl::OUString   sNumberingRules;
    ::rtl::OUString         sStyleName;
    SvXMLImportContextRef   xParentListBlock;
    sal_Int16               nLevel;
    sal_Int32               nLevels;
    sal_Bool                bRestartNumbering : 1;
    sal_Bool                bSetDefaults : 1;


public:

    TYPEINFO();

    XMLTextListBlockContext( SvXMLImport& rImport,
            XMLTextImportHelper& rTxtImp, sal_uInt16 nPrfx,
                const ::rtl::OUString& rLName,
                   const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
    virtual ~XMLTextListBlockContext();

    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                const ::rtl::OUString& rLocalName,
                 const ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    const ::rtl::OUString& GetStyleName() const { return sStyleName; }
    sal_Int16 GetLevel() const { return nLevel; }
    sal_Bool IsRestartNumbering() const { return bRestartNumbering; }
    void ResetRestartNumbering() { bRestartNumbering = sal_False; }

//  sal_Bool HasGeneratedStyle() const { return xGenNumRule.is(); }
    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return xNumRules; }
};


#endif

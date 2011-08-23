/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _XMLOFF_STYLEEXP_HXX_
#define _XMLOFF_STYLEEXP_HXX_

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>

#include <bf_xmloff/uniref.hxx>

namespace com { namespace sun { namespace star
{
    namespace style
    {
        class XStyle;
    }
} } }
namespace binfilter {

class XMLPropertySetMapper;
class SvXMLExportPropertyMapper;
class SvXMLAutoStylePoolP;
class SvXMLExport;

class XMLStyleExport : public UniRefBase
{
    SvXMLExport& rExport;
protected:
    const ::rtl::OUString sIsPhysical;
    const ::rtl::OUString sIsAutoUpdate;
    const ::rtl::OUString sFollowStyle;
    const ::rtl::OUString sNumberingStyleName;

    SvXMLExport& GetExport() { return rExport; }
    const SvXMLExport& GetExport() const  { return rExport; }

private:

    const ::rtl::OUString sPoolStyleName;

    SvXMLAutoStylePoolP *pAutoStylePool;

protected:

    virtual sal_Bool exportStyle(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle,
        const ::rtl::OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        const ::rtl::OUString* pPrefix = 0L );

    virtual void exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );

    virtual void exportStyleContent(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );
public:
    XMLStyleExport(
        SvXMLExport& rExp,
        const ::rtl::OUString& rPoolStyleName,
        SvXMLAutoStylePoolP *pAutoStyleP=0 );
    virtual ~XMLStyleExport();

//	void exportStyleFamily(
//		const ::rtl::OUString& rFamily, const ::rtl::OUString& rXMLFamily,
//		const UniReference < XMLPropertySetMapper >& rPropMapper,
//		sal_Bool bUsed, sal_uInt16 nFamily = 0,
//		const ::rtl::OUString* pPrefix = 0L);

//	void exportStyleFamily(
//		const sal_Char *pFamily, const ::rtl::OUString& rXMLFamily,
//		const UniReference < XMLPropertySetMapper >& rPropMapper,
//		sal_Bool bUsed, sal_uInt16 nFamily = 0,
//		const ::rtl::OUString* pPrefix = 0L);

    virtual sal_Bool exportDefaultStyle(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xPropSet,
        const ::rtl::OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper );

    void exportStyleFamily(
        const ::rtl::OUString& rFamily, const ::rtl::OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        sal_Bool bUsed, sal_uInt16 nFamily = 0,
        const ::rtl::OUString* pPrefix = 0L);

    void exportStyleFamily(
        const sal_Char *pFamily, const ::rtl::OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        sal_Bool bUsed, sal_uInt16 nFamily = 0,
        const ::rtl::OUString* pPrefix = 0L);
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

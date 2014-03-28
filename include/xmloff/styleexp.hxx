/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_XMLOFF_STYLEEXP_HXX
#define INCLUDED_XMLOFF_STYLEEXP_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>

#include <xmloff/uniref.hxx>

namespace com { namespace sun { namespace star
{
    namespace style
    {
        class XStyle;
    }
    namespace beans
    {
        class XPropertySet;
    }
    namespace container
    {
        class XNameAccess;
    }

} } }

class XMLPropertySetMapper;
class SvXMLExportPropertyMapper;
class SvXMLAutoStylePoolP;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLStyleExport : public UniRefBase
{
    SvXMLExport& rExport;
protected:
    const OUString sIsPhysical;
    const OUString sIsAutoUpdate;
    const OUString sFollowStyle;
    const OUString sNumberingStyleName;
    const OUString sOutlineLevel;

    SvXMLExport& GetExport() { return rExport; }
    const SvXMLExport& GetExport() const  { return rExport; }

private:

    const OUString sPoolStyleName;

    SvXMLAutoStylePoolP *pAutoStylePool;

protected:

    virtual bool exportStyle(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::style::XStyle > & rStyle,
        const OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xStyles,
        const OUString* pPrefix = 0L );

    virtual void exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );

    virtual void exportStyleContent(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );
public:
    XMLStyleExport(
        SvXMLExport& rExp,
        const OUString& rPoolStyleName,
        SvXMLAutoStylePoolP *pAutoStyleP=0 );
    virtual ~XMLStyleExport();

//  void exportStyleFamily(
//      const OUString& rFamily, const OUString& rXMLFamily,
//      const UniReference < XMLPropertySetMapper >& rPropMapper,
//      bool bUsed, sal_uInt16 nFamily = 0,
//      const OUString* pPrefix = 0L);

//  void exportStyleFamily(
//      const sal_Char *pFamily, const OUString& rXMLFamily,
//      const UniReference < XMLPropertySetMapper >& rPropMapper,
//      bool bUsed, sal_uInt16 nFamily = 0,
//      const OUString* pPrefix = 0L);

    virtual bool exportDefaultStyle(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > & xPropSet,
        const OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper );

    void exportStyleFamily(
        const OUString& rFamily, const OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        bool bUsed, sal_uInt16 nFamily = 0,
        const OUString* pPrefix = 0L);

    void exportStyleFamily(
        const sal_Char *pFamily, const OUString& rXMLFamily,
        const UniReference < SvXMLExportPropertyMapper >& rPropMapper,
        bool bUsed, sal_uInt16 nFamily = 0,
        const OUString* pPrefix = 0L);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

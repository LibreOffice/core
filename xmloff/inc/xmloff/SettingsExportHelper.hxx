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

#ifndef _XMLOFF_SETTINGSEXPORTHELPER_HXX
#define _XMLOFF_SETTINGSEXPORTHELPER_HXX

#include "xmloff/dllapi.h"

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <xmloff/xmlexp.hxx>

namespace com
{
    namespace sun { namespace star {
        namespace container { class XNameAccess; class XIndexAccess; }
        namespace util { struct DateTime; }
    } }
}

namespace xmloff
{
    class XMLSettingsExportContext;
}

class XMLOFF_DLLPUBLIC XMLSettingsExportHelper
{
    ::xmloff::XMLSettingsExportContext& m_rContext;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringSubstitution > mxStringSubsitution;

    const rtl::OUString msPrinterIndependentLayout;
    const rtl::OUString msColorTableURL;
    const rtl::OUString msLineEndTableURL;
    const rtl::OUString msHatchTableURL;
    const rtl::OUString msDashTableURL;
    const rtl::OUString msGradientTableURL;
    const rtl::OUString msBitmapTableURL;

    void ManipulateSetting( com::sun::star::uno::Any& rAny, const rtl::OUString& rName ) const;

    void CallTypeFunction(const com::sun::star::uno::Any& rAny,
                        const rtl::OUString& rName) const;

    void exportBool(const sal_Bool bValue, const rtl::OUString& rName) const;
    void exportByte(const sal_Int8 nValue, const rtl::OUString& rName) const;
    void exportShort(const sal_Int16 nValue, const rtl::OUString& rName) const;
    void exportInt(const sal_Int32 nValue, const rtl::OUString& rName) const;
    void exportLong(const sal_Int64 nValue, const rtl::OUString& rName) const;
    void exportDouble(const double fValue, const rtl::OUString& rName) const;
    void exportString(const rtl::OUString& sValue, const rtl::OUString& rName) const;
    void exportDateTime(const com::sun::star::util::DateTime& aValue, const rtl::OUString& rName) const;
    void exportSequencePropertyValue(
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps,
        const rtl::OUString& rName) const;
    void exportbase64Binary(
        const com::sun::star::uno::Sequence<sal_Int8>& aProps,
        const rtl::OUString& rName) const;
    void exportMapEntry(const com::sun::star::uno::Any& rAny,
                        const rtl::OUString& rName,
                        const sal_Bool bNameAccess) const;
    void exportNameAccess(
        const com::sun::star::uno::Reference<com::sun::star::container::XNameAccess>& aNamed,
        const rtl::OUString& rName) const;
    void exportIndexAccess(
        const com::sun::star::uno::Reference<com::sun::star::container::XIndexAccess> aIndexed,
        const rtl::OUString rName) const;

    void exportSymbolDescriptors(
                    const com::sun::star::uno::Sequence < com::sun::star::formula::SymbolDescriptor > &rProps,
                    const rtl::OUString rName) const;
    void exportForbiddenCharacters(
                    const com::sun::star::uno::Any &rAny,
                    const rtl::OUString rName) const;

public:
    XMLSettingsExportHelper( ::xmloff::XMLSettingsExportContext& i_rContext );
    ~XMLSettingsExportHelper();

    void exportAllSettings(
        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& aProps,
        const rtl::OUString& rName) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

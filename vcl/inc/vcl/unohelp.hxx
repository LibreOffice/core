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

#ifndef _VCL_UNOHELP_HXX
#define _VCL_UNOHELP_HXX

#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>

namespace com {
namespace sun {
namespace star {
namespace i18n {
    class XBreakIterator;
    class XCharacterClassification;
    class XCollator;
}}}}

namespace com {
namespace sun {
namespace star {
namespace accessibility {
    struct AccessibleEventObject;
}
}}}

namespace comphelper {
    namespace string {
        class NaturalStringSorter;
}}

namespace vcl
{
namespace unohelper
{
VCL_DLLPUBLIC ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XBreakIterator > CreateBreakIterator();
VCL_DLLPUBLIC ::com::sun::star::uno::Reference < ::com::sun::star::i18n::XCharacterClassification> CreateCharacterClassification();
//Get access to singleton Natural String Sorter collating for Application::GetLocale
VCL_DLLPUBLIC const comphelper::string::NaturalStringSorter& getNaturalStringSorterForAppLocale();
VCL_DLLPUBLIC ::rtl::OUString CreateLibraryName( const sal_Char* pModName, sal_Bool bSUPD );
VCL_DLLPUBLIC void NotifyAccessibleStateEventGlobally( const ::com::sun::star::accessibility::AccessibleEventObject& rEventObject );
}}  // namespace vcl::unohelper

#endif  // _VCL_UNOHELP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

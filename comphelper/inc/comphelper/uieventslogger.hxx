/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef _COMPHELPER_UIEVENTSLOGGER_HXX
#define _COMPHELPER_UIEVENTSLOGGER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/URL.hpp>
#include <comphelper/comphelperdllapi.h>
#include <rtl/ustring.hxx>


namespace comphelper
{
    // this class is part of the OOo Improvement Program Core
    class COMPHELPER_DLLPUBLIC UiEventsLogger
    {
        public:
            static sal_Bool isEnabled();
            static sal_Int32 getSessionLogEventCount();
            static void appendDispatchOrigin( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& args, const ::rtl::OUString& originapp, const ::rtl::OUString& originwidget);
            static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue> purgeDispatchOrigin(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& args);
            static void logDispatch(const ::com::sun::star::util::URL& url, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& args);
            static void logVcl(const ::rtl::OUString& parent_id, sal_Int32 window_type, const ::rtl::OUString& id, const ::rtl::OUString& method, const ::rtl::OUString& param);
            static void logVcl(const ::rtl::OUString& parent_id, sal_Int32 window_type, const ::rtl::OUString& id, const ::rtl::OUString& method, sal_Int32 param);
            static void logVcl(const ::rtl::OUString& parent_id, sal_Int32 window_type, const ::rtl::OUString& id, const ::rtl::OUString& method);
            static void disposing();
            static void reinit();
            virtual ~UiEventsLogger() {}
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

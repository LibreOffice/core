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
#ifndef _XMLOFF_XFORMSEXPORT_HXX
#define _XMLOFF_XFORMSEXPORT_HXX

#include "xmloff/dllapi.h"

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace uno { template<typename T> class Reference; }
    namespace uno { template<typename T> class Sequence; }
    namespace frame { class XModel; }
    namespace beans { class XPropertySet; struct PropertyValue; }
    namespace container { class XNameAccess; }
} } }
namespace rtl { class OUString; }


/** export an XForms model. */
void SAL_DLLPRIVATE exportXForms( SvXMLExport& );

rtl::OUString SAL_DLLPRIVATE getXFormsBindName( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xBinding );

rtl::OUString SAL_DLLPRIVATE getXFormsListBindName( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xBinding );

rtl::OUString SAL_DLLPRIVATE getXFormsSubmissionName( const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xBinding );


/** returns the settings of the given XForms container, to be exported as document specific settings
*/
void XMLOFF_DLLPUBLIC getXFormsSettings(
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rXForms,
              ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _out_rSettings
    );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

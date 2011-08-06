/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef XML_SETTINGS_EXPORT_CONTEXT_HXX
#define XML_SETTINGS_EXPORT_CONTEXT_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
/** === end UNO includes === **/

#include "xmloff/xmltoken.hxx"

//........................................................................
namespace xmloff
{
//........................................................................

    //====================================================================
    //= XMLExporter
    //====================================================================
    class SAL_NO_VTABLE XMLSettingsExportContext
    {
    public:
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      const ::rtl::OUString& i_rValue ) = 0;
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      enum ::xmloff::token::XMLTokenEnum i_eValue ) = 0;

        virtual void    StartElement( enum ::xmloff::token::XMLTokenEnum i_eName,
                                      const sal_Bool i_bIgnoreWhitespace ) = 0;
        virtual void    EndElement(   const sal_Bool i_bIgnoreWhitespace ) = 0;

        virtual void    Characters( const ::rtl::OUString& i_rCharacters ) = 0;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        GetServiceFactory() const = 0;

    };

//........................................................................
} // namespace xmloff
//........................................................................

#endif // XML_SETTINGS_EXPORT_CONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

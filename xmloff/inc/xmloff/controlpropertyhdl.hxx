/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: controlpropertyhdl.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#define _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/prhdlfac.hxx>
#include <vos/ref.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmlexppr.hxx>
#include <xmloff/ControlBorderHandler.hxx>

struct SvXMLEnumMapEntry;
class XMLConstantsPropertyHandler;
class Color;
//.........................................................................
namespace xmloff
{
//.........................................................................

    class IFormsExportContext;
    class IFormsImportContext;

    //=====================================================================
    //= ORotationAngleHandler
    //=====================================================================
    class ORotationAngleHandler : public XMLPropertyHandler
    {
    public:
        ORotationAngleHandler();

        virtual sal_Bool importXML( const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
    };

    //=====================================================================
    //= OFontWidthHandler
    //=====================================================================
    class OFontWidthHandler : public XMLPropertyHandler
    {
    public:
        OFontWidthHandler();

        virtual sal_Bool importXML( const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
    };

    //=====================================================================
    //= OControlBorderHandlerBase
    //=====================================================================
    class OControlBorderHandlerBase : public XMLPropertyHandler
    {
    public:
        OControlBorderHandlerBase();

        virtual sal_Bool importXML( const ::rtl::OUString& _rStrImpValue, ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;
        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const = 0;

    protected:
        /** called during importXML, after the string value has been split
        */
        virtual void pickOne( const Color& _rColor, sal_Int16 _nStyle, ::com::sun::star::uno::Any& _rValue ) const = 0;
    };

    //=====================================================================
    //= OControlBorderStyleHandler
    //=====================================================================
    class OControlBorderStyleHandler : public OControlBorderHandlerBase
    {
    public:
        OControlBorderStyleHandler();

        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;

    protected:
        virtual void pickOne( const Color& _rColor, sal_Int16 _nStyle, ::com::sun::star::uno::Any& _rValue ) const;
    };

    //=====================================================================
    //= OControlBorderStyleHandler
    //=====================================================================
    class OControlBorderColorHandler : public OControlBorderHandlerBase
    {
    public:
        OControlBorderColorHandler();

        virtual sal_Bool exportXML( ::rtl::OUString& _rStrExpValue, const ::com::sun::star::uno::Any& _rValue, const SvXMLUnitConverter& _rUnitConverter ) const;

    protected:
        virtual void pickOne( const Color& _rColor, sal_Int16 _nStyle, ::com::sun::star::uno::Any& _rValue ) const;
    };

    //=====================================================================
    //= OControlPropertyHandlerFactory
    //=====================================================================
    class XMLOFF_DLLPUBLIC OControlPropertyHandlerFactory:
        public XMLPropertyHandlerFactory
    {
    protected:
        mutable XMLConstantsPropertyHandler*    m_pTextAlignHandler;
        mutable OControlBorderStyleHandler*     m_pControlBorderStyleHandler;
        mutable OControlBorderColorHandler*     m_pControlBorderColorHandler;
        mutable ORotationAngleHandler*          m_pRotationAngleHandler;
        mutable OFontWidthHandler*              m_pFontWidthHandler;
        mutable XMLConstantsPropertyHandler*    m_pFontEmphasisHandler;
        mutable XMLConstantsPropertyHandler*    m_pFontReliefHandler;

    protected:
        virtual ~OControlPropertyHandlerFactory();

    public:
        OControlPropertyHandlerFactory();

        virtual const XMLPropertyHandler* GetPropertyHandler(sal_Int32 _nType) const;
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_


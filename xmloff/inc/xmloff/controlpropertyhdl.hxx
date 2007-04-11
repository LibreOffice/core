/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: controlpropertyhdl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:26:32 $
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

#ifndef _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_
#define _XMLOFF_FORMS_CONTROLPROPERTYHDL_HXX_

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <xmloff/prhdlfac.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _XMLOFF_XMLEXPPR_HXX
#include <xmloff/xmlexppr.hxx>
#endif
#ifndef _XMLOFF_CONTROLBORDERHDL_HXX_
#include <xmloff/ControlBorderHandler.hxx>
#endif

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


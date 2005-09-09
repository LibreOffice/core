/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formstyles.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:11:35 $
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

#ifndef XMLOFF_FORMSTYLES_HXX
#define XMLOFF_FORMSTYLES_HXX

#ifndef _XMLOFF_PRSTYLEI_HXX_
#include "prstylei.hxx"
#endif

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OControlStyleContext
    //=====================================================================
    class OControlStyleContext : public XMLPropStyleContext
    {
    protected:
        ::rtl::OUString     m_sNumberStyleName;

    public:
        OControlStyleContext(
            SvXMLImport& _rImport,
            const sal_uInt16 _Prefix,
            const ::rtl::OUString& _rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& _rxAttrList,
            SvXMLStylesContext& _rParentStyles,
            const sal_uInt16 _nFamily,
            const sal_Bool _bDefaultStyle
        );

        inline const ::rtl::OUString& getNumberStyleName( ) const { return m_sNumberStyleName; }

    protected:
        virtual void SetAttribute(
            sal_uInt16 _nPrefixKey,
            const ::rtl::OUString& _rLocalName,
            const ::rtl::OUString& _rValue
        );
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // XMLOFF_FORMSTYLES_HXX


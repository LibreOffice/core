/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: characterattributeshelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:23:00 $
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

#ifndef ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX
#define ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif

#include <map>

// -----------------------------------------------------------------------------
// class CharacterAttributesHelper
// -----------------------------------------------------------------------------

class CharacterAttributesHelper
{
private:

    typedef ::std::map< ::rtl::OUString, ::com::sun::star::uno::Any, ::std::less< ::rtl::OUString > > AttributeMap;

    AttributeMap    m_aAttributeMap;

public:

    CharacterAttributesHelper( const Font& rFont, sal_Int32 nBackColor, sal_Int32 nColor );
    ~CharacterAttributesHelper();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > GetCharacterAttributes( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes );
};

#endif  // ACCESSIBILITY_HELPER_CHARACTERATTRIBUTESHELPER_HXX

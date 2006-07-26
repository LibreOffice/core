/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linedescriptor.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:58:36 $
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
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#define _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_

#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_INSPECTION_LINEDESCRIPTOR_HPP_
#include <com/sun/star/inspection/LineDescriptor.hpp>
#endif
/** === end UNO includes === **/

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= OLineDescriptor
    //========================================================================
    struct OLineDescriptor : public ::com::sun::star::inspection::LineDescriptor
    {
        ::rtl::OUString             sName;              // the name of the property
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >
                                    xPropertyHandler;   // the handler for this property
        ::com::sun::star::uno::Any  aValue;             // the current value of the property

        bool                        bUnknownValue    : 1;   // is the property value currently "unknown"? (PropertyState_AMBIGUOUS)

        OLineDescriptor()
            :bUnknownValue( false )
        {
        }

        void assignFrom( const ::com::sun::star::inspection::LineDescriptor& _rhs )
        {
            ::com::sun::star::inspection::LineDescriptor::operator=( _rhs );
        }
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_


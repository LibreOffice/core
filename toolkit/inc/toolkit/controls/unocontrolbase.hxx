/*************************************************************************
 *
 *  $RCSfile: unocontrolbase.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:02:08 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLKIT_AWT_UNOCONTROLBASE_HXX_
#define _TOOLKIT_AWT_UNOCONTROLBASE_HXX_

#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

#include <toolkit/controls/unocontrol.hxx>

//  ----------------------------------------------------
//  class UnoControlBase
//  ----------------------------------------------------

class UnoControlBase : public UnoControl
{
protected:
    sal_Bool                    ImplHasProperty( const ::rtl::OUString& aPropertyName );
    void                        ImplSetPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Bool bUpdateThis );
    ::com::sun::star::uno::Any  ImplGetPropertyValue( const ::rtl::OUString& aPropertyName );

    sal_Bool        ImplGetPropertyValue_BOOL( sal_uInt16 nProp );
    sal_Int16       ImplGetPropertyValue_INT16( sal_uInt16 nProp );
    sal_uInt16      ImplGetPropertyValue_UINT16( sal_uInt16 nProp );
    sal_Int32       ImplGetPropertyValue_INT32( sal_uInt16 nProp );
    sal_uInt32      ImplGetPropertyValue_UINT32( sal_uInt16 nProp );
    double          ImplGetPropertyValue_DOUBLE( sal_uInt16 nProp );
    ::rtl::OUString ImplGetPropertyValue_UString( sal_uInt16 nProp );

    // XLayoutConstrains (nur wenn das Control es unterstuetzt!)
    ::com::sun::star::awt::Size Impl_getMinimumSize();
    ::com::sun::star::awt::Size Impl_getPreferredSize();
    ::com::sun::star::awt::Size Impl_calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize );

    // XTextLayoutConstrains (nur wenn das Control es unterstuetzt!)
    ::com::sun::star::awt::Size Impl_getMinimumSize( sal_Int16 nCols, sal_Int16 nLines );
    void                        Impl_getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines );
};



#endif // _TOOLKIT_AWT_UNOCONTROLBASE_HXX_


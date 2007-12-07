/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapropvalue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 11:00:25 $
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
#ifndef SC_VBA_PROPVALULE_HXX
#define SC_VBA_PROPVALULE_HXX
#include <org/openoffice/vba/XPropValue.hpp>
#include <cppuhelper/implbase1.hxx>

#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1< oo::vba::XPropValue > PropValueImpl_BASE;

class PropListener
{
public:
    virtual void setValueEvent( const css::uno::Any& value ) = 0;
    virtual css::uno::Any getValueEvent() = 0;
};


class ScVbaPropValue : public PropValueImpl_BASE
{
    PropListener* m_pListener;
public:
    ScVbaPropValue( PropListener* pListener );

    // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException);

    rtl::OUString SAL_CALL getDefaultPropertyName() throw (css::uno::RuntimeException) { return ::rtl::OUString::createFromAscii("Value"); }

};
#endif //SC_VBA_PROPVALULE_HXX

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedNamedProperty.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:22:14 $
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
#ifndef CHART_WRAPPEDNAMEDPROPERTY_HXX
#define CHART_WRAPPEDNAMEDPROPERTY_HXX

#include "WrappedProperty.hxx"
#include "Chart2ModelContact.hxx"

#include <boost/shared_ptr.hpp>
#include <vector>

//.............................................................................
namespace chart
{
namespace wrapper
{

class WrappedNamedProperty : public WrappedProperty
{
public:
    WrappedNamedProperty( const ::rtl::OUString& rOuterName, const ::rtl::OUString& rInnerName, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedNamedProperty();

    static void addWrappedProperties( std::vector< WrappedProperty* >& rList, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  );
    static void addWrappedFillProperties( std::vector< WrappedProperty* >& rList, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  );
    static void addWrappedLineProperties( std::vector< WrappedProperty* >& rList, ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact  );

protected:
    virtual ::com::sun::star::uno::Any convertInnerToOuterValue( const ::com::sun::star::uno::Any& rInnerValue ) const;
    virtual ::com::sun::star::uno::Any convertOuterToInnerValue( const ::com::sun::star::uno::Any& rOuterValue ) const;

protected:
    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
};

//-----------------------------------------------------------------------------

class WrappedFillGradientNameProperty : public WrappedNamedProperty
{
public:
    WrappedFillGradientNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedFillGradientNameProperty();
};

//-----------------------------------------------------------------------------

class WrappedFillHatchNameProperty : public WrappedNamedProperty
{
public:
    WrappedFillHatchNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedFillHatchNameProperty();
};

//-----------------------------------------------------------------------------

class WrappedFillBitmapNameProperty : public WrappedNamedProperty
{
public:
    WrappedFillBitmapNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedFillBitmapNameProperty();
};

//-----------------------------------------------------------------------------

class WrappedFillTransparenceGradientNameProperty : public WrappedNamedProperty
{
public:
    WrappedFillTransparenceGradientNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedFillTransparenceGradientNameProperty();
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class WrappedLineDashNameProperty : public WrappedNamedProperty
{
public:
    WrappedLineDashNameProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~WrappedLineDashNameProperty();
};

} //namespace wrapper
} //namespace chart
//.............................................................................

// CHART_WRAPPEDNAMEDPROPERTY_HXX
#endif

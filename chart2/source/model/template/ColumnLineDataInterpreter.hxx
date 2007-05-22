/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColumnLineDataInterpreter.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:48:07 $
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
#ifndef CHART_COLUMNLINEDATAINTERPRETER_HXX
#define CHART_COLUMNLINEDATAINTERPRETER_HXX

#ifndef CHART_DATAINTERPRETER_HXX
#include "DataInterpreter.hxx"
#endif

namespace chart
{

class ColumnLineDataInterpreter : public DataInterpreter
{
public:
    explicit ColumnLineDataInterpreter(
        sal_Int32 nNumberOfLines,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    ~ColumnLineDataInterpreter();

protected:
    // ____ XDataInterpreter ____
    virtual ::com::sun::star::chart2::InterpretedData SAL_CALL interpretDataSource(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XDataSource >& xSource,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArguments,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > >& aSeriesToReUse )
        throw (::com::sun::star::uno::RuntimeException);

private:
    sal_Int32 m_nNumberOfLines;
};

} // namespace chart

// CHART_COLUMNLINEDATAINTERPRETER_HXX
#endif

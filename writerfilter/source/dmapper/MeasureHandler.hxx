/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MeasureHandler.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2007-05-09 13:49:23 $
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
#ifndef INCLUDED_MEASUREHANDLER_HXX
#define INCLUDED_MEASUREHANDLER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <doctok/WW8ResourceModel.hxx>
#include <boost/shared_ptr.hpp>

namespace dmapper
{
class PropertyMap;
/** Handler for sprms that contain a measure and a unit
    - Left indent of tables
    - Preferred width of tables
 */
class WRITERFILTER_DLLPRIVATE MeasureHandler : public doctok::Properties
{
    sal_Int32 m_nMeasureValue;
    sal_Int32 m_nUnit;

public:
    MeasureHandler();
    virtual ~MeasureHandler();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    sal_Int32 getTwipValue() const;
};
typedef boost::shared_ptr< MeasureHandler >  MeasureHandlerPtr;
}

#endif //

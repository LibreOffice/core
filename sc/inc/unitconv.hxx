/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unitconv.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-04-07 16:22:35 $
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

#ifndef SC_UNITCONV_HXX
#define SC_UNITCONV_HXX

#include "collect.hxx"


class ScUnitConverterData : public StrData
{
            double          fValue;

                            // not implemented
    ScUnitConverterData&    operator=( const ScUnitConverterData& );

public:
                            ScUnitConverterData( const String& rFromUnit,
                                const String& rToUnit, double fValue = 1.0 );
                            ScUnitConverterData( const ScUnitConverterData& );
    virtual                 ~ScUnitConverterData() {};

    virtual DataObject*     Clone() const;

            double          GetValue() const    { return fValue; }

    static  void            BuildIndexString( String& rStr,
                                const String& rFromUnit, const String& rToUnit );

};


class ScUnitConverter : public StrCollection
{
                            // not implemented
                            ScUnitConverter( const ScUnitConverter& );
        ScUnitConverter&    operator=( const ScUnitConverter& );

public:
                            ScUnitConverter( USHORT nInit = 16, USHORT nDelta = 4 );
    virtual                 ~ScUnitConverter() {};

        BOOL                GetValue( double& fValue, const String& rFromUnit,
                                const String& rToUnit ) const;
};


#endif


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConfigurationAccess.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:21:55 $
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

#ifndef _CHART2_CONFIGURATIONACCESS_HXX
#define _CHART2_CONFIGURATIONACCESS_HXX

// header for enum FieldUnit
#ifndef _VCL_FLDUNIT_HXX
#include <vcl/fldunit.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class CalcConfigItem;
class ConfigurationAccess
{
public:
    ConfigurationAccess();
    virtual ~ConfigurationAccess();

    /** @descr Retrieve the FieldUnit to be used for the UI.  This unit is retrieved
    from the registry settings of the Calc application.

    If this setting can not be found there is a fallback to cm which is the most
    common setting worldwide (or not?)

    @return the FieldUnit enum.  See <vcl/fldunit.hxx> for definition
    */
    FieldUnit getFieldUnit();

private:
    CalcConfigItem* m_pCalcConfigItem;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

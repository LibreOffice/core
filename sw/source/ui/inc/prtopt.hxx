/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prtopt.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:58:29 $
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
#ifndef _PRTOPT_HXX
#define _PRTOPT_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _SW_PRINTDATA_HXX
#include <printdata.hxx>
#endif

class SwPrintOptions : public SwPrintData, public utl::ConfigItem
{
    sal_Bool            bIsWeb;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    SwPrintOptions(sal_Bool bWeb);
    virtual ~SwPrintOptions();

    virtual void            Commit();
    virtual void            doSetModified( ) { bModified = sal_True; SetModified();}

    SwPrintOptions& operator=(const SwPrintData& rData)
    {
        SwPrintData::operator=( rData );
        SetModified();
        return *this;
    }
};

#endif


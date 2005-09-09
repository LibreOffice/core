/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: labelcfg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:24:13 $
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
#ifndef _LABELCFG_HXX
#define _LABELCFG_HXX

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

class SwLabRecs;
class SwLabRec;

class SW_DLLPUBLIC SwLabelConfig : public utl::ConfigItem
{
    com::sun::star::uno::Sequence<rtl::OUString> aNodeNames;

    SW_DLLPRIVATE com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();

public:
    SwLabelConfig();
    virtual ~SwLabelConfig();

    virtual void            Commit();

    void    FillLabels(const rtl::OUString& rManufacturer, SwLabRecs& rLabArr);
    const com::sun::star::uno::Sequence<rtl::OUString>&
            GetManufacturers() const {return aNodeNames;}

    sal_Bool    HasLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType);
    void        SaveLabel(const rtl::OUString& rManufacturer, const rtl::OUString& rType,
                            const SwLabRec& rRec);
};

#endif


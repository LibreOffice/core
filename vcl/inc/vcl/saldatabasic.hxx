/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: saldatabasic.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:06:05 $
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

#ifndef _SV_SALDATABASIC_HXX
#define _SV_SALDATABASIC_HXX

#ifndef _SV_SVDATA_HXX
#include <vcl/svdata.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <vcl/salinst.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif

class VCL_DLLPUBLIC SalData
{
public:
    SalInstance*        m_pInstance; // pointer to instance
    oslModule           m_pPlugin;   // plugin library handle

    SalData();
    virtual ~SalData();

};

// -=-= inlines =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline void SetSalData( SalData* pData )
{ ImplGetSVData()->mpSalData = (void*)pData; }

inline SalData* GetSalData()
{ return (SalData*)ImplGetSVData()->mpSalData; }

#endif

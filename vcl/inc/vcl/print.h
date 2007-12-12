/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: print.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2007-12-12 15:03:43 $
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

#ifndef _SV_PRINT_H
#define _SV_PRINT_H

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#include <vector>
#include <hash_map>

struct SalPrinterQueueInfo;
class QueueInfo;

// ------------------------
// - private printer data -
// ------------------------
struct ImplPrivatePrinterData
{
    bool        mbNextJobIsQuick;

    ImplPrivatePrinterData() :
        mbNextJobIsQuick( false )
    {}
};

// --------------------
// - ImplPrnQueueData -
// --------------------

struct ImplPrnQueueData
{
    QueueInfo*              mpQueueInfo;
    SalPrinterQueueInfo*    mpSalQueueInfo;
};

// --------------------
// - ImplPrnQueueList -
// --------------------

class VCL_DLLPUBLIC ImplPrnQueueList
{
public:
    std::hash_map< rtl::OUString, sal_Int32, rtl::OUStringHash >
                                        m_aNameToIndex;
    std::vector< ImplPrnQueueData >     m_aQueueInfos;
    std::vector< rtl::OUString >        m_aPrinterList;

    ImplPrnQueueList() {}
    ~ImplPrnQueueList();

    void                    Add( SalPrinterQueueInfo* pData );
    ImplPrnQueueData*       Get( const rtl::OUString& rPrinter );
};

// --------------
// - Prototypes -
// --------------

void ImplDeletePrnQueueList();

#endif // _SV_PRINT_H

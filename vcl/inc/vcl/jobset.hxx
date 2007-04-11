/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jobset.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:58:42 $
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

#ifndef _SV_JOBSET_HXX
#define _SV_JOBSET_HXX

#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_PRNTYPES_HXX
#include <vcl/prntypes.hxx>
#endif

class SvStream;
struct ImplJobSetup;

// ------------
// - JobSetup -
// ------------

class VCL_DLLPUBLIC JobSetup
{
    friend class Printer;

private:
    ImplJobSetup*       mpData;

//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE ImplJobSetup*        ImplGetData();
    SAL_DLLPRIVATE ImplJobSetup*        ImplGetConstData();
    SAL_DLLPRIVATE const ImplJobSetup*  ImplGetConstData() const;
//#endif

public:
                        JobSetup();
                        JobSetup( const JobSetup& rJob );
                        ~JobSetup();

    String              GetPrinterName() const;
    String              GetDriverName() const;

    /*  Get/SetValue are used to read/store additional
     *  Parameters in the job setup that may be used
     *  by the printer driver. One possible use are phone
     *  numbers for faxes (which disguise as printers)
     */
    String              GetValue( const String& rKey ) const;
    void                SetValue( const String& rKey, const String& rValue );

    JobSetup&           operator=( const JobSetup& rJob );

    BOOL                operator==( const JobSetup& rJobSetup ) const;
    BOOL                operator!=( const JobSetup& rJobSetup ) const
                            { return !(JobSetup::operator==( rJobSetup )); }

    friend VCL_DLLPUBLIC SvStream&  operator>>( SvStream& rIStream, JobSetup& rJobSetup );
    friend VCL_DLLPUBLIC SvStream&  operator<<( SvStream& rOStream, const JobSetup& rJobSetup );
};

#endif  // _SV_JOBSET_HXX

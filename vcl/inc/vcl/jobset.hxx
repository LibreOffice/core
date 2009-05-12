/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: jobset.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_JOBSET_HXX
#define _SV_JOBSET_HXX

#include <tools/string.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/prntypes.hxx>

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

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_JOBSET_HXX
#define _SV_JOBSET_HXX

#include <tools/solar.h>
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

public:
    SAL_DLLPRIVATE ImplJobSetup*        ImplGetData();
    SAL_DLLPRIVATE ImplJobSetup*        ImplGetConstData();
    SAL_DLLPRIVATE const ImplJobSetup*  ImplGetConstData() const;

public:
                        JobSetup();
                        JobSetup( const JobSetup& rJob );
                        ~JobSetup();

    OUString              GetPrinterName() const;
    OUString              GetDriverName() const;

    /*  Get/SetValue are used to read/store additional
     *  Parameters in the job setup that may be used
     *  by the printer driver. One possible use are phone
     *  numbers for faxes (which disguise as printers)
     */
    void                SetValue( const OUString& rKey, const OUString& rValue );

    JobSetup&           operator=( const JobSetup& rJob );

    sal_Bool                operator==( const JobSetup& rJobSetup ) const;
    sal_Bool                operator!=( const JobSetup& rJobSetup ) const
                            { return !(JobSetup::operator==( rJobSetup )); }

    friend VCL_DLLPUBLIC SvStream&  operator>>( SvStream& rIStream, JobSetup& rJobSetup );
    friend VCL_DLLPUBLIC SvStream&  operator<<( SvStream& rOStream, const JobSetup& rJobSetup );
};

#endif  // _SV_JOBSET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

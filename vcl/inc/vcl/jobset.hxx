/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

    sal_Bool                operator==( const JobSetup& rJobSetup ) const;
    sal_Bool                operator!=( const JobSetup& rJobSetup ) const
                            { return !(JobSetup::operator==( rJobSetup )); }

    friend VCL_DLLPUBLIC SvStream&  operator>>( SvStream& rIStream, JobSetup& rJobSetup );
    friend VCL_DLLPUBLIC SvStream&  operator<<( SvStream& rOStream, const JobSetup& rJobSetup );
};

#endif  // _SV_JOBSET_HXX

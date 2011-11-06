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


#ifndef _SFX_PRINTER_HXX
#define _SFX_PRINTER_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif

class SfxTabPage;
class SfxItemSet;

struct SfxPrinter_Impl;

// class SfxPrinter ------------------------------------------------------

class SFX2_DLLPUBLIC SfxPrinter : public Printer
{
private:
    JobSetup                aOrigJobSetup;
    SfxItemSet*             pOptions;
    SfxPrinter_Impl*        pImpl;
    sal_Bool                    bKnown;

    SAL_DLLPRIVATE void operator =(SfxPrinter &); // not defined

    SAL_DLLPRIVATE void UpdateFonts_Impl();

public:
                            SfxPrinter( SfxItemSet *pTheOptions );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const String &rPrinterName );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const JobSetup &rTheOrigJobSetup );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const String &rPrinterName,
                                        const JobSetup &rTheOrigJobSetup );
                            SfxPrinter( const SfxPrinter &rPrinter );
                            ~SfxPrinter();

    SfxPrinter*             Clone() const;

    static SfxPrinter*      Create( SvStream &rStream, SfxItemSet *pOptions );
    SvStream&               Store( SvStream &rStream ) const;

    const JobSetup&         GetOrigJobSetup() const { return aOrigJobSetup; }
    void                    SetOrigJobSetup( const JobSetup &rNewJobSetup );

    const SfxItemSet&       GetOptions() const { return *pOptions; }
    void                    SetOptions( const SfxItemSet &rNewOptions );

    sal_Bool                    IsKnown() const { return bKnown; }
    sal_Bool                    IsOriginal() const { return bKnown; }
};

#endif

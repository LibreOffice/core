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
#ifndef INCLUDED_SFX2_PRINTER_HXX
#define INCLUDED_SFX2_PRINTER_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <vcl/print.hxx>

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
    bool                    bKnown;

    SAL_DLLPRIVATE void operator =(SfxPrinter &) = delete;

public:
                            SfxPrinter( SfxItemSet *pTheOptions );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const OUString &rPrinterName );
                            SfxPrinter( SfxItemSet *pTheOptions,
                                        const JobSetup &rTheOrigJobSetup );
                            SfxPrinter( const SfxPrinter &rPrinter );
                            virtual ~SfxPrinter();
    virtual void            dispose() override;

    VclPtr<SfxPrinter>      Clone() const;

    static VclPtr<SfxPrinter> Create( SvStream &rStream, SfxItemSet *pOptions );
    SvStream&               Store( SvStream &rStream ) const;

    const SfxItemSet&       GetOptions() const { return *pOptions; }
    void                    SetOptions( const SfxItemSet &rNewOptions );

    bool                    IsKnown() const { return bKnown; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

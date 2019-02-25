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

#ifndef INCLUDED_SVTOOLS_PRINTOPTIONS_HXX
#define INCLUDED_SVTOOLS_PRINTOPTIONS_HXX

#include <svtools/svtdllapi.h>
#include <sal/types.h>
#include <unotools/options.hxx>

namespace osl { class Mutex; }

class SvtPrintOptions_Impl;
class PrinterOptions;


class SVT_DLLPUBLIC SvtBasePrintOptions: public utl::detail::Options
{

    SvtPrintOptions_Impl* m_pDataContainer;

protected:

    void        SetDataContainer( SvtPrintOptions_Impl* pDataContainer ) { m_pDataContainer = pDataContainer; }

public:

    static ::osl::Mutex& GetOwnStaticMutex();

public:

                SvtBasePrintOptions();
                virtual ~SvtBasePrintOptions() override;

    bool        IsReduceTransparency() const;
    sal_Int16   GetReducedTransparencyMode() const;
    bool        IsReduceGradients() const;
    sal_Int16   GetReducedGradientMode() const;
    sal_Int16   GetReducedGradientStepCount() const;
    bool        IsReduceBitmaps() const;
    sal_Int16   GetReducedBitmapMode() const;
    sal_Int16   GetReducedBitmapResolution() const;
    bool        IsReducedBitmapIncludesTransparency() const;
    bool        IsConvertToGreyscales() const;
    bool        IsPDFAsStandardPrintJobFormat() const;

    void        SetReduceTransparency( bool bState );
    void        SetReducedTransparencyMode( sal_Int16 nMode );
    void        SetReduceGradients( bool bState );
    void        SetReducedGradientMode( sal_Int16 nMode );
    void        SetReducedGradientStepCount( sal_Int16 nStepCount );
    void        SetReduceBitmaps( bool bState );
    void        SetReducedBitmapMode( sal_Int16   bState );
    void        SetReducedBitmapResolution( sal_Int16 nResolution );
    void        SetReducedBitmapIncludesTransparency( bool bState );
    void        SetConvertToGreyscales( bool bState );
    void        SetPDFAsStandardPrintJobFormat( bool bState );

public:

    void        GetPrinterOptions( PrinterOptions& rOptions ) const;
    void        SetPrinterOptions( const PrinterOptions& rOptions );
};


class SVT_DLLPUBLIC SvtPrinterOptions : public SvtBasePrintOptions
{
private:

    static SvtPrintOptions_Impl*    m_pStaticDataContainer;
    static sal_Int32                m_nRefCount;

public:

    SvtPrinterOptions();
    virtual ~SvtPrinterOptions() override;
};


class SVT_DLLPUBLIC SvtPrintFileOptions : public SvtBasePrintOptions
{
private:

    static SvtPrintOptions_Impl*    m_pStaticDataContainer;
    static sal_Int32                m_nRefCount;

public:

    SvtPrintFileOptions();
    virtual ~SvtPrintFileOptions() override;
};

#endif // INCLUDED_SVTOOLS_PRINTOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

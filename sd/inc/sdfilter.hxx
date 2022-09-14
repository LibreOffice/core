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

#pragma once

#include <osl/module.h>
#include <rtl/ustring.hxx>
#include <tools/ref.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include "sddllapi.h"
#include <vector>

namespace osl { class Module; }
namespace sd { class DrawDocShell; }
class SdDrawDocument;
class SfxMedium;
class SfxObjectShell;
class SotStorage;
class SvMemoryStream;

class SdFilter
{
public:
    SdFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell );
    virtual ~SdFilter();

    bool                    IsDraw() const { return mbIsDraw; }
    virtual bool            Export() = 0;

protected:
    css::uno::Reference< css::frame::XModel >             mxModel;
    css::uno::Reference< css::task::XStatusIndicator >    mxStatusIndicator;

    SfxMedium&                  mrMedium;
    ::sd::DrawDocShell&         mrDocShell;
    SdDrawDocument&             mrDocument;
    bool                        mbIsDraw : 1;
    void                        CreateStatusIndicator();
};

SD_DLLPUBLIC bool ExportPPT( const std::vector< css::beans::PropertyValue >& rMediaData,
                    tools::SvRef<SotStorage> const & rSvStorage,
                    css::uno::Reference< css::frame::XModel > const & rXModel,
                    css::uno::Reference< css::task::XStatusIndicator > const & rXStatInd,
                    SvMemoryStream* pVBA,
                    sal_uInt32 nCnvrtFlags );

// exported function
SD_DLLPUBLIC bool ImportPPT(
        SdDrawDocument* pDocument, SvStream& rDocStream, SotStorage& rStorage, SfxMedium& rMedium );

SD_DLLPUBLIC bool SaveVBA( SfxObjectShell& rDocShell, SvMemoryStream*& pBas );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

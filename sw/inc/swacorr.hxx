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
#ifndef INCLUDED_SW_INC_SWACORR_HXX
#define INCLUDED_SW_INC_SWACORR_HXX

#include <editeng/svxacorr.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XStorage.hpp>

#include "SwXMLTextBlocks.hxx"
#include "swdllapi.h"

class SW_DLLPUBLIC SwAutoCorrect : public SvxAutoCorrect
{
    using  SvxAutoCorrect::PutText;

    std::unique_ptr<SwXMLTextBlocks> m_pTextBlocks;

protected:
    // Return replacement text (only for SWG-format, all others can be obtained from wordlist!).
    // rShort is stream-name - encrypted!
    virtual bool GetLongText( const OUString& rShort, OUString& rLong ) override;

    virtual void refreshBlockList( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& ) override;

    // Text with attributes (only SWG-format!).
    // rShort is stream-name - encrypted!
    virtual bool PutText( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&,
                              const OUString& rFileName, const OUString& rShort, SfxObjectShell&, OUString& ) override;

public:
    SwAutoCorrect( const SvxAutoCorrect& rACorr );
    virtual ~SwAutoCorrect();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

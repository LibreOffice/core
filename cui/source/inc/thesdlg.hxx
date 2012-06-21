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
#ifndef _SVX_THESDLG_HXX
#define _SVX_THESDLG_HXX


#include <com/sun/star/linguistic2/XThesaurus.hpp>

#include "svx/stddlg.hxx"
#include "svx/svxdllapi.h"

#include <memory>


/////////////////////////////////////////////////////////////////

struct SvxThesaurusDialog_Impl;

class SvxThesaurusDialog : public SvxStandardDialog
{
    std::auto_ptr< SvxThesaurusDialog_Impl > m_pImpl;

    SVX_DLLPRIVATE virtual void     Apply();

public:
    SvxThesaurusDialog( Window* pParent,
                        ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XThesaurus >  xThesaurus,
                        const String &rWord, LanguageType nLanguage );
    ~SvxThesaurusDialog();

    void            SetWindowTitle( LanguageType nLanguage );
    String          GetWord();
    sal_uInt16      GetLanguage() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

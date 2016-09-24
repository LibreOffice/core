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
#ifndef INCLUDED_SVTOOLS_SVMEDIT2_HXX
#define INCLUDED_SVTOOLS_SVMEDIT2_HXX

#include <svtools/svtdllapi.h>
#include <svtools/svmedit.hxx>

class TextAttrib;

class SVT_DLLPUBLIC ExtMultiLineEdit : public MultiLineEdit
{
public:
                    ExtMultiLineEdit( vcl::Window* pParent, WinBits nWinStyle = WB_LEFT | WB_BORDER );

                    // methods of TextView
    void            InsertText( const OUString& rNew );
    void            SetAutoScroll( bool bAutoScroll );

                    // methods of TextEngine
    void            SetAttrib( const TextAttrib& rAttr, sal_uInt32 nPara, sal_Int32 nStart, sal_Int32 nEnd );
    void            SetLeftMargin( sal_uInt16 nLeftMargin );
    sal_uInt32      GetParagraphCount() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

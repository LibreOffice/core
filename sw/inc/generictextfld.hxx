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
#ifndef INCLUDED_SW_INC_GENERICTEXTFLD_HXX
#define INCLUDED_SW_INC_GENERICTEXTFLD_HXX

#include "swdllapi.h"
#include "fldbas.hxx"

class SwDoc;

/*--------------------------------------------------------------------
    FieldType for GenericTextField
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC SwGenericTextFieldType : public SwFieldType
{
public:
    SwGenericTextFieldType( );
    virtual SwFieldType* Copy() const;
    ~SwGenericTextFieldType();

};

/*--------------------------------------------------------------------
    GenericText - field
 --------------------------------------------------------------------*/

class SwGenericTextField : public SwField
{
    OUString sText;  // This string represents current presentation of text field.
    OUString sCode;  // This string represents fild code of text field.
public:
    SwGenericTextField(SwGenericTextFieldType*, const OUString& rText, const OUString& rCode);
    virtual OUString        GetPar2() const;
    virtual OUString        GetPar3() const;
    virtual OUString Expand() const;
    virtual SwField* Copy() const;
    ~SwGenericTextField();

};

#endif // INCLUDED_SW_INC_GENERICTEXTFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

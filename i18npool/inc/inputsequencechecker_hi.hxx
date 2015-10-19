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
#ifndef INCLUDED_I18NPOOL_INC_INPUTSEQUENCECHECKER_HI_HXX
#define INCLUDED_I18NPOOL_INC_INPUTSEQUENCECHECKER_HI_HXX

#include <inputsequencechecker.hxx>

namespace com {
namespace sun {
namespace star {
namespace i18n {


//  class InputSequenceChecker_hi

class InputSequenceChecker_hi : public InputSequenceCheckerImpl
{
public:
    InputSequenceChecker_hi();
    virtual ~InputSequenceChecker_hi();

    sal_Bool SAL_CALL checkInputSequence(const OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(css::uno::RuntimeException, std::exception) override;

    sal_Int32 SAL_CALL correctInputSequence(OUString& Text, sal_Int32 nStartPos,
        sal_Unicode inputChar, sal_Int16 inputCheckMode) throw(css::uno::RuntimeException, std::exception) override;
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

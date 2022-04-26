/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include "_XAccessibleExtendedComponent.hxx"

#include <iostream>

#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>

/**
 * @brief Just calls the method.
 */
bool _XAccessibleExtendedComponent::testGetFont()
{
    auto font = mxExtendedComponent->getFont();
    std::cout << "font: " << font << std::endl;
    return true;
}

/**
 * @brief Just calls the method.
 */
bool _XAccessibleExtendedComponent::testGetTitledBorderText()
{
    auto titleBorderText = mxExtendedComponent->getTitledBorderText();
    std::cout << "getTitledBorderText(): '" << titleBorderText << "'" << std::endl;
    return true;
}

/**
 * @brief Just calls the method.
 */
bool _XAccessibleExtendedComponent::testGetToolTipText()
{
    auto toolTipText = mxExtendedComponent->getToolTipText();
    std::cout << "getToolTipText(): '" << toolTipText << "'" << std::endl;
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

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

#ifndef ADC_TOKPROCT_HXX
#define ADC_TOKPROCT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


namespace csi
{
namespace uidl
{


class UnoIDL_PE;


/** is a parent class for classes, which take part in parsing tokens semantically.
    It provides some types for them.
*/
class TokenProcessing_Types
{
  public:
    enum E_TokenDone
    {
        not_done = 0,
        done = 1
    };

    enum E_EnvStackAction
    {
        stay,           // same parse environment
        push_sure,      // push sub environment, which must be the correct one
        push_try,       // push sub environment, which is tried, if it may be the right one
        pop_success,    // return to parent environment, parsing was successful
        pop_failure     // return to parent environment, but an error occurred.
    };

    struct TokenProcessing_Result
    {
        E_TokenDone         eDone;
        E_EnvStackAction    eStackAction;
        UnoIDL_PE *         pEnv2Push;

                            TokenProcessing_Result()
                                                : eDone(not_done), eStackAction(stay), pEnv2Push(0) {}
        void                reset()             { eDone = not_done; eStackAction = stay; pEnv2Push = 0; }
    };

    enum E_ParseResult
    {
        res_error,
        res_complete,
        res_predeclaration
    };
};


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

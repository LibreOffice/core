/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ADC_CPP_TOKPROCT_HXX
#define ADC_CPP_TOKPROCT_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS


class ParseEnvironment;

/**	is a parent class for classes, which take part in parsing tokens semantically.
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
        stay,			// same parse environment
        push,      		// push sub environment
        pop_success,    // return to parent environment, parsing was successful
        pop_failure     // return to parent environment, but an error occured.
    };

    struct TokenProcessing_Result
    {
        E_TokenDone			eDone;
        E_EnvStackAction	eStackAction;
        ParseEnvironment *	pEnv2Push;

                            TokenProcessing_Result()
                                                : eDone(not_done), eStackAction(stay), pEnv2Push(0) {}
        void				Reset()				{ eDone = not_done; eStackAction = stay; pEnv2Push = 0; }
    };

    enum E_ParseResult
    {
        res_error,
        res_complete,
        res_predeclaration
    };
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpp.h"

void
mvl_init(MacroValidatorList * out_pValidators)
{
    out_pValidators->pFirst = NULL;
    out_pValidators->nextFreeIdentifier = 1;
}

void
mvl_destruct(MacroValidatorList * out_pValidators)
{
    MacroValidator * pV = out_pValidators->pFirst;
    MacroValidator * pDel;
    for ( pDel = out_pValidators->pFirst;
          pDel != NULL;
          pDel = pV )
    {
        pV = pV->pNext;

        pDel->pMacro->flag &= (~ISACTIVE);
        dofree(pDel);
    }
}


#define INVALID_TILL_ENDOFROW 32000

/*  If in_pTokenWhereMacroBecomesValid == 0, the macro is at row end
    and therefore there does not exist any token, where the macro becomes
    valid again. It is revalidated, when the row was processed complete.
*/
void
mvl_add( MacroValidatorList *   inout_pValidators,
         Nlist *                in_pMacro,
         Token *                in_pTokenWhereMacroBecomesValid )
{

    MacroValidator * pNew = new(MacroValidator);
    pNew->pMacro = in_pMacro;

    if (in_pTokenWhereMacroBecomesValid == NULL)
    {
        pNew->nTokenWhereMacroBecomesValid = INVALID_TILL_ENDOFROW;
    }
    else if (in_pTokenWhereMacroBecomesValid->identifier > 0)
    {
        pNew->nTokenWhereMacroBecomesValid = in_pTokenWhereMacroBecomesValid->identifier;
    }
    else
    {
        pNew->nTokenWhereMacroBecomesValid = inout_pValidators->nextFreeIdentifier;
        in_pTokenWhereMacroBecomesValid->identifier = inout_pValidators->nextFreeIdentifier;
        inout_pValidators->nextFreeIdentifier++;
    }

    pNew->pNext = inout_pValidators->pFirst;
    inout_pValidators->pFirst = pNew;
}

void
mvl_check(  MacroValidatorList * inout_pValidators,
            Token *              inout_pTokenToCheck)
{
    MacroValidator * pV;            /* Running pointer */
    MacroValidator * pCheckedOnes;  /* Here new list is built.  */
    pCheckedOnes = NULL;

    for ( pV = inout_pValidators->pFirst;
          pV != NULL;
          pV = inout_pValidators->pFirst )
    {
        inout_pValidators->pFirst = pV->pNext;

        if (pV->nTokenWhereMacroBecomesValid == inout_pTokenToCheck->identifier)
        {
            pV->pMacro->flag &= (~ISACTIVE);
            dofree(pV);
        }
        else
        {
            pV->pNext = pCheckedOnes;
            pCheckedOnes = pV;
        }
    }   /* end for  */

    /* Assign new built list (too old ones were removed) to
       original list:
    */
    inout_pValidators->pFirst = pCheckedOnes;
}


void
tokenrow_zeroTokenIdentifiers(Tokenrow* trp)
{
    Token * tp;
    for (tp = trp->bp; tp < trp->lp; tp++)
    {
        tp->identifier = 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

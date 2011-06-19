/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpp.h"

void
mvl_init(MacroValidatorList * out_pValidators)
{
    out_pValidators->pFirst = 0;
    out_pValidators->nextFreeIdentifier = 1;
}

void
mvl_destruct(MacroValidatorList * out_pValidators)
{
    MacroValidator * pV = out_pValidators->pFirst;
    MacroValidator * pDel;
    for ( pDel = out_pValidators->pFirst;
          pDel != 0;
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

    if (in_pTokenWhereMacroBecomesValid == 0)
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
    pCheckedOnes = 0;

    for ( pV = inout_pValidators->pFirst;
          pV != 0;
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

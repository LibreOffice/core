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
#include "precompiled_sfx2.hxx"

#include "DeckConfiguration.hxx"
#include "Deck.hxx"


namespace sfx2 { namespace sidebar {

DeckConfiguration::DeckConfiguration (void)
    : mpDeck(NULL),
      maPanels()
{
}




void DeckConfiguration::Dispose (void)
{
    if (mpDeck != NULL)
    {
        mpDeck->Dispose();

        Deck* pDeck = mpDeck;
        mpDeck = NULL;
        OSL_TRACE("deleting deck window subtree");
        pDeck->PrintWindowTree();
        delete pDeck;
    }
}



} } // end of namespace sfx2::sidebar

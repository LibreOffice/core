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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_NODE2LAY_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_NODE2LAY_HXX

#include <tools/solar.h>

/*
 * Die Klasse SwNode2Layout stellt die Verbindung von Nodes zum Layout her.
 * Sie liefert einen intelligenten Iterator ueber die zum Node oder Nodebereich
 * gehoerenden Frames. Je nach Zweck der Iteration, z.B. um vor oder hinter
 * den Frames andere Frames einzufuegen, werden Master/Follows erkannt und nur
 * die relevanten zurueckgegeben. Auch wiederholte Tabellenueberschriften werden
 * beachtet.
 * Es ist auch moeglich, ueber SectionNodes zu iterieren, die durch Schachtelung
 * manchmal gar keinem SectionFrm direkt zugeordnet sind, manchmal aber sogar
 * mehreren.
 * SwNode2Layout ist ein Schnittstelle zwischen der aufrufenden Methode und
 * einem SwClientIter, sie waehlt je nach Aufgabenstellung das richtige
 * SwModify aus, erzeugt einen SwClientIter und filtert dessen Iterationen
 * je nach Aufgabenstellung.
 * Die Aufgabenstellung wird durch die Wahl des Ctors bestimmt.
 * 1. Das Einsammeln der UpperFrms, damit spaeter RestoreUpperFrms wird,
 *    wird von MakeFrms gerufen, wenn es keinen PrevNext gibt, vor/hinter den
 *    die Frames gehaengt werden koennen.
 * 2. Die Lieferung der Frames hinter/vor die die neuen Frames eines Nodes
 *    gehaengt werden muessen, ebenfalls von MakeFrms gerufen.
 */

class SwNode2LayImpl;
class SwFrm;
class SwLayoutFrm;
class SwNode;
class SwNodes;
class Point;
struct SwPosition;

class SwNode2Layout
{
    SwNode2LayImpl *pImpl;
public:
    // Dieser Ctor ist zum Einsammeln der UpperFrms gedacht.
    SwNode2Layout( const SwNode& rNd );
    // Dieser Ctor ist fuer das Einfuegen vor oder hinter rNd gedacht,
    // nIdx ist der Index des einzufuegenden Nodes
    SwNode2Layout( const SwNode& rNd, sal_uLong nIdx );
    ~SwNode2Layout();
    SwFrm* NextFrm();
    SwLayoutFrm* UpperFrm( SwFrm* &rpFrm, const SwNode& rNode );
    void RestoreUpperFrms( SwNodes& rNds, sal_uLong nStt, sal_uLong nEnd );

    SwFrm *GetFrm( const Point* pDocPos = 0,
                    const SwPosition *pPos = 0,
                    const bool bCalcFrm = true ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: node2lay.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _NODE2LAY_HXX
#define _NODE2LAY_HXX

/* -----------------23.02.99 11:33-------------------
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
 * --------------------------------------------------*/

class SwNode2LayImpl;
class SwFrm;
class SwLayoutFrm;
class SwNode;

class SwNode2Layout
{
    SwNode2LayImpl *pImpl;
public:
    // Dieser Ctor ist zum Einsammeln der UpperFrms gedacht.
    SwNode2Layout( const SwNode& rNd );
    // Dieser Ctor ist fuer das Einfuegen vor oder hinter rNd gedacht,
    // nIdx ist der Index des einzufuegenden Nodes
    SwNode2Layout( const SwNode& rNd, ULONG nIdx );
    ~SwNode2Layout();
    SwFrm* NextFrm();
    SwLayoutFrm* UpperFrm( SwFrm* &rpFrm, const SwNode& rNode );
    void RestoreUpperFrms( SwNodes& rNds, ULONG nStt, ULONG nEnd );

    SwFrm *GetFrm( const Point* pDocPos = 0,
                    const SwPosition *pPos = 0,
                    const BOOL bCalcFrm = TRUE ) const;
};

#endif

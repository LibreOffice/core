/*************************************************************************
 *
 *  $RCSfile: svditer.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:00 $
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

#ifndef _SVDITER_HXX
#define _SVDITER_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

/* Iterieren ueber eine Objektliste:
   SdrObjListIter aIter(pPage);
   while (aIter.IsMore()) Next()->...;

   SdrIterMode:
   IM_FLAT: Die Suche erfolgt flach entlang der uebergebenen Liste. Next()
       liefert also nacheinander alle Objekte die direkt in der Liste
       verankert sind.
       Der HitTest() bei einem Mausklick erfolgt z.B. in diesem Modus.
   IM_DEEPWITHGROUPS: Stoesst Next() bei der Suche auf ein Gruppenobjekt, ein
       Zeiger auf dieses Gruppenobjekt zuruekgegeben (wie IM_Flat). Das
       naechste Next() sucht allerdings nicht in der selben Liste weiter,
       sondern durchsucht zunaechst die Unterliste des Gruppenobjekts. Wird
       in dieser Unterliste wieder ein Gruppenobjekt gefunden, dann wird
       spaeter auch dessen Unterliste durchsucht, ... .
       Dieser Modus wird benoetigt, wenn man Zugriff auf alle Einzelobjekte
       inkl. der Gruppenobjekte benoetigt.
   IM_DEEPNOGROUPS: Die Suche erfolgt nach aehnlichem Muster wie im Modus
       IM_DEEPWITHGROUPS. Die einzige Ausnahme ist, daﬂ Gruppenobjekte
       selbst uebersprungen werden. Sobald Next() auf ein Gruppenobjekt
       stoesst liefert es sofort das erste Objekt dessen Unterliste, ohne
       zuvor einen Zeiger auf das Gruppenobjekt zu liefern.
       Dieser Modus z.B. beim Repaint verwendet, da SdrObjGroup.Paint()
       ja sonst die gesamte Unterliste der Gruppe painten wuerde, ohne
       eine Unterbrechung durch einen Event zuzulassen.
   Die Rueckwaertssuche ist derzeit nur fuer den Modus IM_FLAT implementiert.
*/

enum SdrIterMode {IM_FLAT,           // Flach ueber die Liste
                  IM_DEEPWITHGROUPS, // Mit rekursivem Abstieg, Next() liefert auch Gruppenobjekte
                  IM_DEEPNOGROUPS};  // Mit rekursivem Abstieg, Next() liefert keine Gruppenobjekte

class SdrObject;
class SdrObjList;

class SdrObjListIter {
protected:
    const SdrObjList* pMainList; // zu durchsuchende Liste
    const SdrObjList* pAktList;  // Aktuelle (Sub-)Liste
    const SdrObject*  pAktGroup; // Aktuelles Gruppenobjekt
    const SdrObject*  pNextObj;  //
    ULONG         nAktNum;   // Objektnummer in aktueller (Sub-)Liste
    ULONG         nObjAnz;   // Anzahl der Objekte in der Hauptliste
    FASTBOOL      bReverse;  // rueckwaerts durchsuchen
    FASTBOOL      bRecurse;  // In Gruppen hinabsteigen
    FASTBOOL      bSkipGrp;  // Gruppenobjekte ueberspringen ?
public:
    SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode=IM_DEEPNOGROUPS, FASTBOOL bRevSearch=FALSE);
    SdrObjListIter(const SdrObject& rGroup, SdrIterMode eMode=IM_DEEPNOGROUPS, FASTBOOL bRevSearch=FALSE);
    void Reset();
    FASTBOOL IsMore() const { return pNextObj!=NULL; }
    SdrObject* Next();
};

#endif //_SVDITER_HXX


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

#ifndef _SVDVITER_HXX
#define _SVDVITER_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
class OutputDevice;
class Window;
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////
class SdrView;
class SdrPageView;
class SdrModel;
class SdrPage;
class SdrObject;
class SetOfByte;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrViewIter {
    const SdrModel*  pModel;
    const SdrPage*   pPage;
    const SdrObject* pObject;
    SdrView*   pAktView;
    FASTBOOL   bNoMasterPage;
    USHORT     nListenerNum;
    USHORT     nPageViewNum;
    USHORT     nOutDevNum;
private:
    void          ImpInitVars();
    SdrView*      ImpFindView();
    FASTBOOL      ImpCheckPageView(SdrPageView* pPV) const;
public:
    SdrViewIter(const SdrObject* pObject, FASTBOOL bNoMasterPage=FALSE);
    SdrView*      FirstView();
    SdrView*      NextView();
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _JUST_DESCRIPTION

Mit dieser Klasse kann man rausbekommen:
- SdrView* First/NextView()
  - Alle Views in denen ein Models dargestellt wird
  - Alle Views in denen eine bestimme Page sichtbar ist (ww. auch als MasterPage)
  - Alle Views in denen ein bestimmes Objekt sichtbar ist (ww. auch auf MasterPage)
- SdrPageView* First/NextPageView()
  - Alle PageViews in denen ein Models dargestellt wird
  - Alle PageViews in denen eine bestimme Page sichtbar ist (ww. auch als MasterPage)
  - Alle PageViews in denen ein bestimmes Objekt sichtbar ist (ww. auch auf MasterPage)
- OutputDevice* First/NextOutDev()
  - Alle OutputDevices in denen ein Models dargestellt wird
  - Alle OutputDevices in denen eine bestimme Page sichtbar ist (ww. auch als MasterPage)
  - Alle OutputDevices in denen ein bestimmes Objekt sichtbar ist (ww. auch auf MasterPage)
- Window* First/NextWindow()
  - Alle Windows in denen ein Models dargestellt wird
  - Alle Windows in denen eine bestimme Page sichtbar ist (auch als MasterPage)
  - Alle Windows in denen ein bestimmes Objekt sichtbar ist (auch auf MasterPage)
Ob die Auswahl auf ein(e) bestimmte(s) Page/Objekt beschraenkt wird, bestimmt man
durch die Wahl des Konstruktors.

Es werden u.a. auch berueksichtigt:
- Layer Sichtbarkeitsstatus
- Visible Layer von MasterPages
- Mehrfachlayer bei Gruppenobjekten

Es wird nicht berueksichtigt:
- Ob die Pages/Objekte wirklich schon gepaintet wurden oder noch ein Invalidate ansteht, ...
- Ob die Pages/Objekte in einem Window im sichtbaren Bereich liegen

#endif // _JUST_DESCRIPTION

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDVITER_HXX

